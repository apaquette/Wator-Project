// wator.cpp --- 
// 
// Filename: wator.cpp
// Description: 
// Author: Joseph
// Maintainer: 
// Created: Fri Nov  3 15:51:15 2023 (+0000)
// Last-Updated: Fri Nov  24
//           By: Alex Paquette
//     Update #: 19
// 
// 

// Commentary: 
// 
// 
// 
// 
// 
// 
// 
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or (at
// your option) any later version.
// 
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with GNU Emacs.  If not, see <http://www.gnu.org/licenses/>.
// 
// some helpful code here!
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// https://www.sfml-dev.org/tutorials/2.5/start-linux.php
// https://learnsfml.com/
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Code:

/*!
  \file wator.cpp
  \brief main file for the wator project
*/

#include <SFML/Graphics.hpp>
#include <stdlib.h>
#include <time.h>
#include <random>
#include <iostream>
#include <omp.h>

#include <chrono>
#include <thread>

const double fishPercent = 0.05; /*! Percentage of fish that should be rendered */
const double sharkPercent = 0.01; /*! Percentage of sharks that should be rendered */
const int xdim = 500; /*! Number of columns in the grid */
const int ydim = 500; /*! Number of rows in the grid*/
const int numSharks = xdim * ydim * sharkPercent; /*! Number of sharks to be spawned */
const int numFish = xdim*ydim * fishPercent; /*! Number of fish to be spawned */
const int FishBreed = 3; /*! Chronon before fish breed */
const int SharkBreed = 5; /*! Chronon before shark breed */
const int SharkStarve = 7; /*! Chronon before shark starve */
int threads = 1; /*! Number of threads to run */

const int WindowXSize = 1080; /*! X window size*/
const int WindowYSize = 1920; /*! Y window size*/
const int cellXSize = WindowXSize / xdim; /*! Cell X dimensions */
const int cellYSize = WindowYSize / ydim; /*! Cell Y dimensions*/

using namespace std;

/*! CellType enum
    Represents contents of the cell. Can be EMPTY, FISH, or SHARK
*/
enum CellType{
  EMPTY,
  FISH,
  SHARK
};

/*! Direction enum
    Represents possible movements a fish or shark can take. Can be UP, DOWN, LEFT, or RIGHT
*/
enum Direction {
    UP,
    DOWN,
    LEFT,
    RIGHT
};

/*! \struct cell
    \brief Cell struct
*/
struct cell{
  bool hasMoved = false;  /*! field that indicates whether the cell has moved. Only useful for FISH and SHARK types */
  CellType type = EMPTY; /*! CellType enum that represents the type of cell. Defaults to EMPTY. */
  sf::Color color = sf::Color::Blue; /*! Color of the cell when displayed to the user. Defaults to blue. */
  int x; /*! x coordinate of the cell */
  int y; /*! y coordinate of the cell*/
  int breed = 0; /*! Breed tracker for the cell. Only used for FISH or SHARK */
  int starve = 0; /*! Starve tracker for the cell. Only used for SHARK */
};

sf::RectangleShape recArray[xdim][ydim]; /*! Global array for RectangleShape to be used in GUI representation of the simulation */
vector<vector<cell>> worldData(xdim, vector<cell>(ydim)); /*! 2D vector of cells containing each cell */

/*!
  \fn void initializeEcoSystem()
  \brief Randomnly generates fish and shark
  
  Fish and shark are generated based on the fishCount and sharkCount parameters
*/
void initializeEcoSystem(){
  random_device rd;  // a seed source for the random number engine
  int seed = rd();
  seed = 1868995012;
  mt19937 gen(seed); // mersenne_twister_engine seeded with rd()

  //assign coordinates to cells
  for(int x = 0; x < xdim; ++x){
    for(int y = 0; y < ydim; ++y){
      worldData[x][y].x = x;
      worldData[x][y].y = y;
    }
  }

  //shuffle the assignment order to remove skewed assignment to top of grid
  vector<int> assignmentOrder(xdim * ydim);
  iota(assignmentOrder.begin(), assignmentOrder.end(), 0);
  shuffle(assignmentOrder.begin(), assignmentOrder.end(), gen);

  //assign fish
  for (int i = 0; i < numFish; ++i) {
    int row = assignmentOrder[i] / ydim;
    int col = assignmentOrder[i] % ydim;

    worldData[row][col].type = FISH;
    worldData[row][col].color = sf::Color::Green;
  }

  //assign shark
  for (int i = numFish; i < (numFish + numSharks); ++i) {
    int row = assignmentOrder[i] / ydim;
    int col = assignmentOrder[i] % ydim;

    worldData[row][col].type = SHARK;
    worldData[row][col].color = sf::Color::Red;
  }
}

/*!
  \fn void setGrid()
  \brief creates the grid to be displayed

  Loops through the recArray and assigns the size, position, and color from corresponding worldData coordinate.

*/
void setGrid(){
  for(int i = 0; i < xdim; ++i){
    for(int k = 0; k < ydim; ++k){//give each one a size, position and color
      recArray[i][k].setSize(sf::Vector2f(cellXSize,cellYSize));
      recArray[i][k].setPosition(i*cellXSize,k*cellYSize);//position is top left corner!
      recArray[i][k].setFillColor(worldData[i][k].color);
    }
  }
}

/*!
  \fn void moveCreature(Direction dir, cell cell)
  \brief Moves a shark or fish in a given direction

  Moves a shark or fish in a direction. Also detemines if a fish or shark breeds, and if a shark starves.

  \param dir Enum of the cells direction
  \param oldCell Cell to be moved
*/
void moveCreature(Direction dir, cell oldCell){
  cell newCell = oldCell;
  newCell.hasMoved = true;
  worldData[oldCell.x][oldCell.y].type = EMPTY;
  worldData[oldCell.x][oldCell.y].color = sf::Color::Blue;
  
  int breedType;
  
  switch(newCell.type){
    case FISH:
      breedType = FishBreed;
      break;
    case SHARK:
      ++newCell.starve;//shark is starving
      if(newCell.starve >= SharkStarve){
        return;
      }
      breedType = SharkBreed;
      break;
    case EMPTY:
      return; //this should never happen but if somehow an empty cell is passed, exit early
      break;
  }

  if(newCell.breed >= breedType){//fish breeds
    worldData[oldCell.x][oldCell.y] = oldCell;
    worldData[oldCell.x][oldCell.y].breed = 0;
    worldData[oldCell.x][oldCell.y].hasMoved = true;//prevent the new fish/shark from moving after spawning
    newCell.breed = -1; //this will be incremented later and reset to 0
  }

  ++newCell.breed;

  switch(dir){
    case UP:
      newCell.y = ((oldCell.y - 1) + ydim) % ydim;
      break;
    case DOWN:
      newCell.y = (oldCell.y + 1) % ydim;
      break;
    case LEFT:
      newCell.x = ((oldCell.x - 1) + xdim) % xdim;
      break;
    case RIGHT:
      newCell.x = (oldCell.x + 1) % xdim;
      break;
  }
  worldData[newCell.x][newCell.y] = newCell;
}

/*!
  \fn vector<Direction> getLegalSharkMoves(cell shark, Direction foundFish)
  \brief Returns a vector of legal directions a shark can take. 
  
  Returns a vector of legal directions a shark can take. This is only used if a fish is found near a shark in getLegalMoves

  \param shark Shark cell to be moved
  \param foundFish Direction a fish was found
  \return vector of directions a shark can take
*/
vector<Direction> getLegalSharkMoves(cell shark, Direction foundFish){
  vector<Direction> legalMoves;
  int x = shark.x;
  int y = shark.y;

  switch(foundFish){
    case UP:
      {
        int checkY = ((y - 1) + xdim) % xdim;
        if(worldData[x][checkY].type == FISH){
          legalMoves.insert(legalMoves.end(), UP);
        }
        foundFish = DOWN;
      }
    case DOWN:
      {
        int checkY = (y + 1) % ydim;
        if(worldData[x][checkY].type == FISH){
          legalMoves.insert(legalMoves.end(), DOWN);
        }
        foundFish = LEFT;
      }
    case LEFT:
    {
      int checkX = ((x - 1) + xdim) % xdim;
      if(worldData[checkX][y].type == FISH){
        legalMoves.insert(legalMoves.end(), LEFT);
      }
      foundFish = RIGHT;
    }
      
    case RIGHT:
      {
        int checkX = (x + 1) % xdim;
        if(worldData[checkX][y].type == FISH){
          legalMoves.insert(legalMoves.end(), RIGHT);
        }
      }
  }

  if(legalMoves.size() >= 1 && worldData[x][y].starve > 0){
    --worldData[x][y].starve;
  }

  return legalMoves;
}

/*!
  \fn vector<Direction> getLegalMoves(cell aCell)
  \brief Returns a vector of legal directions a creature can take.
  
   Returns a vector of legal directions a creature can take based on a given cell position. Will return an empty vector if no legal moves are found.
   If the cell is a shark and a fish is found, will call getLegalSharkMoves and return its results instead.

   \param aCell The cell to get legal moves for
   \return vector of legal directions. Will be empty if no legal moves are found
*/
vector<Direction> getLegalMoves(cell aCell){
  vector<Direction> legalMoves;
  int x = aCell.x;
  int y = aCell.y;
  //UP
  int checkY = ((y - 1) + xdim) % xdim;
  if(aCell.type == SHARK && worldData[x][checkY].type == FISH){
    return getLegalSharkMoves(aCell, UP);
  }
  if(worldData[x][checkY].type == EMPTY){
    legalMoves.insert(legalMoves.end(), UP);
  }

  //DOWN
  checkY = (y + 1) % ydim;
  if(aCell.type == SHARK && worldData[x][checkY].type == FISH){
    return getLegalSharkMoves(aCell, DOWN);
  }
  if(worldData[x][checkY].type == EMPTY){
    legalMoves.insert(legalMoves.end(), DOWN);
  }

  //LEFT
  int checkX = ((x - 1) + xdim) % xdim;
  if(aCell.type == SHARK && worldData[checkX][y].type == FISH){
    return getLegalSharkMoves(aCell, LEFT);
  }
  if(worldData[checkX][y].type == EMPTY){
    legalMoves.insert(legalMoves.end(), LEFT);
  }

  //RIGHT
  checkX = (x + 1) % xdim;
  if(aCell.type == SHARK && worldData[checkX][y].type == FISH){
    return getLegalSharkMoves(aCell, RIGHT);
  }
  if(worldData[checkX][y].type == EMPTY){
    legalMoves.insert(legalMoves.end(), RIGHT);
  }


  return legalMoves;
}

/*!
  \fn void updateGrid()
  \brief Moves fish and sharks in the grid based on predefined rules randomly

  Will used getLegalMoves and getLegalSharkMoves to move the fish and shark in the grid. Also uses pragma omp parallel to run
  in parallel. Will tile the grid between each thread.

*/
void updateGrid(){
  #pragma omp parallel num_threads(threads)
  {
    int rowSize = ydim / threads;
    int start = (rowSize * omp_get_thread_num());
    int end = start + rowSize;
    if(omp_get_thread_num() + 1 == threads && ydim % threads != 0){
      end = ydim;
    }
    //printf("Thread: %d, rowSize: %d, start: %d, end: %d\n",omp_get_thread_num(), rowSize, start, end);
    for(int x = 0; x < xdim; ++x){
      for(int y = start; y < end; ++y){
        if(!worldData[x][y].hasMoved){
          vector<Direction> legalMoves = getLegalMoves(worldData[x][y]);
          if(legalMoves.size() >= 1){
            moveCreature(legalMoves[rand() % legalMoves.size()], worldData[x][y]);
          }
        }
      }//end y for-loop
    }//end x for-loop
  }
}

/*! \fn void resetMovement()
    \brief Resets creature movement

    Iterates through worldData and resets all cell hasMoved to false.
*/
void resetMovement(){
  //reset hasMoved
  for(int x = 0; x < xdim; ++x){
    for(int y= 0; y < ydim; ++y){
      worldData[x][y].hasMoved = false;
    }
  }
}

void runBenchmark(){
  int cores[4] = { 1, 2, 4, 8};
  for(int t = 0; t < 4; ++t){
    threads = cores[t];
    for(int test = 1; test < 21; ++test){
      double start, end;
      start = omp_get_wtime();
      for(int t = 0;t < 100; ++t){
        updateGrid();
        resetMovement();
      }

      end = omp_get_wtime();
      double diff = end - start;
      cout << "Thread Count: " << cores[t] << ", Time: " << diff << " seconds." << endl;
    }
  }
}

void runWithGUI(){
  setGrid();
  sf::RenderWindow window(sf::VideoMode(WindowXSize,WindowYSize), "SFML Wa-Tor world");
  while (window.isOpen())
  {
    sf::Event event;
    while (window.pollEvent(event)){
        if (event.type == sf::Event::Closed)
            window.close();
    }
    window.clear(sf::Color::Black);
    for(int i=0;i<xdim;++i){
      for(int k=0;k<ydim;++k){
        window.draw(recArray[i][k]);
      }
    }
    window.display();
    updateGrid();
    resetMovement();
    setGrid();
  }
}

int main(){
  initializeEcoSystem();
  runBenchmark();
  //runWithGUI();

  return 0;
}

// 
// test.cpp ends here
