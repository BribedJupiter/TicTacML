#include "glad/glad.h"
#include <SFML/OpenGL.hpp>

#include <exception>
#include <iostream>
#include <fstream>
#include <iterator>
#include <string>

#include "Game.h"
#include "constants.h"

Renderer::Renderer() {
    //*********************************************************
    // Load shaders from their corresponding files
    //*********************************************************

    // Load Shaders
    std::cout << "Shader path is " << SHADER_PATH << std::endl;
    std::string vertexShaderString = loadShader(TTT::vertexShaderPath);
    std::string fragmentShaderString = loadShader(TTT::fragmentShaderPath);
    if (vertexShaderString == "" || fragmentShaderString == "") {
        initFailure = true;
    }

    //*********************************************************
    // Prepare OpenGL and its shader pipeline
    //*********************************************************
    // GL Setup
    glClearColor(0.1f, 0.0f, 0.1f, 1.0f);

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
        initFailure = true;
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
        initFailure = true;
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
        initFailure = true;
    }
    shaderProgramObject = shaderProgram; 

    // Delete the now unneeded (after linking) shader objects
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void Renderer::draw() {
    if (!readyToRender) {
        std::cout << "ERROR::DRAW::NOT_READY" << std::endl;
        return;
    }

    // Clear buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Prepare to draw
    glUseProgram(shaderProgramObject);
    glBindVertexArray(vertexArrayObject); // Remembers which buffers are bound already automatically
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Renderer::toggleWireframe() {
    // Default to true so that it turns on on the first call of this function
    static bool showWires = true; 
    if (showWires) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    showWires = !showWires;
}

void Renderer::setVertices(const std::pair<std::vector<float>, std::vector<int>> vertPair) {
    // TODO: We're recreating these buffers every time. Ideally we'd re-use them instead.
    vertices = vertPair.first;
    indices = vertPair.second;

    // Create a vertex array object (VAO) to store vertex attribute states
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    vertexArrayObject = VAO;

    // Create a vertex buffer object to store the vertex data
    unsigned int VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), vertices.data(), GL_DYNAMIC_DRAW);

    // Store which indices OpenGL should use to draw
    unsigned int EBO;
    glGenBuffers(1, &EBO); 
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * indices.size(), indices.data(), GL_DYNAMIC_DRAW);

    // Link the vertex attributes
    // Note that the previous VBO is still bound, so this will apply to that
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    readyToRender = true;
}

void Renderer::resize(const int width, const int height) {
    glViewport(0, 0, width, height);
}

void Renderer::addVertices(const std::pair<std::vector<float>, std::vector<int>> vertPair) {
    // TODO: This whole flow could be optimized a lot
    auto addVerts = vertPair.first;
    auto addIndices = vertPair.second;
    auto currentVerts = vertices;
    auto currentIndices = indices;

    // Update the new indices to account for vertex offsets and add to the indices list
    constexpr int dimNum = 3; // There are 3 points per vertex currently.
    static int offset = currentVerts.size() / dimNum;
    for  (auto index : addIndices) {
        currentIndices.push_back(index + offset);
    }
    offset += addVerts.size() / dimNum;

    // Add the new vertices to current after updating index offset
    currentVerts.insert(currentVerts.end(), addVerts.begin(), addVerts.end());

    // Set the new vertices
    setVertices(std::pair{currentVerts, currentIndices});
}

std::string Renderer::loadShader(const std::string filename) {
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

Renderer::~Renderer() {

}