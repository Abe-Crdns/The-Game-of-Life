/* 
 * File: lexicon.cpp
 * -----------------
 * A lexicon is a word list. This lexicon is backed by two separate data structures
 * for storing the words in the list:
 * 1) a dawg (directed acyclic word graph) 
 * 2) a STL set of other words. 
 * Typically the dawg is used for a large list read from a file in binary format.
 * The STL set is for words added piecemeal at runtime.
 *
 * The dawg idea comes from an article by Appel & Jacobson, CACM May 1988.
 * This lexicon implementation only has the code to load/search the dawg.
 * The dawg builder code is quite a bit more intricate, see me (Julie)
 * if you need it. 
 *
 * I originally wrote this for CS016X May 1993, it has had minor tweaks
 * over the years.  This latest C++ incarnation written summer of 2002.
 * In 2006, I added some improvements and updated the API.
 * 
 * Modified Tue Oct 10 20:18:01 PDT 2006 jzelenski
 * 
 ***************************************************************************************
 * Last modified September 22 2100 by Colin L
 * 
 * The Stanford-supplied version of Lexicon assumes 4-byte int values.
 * However, gcc on 64-bit Linux systems uses 8-byte ints, and calling the Lexicon()
 * constructor reliably crashes the program with a segfault.
 * As a hack to get round this, all instances of "int" are replaced by "int".
 * The constructor checks to confirm sizeof(int)==4
 *
 * Also, some minimal implementations of FE_Iterator and FE_State have been added at the end
 * - attempting to follow the comments in foreach.h 
 * (without really understanding them, unfortunately - programmer beware!)
 *
 */

#include "lpclib.h"
#include "lexicon.h"
#include <fstream>	// for ifstream
#include <cstring>	// for strncmp
#include <algorithm> 	// find, compare

/* The dawg is stored as an array of edges. Each edge is represented by 
 * one 32-bit struct.  The 5 "letter" bits indicate the character on this 
 * transition (expressed as integer from 1 to 26), the  "accept" bit indicates 
 * if you accept after appending that char (current path forms word), and the 
 * "lastEdge" bit marks this as the last edge in a sequence of childeren.  
 * The bulk of the bits (24) are used for the index within the edge array for 
 * the children of this node. The children are laid out contiguously in alphabetical order.
 * Since we read edges as binary bits from a file in a big-endian format, 
 * we have to swap the struct order for little-endian machines.
 */


Lexicon::Lexicon() 
{
    if (sizeof(int) != 4) {
	std::string msg = "Lexicon assumes the int type is 4 bytes. On this system the size is " + sizeof(int);
	Error(msg);
    }
    edges = start = NULL;
    numEdges = numDawgWords = 0;
}


Lexicon::Lexicon(std::string filename) 
{
    edges = start = NULL;
    numEdges = numDawgWords = 0;
    addWordsFromFile(filename);
}

Lexicon::~Lexicon() 
{
    if (edges) delete[] edges;
}

#if defined(BYTE_ORDER) && BYTE_ORDER == LITTLE_ENDIAN
// swaps a 4-byte int from big to little endian byte order
static unsigned int my_ntohl(unsigned int arg)
{
    unsigned int result =  ((arg & 0xff000000) >> 24) |
	((arg & 0x00ff0000) >> 8) |
	((arg & 0x0000ff00) << 8) |
	((arg & 0x000000ff) << 24);
    return result;
}
#endif

static void CountWord(std::string word, int &count) { count++; }

// Binary lexicon file format must follow this pattern
// DAWG:<startnode index>:<num bytes>:<num bytes block of edge data>:
void Lexicon::readBinaryFile(std::string filename) 
{
    int startIndex, numBytes;
    char firstFour[4], expected[] = "DAWG";
    std::ifstream istr(filename.c_str(), std::ios::in|std::ios::binary);

    if (istr.fail())
	Error("Couldn't open lexicon file " + filename);

    istr.read(firstFour, 4);
    istr.get(); // skip first colon
    istr >> startIndex;
    istr.get(); // skip second colon
    istr >> numBytes;
    istr.get(); // skip third colon
    if (istr.fail() || strncmp(firstFour, expected, 4) != 0 || startIndex < 0 || numBytes < 0)
	Error("Improperly formed lexicon file " + filename);

    numEdges = numBytes/sizeof(Edge);
    edges = new Edge[numEdges];
    start = &edges[startIndex];
    istr.read((char *)edges, numBytes);
    if (istr.fail() && !istr.eof())
	Error("Improperly formed lexicon file " + filename);

#if defined(BYTE_ORDER) && BYTE_ORDER == LITTLE_ENDIAN
    unsigned int *cur = (unsigned int *)edges;
    for (int i = 0; i < numEdges; i++, cur++)
	*cur = my_ntohl(*cur);	// swap to get children index in LSB order
#endif
    istr.close();
    numDawgWords = 0;
    recMapAll(start, true, "", CountWord, numDawgWords);
}

