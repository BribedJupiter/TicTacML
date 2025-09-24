#include <iostream>

#include "Game.h"

GameBoard::GameBoard(Renderer& renderer) : glRenderer(renderer) {
    for (auto& row : grid) {
        row.fill(CLEAR);
    }
}

void GameBoard::drawBoard() {
    // We want to draw 4 lines on the board. 
    // As an example, let's say our screen is 600x600 pixels.
    // We want lines in the x direction from 0 to 600 at heights
    // 200 and 400. We want the same in the y direction, from height
    // 0 to 600 and at x positions 200 and 400. We can generalize these
    // 200 and 400 numbers to 1/3 and 2/3 of the screen's dimensions.
    glRenderer.prepareRender();
}

void GameBoard::placeCircle(const int cellIndex) {

}

void GameBoard::placeX(const int cellIndex) {

}

void GameBoard::printGrid() {
    for (auto& row : grid) {
        std::string out = "[";
        for (auto col : row) {
            std::string name;
            switch (col) {
                case CIRCLE:
                    name = "C";
                    break;
                case X:
                    name = "X";
                    break;
                case CLEAR:
                default:
                    name = "_";
            }
            out += name;
        }
        out += "]";
        std::cout << out << std::endl;
    }
}

GameBoard::~GameBoard() {

}