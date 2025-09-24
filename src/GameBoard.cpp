#include <iostream>

#include "Game.h"
#include "constants.h"

GameBoard::GameBoard(Renderer& renderer) : glRenderer(renderer) {
    for (auto& row : grid) {
        row.fill(CLEAR);
    }

    const auto vertPair = generateBoardVertices();
    glRenderer.setVertices(vertPair);
}

std::pair<std::vector<float>, std::vector<int>> GameBoard::generateBoardVertices() {
    std::vector<float> verts;
    // When drawing the base tic tac toe board, we will need to draw 4
    // lines (2 horizontal, 2 vertical) across the entire height and width
    // of the screen. Each of these lines will contain 4 points.
    // So, at a minimum we need to generate 4 * 4 = 16 vertices.
    // None of these vertices will be shared. Each line will be TTT::lineWidth wide
    float offset = static_cast<float>(TTT::lineWidth) / 100.0f;
    std::vector<std::array<float, 3>> lineArrays;
    std::vector<int> indices = {
        0, 2, 3, // left horizontal triangle one - bl, tr, tl
        0, 1, 3, // left horizontal triangle two - bl, br, tr
        4, 6, 7, // right horizontal triangle one
        4, 5, 7, // right horizontal triangle two 
        8, 9, 10, // top vertical triangle one
        9, 10, 11, // top vertical triangle two
        12, 13, 14, // bottom vertical triangle one
        13, 14, 15 // bottom vertical triangle two
    };
    
    // Left Horizontal
    std::array<float, 3> lh_botLeft  = {-0.5f - offset, -1.0f, 0.0f};
    std::array<float, 3> lh_botRight = {-0.5f + offset, -1.0f, 0.0f};
    std::array<float, 3> lh_topLeft  = {-0.5f - offset, 1.0f, 0.0f};
    std::array<float, 3> lh_topRight = {-0.5f + offset, 1.0f, 0.0f};
    lineArrays.push_back(lh_botLeft);
    lineArrays.push_back(lh_botRight);
    lineArrays.push_back(lh_topLeft);
    lineArrays.push_back(lh_topRight);

    // Right Horizontal
    std::array<float, 3> rh_botLeft  = {0.5f - offset, -1.0f, 0.0f};
    std::array<float, 3> rh_botRight = {0.5f + offset, -1.0f, 0.0f};
    std::array<float, 3> rh_topLeft  = {0.5f - offset, 1.0f, 0.0f};
    std::array<float, 3> rh_topRight = {0.5f + offset, 1.0f, 0.0f};
    lineArrays.push_back(rh_botLeft);
    lineArrays.push_back(rh_botRight);
    lineArrays.push_back(rh_topLeft);
    lineArrays.push_back(rh_topRight);

    // Top vertical
    std::array<float, 3> tv_botLeft  = {-1.0f, 0.5f - offset, 0.0f};
    std::array<float, 3> tv_topLeft  = {-1.0f, 0.5f + offset, 0.0f};
    std::array<float, 3> tv_botRight = {1.0f, 0.5f - offset, 0.0f};
    std::array<float, 3> tv_topRight = {1.0f, 0.5f + offset, 0.0f};
    lineArrays.push_back(tv_botLeft);
    lineArrays.push_back(tv_topLeft);
    lineArrays.push_back(tv_botRight);
    lineArrays.push_back(tv_topRight);

    // Bottom vertical
    std::array<float, 3> bv_botLeft  = {-1.0f, -0.5f - offset, 0.0f};
    std::array<float, 3> bv_topLeft  = {-1.0f, -0.5f + offset, 0.0f};
    std::array<float, 3> bv_botRight = {1.0f, -0.5f - offset, 0.0f};
    std::array<float, 3> bv_topRight = {1.0f, -0.5f + offset, 0.0f};
    lineArrays.push_back(bv_botLeft);
    lineArrays.push_back(bv_topLeft);
    lineArrays.push_back(bv_botRight);
    lineArrays.push_back(bv_topRight);

    // Concatenate into one giant vector
    for (auto arr : lineArrays) {
        for (auto elem : arr) {
            verts.push_back(elem);
        }
    }

    return std::pair{verts, indices};
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