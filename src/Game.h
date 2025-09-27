#pragma once

#include <array>
#include <vector>
#include <string>

class Renderer {
    // Abstract much of the OpenGL setup and rendering calls
    public:
        // Setup and initialize the OpenGL context
        Renderer();

        // Perform all the necessary setup steps before rendering can occur
        // and then draw.
        void draw();

        // Toggle wireframe mode
        void toggleWireframe();

        // Set the vertices and indices to be drawn from an outside source
        void setVertices(const std::pair<std::vector<float>, std::vector<int>> vertPair);

        // Called if a resize window event occurs
        void resize(const int width, const int height);

        // Add vertices to the renderer to be drawn
        void addVertices(const std::pair<std::vector<float>, std::vector<int>> vertPair);

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
        std::vector<float> vertices;
        std::vector<int> indices;
        int shaderProgramObject = 0;
        int vertexArrayObject = 0;

        // If construction fails, will be set to true
        bool initFailure = false;

        // To ensure that vertices have been set first
        bool readyToRender = false;
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

        // Generate the OpenGL vertices and relevant indices for the board
        // in normalized device coordinates
        std::pair<std::vector<float>, std::vector<int>> generateBoardVertices();

        // Place an X on the game board
        void placeX(const int cellIndex);

        // Place a Circle on the game board
        void placeCircle(const int cellIndex);

        // Draw the game board outline
        void drawBoard();

        // Debug print the grid to output
        void printGrid();

        // Switch to the next player
        void setNextTurn();

        // Get who's turn it is
        // 0 = X, 1 = Circle
        int getTurn();

        // Check if the game has been won or has ended in a draw
        // Returns the integer corresponding to CellState for a win,
        // returns the integer value of CellState.CLEAR for no win,
        // returns -1 on draw.
        std::pair<int, std::array<int, 3>> checkWin();

        // Display the win screen, end the game
        void endGame(const std::pair<int, std::array<int, 3>>);

        // Check if the game is still playing
        bool isOver() {return gameState != PLAYING && gameState != STARTING;}

        ~GameBoard();
    private:
        enum CellState {
            CLEAR = 0,
            CIRCLE = 1,
            X = 2
        };

        enum GameState {
            STARTING = 0,
            PLAYING = 1,
            X_WIN = 2,
            C_WIN = 3,
            DRAW = 4
        };

        // Will either be 1 or 0
        unsigned int turn = 0;

        // store the current GameState
        GameState gameState = STARTING;

        // Reference to our OpenGL rendering object
        Renderer& glRenderer;

        // Draw a circle centered in specific a cell
        std::pair<std::vector<float>, std::vector<int>> generateCircleVertices(const int cellIndex);

        // Draw an X centered in specific a cell
        std::pair<std::vector<float>, std::vector<int>> generateXVertices(const int cellIndex);

        // Draw a line in across the winning set of 3 elements
        std::pair<std::vector<float>, std::vector<int>> generateWinVertices(const std::array<int, 3> winVector);

        // Get the proper coordinate range for each cell
        std::array<std::array<float, 2>, 2> getCoordinateRange(const int cellIndex);

        // A 2D array storing the current state of each tic-tac-toe cell on the grid 
        std::array<std::array<CellState, 3>, 3> grid;
};