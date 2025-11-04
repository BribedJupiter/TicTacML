#include "glad/glad.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>

#include <chrono>
#include <iostream>
#include <queue>
#include <mutex>
#include <stdio.h>
#include <stdlib.h>
#include <thread>
#include <windows.h>
#include <atomic>
#include <string>

#include "Game.h"
#include "constants.h"

bool trainingMode = true; // If we're in training or testing mode
std::atomic<bool> killThread = false;
std::queue<std::string> msgQueue;
std::mutex queueLock;
int g_cellMove = -1;
std::mutex moveLock;

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

void playMove(GameBoard& board, const int cell) {
    // Draw a shape depending on the current turn
    if (board.getTurn()) {
        board.placeCircle(cell);
    } else {
        board.placeX(cell);
    }

    // Output debug board
    board.printGrid();

    // Check if either side has won
    const auto winData = board.checkWin();
    if (winData.first != 0 && !board.isOver()) {
        // If we detect a game ending condition but haven't yet updated the board, end the game
        board.endGame(winData);
    }
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
    // If we're in training mode, then we want to export our move data
    if (trainingMode && cell >= 0) {
        csvHandler.exportMove(cell);
    }

    // Apply our move to the board
    playMove(board, cell);
    
    // Some debug output
    std::cout << "Clicked cell: " << cell << std::endl;
    return cell;
}

void writeToPython(HANDLE pyStdInWr, std::string message) {
    auto tid = std::this_thread::get_id();
    // See https://learn.microsoft.com/en-us/windows/win32/procthread/creating-a-child-process-with-redirected-input-and-output

    // Prepare message
    std::string msg = message += "\n";

    // Write the message (Python must consume it to be non-blocking)
    // See https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-writefile
    DWORD bytesWritten = 0;
    BOOL success = WriteFile(pyStdInWr, msg.c_str(),(DWORD)msg.size(), &bytesWritten, NULL);
    
    // Error checking
    if (!success) {
        std::cerr << "[" << tid << "] " << "Failed to write to Python: " << GetLastError() << std::endl;
        return;
    }

    // Otherwise
    std::cout << std::flush << "[" << tid << "] " << "Wrote " << bytesWritten << " bytes to Python saying " << message << std::endl; 
}

void readFromPython(HANDLE pyStdOutRd) {
    auto tid = std::this_thread::get_id();
    // See https://learn.microsoft.com/en-us/windows/win32/procthread/creating-a-child-process-with-redirected-input-and-output    

    // See https://learn.microsoft.com/en-us/windows/win32/api/namedpipeapi/nf-namedpipeapi-peeknamedpipe
    DWORD bytesToRead = 0;
    if (!PeekNamedPipe(pyStdOutRd, NULL, 0, NULL, &bytesToRead, NULL)) {
        std::cerr << "[" << tid << "] " << "Failure to peek at pyStdOutRd" << std::endl;
        return;
    }

    // If we have nothing available to read, then return
    if (bytesToRead == 0) {
        return;
    }
    
    // See also: https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-readfile
    BOOL success = FALSE;
    DWORD bytesRead = 0;
    char buf[512];
    success = ReadFile(pyStdOutRd, buf, sizeof(buf), &bytesRead, NULL);
    if (success && bytesRead > 0) {
        // Form a string with the result and output it
        std::string result(buf, bytesRead);
        std::cout << result << std::endl;

        // Process data from Python
        // std::string::find will search for a string and then return the index of the occurrence
        size_t cmd = result.find("READY");
        if (cmd != std::string::npos) {
            std::cout << "[" << tid << "] " << "Python ready..." << std::endl;
        }

        size_t rspmv = result.find("RSPMV");
        if (rspmv != std::string::npos) {
            std::cout << "[" << tid << "] " << "RSPMV - Attempting to lock on move" << std::endl;
            const std::lock_guard<std::mutex> lock(moveLock);
            std::cout << "[" << tid << "] "<< "Received move: " << result[rspmv + 6] << std::endl;
            const char* move = result.c_str() + rspmv + 6;
            g_cellMove = atoi(move); // convert from ASCII character to integer value
        }
    }
    std::cout << std::flush << "[" << tid << "] " << "Read " << bytesRead << " bytes from Python" << std::endl;
}

