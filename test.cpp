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

int main()
{
    int xdim = 30;
    int ydim= 30;
    int WindowXSize=800;
    int WindowYSize=600;
    int cellXSize=WindowXSize/xdim;
    int cellYSize=WindowYSize/ydim;
    //each shape will represent either a fish, shark or empty space
    //e.g. blue for empty, red for shark and green for fish
    sf::RectangleShape recArray[xdim][ydim];
    int worldData[xdim][ydim];
    for(int i=0;i<xdim;++i){
      for(int k=0;k<ydim;++k){//give each one a size, position and color
        recArray[i][k].setSize(sf::Vector2f(cellXSize,cellYSize));
        recArray[i][k].setPosition(i*cellXSize,k*cellYSize);//position is top left corner!
        int id=i*1-+k;
        recArray[i][k].setFillColor(sf::Color::Blue);
        if (id%2==0) recArray[i][k].setFillColor(sf::Color::Red);
        else recArray[i][k].setFillColor(sf::Color::Green);
      }
    }
    sf::RenderWindow window(sf::VideoMode(WindowXSize,WindowYSize), "SFML Wa-Tor world");
    while (window.isOpen())
    {
      sf::Event event;
      while (window.pollEvent(event))
      {
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
      //}//for - simulation loop
    }

    return 0;
}

// 
// test.cpp ends here
