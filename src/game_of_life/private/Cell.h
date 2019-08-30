#ifndef _CELL_H
#define _CELL_H

#include "../GameGlobals.h"

class Cell {
  private:
    Coords position;
    bool is_alive;
    bool needs_update;

  public:
    Cell(Coords pos);

    bool operator==(const Cell& p) const;

    Coords getPosition() const;
    bool getIsAlive() const;
    bool getNeedsUpdate() const;

    void setNeedsUpdate();
    void aliveOrDead();
};

struct CellHasher {
  std::size_t operator()(const Cell* _cell) const {
    return ((std::hash<double>()(_cell->getPosition().x)) ^ 
           (std::hash<double>()(_cell->getPosition().y)));
  }
};

#endif