void runModel() {
    auto tid = std::this_thread::get_id();

    // Run our python model as a subprocess, we'll exchange input and output in the main update loop
    std::string cmd = "python " + std::string(MODEL_PATH) + " " + std::string(CSV_PATH) + "/out_log.csv";

    // see https://learn.microsoft.com/en-us/windows/win32/procthread/creating-a-child-process-with-redirected-input-and-output
    HANDLE pyStd_IN_RD = NULL;
    HANDLE pyStd_IN_WR = NULL;
    HANDLE pyStd_OUT_RD = NULL;
    HANDLE pyStd_OUT_WR = NULL;
    SECURITY_ATTRIBUTES sas;
    sas.nLength = sizeof(SECURITY_ATTRIBUTES);
    sas.bInheritHandle = TRUE; // Allow inherited pipe handles
    sas.lpSecurityDescriptor = NULL;

    // Create pipes (see https://learn.microsoft.com/en-us/windows/win32/api/namedpipeapi/nf-namedpipeapi-createpipe)
    if (!CreatePipe(&pyStd_OUT_RD, &pyStd_OUT_WR, &sas, 0)) {
        std::cerr << "[" << tid << "] " << "Failed to create pipe: STDOUT" << std::endl;
        return; // TODO: Beware these early returns
    }

    // Block read STDOUT inheritance (this avoids conflicts with the normal CreateProcess inheritance mechanism, I think)
    SetHandleInformation(pyStd_OUT_RD, HANDLE_FLAG_INHERIT, 0);

    // Handle STDIN pipe
    if (!CreatePipe(&pyStd_IN_RD, &pyStd_IN_WR, &sas, 0)) {
        std::cerr << "[" << tid << "] " << "Failed to create pipe: STDIN" << std::endl;
        return;
    }

    // Block write to STDIN inheritance
    SetHandleInformation(pyStd_IN_WR, HANDLE_FLAG_INHERIT, 0);

    // Create the child process for Python (see https://learn.microsoft.com/en-us/windows/win32/procthread/creating-processes)
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.hStdError = pyStd_OUT_WR; // Connect to the pre-defined pipes we made earlier
    si.hStdOutput = pyStd_OUT_WR;
    si.hStdInput = pyStd_IN_RD;
    si.dwFlags |= STARTF_USESTDHANDLES;
    ZeroMemory(&pi, sizeof(pi));

    if (!CreateProcess(NULL, cmd.data(), NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
        std::cerr << "[" << tid << "] " << "Failed to spawn subprocess" << std::endl;
        return;
    }

    // Close handles unneeded by parent
    CloseHandle(pyStd_OUT_WR);
    CloseHandle(pyStd_IN_RD);

    // We're running!
    std::cout << "[" << tid << "] " << "Model running..." << std::endl;

    while(!killThread) {
        // Read whatever python output may exist
        readFromPython(pyStd_OUT_RD);

        // Process queue elements
        {
            std::cout << "[" << tid << "] " << "QUEUECHECK - Attempting to lock on queue" << std::endl;
            const std::lock_guard<std::mutex> lock(queueLock); // We've now locked the message queue and can process safely
            if (!msgQueue.empty()) {
                while (!msgQueue.empty()) {
                    writeToPython(pyStd_IN_WR, msgQueue.front());
                    msgQueue.pop();
                }
            }
        } // End of processing scope, we use this scope since the mutex is unlocked when it leaves scope

        // Sleep
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    // Shutdown
    writeToPython(pyStd_IN_WR, "shutdown"); // tell python to shutdown
    std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Give python time to shutdown
    readFromPython(pyStd_OUT_RD); // Read final Python output
    int modelReturn = WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    CloseHandle(pyStd_OUT_RD);
    CloseHandle(pyStd_IN_WR);
    std::cout << "[" << tid << "] " << "Model subprocess exited with code " << modelReturn << std::endl;
}

int main() {
    //*********************************************************
    // Start machine learning model (Windows only)
    //*********************************************************
    std::thread mgr(runModel);

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
                if (trainingMode && mouse->button == sf::Mouse::Button::Left) {
                    // Get the mouse position in window coordinates and hand off to handler 
                    sf::Vector2f mousePosWindow = window.mapPixelToCoords(sf::Mouse::getPosition(window));
                    std::cout << "Clicked: (" << mousePosWindow.x << "," << mousePosWindow.y << ")" << std::endl;
                    int move = handleClick(mousePosWindow, window, board, csvHandler);
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

                if (key->scancode == sf::Keyboard::Scancode::M) {
                    trainingMode = !trainingMode;
                    std::cout << "TRAINING MODE = " << trainingMode << std::endl;
                }

                if (!trainingMode && key->scancode == sf::Keyboard::Scancode::N) {
                    if (!board.isOver()) { // Why would you ask for a move after the game ends
                        std::cout << "Asking AI for move..." << std::endl; 
                        {
                            std::cout << "AIREQUEST - Attempting to lock on queue" << std::endl;
                            const std::lock_guard<std::mutex> lock(queueLock); // we are now locked until lock goes out of scope
                            msgQueue.push(std::string("RQSTMV[" + csvHandler.generateRowData(-1) + "]&" + std::to_string(0)));
                        }
                    }
                }
            }
        }

        // Update board if the model has made a move
        if (g_cellMove >= 0) {
            int tempMove = -1;
            {
                // Read and reset the cell move value
                std::cout << "FIRSTAIMOVE - Attempting to lock on move" << std::endl;
                const std::lock_guard<std::mutex> lock(moveLock);
                tempMove = g_cellMove;
                g_cellMove = -1;
            }

            // Check if we can place
            if (board.canPlace(tempMove)) {
                playMove(board, tempMove);
            } else {
                // Otherwise, request a move (blocking) over and over again until we find a valid one
                bool found = false;
                int attempts = 0;
                bool receivedResp = true;
                while (!found) {
                    std::cout << "Model returned a bad value " << tempMove << ". Trying again: Attempt: " << attempts << std::endl;
                    {
                        // Publish a request
                        std::cout << "UPDATE - Attempting to lock on queue" << std::endl;
                        const std::lock_guard<std::mutex> lock(queueLock);
                        if (receivedResp) { // Only publish a message if we aren't currently working on one
                            std::cout << "Empty message queue. Making request..." << std::endl;
                            msgQueue.push(std::string("RQSTMV[" + csvHandler.generateRowData(-1) + "]&" + std::to_string(attempts)));
                            receivedResp = false;
                        }
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
                    {
                        // See if we have a valid choice
                        std::cout << "UPDATE - Attempting to lock on move" << std::endl;
                        const std::lock_guard<std::mutex> lock(moveLock);
                        if (g_cellMove >= 0) {
                            receivedResp = true; // we have now received a response, so see if we can play it
                            if (board.canPlace(g_cellMove)) {
                                std::cout << "Finally found a move on attempt " << attempts << ". Playing " << g_cellMove << std::endl;
                                found = true;
                                playMove(board, g_cellMove);
                            } else {
                                attempts += 1;
                                std::cout << "Bad response: " << g_cellMove << std::endl;
                            }
                        } else {
                            std::cout << "Move not ready yet: " << g_cellMove << std::endl;
                        }
                        g_cellMove = -1; // reset g_cellMove to force a new value
                    }
                }
            }
        }

        // Draw the TicTacToe board on the screen
        board.drawBoard();

        // End the frame (internally swaps front and back buffers)
        window.display();
    }

    // Clean up & release resources
    std::cout << "Closing..." << std::endl;
    killThread = true;
    mgr.join();
}