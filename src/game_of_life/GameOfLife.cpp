/*
 *  Author: Abraham Cardenas / https://github.com/Abe-Crdns
 */

#ifndef _GAME_OF_LIFE_CPP
#define _GAME_OF_LIFE_CPP

#include <iostream>
#include <cmath>
#include <new>
#include <map>
#include "GameOfLife.h"
#include "GameGlobals.h"
#include "private/Timer.h"

#define WINDOW_HEIGHT   680.0
#define WINDOW_WIDTH    980.0
#define BUTTON_X_OFFSET 20.0
#define BUTTON_Y_OFFSET 5.0
#define GRID_OFFSET     10.0
#define CELL_OFFSET     2.0


// Define program's global variables
GraphicsWindow GAME_WINDOW(WINDOW_WIDTH, WINDOW_HEIGHT, "The Game of Life");
unsigned CELL_SIZE = 10;

const Color BLACK(0, 0, 0);
const Color YELLOW(255, 250, 0);
const Color GREY(150, 150, 150);
const Color DARK_GREY(50, 50, 50);

// Button types
enum ButtonValue{
  evGrid, evClear, evStep, 
  evRun, evExit
};

enum BoundaryScheme{
  Flat, Donut, Mirror
};

static const std::map<std::string, ButtonValue> init_map(){
  std::map<std::string, ButtonValue> m;
  m["00_Grid"] = evGrid;
  m["01_Clear"] = evClear;
  m["02_Step"] = evStep;
  m["03_Run"] = evRun;
  m["04_Exit"] = evExit;
  return m;
}

static const std::map<std::string, ButtonValue> mapButtonValues = init_map();
static const double GRID_MAX_HEIGHT = WINDOW_HEIGHT - (BUTTON_HEIGHT + (4 * BUTTON_Y_OFFSET));
static const double GRID_MAX_WIDTH = WINDOW_WIDTH - ((2 * GRID_OFFSET) + CELL_OFFSET);
static const double GAME_FRAME_RATE = 20;

static unsigned calc_rows(unsigned _cell_size){
  return (unsigned)floor((GRID_MAX_HEIGHT)/(_cell_size + CELL_OFFSET));
}

static unsigned calc_cols(unsigned _cell_size){
  return (unsigned)floor((GRID_MAX_WIDTH)/(_cell_size + CELL_OFFSET));
}

static double calc_rows_margin(unsigned _grid_rows, unsigned _cell_size){
  return GRID_MAX_HEIGHT - (_grid_rows * (_cell_size + CELL_OFFSET));
}

static double calc_cols_margin(unsigned _grid_cols, unsigned _cell_size){
  return GRID_MAX_WIDTH - (_grid_cols * (_cell_size + CELL_OFFSET)); 
}

static BoundaryScheme BOUND_SCHEME = Flat;

static unsigned GRID_ROWS = calc_rows(CELL_SIZE) - 1;
static unsigned GRID_COLS = calc_cols(CELL_SIZE) - 1;

static double GRID_ROWS_MARGIN = calc_rows_margin(GRID_ROWS, CELL_SIZE);
static double GRID_COLS_MARGIN = calc_cols_margin(GRID_COLS, CELL_SIZE); 
static double GRID_HEIGHT = GRID_MAX_HEIGHT - GRID_ROWS_MARGIN;                                
static double GRID_WIDTH = GRID_MAX_WIDTH - GRID_COLS_MARGIN; 


GameOfLife::GameOfLife(){
  try{
    cells = new Cell*[GRID_ROWS * GRID_COLS];
    buttons = new Button*[mapButtonValues.size()];
  }
  catch(std::bad_alloc& ba){
    std::cerr << "bad_alloc caught: " << ba.what() << std::endl;
    exit(1);
  }

  // initialize and draw the buttons
  unsigned button_size = (mapButtonValues.size() * BUTTON_WIDTH) + 
                         ((mapButtonValues.size() - 1) * BUTTON_X_OFFSET);
  double button_x = (WINDOW_WIDTH - button_size)/2;
  double button_y = WINDOW_HEIGHT - 
    (BUTTON_HEIGHT + BUTTON_Y_OFFSET + (GRID_ROWS_MARGIN/4));

  std::map<std::string, ButtonValue>::const_iterator itor;
  for(itor = mapButtonValues.begin(); itor != mapButtonValues.end(); ++itor){
    Coords pos = {button_x, button_y};
    buttons[itor->second] = new Button(pos, itor->first, 3);
    button_x += (BUTTON_WIDTH + BUTTON_X_OFFSET);
  } 

  // draw the grid
  drawGrid();
  GAME_WINDOW.Refresh();

  // initialize all the cells
  initCells();
}

