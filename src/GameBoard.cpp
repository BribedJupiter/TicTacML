#include <iostream>

#include "Game.h"
#include "constants.h"

GameBoard::GameBoard(Renderer& renderer) : glRenderer(renderer) {
    for (auto& row : grid) {
        row.fill(CLEAR);
    }

    // auto vertPair = generateBoardVertices();
    // for (int i = 0; i < 9; i++) {
    //     auto pair = generateXVertices(i);
    //     for (auto vert : pair.first) {
    //         vertPair.first.push_back(vert);
    //     }
    //     int offset = vertPair.second.size();
    //     for (auto index : pair.second) {
    //         vertPair.second.push_back(index + offset);
    //     }
    // }
    const auto vertPair = generateXVertices(5);
    glRenderer.setVertices(vertPair);
}

std::pair<std::vector<float>, std::vector<int>> GameBoard::generateBoardVertices() {
    std::vector<float> verts;
    // When drawing the base tic tac toe board, we will need to draw 4
    // lines (2 horizontal, 2 vertical) across the entire height and width
    // of the screen. Each of these lines will contain 4 points.
    // So, at a minimum we need to generate 4 * 4 = 16 vertices.
    // None of these vertices will be shared. Each line will be TTT::lineWidth wide
    float offset = TTT::lineWidth;
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
    std::array<float, 3> lh_botLeft  = {-0.33f - offset, -0.9f, 0.0f};
    std::array<float, 3> lh_botRight = {-0.33f + offset, -0.9f, 0.0f};
    std::array<float, 3> lh_topLeft  = {-0.33f - offset, 0.9f, 0.0f};
    std::array<float, 3> lh_topRight = {-0.33f + offset, 0.9f, 0.0f};
    lineArrays.push_back(lh_botLeft);
    lineArrays.push_back(lh_botRight);
    lineArrays.push_back(lh_topLeft);
    lineArrays.push_back(lh_topRight);

    // Right Horizontal
    std::array<float, 3> rh_botLeft  = {0.33f - offset, -0.9f, 0.0f};
    std::array<float, 3> rh_botRight = {0.33f + offset, -0.9f, 0.0f};
    std::array<float, 3> rh_topLeft  = {0.33f - offset, 0.9f, 0.0f};
    std::array<float, 3> rh_topRight = {0.33f + offset, 0.9f, 0.0f};
    lineArrays.push_back(rh_botLeft);
    lineArrays.push_back(rh_botRight);
    lineArrays.push_back(rh_topLeft);
    lineArrays.push_back(rh_topRight);

    // Top vertical
    std::array<float, 3> tv_botLeft  = {-0.9f, 0.33f - offset, 0.0f};
    std::array<float, 3> tv_topLeft  = {-0.9f, 0.33f + offset, 0.0f};
    std::array<float, 3> tv_botRight = {0.9f, 0.33f - offset, 0.0f};
    std::array<float, 3> tv_topRight = {0.9f, 0.33f + offset, 0.0f};
    lineArrays.push_back(tv_botLeft);
    lineArrays.push_back(tv_topLeft);
    lineArrays.push_back(tv_botRight);
    lineArrays.push_back(tv_topRight);

    // Bottom vertical
    std::array<float, 3> bv_botLeft  = {-0.9f, -0.33f - offset, 0.0f};
    std::array<float, 3> bv_topLeft  = {-0.9f, -0.33f + offset, 0.0f};
    std::array<float, 3> bv_botRight = {0.9f, -0.33f - offset, 0.0f};
    std::array<float, 3> bv_topRight = {0.9f, -0.33f + offset, 0.0f};
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
    glRenderer.draw();
}

std::pair<std::vector<float>, std::vector<int>> GameBoard::generateCircleVertices(const int cellIndex) {
    std::vector<float> verts;
    std::vector<int> indices;
    return std::pair{verts, indices};
}

