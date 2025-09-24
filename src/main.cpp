#include "glad/glad.h"
#include <SFML/Window.hpp>

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
    sf::Window window(sf::VideoMode({1920u, 1080u}), "Obelisk", sf::Style::Default, sf::State::Windowed, contextSettings);
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
    
    //*********************************************************
    // Begin the main game loop
    //*********************************************************

    bool running = true;
    while (running) {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                running = false;
            } 
            else if (const auto* resized = event->getIf<sf::Event::Resized>())
            {
                glRenderer.resize(resized->size.x, resized->size.y);
            }
        }

        // Render a new frame to the screen
        glRenderer.render();

        // End the frame (internally swaps front and back buffers)
        window.display();
    }

    // Clean up & release resources
}