GameOfLife::~GameOfLife(){
  if(buttons != nullptr){
    for(unsigned i = 0; i < mapButtonValues.size(); ++i){
      if(buttons[i] != nullptr)    
        delete buttons[i];
    }

    delete[] buttons;
  }

  if(cells != nullptr){
    for(unsigned i = 0; i < GRID_ROWS; ++i){
      for(unsigned j = 0; j < GRID_COLS; ++j){
        if(cells[(i * GRID_COLS) + j] != nullptr)
          delete cells[(i * GRID_COLS) + j];
      }
    }
    
    delete[] cells;
  }
}

void GameOfLife::drawGrid(bool drawGridLines){
  double grid_x = GRID_OFFSET + (GRID_COLS_MARGIN/2);
  double grid_y = GRID_OFFSET + (GRID_ROWS_MARGIN/2);

  GAME_WINDOW.DrawRectangle(grid_x, grid_y, GRID_WIDTH, 
                            GRID_HEIGHT, DARK_GREY, true);

  if(drawGridLines){
    for(unsigned i = 0; i < GRID_ROWS + 1; ++i){
      GAME_WINDOW.DrawLine(grid_x, grid_y, grid_x + GRID_WIDTH, grid_y, GREY);
      grid_y += CELL_SIZE + CELL_OFFSET;
    }
    grid_y = GRID_OFFSET + (GRID_ROWS_MARGIN/2);
    for(unsigned j = 0; j < GRID_COLS + 1; ++j){
      GAME_WINDOW.DrawLine(grid_x, grid_y, grid_x, grid_y + GRID_HEIGHT, GREY);
      grid_x += CELL_SIZE + CELL_OFFSET;
    }
  }
}

void GameOfLife::initCells(){
  double start_pixel = GRID_OFFSET + floor(CELL_OFFSET/2);
  double step = CELL_SIZE + CELL_OFFSET;
  double cell_x = start_pixel + (GRID_COLS_MARGIN/2);
  double cell_y = start_pixel + (GRID_ROWS_MARGIN/2); 

  for(unsigned i = 0; i < GRID_ROWS; ++i){
    for(unsigned j = 0; j < GRID_COLS; ++j){
      Coords pos = {cell_x, cell_y};
      cells[(i * GRID_COLS) + j] = new Cell(pos);
      cell_x += step;
    }
    cell_x = start_pixel + (GRID_COLS_MARGIN/2);
    cell_y += step;
  }

  GAME_WINDOW.Refresh();
}

Cell* GameOfLife::searchCellHelper(Coords mouse, unsigned l, unsigned r, 
                                   int* ind, unsigned offset){
  Cell* _cell = nullptr;

  if(ind != nullptr) 
    *ind = -1;

  while(l <= r){
    unsigned m = l + (r - l)/2;
    Coords curr_pos = cells[m * offset]->getPosition();

    double min_x = curr_pos.x - (CELL_OFFSET/2);
    double min_y = curr_pos.y - (CELL_OFFSET/2);
    double max_x = curr_pos.x + CELL_SIZE + (CELL_OFFSET/2);
    double max_y = curr_pos.y + CELL_SIZE + (CELL_OFFSET/2);

    if(mouse.x >= min_x && mouse.y >= min_y && 
       mouse.x <= max_x && mouse.y <= max_y){
      _cell = cells[m * offset];
      break;
    }
    else if(ind != nullptr && 
            ((offset == 1 && mouse.x >= min_x && mouse.x <= max_x) || 
            (offset > 1 && mouse.y >= min_y && mouse.y <= max_y))){
      *ind = (int)(m * offset);
      break;
    }
    else if((offset == 1 && mouse.x >= max_x) || 
            (offset > 1 && mouse.y >= max_y))
      l = m + 1;
    else
      r = m - 1;
  }

  return _cell;
}