// Check for DAWG in first 4 to identify as special binary format,
// otherwise assume ascii, one word per line
void Lexicon::addWordsFromFile(std::string filename) 
{
    char firstFour[4], expected[] = "DAWG";
    std::ifstream istr(filename.c_str());

    if (istr.fail())
	Error("Couldn't open lexicon file " + filename);

    istr.read(firstFour, 4);
    if (strncmp(firstFour, expected, 4) == 0) {
	readBinaryFile(filename);
	return;
    }

    istr.seekg(0);	// return back to beginning
    std::string line;
    while (getline(istr, line))
	add(line);
    istr.close();
}

int Lexicon::size() 
{
    return numDawgWords + (int) otherWords.size();
}

bool Lexicon::isEmpty() 
{
    return size() == 0;
}


void Lexicon::clear() 
{
    if (edges) delete[] edges;
    edges = start = NULL;
    numEdges = numDawgWords = 0;
    otherWords.clear();
}

// Iterate over sequence of children to find one that
// matches the given char.  returns NULL if we get to
// last child without finding a match (thus no such
// child edge exists).
Lexicon::Edge *Lexicon::findEdgeForChar(Edge *children, char ch) 
{
    Edge *curEdge = children;
    while (true) {
	if (curEdge->letter == charToOrd(ch))
	    return curEdge;
	if (curEdge->lastEdge)
	    return NULL;
	curEdge++;	// advance to next sibilng
    }
}

// given a string, trace out path through
// dawg edge-by-edge. If path exists, return last edge
// else returns NULL
Lexicon::Edge *Lexicon::traceToLastEdge(const std::string& s) 
{
    if (!start) return NULL;
    Edge *curEdge = findEdgeForChar(start, s[0]);
    // string length cast to int - added July 2011 to avoid gcc warning
    // length() seems to be implemented as unsigned type?
    for (int i = 1;  i < int(s.length()); i++) {
	if (!curEdge || !curEdge->children) return NULL;
	curEdge = findEdgeForChar(&edges[curEdge->children], s[i]);
    }
    return curEdge;
}

// Predicate function object used to prefix-match on the
// other words collection.
class MatchPrefix {
private:
    std::string p;
public:
    MatchPrefix(const std::string &prefix) : p(ConvertToLowerCase(prefix)) {}
    bool operator()(const std::string& str) const { 
#if defined(__GNUC__) && __GNUC__ < 3 // 2.95 has the old parameter order... sigh
        return str.compare(p, 0, p.length()) == 0; 
#else
        return str.compare(0, p.length(), p) == 0;
#endif
    }
};


bool Lexicon::containsPrefix(std::string prefix)
{
    if (prefix.empty()) return true;
    if (traceToLastEdge(prefix)) return true;
    return find_if(otherWords.begin(), otherWords.end(), MatchPrefix(prefix)) != otherWords.end();	
}


bool Lexicon::containsWord(std::string word) 
{
    Edge *lastEdge = traceToLastEdge(word);
    if (lastEdge && lastEdge->accept) return true;
    // check list of other words if not found in dawg
    return otherWords.find(ConvertToLowerCase(word)) != otherWords.end();
}


void Lexicon::add(std::string word)
{
    if (!containsWord(word)) // don't add duplicate (set uniques, but need check dawg too)
	otherWords.insert(ConvertToLowerCase(word)); // we store words in all lowercase
}

Lexicon::Lexicon(const Lexicon &rhs)
{    
    copyContentsFrom(rhs);
}

const Lexicon & Lexicon::operator=(const Lexicon &rhs)
{
    if (this != &rhs) {
	delete[] edges;
	copyContentsFrom(rhs);			
    }
    return *this;
}

void Lexicon::copyContentsFrom(const Lexicon &rhs)
{
    if (rhs.edges != NULL && rhs.numEdges != 0) {
        numEdges = rhs.numEdges;
        edges = new Edge[rhs.numEdges];
        memcpy(edges, rhs.edges, sizeof(Edge)*rhs.numEdges);
        start = edges + (rhs.start - rhs.edges);
        numDawgWords = rhs.numDawgWords;
    } else {
        edges = start = NULL;
        numEdges = numDawgWords = 0;
    }
    otherWords = rhs.otherWords;
}


