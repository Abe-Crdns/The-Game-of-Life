#ifndef LPCLIB_H
#define LPCLIB_H

#include <string>
#include <vector>
#include <iostream>
#include <iomanip>
#include <fstream>

typedef unsigned char smallint;

#define Error(x) { std::cerr << "*** Runtime Error: " << x << std::endl; exit(-1); }

void Pause(double seconds);

std::string IntToString(int num);

std::string DoubleToString(double num);

int StringToInt(std::string str);

double StringToDouble(std::string str);

std::string ConvertToLowerCase(std::string str);

std::string ConvertToUpperCase(std::string str);

struct ColImpl;
struct Color
{
public:
    Color(unsigned char r=0, unsigned char g=0, unsigned char b=0);
    
private:
    ColImpl * _priv;
    friend class GraphicsWindow;
};

struct GWImpl;
class GraphicsWindow
{
    
public:
    GraphicsWindow(int width, int height, std::string title);
    int GetWidth() const;
    int GetHeight() const;
    void Refresh();
    
    void DrawLine(int x1, int y1, int x2, int y2, Color color);
    void DrawCircle(int x, int y, int radius, Color color, bool filled = false);
    void DrawEllipse(int x, int y, int xradius, int yradius, Color color, bool filled = false);
    void DrawRectangle(int x, int y, int width, int height, Color color, bool filled = false);
    void DrawString(std::string str, int x, int y, Color color, int fontsize = 13);
    void DrawImage(std::string imageFileName, int x, int y, int width = 0, int height = 0);

    void WaitForMouseDown();
    void WaitForMouseUp();
    int MouseX();
    int MouseY();
    bool MouseIsDown();
    
    char WaitForKeyPress();
    bool KeyPressed(std::string key);
    
    void Pause(double seconds);
    
    std::string WhatKey();

    
public:
    ~GraphicsWindow();
    
private:
    GWImpl * _priv;

    
};

#endif
