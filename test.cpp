// test.cpp --- 
// 
// Filename: test.cpp
// Description: 
// Author: Joseph
// Maintainer: 
// Created: Fri Nov  3 15:51:15 2023 (+0000)
// Last-Updated: Fri Nov  3 16:49:06 2023 (+0000)
//           By: Joseph
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

#include <SFML/Graphics.hpp>
#include <stdlib.h>
#include <time.h>
#include <random>
#include <iostream>

#include <chrono>
#include <thread>

const int xdim = 30; //rows
const int ydim= 30; //cols
const int WindowXSize=800;
const int WindowYSize=600;
const int cellXSize=WindowXSize/xdim;
const int cellYSize=WindowYSize/ydim;

const double fishPercent = 0.05;
const double sharkPercent = 0.01;

const int numSharks = xdim*ydim*sharkPercent;
const int numFish = xdim*ydim*fishPercent;

const int FishBreed = 3;
const int SharkBreed = 5;
const int SharkStarve = 10;

enum CellType{
  EMPTY,
  FISH,
  SHARK
};

enum Direction {
    UP,
    DOWN,
    LEFT,
    RIGHT
};

struct cell{
  bool hasMoved = false;
  CellType type = EMPTY;
  sf::Color color = sf::Color::Blue;
  int x, y, breed = 0, starve = 0;
};

sf::RectangleShape recArray[xdim][ydim];
cell worldData[xdim][ydim];

/*!
  \fn initializeEcosystem
  \brief Randomnly generates fish and shark on the map based on the parameters provided by the user
*/
void initializeEcoSystem(){
  std::random_device rd;  // a seed source for the random number engine
  std::mt19937 gen(rd()); // mersenne_twister_engine seeded with rd()

  //assign coordinates to cells
  for(int x = 0; x < xdim; ++x){
    for(int y = 0; y < ydim; ++y){
      worldData[x][y].x = x;
      worldData[x][y].y = y;
    }
  }

  //shuffle the assignment order to remove skewed assignment to top of grid
  std::vector<int> assignmentOrder(xdim * ydim);
  std::iota(assignmentOrder.begin(), assignmentOrder.end(), 0);
  std::shuffle(assignmentOrder.begin(), assignmentOrder.end(), gen);

  //assign fish
  for (int i = 0; i < fishPercent * xdim * ydim; ++i) {
    int row = assignmentOrder[i] / ydim;
    int col = assignmentOrder[i] % ydim;

    worldData[row][col].type = FISH;
    worldData[row][col].color = sf::Color::Green;
  }

  //assign shark
  for (int i = fishPercent * xdim * ydim; i < (fishPercent + sharkPercent) * xdim * ydim; ++i) {
    int row = assignmentOrder[i] / ydim;
    int col = assignmentOrder[i] % ydim;

    worldData[row][col].type = SHARK;
    worldData[row][col].color = sf::Color::Red;
  }
}

/*!
  \fn setGrid
  \brief creates the grid to be displayed
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
  \fn moveFish
  \brief 
*/
void moveFish(Direction dir, cell oldCell){
  cell newCell = oldCell;
  newCell.hasMoved = true;
  worldData[oldCell.x][oldCell.y].type = EMPTY;
  worldData[oldCell.x][oldCell.y].color = sf::Color::Blue;
  switch(newCell.type){
    case FISH:
      if(newCell.breed >= FishBreed){//fish breeds
        worldData[oldCell.x][oldCell.y] = oldCell;
        worldData[oldCell.x][oldCell.y].breed = 0;
        worldData[oldCell.x][oldCell.y].hasMoved = true;//prevent the new fish from moving after spawning
        newCell.breed = -1; //this will be incremented later and reset to 0
      }
      ++newCell.breed;
      break;
    case SHARK:
      ++newCell.starve;//shark is starving
      if(newCell.starve >= SharkStarve){
        return;
      }
      if(newCell.breed >= SharkBreed){//shark breeds
        worldData[oldCell.x][oldCell.y] = oldCell;
        worldData[oldCell.x][oldCell.y].breed = 0;
        worldData[oldCell.x][oldCell.y].hasMoved = true;
        newCell.breed = -1;
      }
      ++newCell.breed;
      break;
  }


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
  \fn getLegalSharkMoves
  \brief returns a vector of legal directions a shark can take. This is only used if a fish is found near a shark
*/
std::vector<Direction> getLegalSharkMoves(cell shark, Direction foundFish){
  std::vector<Direction> legalMoves;
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

  //UP
  

  //DOWN
  

  //LEFT
  

  //RIGHT
  

  if(legalMoves.size() >= 1 && worldData[x][y].starve > 0){
    --worldData[x][y].starve;
  }

  return legalMoves;
}

/*!
  \fn getLegalMoves
  \brief returns a vector of legal directions a creature can take based on a given cell position
*/
std::vector<Direction> getLegalMoves(cell aCell){
  std::vector<Direction> legalMoves;
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
  \fn updateGrid
  \brief Moves fish and sharks in the grid based on predefined rules
*/
void updateGrid(){
  for(int x = 0; x < xdim; ++x){
    for(int y = 0; y < ydim; ++y){
      if(!worldData[x][y].hasMoved){
        std::vector<Direction> legalMoves;
        switch(worldData[x][y].type){
          case FISH:
            legalMoves = getLegalMoves(worldData[x][y]);
            break;
          case SHARK:
            legalMoves = getLegalMoves(worldData[x][y]);
            break;
        }
        if(legalMoves.size() >= 1){
          moveFish(legalMoves[rand() % legalMoves.size()], worldData[x][y]);
        }
      }
    }//end y for-loop
  }//end x for-loop
  //reset hasMoved
  for(int x = 0; x < xdim; ++x){
    for(int y= 0; y < ydim; ++y){
      worldData[x][y].hasMoved = false;
    }
  }
  setGrid();
}

int main(){
  initializeEcoSystem();
  setGrid();

  sf::RenderWindow window(sf::VideoMode(WindowXSize,WindowYSize), "SFML Wa-Tor world");
  while (window.isOpen())
  {
    sf::Event event;
    while (window.pollEvent(event)){
        if (event.type == sf::Event::Closed)
            window.close();
    }
    //for(;;){
    //loop these three lines to draw frames
      window.clear(sf::Color::Black);
      for(int i=0;i<xdim;++i){
        for(int k=0;k<ydim;++k){
          window.draw(recArray[i][k]);
        }
      }
      window.display();
      updateGrid();
      std::this_thread::sleep_until(std::chrono::system_clock::now() + std::chrono::milliseconds(800));

    //}//for - simulation loop
  }

  return 0;
}

// 
// test.cpp ends here