Cell* GameOfLife::searchCell(Coords mouse){
  Cell* _cell = nullptr;

  if(GRID_ROWS < GRID_COLS){
    // search the first column for the mouse y value
    int row_ind;
    _cell = searchCellHelper(mouse, 0, GRID_ROWS - 1, &row_ind, GRID_COLS);

    // check if a valid row index was returned and 
    // cell wasn't found, to search the row at row_ind
    if(_cell == nullptr && row_ind > -1)
      _cell = searchCellHelper(mouse, row_ind, row_ind + GRID_COLS - 1);
  }
  else{
    // search the first row for the mouse x value
    int col_ind;
    _cell = searchCellHelper(mouse, 0, GRID_COLS - 1, &col_ind);

    // check if a valid column index was returned and 
    // cell wasn't found, to search the column at col_ind
    if(_cell == nullptr && col_ind > -1)
      _cell = searchCellHelper(mouse, col_ind, col_ind + GRID_ROWS - 1, 
                               nullptr, GRID_COLS);
  }

  return _cell;
}

Button* GameOfLife::searchButton(Coords mouse){
  Button* _button = nullptr;

  unsigned l = 0;
  unsigned r = mapButtonValues.size() - 1;

  while(l <= r){
    unsigned m = l + (r - l)/2;
    Coords curr_pos = buttons[m]->getPosition();

    if(mouse.x >= curr_pos.x &&
       mouse.x <= (curr_pos.x + BUTTON_WIDTH)){
      _button = buttons[m]; 
      break;
    }

    if(mouse.x >= (curr_pos.x + BUTTON_WIDTH))
      l = m + 1;
    else
      r = m - 1;
  }

  return _button;
}

void GameOfLife::turnOffButton(Button* btn){
  switch(mapButtonValues.at(btn->getText())){
    case evClear:
    case evStep:
    case evExit:
      if(btn->getIsClicked()){
        btn->click();
        GAME_WINDOW.Refresh();
      }
      break;

    default:
      break;
  }
}

