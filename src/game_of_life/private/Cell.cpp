/*
 *  Author: Abraham Cardenas / https://github.com/Abe-Crdns
 */

#ifndef _CELL_CPP
#define _CELL_CPP

#include "Cell.h"

Cell::Cell(Coords pos){
  position = pos;
  is_alive = false;
  needs_update = false;
}

bool Cell::operator==(const Cell& _cell) const{
  return (position.x == _cell.position.x && position.y == _cell.position.y);
}

Coords Cell::getPosition() const{
  return position;
}

bool Cell::getIsAlive() const{
  return is_alive;
}

bool Cell::getNeedsUpdate() const {
  return needs_update;
};

void Cell::setNeedsUpdate(){
  needs_update = !needs_update;
}

void Cell::aliveOrDead(){
  is_alive = !is_alive;

  if(is_alive){
    GAME_WINDOW.DrawRectangle(
      position.x, position.y, 
      CELL_SIZE, CELL_SIZE, 
      YELLOW, true
    );
  }
  else{
    GAME_WINDOW.DrawRectangle(
      position.x, position.y, 
      CELL_SIZE, CELL_SIZE, 
      DARK_GREY, true
    );
  }
}

#endif