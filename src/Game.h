#pragma once

#include <array>
#include <string>

class GameBoard {
    // The Tic Tac Toe board is composed of 9 cells
    // A sector can either have nothing, an X, or a circle
    // The board looks something like this:
    /*
        _|_|_
        _|_|_
         | |
    */
    public:
        // Draw the game board outline
        void draw_board();

        // Draw a circle centered in specific a cell
        void place_circle(const int cellIndex);

        // Draw an X centered in specific a cell
        void place_x(const int cellIndex);

        // Debug print the grid to output
        void print_grid();

    private:
        enum CellState {
            CLEAR,
            CIRCLE,
            X
        };

        // A 2D array storing the current state of each tic-tac-toe cell on the grid 
        std::array<std::array<CellState, 3>, 3> grid;

        // Helper function to draw one of the rectangle lines on the board 
        void draw_rectangle();
};

class Renderer {
    // Abstract much of the OpenGL setup and rendering calls
    public:
        // Setup and initialize the OpenGL context
        Renderer();

        // Render once. This should be called every update
        void render();

        // Called if a resize window event occurs
        void resize(const int width, const int height);

        // Load a shader and return an empty string on failure
        std::string loadShader(const std::string filename);

        bool initFailed() {return initFailure;}

        // Clean up and shut down OpenGL and its context
        ~Renderer();
    
    private:
        // If construction fails, will be set to true
        int shaderProgramObject = 0;
        int vertexArrayObject = 0;
        bool initFailure = false;
};