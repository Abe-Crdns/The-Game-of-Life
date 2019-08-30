#ifndef _GAME_GLOBALS_H
#define _GAME_GLOBALS_H

#include "../lpc_lib/lpclib.h"

struct Coords { 
  double x;
  double y;
};

// Defined in GameOfLife.cpp
extern GraphicsWindow GAME_WINDOW;
extern unsigned CELL_SIZE;

extern const Color BLACK;
extern const Color YELLOW;
extern const Color GREY;
extern const Color DARK_GREY;

#endif