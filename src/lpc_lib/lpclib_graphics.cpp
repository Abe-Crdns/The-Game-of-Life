/***
====================================================================================
 
 WARNING
 
 The code in this file only contains INTERNAL IMPLEMENTATION code for the
 LPCGraphics library. You SHOULD NOT edit it, and you do NOT need
 to read or understand it in order to use the library.

====================================================================================
***/

#include <iostream>
#include <map>
#include <vector>
#include "CImg.h"
#include "lpclib.h"
using namespace cimg_library;


/// Macro to print an error then exit
#define Err(x) { std::cerr << "*** Runtime Error: " << x << std::endl; exit(-1); }


/// PImpl structs for color and graphics window
struct ColImpl
{
    unsigned char components[3];
};

struct GWImpl
{
    std::map<std::string, CImg<unsigned char> *> imagemap;
    CImgDisplay * gdisplay;
    CImg<unsigned char> * gpixels;
};

Color::Color(unsigned char r, unsigned char g, unsigned char b)
{
    _priv = new ColImpl;
    _priv->components[0] = r;
    _priv->components[1] = g;
    _priv->components[2] = b;
}

void GraphicsWindow::Pause(double seconds)
{
    cimg_library::cimg::sleep(seconds * 1000);
}

GraphicsWindow::GraphicsWindow(int width, int height, std::string title)
{
    _priv = new GWImpl;
    
    _priv->gpixels = new CImg<unsigned char>(width, height, 1, 3);
    _priv->gdisplay = new CImgDisplay(*(_priv->gpixels), title.c_str());
    _priv->gpixels->display(*(_priv->gdisplay));

    if (_priv->gdisplay->window_width() != width)
        Err("Requested width " + IntToString(width) + " does not fit the screen.");
    if (_priv->gdisplay->height() != height)
        Err("Requested height " + IntToString(height) + " does not fit the screen.");
}

GraphicsWindow::~GraphicsWindow()
{
    delete _priv;
}

int GraphicsWindow::GetWidth() const
{
    return _priv->gpixels->width();
}

int GraphicsWindow::GetHeight() const
{
    return _priv->gpixels->height();
}

void GraphicsWindow::Refresh()
{
    _priv->gpixels->display(*(_priv->gdisplay));
}

void GraphicsWindow::DrawRectangle(int x, int y, int width, int height, Color color, bool filled)
{
    if (filled)
        _priv->gpixels->draw_rectangle(x, y, x+width, y+height, color._priv->components);
    else
        _priv->gpixels->draw_rectangle(x, y, x+width, y+height, color._priv->components, 1, ~0);
    
}

void GraphicsWindow::DrawImage(std::string imageFileName, int x, int y, int width, int height)
{
    if (width < 0 || height < 0)
        Err("Invalid dimensions given for image");

    CImg<unsigned char> * img;
    if (_priv->imagemap.find(imageFileName) == _priv->imagemap.end())
    {
        img = new CImg<unsigned char> ((std::string("Images/") + imageFileName).c_str());
        if (!img)
            Err (std::string("Could not load image '") + imageFileName + "'");
        _priv->imagemap[imageFileName] = img;
    }
    else
        img = _priv->imagemap[imageFileName];
    
    if (width == 0 && height == 0)
        _priv->gpixels->draw_image(x, y, *img);
    else
        _priv->gpixels->draw_image(x, y, img->get_resize(width, height));
}


void GraphicsWindow::DrawLine(int x1, int y1, int x2, int y2, Color color)
{
    _priv->gpixels->draw_line(x1, y1, x2, y2, color._priv->components);
}



void GraphicsWindow::DrawCircle(int x, int y, int radius, Color color, bool filled)
{
    if (filled)
        _priv->gpixels->draw_circle(x, y, radius, color._priv->components);
    else
        _priv->gpixels->draw_circle(x, y, radius, color._priv->components, 1, 1);
}

void GraphicsWindow::DrawString(std::string str, int x, int y, Color color, int fontsize)
{
    _priv->gpixels->draw_text (x, y, str.c_str(), color._priv->components, 0, 1, fontsize);
    //gpixels->display(*gdisplay);
}

void GraphicsWindow::DrawEllipse(int x, int y, int xradius, int yradius, Color color, bool filled)
{
    if (filled)
        _priv->gpixels->draw_ellipse(x, y, xradius, yradius, 0, color._priv->components);
    else
        _priv->gpixels->draw_ellipse(x, y, xradius, yradius, 0, color._priv->components, 1, 1);
}

void GraphicsWindow::WaitForMouseDown()
{
    while (1)
    {
        _priv->gdisplay->wait();
        if (_priv->gdisplay->button() > 0)
            break;
    }
}

void GraphicsWindow::WaitForMouseUp()
{
    while (1)
    {
        _priv->gdisplay->wait();
        if (_priv->gdisplay->button() == 0)
            break;
    }
}

int GraphicsWindow::MouseX()
{
    return (_priv->gdisplay->mouse_x());
}

int GraphicsWindow::MouseY()
{
    return (_priv->gdisplay->mouse_y());
}

bool GraphicsWindow::MouseIsDown()
{
    return (_priv->gdisplay->button() > 0);
}

char GraphicsWindow::WaitForKeyPress()
{
    char c;
    while(1)
    {
        _priv->gdisplay->wait();
        c = _priv->gdisplay->key();
        if (c > 0)
            return (c == 13 ? '\n' : c);
    }
}


bool GraphicsWindow::KeyPressed(std::string key)
{
    static unsigned int last = 0;
    
    unsigned int what = 0;
    if (key.length() == 1)
        what = key[0];
    else if (key == "up")
        what = cimg_library::cimg::keyARROWUP;
    else if (key == "down")
        what = cimg_library::cimg::keyARROWDOWN;
    else if (key == "left")
        what = cimg_library::cimg::keyARROWLEFT;
    else if (key == "right")
        what = cimg_library::cimg::keyARROWRIGHT;
    
    if (_priv->gdisplay->key() == what)
    {
        last = what;
        return true;
    }
    else if (_priv->gdisplay->released_key() == what)
    {
//        _priv->gdisplay->unrelease_key();
        if (last == what)
        {
            last = 0;
            return false;
        }
        else
            return true;
    }
    return false;
}

std::string GraphicsWindow::WhatKey()
{
    char c =_priv->gdisplay->key();
    std::string s = "";
    s += c;
    return s;
}



