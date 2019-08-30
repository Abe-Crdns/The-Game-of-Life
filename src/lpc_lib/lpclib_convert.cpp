/***
 ====================================================================================

 WARNING

 The code in this file only contains INTERNAL IMPLEMENTATION code for the
 LPCGraphics library. You SHOULD NOT edit it, and you do NOT need
 to read or understand it in order to use the library.

 ====================================================================================
 ***/


#include <iostream>
#include <sstream>
#include <cstdlib>
#include "lpclib.h"

#define Err(x) { std::cerr << "*** Runtime Error: " << x << std::endl; exit(-1); }

template <typename In, typename Out>
Out ConvertValue(In val)
{
    Out result;
    std::stringstream ss;
    ss << val;
    ss >> result;
    if (! ss)
    {
        std::stringstream ss2;
        ss2 << "Bad conversion attempted with value " << val;
        Err(ss2.str());
    }
    return (result);
}

std::string IntToString(int num)
{
    return (ConvertValue<int, std::string>(num));
}

std::string DoubleToString(double num)
{
    return (ConvertValue<double, std::string>(num));
}

int StringToInt(std::string str)
{
    return (ConvertValue<std::string, int>(str));
}

double StringToDouble(std::string str)
{
    return (ConvertValue<std::string, double>(str));
}

std::string ConvertToLowerCase(std::string s)
{
    std::string result = s;
    for (unsigned int i = 0; i < s.length(); i++)
    {
        result[i] = tolower(result[i]);
    }
    return (result);
}

std::string ConvertToUpperCase(std::string s)
{
    std::string result = s;
    for (unsigned int i = 0; i < s.length(); i++)
    {
        result[i] = toupper(result[i]);
    }
    return (result);
}

