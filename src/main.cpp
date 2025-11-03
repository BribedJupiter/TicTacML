#include "glad/glad.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>

#include "Game.h"
#include "constants.h"

// Khronos debug function (see https://www.khronos.org/opengl/wiki/OpenGL_Error)
void GLAPIENTRY MessageCallback( GLenum source,
                 GLenum type,
                 GLuint id,
                 GLenum severity,
                 GLsizei length,
                 const GLchar* message,
                 const void* userParam )
{
    std::cerr << "GL CALLBACK: " << message << std::endl;
}

// Translate a mouse click into placing an element on the board
int handleClick(const sf::Vector2f mousePosWindow, const sf::RenderWindow& window, GameBoard& board, CSVHandler& csvHandler) {
    // If the game is over, do nothing.
    if (board.isOver()) {
        return -1;
    }

    // Determine the widths and heights of the colums and rows
    sf::Vector2u windowSize = window.getSize();
    float columnWidth = static_cast<float>(windowSize.x) / 3.0f;
    float rowHeight = static_cast<float>(windowSize.y) / 3.0f; 

    // We know that board is divided into three rows and three columns. Thus
    // we have 3 possible ranges each for our point to be inside of
    // Check if the position is invalid
    if (mousePosWindow.x < 0 || mousePosWindow.x > windowSize.x || mousePosWindow.y < 0 || mousePosWindow.y > windowSize.y) {
        std::cout << "ERROR::WINDOW::MOUSE_BEYOND_BOUNDS" << std::endl;
        return  -1;
    }

    // TODO: There's likely a better algorithm to do this. Can I remove any of these if statements?
    // Determine the column
    int cell = 0;
    if (mousePosWindow.x <= columnWidth) {
        // first column - do nothing 
    } else if (mousePosWindow.x <= columnWidth * 2.0f) {
        // second column
        cell = 1;
    } else {
        // third column
        cell = 2;
    }

    // Determine the row
    if (mousePosWindow.y <= rowHeight) {
        // first row - do nothing
    } else if (mousePosWindow.y <= rowHeight * 2.0f) {
        // second column - move to the second row
        cell += 3;
    } else {
        // third column - move to the third row
        cell += 6;
    }

    // Check to make sure we don't place a shape an an occupied cell 
    if (!board.canPlace(cell)) {
        std::cout << "Cannot place on already placed cell." << std::endl;
        return  -1;
    }

    // Export screen data to CSV if it's a valid move BEFORE we update the game board
    // The idea is we want the screen data to represent the state before we make the move,
    // and the move we provide to be the "next move". This is because we want to predict future moves
    // based o ncurrent screen data.
    if (cell >= 0) {
        csvHandler.exportMove(cell);
    }

    // Draw a shape depending on the current turn
    if (board.getTurn()) {
        board.placeCircle(cell);
    } else {
        board.placeX(cell);
    }

    // Some debug output
    std::cout << "Clicked cell: " << cell << std::endl;
    board.printGrid();
    return cell;
}

// See here: https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/popen-wpopen
void checkPipeOutput(FILE* proc) {
    if (proc == NULL) return;
    char buf[128];
    while (fgets(buf, sizeof(buf), proc)) {
        std::cout << buf;
    } std::cout << std::endl;
}

int main() {
    //*********************************************************
    // Start machine learning model (Windows only)
    //*********************************************************
    // Run our python model as a subprocess, we'll exchange input and output in the main update loop
    std::string cmd = "python " + std::string(MODEL_PATH);
    FILE* proc = _popen(cmd.c_str(), "r");
    if (proc == NULL) {
        return -1;
    };
    std::cout << "Model running..." << std::endl;


    //*********************************************************
    // Create the SFML window, OpenGL context, and setup GLAD
    //*********************************************************

    // OpenGL Context Setup
    sf::ContextSettings contextSettings;
    contextSettings.depthBits = 24;
    contextSettings.stencilBits = 8;
    contextSettings.antiAliasingLevel = 4;
    contextSettings.majorVersion = 4;
    contextSettings.minorVersion = 3;
    contextSettings.attributeFlags = contextSettings.Default;
    contextSettings.sRgbCapable = true;

    // SFML Window Setup
    sf::RenderWindow window(sf::VideoMode({TTT::screenWidth, TTT::screenHeight}), "TicTacML", sf::Style::Default, sf::State::Windowed, contextSettings);
    window.setFramerateLimit(144);
    window.setVerticalSyncEnabled(true);
    if (!window.setActive(true)) {
        return -1;
    };

    // Initialize OpenGL extension loader
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(sf::Context::getFunction))) {
        return -1;
    }

    // Setup renderer
    Renderer glRenderer = Renderer();
    if (glRenderer.initFailed()) {
        return -1;
    }

    // Setup the game
    GameBoard board = GameBoard(glRenderer);

    // For handling our generate data to implement the ML model
    CSVHandler csvHandler;

    // Enable debug output (see https://www.khronos.org/opengl/wiki/OpenGL_Error)
    glEnable( GL_DEBUG_OUTPUT );
    glDebugMessageCallback( MessageCallback, 0 );
    glPixelStorei(GL_PACK_ROW_LENGTH, TTT::screenWidth); // Set to screenWidth number of pixels per row
    glPixelStorei(GL_PACK_ALIGNMENT, 1); // Set to 1 byte pixel row alignment
    
    //*********************************************************
    // Begin the main game loop
    //*********************************************************

    bool running = true;
    while (running) {
        // Update python subprocess output
        checkPipeOutput(proc);

        while (const std::optional event = window.pollEvent())
        {
            // TODO: Change these to callbacks
            if (event->is<sf::Event::Closed>())
            {
                running = false;
            } 
            else if (const auto* resized = event->getIf<sf::Event::Resized>())
            {
                glRenderer.resize(resized->size.x, resized->size.y);
            }

            if (const auto* mouse = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (mouse->button == sf::Mouse::Button::Left) {
                    // Get the mouse position in window coordinates and hand off to handler 
                    sf::Vector2f mousePosWindow = window.mapPixelToCoords(sf::Mouse::getPosition(window));
                    std::cout << "Clicked: (" << mousePosWindow.x << "," << mousePosWindow.y << ")" << std::endl;
                    int move = handleClick(mousePosWindow, window, board, csvHandler);

                    // Check if either side has won
                    const auto winData = board.checkWin();
                    if (winData.first != 0 && !board.isOver()) {
                        // If we detect a game ending condition but haven't yet updated the board, end the game
                        board.endGame(winData);
                    }
                }
            }
            
            if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
                if (key->scancode == sf::Keyboard::Scancode::Escape) {
                    running = false;
                }

                if (key->scancode == sf::Keyboard::Scancode::R) {
                    // Reset the game to the start
                    board.reset();
                }

                if (key->scancode == sf::Keyboard::Scancode::T) {
                    // Toggle wireframe draw
                    glRenderer.toggleWireframe();
                }
            }
        }

        // Draw the TicTacToe board on the screen
        board.drawBoard();

        // End the frame (internally swaps front and back buffers)
        window.display();
    }

    // Clean up & release resources
    int modelReturn = _pclose(proc);
    std::cout << "Model subprocess exited with code " << modelReturn << std::endl;
}