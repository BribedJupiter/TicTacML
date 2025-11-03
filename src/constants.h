#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <filesystem>
#include <string>

namespace TTT {
    const std::filesystem::path shaderSourceDir = SHADER_PATH;
    const std::string vertexShaderPath = shaderSourceDir.string() + "/vertexShader.glsl";
    const std::string fragmentShaderPath = shaderSourceDir.string() + "/fragmentShader.glsl";
    constexpr int screenWidth = 600;
    constexpr int screenHeight = 600;
    constexpr float lineWidth = 0.05f;
};
#endif
