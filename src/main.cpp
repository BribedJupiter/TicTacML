#include "glad/glad.h"
#include <SFML/Window.hpp>
#include <SFML/OpenGL.hpp>

#include <array>
#include <exception>
#include <iostream>
#include <fstream>
#include <iterator>
#include <string>

std::string loadShaders(const std::string filename) {
    std::ifstream file;
    file.exceptions(file.exceptions() | std::ios::failbit);
    try {
        file.open(filename.c_str());
    } catch (std::exception e) {
        std::cout << "Error opening " << filename  << " -> " << e.what() << std::endl;
        return "";
    }

    std::string text((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    return text;
}

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



int main() {
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

    // Load Shaders
    std::string vertexShaderString = loadShaders("C:/Users/jgb10/dev/obelisk/src/vertexShader.glsl");
    std::string fragmentShaderString = loadShaders("C:/Users/jgb10/dev/obelisk/src/fragmentShader.glsl");
    if (vertexShaderString == "" || fragmentShaderString == "") {
        return -1;
    }

    // GL Setup
    glClearColor(0.1f, 0.0f, 0.1f, 1.0f);

    // See the LearnOpenGL textbook
    // Store the rectangle's vertices
    float vertices[] = {
        -0.5f, -0.5f, 0.0f, // bottom left
        0.5f, -0.5f, 0.0f, // bottom right
        0.5f, 0.5f, 0.0f, // top right
        -0.5f, 0.5f, 0.0f // top left
    };

    // Store which vertices correspond to which shape
    unsigned int indices[] = {
        0, 1, 3, // triangle 1
        1, 2, 3 // triangle 2
    };

    // Create a vertex array object (VAO) to store vertex attribute states
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // Create a vertex buffer object to store the vertex data
    unsigned int VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Store which indices OpenGL should use to draw
    unsigned int EBO;
    glGenBuffers(1, &EBO); 
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Compile vertex shader
    const char* vertexShaderSource = vertexShaderString.c_str();
    unsigned int vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    // Check if vertex shader compilation was successful
    int vertexShaderCompilationSuccess;
    char vertexInfoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &vertexShaderCompilationSuccess);
    if (!vertexShaderCompilationSuccess) {
        glGetShaderInfoLog(vertexShader, 512, NULL, vertexInfoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << vertexInfoLog << std::endl;
    }

    // Compile fragment shader
    const char* fragmentShaderSource = fragmentShaderString.c_str();
    unsigned int fragmentShader;
    fragmentShader= glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    // Check if fragment shader compilation was successful
    int fragmentShaderCompilationSuccess;
    char fragmentInfoLog[512];
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &fragmentShaderCompilationSuccess);
    if (!fragmentShaderCompilationSuccess) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, fragmentInfoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << fragmentInfoLog << std::endl;
    }

    // Setup shader program
    unsigned int shaderProgram;
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Check for shader program success
    int shaderProgramSuccess;
    char programInfoLog[512];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &shaderProgramSuccess);
    if (!shaderProgramSuccess) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, programInfoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINK_FAILED\n" << programInfoLog << std::endl;
    }

    // Delete the now unneeded (after linking) shader objects
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Link the vertex attributes
    // Note that the previous VBO is still bound, so this will apply to that
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

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
                glViewport(0, 0, resized->size.x, resized->size.y);
            }
        }

        // Clear buffers
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Prepare to draw
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO); // Remembers which buffers are bound already automatically
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // Draw
        static int xPos = 0;

        // End the frame (internally swaps front and back buffers)
        window.display();
    }

    // Clean up & release resources
}