#pragma once

#include <array>
#include <string>

class Renderer {
    // Abstract much of the OpenGL setup and rendering calls
    public:
        // Setup and initialize the OpenGL context
        Renderer();

        // Perform all the necessary setup steps before rendering can occur
        void prepareRender();

        // Called if a resize window event occurs
        void resize(const int width, const int height);

        // Helper function to draw one of the lines on the board 
        void drawLine();

        // Helper function to draw one of the circles on the board
        void drawCircle();

        // Load a shader and return an empty string on failure
        // It will convert the text from the shader file into an
        // std::string object so that it can be compiled by OpenGL.
        std::string loadShader(const std::string filename);

        // Returns true if the renderer initialized failed,
        // otherwise returns false
        bool initFailed() {return initFailure;}

        // Clean up and shut down OpenGL and its context
        ~Renderer();
    
    private:
        // If construction fails, will be set to true
        int shaderProgramObject = 0;
        int vertexArrayObject = 0;
        bool initFailure = false;
};

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
        GameBoard(Renderer& renderer);

        // Draw the game board outline
        void drawBoard();

        // Draw a circle centered in specific a cell
        void placeCircle(const int cellIndex);

        // Draw an X centered in specific a cell
        void placeX(const int cellIndex);

        // Debug print the grid to output
        void printGrid();

        ~GameBoard();
    private:
        enum CellState {
            CLEAR,
            CIRCLE,
            X
        };

        Renderer& glRenderer;

        // A 2D array storing the current state of each tic-tac-toe cell on the grid 
        std::array<std::array<CellState, 3>, 3> grid;
};

class GameManager {
    public:
        GameManager();

        void update();
        void render();

        ~GameManager();

    private:
        GameBoard board;
        Renderer renderer;
};