std::pair<std::vector<float>, std::vector<int>> GameBoard::generateXVertices(const int cellIndex) {
    // We imagine each cell has its own local coordinate system
    // in the range [0, 1] for each axis. 
    // The below array lists the central points for each X that
    // will be connected. 
    // std::array<std::array<float, 2>, 4> xPoints = {
    //     {
    //         {0.33f, 0.66f},
    //         {0.33f, 0.33f},
    //         {0.66f, 0.33f},
    //         {0.66f, 0.66f}
    //     }
    // };

    // Next, we translate these local points to global points depending
    // which cell we're in. There are 3 possible ranges to which we 
    // need to translate:
    //  - 0: [-1.0f, -0.33f]
    //  - 1: [-0.33f, 0.33f]
    //  - 2: [0.33f, 1.0f]
    // The cell grid is outlined as follows:
    // 0, 1, 2
    // 3, 4, 5
    // 6, 7, 8
    // So, 0, 3, 6 belong to 0, 
    // 1, 4, 7 belong to 1, 
    // and 2, 5, 8 belong to 2
    std::array<std::array<float, 2>, 3> ranges = {
      {
        {-1.0f, -0.33f},
        {-0.33f, 0.33f},
        {0.33f, 1.0f}
      }  
    };
    std::array<std::array<float, 2>, 2> localRange;
    switch(cellIndex) {
        case 0:
            localRange = {ranges[0], ranges[2]};
            break;
        case 3:
            localRange = {ranges[0], ranges[1]};
            break;
        case 6:
            localRange = {ranges[0], ranges[0]};
            break;
        case 1:
            localRange = {ranges[1], ranges[2]};
            break;
        case 4:
            localRange = {ranges[1], ranges[1]};
            break;
        case 7:
            localRange = {ranges[1], ranges[0]};
            break;
        case 2:
            localRange = {ranges[2], ranges[2]};
            break;
        case 5:
            localRange = {ranges[2], ranges[1]};
            break;
        case 8:
        default:
            localRange = {ranges[2], ranges[0]};
            break;
    }

    // Now that we know the correct range, we figure out where the x points should be globally
    const float quarterRangeWidthX = (localRange[0][1] - localRange[0][0]) / 4.0f;
    const float quarterRangeWidthY = (localRange[1][1] - localRange[1][0]) / 4.0f;
    std::array<std::array<float, 2>, 4> xPoints = {
        {
            {localRange[0][0] + quarterRangeWidthX, localRange[1][1] - quarterRangeWidthY},
            {localRange[0][0] + quarterRangeWidthX, localRange[1][0] + quarterRangeWidthY},
            {localRange[0][1] - quarterRangeWidthX, localRange[1][0] + quarterRangeWidthY},
            {localRange[0][1] - quarterRangeWidthX, localRange[1][1] - quarterRangeWidthY}
        }
    };


    // Finally, we construct the resulting array of points and triangles
    // by determining the corner points of each line from the central points
    // using the TTT::lineWidth offset.
    // We could do some fancy trig to make it look better, but for now we'll just 
    // add / subtract half the lineWidth to each dimension from the point
    float halfOffset = TTT::lineWidth; 
    // We have 4 points for each line, each with 3 dimensions, so we need a flat array
    // of length 12 for each line. Since we have two lines (0,2 and 1,3), we need an array
    // of length 24. 
    std::vector<float> rawXPoints = {
        xPoints[0][0] + halfOffset, xPoints[0][1] + halfOffset, 0.0f, // xyz for point 0 (top left)
        xPoints[0][0] - halfOffset, xPoints[0][1] - halfOffset, 0.0f, 
        xPoints[2][0] + halfOffset, xPoints[2][1] + halfOffset, 0.0f, // xyz for point 2 (bottom right)
        xPoints[2][0] - halfOffset, xPoints[2][1] - halfOffset, 0.0f,
        xPoints[1][0] + halfOffset, xPoints[1][1] - halfOffset, 0.0f, // xyz for point 1 (bottom left)
        xPoints[1][0] - halfOffset, xPoints[1][1] + halfOffset, 0.0f, 
        xPoints[3][0] + halfOffset, xPoints[3][1] - halfOffset, 0.0f, // xyz for point 3 (top right)
        xPoints[3][0] - halfOffset, xPoints[3][1] + halfOffset, 0.0f,
    };
    // Since we have 4 triangles, each consisting of 3 points, we will need an array of length 12.
    std::vector<int> indices = {
        0, 1, 2,
        1, 2, 3,
        4, 5, 7,
        4, 6, 7
    };

    return std::pair{rawXPoints, indices};
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