// Behavior of all the boundary cells (first and last row/column cells)
// TODO: switch on BOUND_SCHEME here to change boundary behavior 
ptr_cell_set_t GameOfLife::boundCellNeighbors(Coords curr_cell_pos, 
                                              unsigned* live_neighbors){
  ptr_cell_set_t bound_neighbors;

  Coords first_cell_pos = cells[0]->getPosition();
  Coords last_cell_pos = cells[(GRID_ROWS * GRID_COLS) - 1]->getPosition();

  unsigned curr_row = (unsigned)floor(curr_cell_pos.y/(CELL_SIZE + CELL_OFFSET)) - 1;
  unsigned curr_col = (unsigned)floor(curr_cell_pos.x/(CELL_SIZE + CELL_OFFSET)) - 1;

  // top left, top right, bottom left, and bottom right corner cells
  if((curr_cell_pos.x == first_cell_pos.x && curr_cell_pos.y == first_cell_pos.y) || 
     (curr_cell_pos.x == last_cell_pos.x && curr_cell_pos.y == first_cell_pos.y) || 
     (curr_cell_pos.x == first_cell_pos.x && curr_cell_pos.y == last_cell_pos.y) ||
     (curr_cell_pos.x == last_cell_pos.x && curr_cell_pos.y == last_cell_pos.y)){

    if(curr_cell_pos.x == first_cell_pos.x && curr_cell_pos.y == first_cell_pos.y){
      if(cells[((curr_row + 1) * GRID_COLS) + (curr_col + 1)]->getIsAlive())
        live_neighbors++;
      bound_neighbors.insert(cells[((curr_row + 1) * GRID_COLS) + (curr_col + 1)]);
    }
    else if(curr_cell_pos.x == last_cell_pos.x && curr_cell_pos.y == first_cell_pos.y){
      if(cells[((curr_row + 1) * GRID_COLS) + (curr_col - 1)]->getIsAlive())
        live_neighbors++;
      bound_neighbors.insert(cells[((curr_row + 1) * GRID_COLS) + (curr_col - 1)]);
    }
    else if(curr_cell_pos.x == first_cell_pos.x && curr_cell_pos.y == last_cell_pos.y){
      if(cells[((curr_row - 1) * GRID_COLS) + (curr_col + 1)]->getIsAlive())
        live_neighbors++;
      bound_neighbors.insert(cells[((curr_row - 1) * GRID_COLS) + (curr_col + 1)]);
    }
    else{
      if(cells[((curr_row - 1) * GRID_COLS) + (curr_col - 1)]->getIsAlive())
        live_neighbors++;
      bound_neighbors.insert(cells[((curr_row - 1) * GRID_COLS) + (curr_col - 1)]);
    }

    if((curr_cell_pos.x == first_cell_pos.x && curr_cell_pos.y == first_cell_pos.y) || 
       (curr_cell_pos.x == first_cell_pos.x && curr_cell_pos.y == last_cell_pos.y)){
      if(cells[(curr_row * GRID_COLS) + (curr_col + 1)]->getIsAlive())
        live_neighbors++;
      bound_neighbors.insert(cells[(curr_row * GRID_COLS) + (curr_col + 1)]);
    }    
    else{
      if(cells[(curr_row * GRID_COLS) + (curr_col - 1)]->getIsAlive())
        live_neighbors++;
      bound_neighbors.insert(cells[(curr_row * GRID_COLS) + (curr_col - 1)]);
    }
    if((curr_cell_pos.x == first_cell_pos.x && curr_cell_pos.y == first_cell_pos.y) || 
       (curr_cell_pos.x == last_cell_pos.x && curr_cell_pos.y == first_cell_pos.y)){
      if(cells[((curr_row + 1) * GRID_COLS) + curr_col]->getIsAlive())
        live_neighbors++;
      bound_neighbors.insert(cells[((curr_row + 1) * GRID_COLS) + curr_col]);
    }
    else{
      if(cells[((curr_row - 1) * GRID_COLS) + curr_col]->getIsAlive())
        live_neighbors++;
      bound_neighbors.insert(cells[((curr_row - 1) * GRID_COLS) + curr_col]);
    }
  }
  // first column and last column of cells (excluding corners)
  else if((curr_cell_pos.x == first_cell_pos.x) || (curr_cell_pos.x == last_cell_pos.x)){
    for(int i = -1; i < 2; i+=2){
      if(cells[((curr_row + i) * GRID_COLS) + curr_col]->getIsAlive())
        live_neighbors++;
      bound_neighbors.insert(cells[((curr_row + i) * GRID_COLS) + curr_col]);
    }

    for(int i = -1; i < 2; ++i){
      if(curr_cell_pos.x == first_cell_pos.x){
        if(cells[((curr_row + i) * GRID_COLS) + (curr_col + 1)]->getIsAlive())
          live_neighbors++;
        bound_neighbors.insert(cells[((curr_row + i) * GRID_COLS) + (curr_col + 1)]);
      }
      else{
        if(cells[((curr_row + i) * GRID_COLS) + (curr_col - 1)]->getIsAlive())
          live_neighbors++;
        bound_neighbors.insert(cells[((curr_row + i) * GRID_COLS) + (curr_col - 1)]);
      }
    }
  }
  // first and last rows of cells (excluding corners)
  else{
    for(int i = -1; i < 2; i+=2){
      if(cells[(curr_row * GRID_COLS) + (curr_col + i)]->getIsAlive())
        live_neighbors++;
      bound_neighbors.insert(cells[(curr_row * GRID_COLS) + (curr_col + i)]);
    }

    if(curr_cell_pos.y == first_cell_pos.y){
      for(int i = -1; i < 2; ++i){
        if(cells[((curr_row + 1) * GRID_COLS) + (curr_col + i)]->getIsAlive())
          live_neighbors++;
        bound_neighbors.insert(cells[((curr_row + 1) * GRID_COLS) + (curr_col + i)]);
      }
    }
    else{
      for(int i = -1; i < 2; ++i){
        if(cells[((curr_row - 1) * GRID_COLS) + (curr_col + i)]->getIsAlive())
          live_neighbors++;
        bound_neighbors.insert(cells[((curr_row - 1) * GRID_COLS) + (curr_col + i)]);
      }
    }
  }

  return bound_neighbors;
}

