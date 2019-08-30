#ifndef _GAME_OF_LIFE_H
#define _GAME_OF_LIFE_H

#include <unordered_set>
#include "../lpc_lib/lpclib.h"
#include "private/Cell.h"
#include "private/Button.h"

typedef std::unordered_set<Cell*, CellHasher> ptr_cell_set_t;

class GameOfLife {
  private: 
    Cell** cells;
    Button** buttons;

    void drawGrid(bool drawGridLines = false);
    void initCells();
    void turnOffButton(Button* btn);

    Cell* searchCellHelper(Coords mouse, unsigned l, unsigned r, 
                           int* ind = nullptr, unsigned offset = 1);
    Cell* searchCell(Coords mouse);
    Button* searchButton(Coords mouse);

    ptr_cell_set_t searchNeighborCells(ptr_cell_set_t __cells, ptr_cell_set_t *cells_searched);
    ptr_cell_set_t boundCellNeighbors(Coords curr_cell_pos, unsigned* live_neighbors);

  public:
    GameOfLife();
    ~GameOfLife();

    void run();
};

#endif
