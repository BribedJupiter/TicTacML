# Overview
In this project we train a machine learning model to play Tic-Tac-Toe. The application runs in two modes. In training mode, the game is manually controlled by the player and every time the player makes a move, screen data is captured, processed, and exported to a CSV file (before the move is made) along with the desired move. In testing mode, the player makes requests to the  model for moves which will then be played in the game.

Some quick details:

- The Tic-Tac-Toe game was built using OpenGL for graphics and SFML for window and context management. When a move is made, the needed vertices are computed and provided to OpenGL.
- The game spawns a secondary managment thread on startup that launches and communicates with a Python subprocess running our machine learning model. 
- On launch, the model trains on the CSV data generated in training mode.
- Every screen capture generates ~1M bytes of data. During processing, this is reduced to a sequence of 9 hexadecimal characters before being exported.
- The input features to the model are these 9 hexadecimal characters, the output class is the next predicted move.
- X always has the first move. 
- On win, a line will be drawn across the winning row / column / diagonal. 

# Input
- "M" - Switch between training and testing mode.
- "N" - In testing mode, request a move from the model.
- "T" - Toggle wireframe view (a fun OpenGL feature).
- "R" - Restart the game state.
- "Left mouse click" - On a cell, play a move in that cell. Either X or O depending on the turn. 

# File Structure
### Folders
- /csvout/out_log.csv: The CSV file where we store the training data.
- /lib/glad: Where we store the GLAD files generated for this application.
- /shaders: Where we store the shaders necessary for running our OpenGL application
- /src: Where we store all the C++ and Python files for our program. 

### Source files
- constants.h - Provide constants for use across the whole program.
- csvHandler.cpp/.h - Manage the export of CSV data.
- Game.h - Header file for game logic-related classes.
- GameBoard.cpp - The class responsible for managing all logical game state information.
- Renderer.cpp - The class responsible for managing all rendering and most OpenGL code.
- model.py - The Python file run as a subprocess by our application that trains the model and then waits and responds to move requests from the Tic-Tac-Toe game. 
- main.py - Handle general processes for the application. Launch the application, process user-input, manage the IPC thread and the Python subprocess. 

# How to run
This project uses CMake as its build system. I use the CMake extension for VSCode to automatically build and run the project (built with the Ninja generator to export compile commands). However, you should just be able to use the provided CMakeLists.txt file by itself to build the project if you don't want to use the extension. 

Once it's built, either launch it through VSCode or navigate to build/bin/main.exe to launch the executable. 

Note that this project is currently only supported on Windows due to how I run the machine learning model subprocess (I used Windows system calls). 

# Dependencies

### C++
- OpenGL: Should be available on your device by default.
- SFML: For window management and to provide an OpenGL context. Fetched by CMake during the build process.
- GLAD: To load OpenGL extensions. Provided with this repository (see /lib).

### Python
- Pandas - Primarily for the read_csv function.
- Numpy - For array management.
- scikit-learn - For their awesome machine learning tools.
