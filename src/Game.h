#include <array>

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
        void place_circle(int cellIndex);

        // Draw an X centered in specific a cell
        void place_x(int cellIndex);

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