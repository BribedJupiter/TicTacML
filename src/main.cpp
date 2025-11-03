#include "glad/glad.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>

#include <iostream>

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
void handleClick(const sf::Vector2f mousePosWindow, const sf::RenderWindow& window, GameBoard& board) {
    // If the game is over, do nothing.
    if (board.isOver()) {
        return;
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
        return;
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
        return;
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
}

int main() {
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
                    handleClick(mousePosWindow, window, board);

                    // Export screen data to CSV
                    csvHandler.exportMove();

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
}