####### TicTacML v0.1 #######
cmake_minimum_required(VERSION 3.28)
project(TicTacML LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_EXPORT_COMPILE_COMMANDS  ON)

include(FetchContent)
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.0.1
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM)
FetchContent_MakeAvailable(SFML)

add_executable(main src/main.cpp src/GameBoard.cpp src/Renderer.cpp src/csvHandler.cpp lib/glad/src/glad.c)
target_include_directories(main PRIVATE src lib/glad/include PRIVATE lib/glad/KHR)
target_compile_features(main PRIVATE cxx_std_17)
target_compile_definitions(main PRIVATE
    SHADER_PATH="${CMAKE_SOURCE_DIR}/shaders"
    CSV_PATH="${CMAKE_SOURCE_DIR}/csvout"
    MODEL_PATH="${CMAKE_SOURCE_DIR}/src/model.py"
)
target_link_libraries(main PRIVATE SFML::Graphics SFML::Audio SFML::Network)
