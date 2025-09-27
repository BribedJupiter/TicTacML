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
                    sf::Vector2f mousePosWindow = window.mapPixelToCoords(sf::Mouse::getPosition(window));
                    std::cout << "Clicked: (" << mousePosWindow.x << "," << mousePosWindow.y << ")" << std::endl;
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