ptr_cell_set_t GameOfLife::searchNeighborCells(ptr_cell_set_t __cells, 
                                               ptr_cell_set_t *cells_searched){
  ptr_cell_set_t neighbors;

  Coords first_cell_pos = cells[0]->getPosition();
  Coords last_cell_pos = cells[(GRID_ROWS * GRID_COLS) - 1]->getPosition();

  ptr_cell_set_t::const_iterator itor;
  for(itor = __cells.begin(); itor != __cells.end(); ++itor){

    // check if current cell is not in the cells searched set
    if(cells_searched->find((*itor)) != cells_searched->end())
      continue;

    unsigned live_neighbors = 0;
    Coords curr_cell_pos = (*itor)->getPosition();

    if(curr_cell_pos.x != first_cell_pos.x && curr_cell_pos.x != last_cell_pos.x && 
       curr_cell_pos.y != first_cell_pos.y && curr_cell_pos.y != last_cell_pos.y){ 
      unsigned curr_row = (unsigned)floor(curr_cell_pos.y/(CELL_SIZE + CELL_OFFSET)) - 1;
      unsigned curr_col = (unsigned)floor(curr_cell_pos.x/(CELL_SIZE + CELL_OFFSET)) - 1;
  
      // Non-boundary cells (cells with 8 neighbors)
      for(int i = -1, j = -1; i < 2; ++i, j+=2){    
        if(cells[((curr_row - 1) * GRID_COLS) + (curr_col + i)]->getIsAlive())
          live_neighbors++;

        if(cells[((curr_row + 1) * GRID_COLS) + (curr_col + i)]->getIsAlive())
          live_neighbors++;

        if(j < 2){
          if(cells[(curr_row * GRID_COLS) + (curr_col + j)]->getIsAlive())
            live_neighbors++;
          neighbors.insert(cells[(curr_row * GRID_COLS) + (curr_col + j)]);
        }

        neighbors.insert(cells[((curr_row - 1) * GRID_COLS) + (curr_col + i)]);
        neighbors.insert(cells[((curr_row + 1) * GRID_COLS) + (curr_col + i)]);
      }
    }
    else{
      ptr_cell_set_t bound_neighbors = boundCellNeighbors(curr_cell_pos, &live_neighbors);
      neighbors.insert(bound_neighbors.begin(), bound_neighbors.end());
    }

    // apply the rules of Game of Life
    if(((*itor)->getIsAlive() && (live_neighbors < 2 || live_neighbors > 3)) || 
       (!(*itor)->getIsAlive() && live_neighbors == 3))
      (*itor)->setNeedsUpdate();

    // mark the current cell as searched 
    cells_searched->insert(*itor);
  }

  return neighbors;
}


