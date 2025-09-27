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

// Translate a mouse click into placing an element on the board
void handleClick(const sf::Vector2f mousePosWindow, const sf::RenderWindow& window, GameBoard& board) {
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

    // TODO: There's likely a better algorithm to do this
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
    std::cout << "Clicked cell: " << cell << std::endl;
    board.placeCircle(cell);
    board.printGrid();
    return;
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
    contextSettings.majorVersion = 3;
    contextSettings.minorVersion = 0;
    contextSettings.attributeFlags = contextSettings.Default;
    contextSettings.sRgbCapable = true;

    // SFML Window Setup
    sf::RenderWindow window(sf::VideoMode({TTT::screenWidth, TTT::screenHeight}), "Obelisk", sf::Style::Default, sf::State::Windowed, contextSettings);
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
                }
            }
            
            if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
                if (key->scancode == sf::Keyboard::Scancode::Escape) {
                    running = false;
                }

                if (key->scancode == sf::Keyboard::Scancode::X) {
                    // Place an X
                    static int cellX = 0;
                    if (cellX < 9) {
                        board.placeX(cellX++);
                    }
                }

                if (key->scancode == sf::Keyboard::Scancode::C) {
                    // Place a circle
                    static int cellC = 0;
                    if (cellC < 9) {
                        board.placeCircle(cellC++);
                    }
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