// TODO: Optimize the program while it's running. Currently capped at 20 fps 
//       since higher fps makes input laggy. Probably b/c Refresh is slow.
void GameOfLife::run(){
  Timer run_delay;
  ptr_cell_set_t live_cells;

  Button* _button = nullptr;
  Cell* _cell = nullptr;

  bool exit_clicked = false;
  bool is_running = false; 
  bool is_step = false;
  bool is_grid = false;

  if(GAME_FRAME_RATE > 20)
    std::cout << "Warning: Fps higher than 20 makes the game laggy\n";

  // main game loop
  while(!exit_clicked){
    if(GAME_WINDOW.MouseIsDown()){
      Coords mouse = { GAME_WINDOW.MouseX(), GAME_WINDOW.MouseY() }; 

      Coords first_btn_pos = buttons[0]->getPosition();
      Coords last_btn_pos = buttons[mapButtonValues.size()-1]->getPosition();
      Coords first_cell_pos = cells[0]->getPosition();
      Coords last_cell_pos = cells[(GRID_ROWS * GRID_COLS)-1]->getPosition();

      // check if a button was clicked while mouse is currently down
      if(_button == nullptr && 
         mouse.x >= first_btn_pos.x &&
         mouse.y >= first_btn_pos.y && 
         mouse.x <= last_btn_pos.x + BUTTON_WIDTH ){
       
        _button = searchButton(mouse);

        if(_button != nullptr){

          ButtonValue _btn_val = mapButtonValues.at(_button->getText());

          if(!is_running || (is_running && _btn_val == evRun))
            _button->click();
          else
            _button = nullptr;

          if(_button != nullptr && _btn_val != evRun)
            GAME_WINDOW.Refresh();
        } 
      }
      else if(_cell == nullptr && !is_running &&
              mouse.x >= first_cell_pos.x - CELL_OFFSET && 
              mouse.y >= first_cell_pos.y - CELL_OFFSET && 
              mouse.x <= last_cell_pos.x + CELL_SIZE + CELL_OFFSET && 
              mouse.y <= last_cell_pos.y + CELL_SIZE + CELL_OFFSET){

        _cell = searchCell(mouse);

        if(_cell != nullptr){
          _cell->aliveOrDead();

          // update live cells
          if(_cell->getIsAlive())
            live_cells.insert(_cell);
          else{
            ptr_cell_set_t::const_iterator itor = live_cells.find(_cell);
            if(itor != live_cells.end())
              live_cells.erase(itor);
          }

          GAME_WINDOW.Refresh();
        }  
      }
    }
    else if(!GAME_WINDOW.MouseIsDown()){

      // check if a cell was previously pressed
      if(_cell != nullptr)
        _cell = nullptr;

      // check if a button was previously pressed
      else if(_button != nullptr){
        Coords mouse = { GAME_WINDOW.MouseX(), GAME_WINDOW.MouseY() }; 

        Coords first_btn_pos = buttons[0]->getPosition();
        Coords last_btn_pos = buttons[mapButtonValues.size()-1]->getPosition();

        if(mouse.x >= first_btn_pos.x && 
           mouse.y >= first_btn_pos.y &&
           mouse.x <= last_btn_pos.x + BUTTON_WIDTH){

          Button* curr_button = searchButton(mouse);

          if(curr_button != nullptr && curr_button->getText() == _button->getText()){

            turnOffButton(curr_button);

            // execute the button
            switch(mapButtonValues.at(curr_button->getText())){
              case evGrid:{
                is_grid = !is_grid;

                if(is_grid)
                  drawGrid(true);
                else
                  drawGrid();

                ptr_cell_set_t::const_iterator itor;
                for(itor = live_cells.begin(); itor != live_cells.end(); ++itor){
                  if((*itor)->getIsAlive()){
                    (*itor)->aliveOrDead();
                    (*itor)->aliveOrDead();
                  }
                }

                GAME_WINDOW.Refresh();
                break;
              }

              case evClear:{
                ptr_cell_set_t::const_iterator itor;
                for(itor = live_cells.begin(); itor != live_cells.end(); ++itor)
                  (*itor)->aliveOrDead();
                
                live_cells.clear();

                GAME_WINDOW.Refresh();
                break;
              }

              case evStep:
                is_step = true;
                break;

              case evRun: 
                is_running = !is_running;

                for(unsigned i = 0; i < mapButtonValues.size(); ++i){
                  ButtonValue _btn_val = mapButtonValues.at(buttons[i]->getText());
                  if(_btn_val != evRun){
                    if(is_running)
                      buttons[i]->disable();
                    else
                      buttons[i]->enable();
                  }
                }

                if(!is_running){
                  GAME_WINDOW.Refresh();
                  run_delay.Reset();
                }
                break;

              case evExit:
                exit_clicked = true;
                break;

              default:
                break;
            }
          }
          else
            turnOffButton(_button);
        }
        else
          turnOffButton(_button);

        _button = nullptr;
      }
    }

    // run or step the game 
    if((is_running && (run_delay.GetDuration() >= (1/GAME_FRAME_RATE) || 
        !run_delay.WasStarted())) || is_step){   
      ptr_cell_set_t neighbors, cells_searched;

      // search the neighbors of all the live cells and
      // the neighbors of the neighbors of all the live cells
      neighbors = searchNeighborCells(live_cells, &cells_searched);
      searchNeighborCells(neighbors, &cells_searched);

      // update the searched cells that need to be updated 
      ptr_cell_set_t::const_iterator itor;
      for(itor = cells_searched.begin(); itor != cells_searched.end(); ++itor){
        if((*itor)->getNeedsUpdate()){
          // change the life status of the current cell
          // and undo needs update 
          (*itor)->aliveOrDead();
          (*itor)->setNeedsUpdate();

          // update live cells set
          if((*itor)->getIsAlive())
            live_cells.insert(*itor);
          else if(live_cells.find(*itor) != live_cells.end())
            live_cells.erase(*itor);
        }
      }

      GAME_WINDOW.Refresh();
      is_step = false;

      // reset the timer and start it again
      if(is_running){
        run_delay.Reset();
        run_delay.Start();
      }
    }
  }
}

#endif