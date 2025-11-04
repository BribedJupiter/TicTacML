#include "glad/glad.h"

#include <iostream>
#include <fstream>
#include <string>
#include <exception>
#include <array>
#include <vector>

#include "csvHandler.h"
#include "constants.h"

std::string CSVHandler::generateRowData(const int move) {
    // Read our screen data from OpenGL
    constexpr int bufSize = TTT::screenWidth * TTT::screenHeight * 3; // 3 bytes for GL_RGB
    GLubyte *data = static_cast<GLubyte*>(malloc(bufSize));
    if (!data) return "FAILURE";
    glReadPixels(0, 0, TTT::screenWidth, TTT::screenHeight, GL_RGB, GL_UNSIGNED_BYTE, data);

    // Each row of pixels has TTT::screenWidth * 3 bytes. Each row has TTT::screenWidth pixels. So, for 600x600 resolution, we have 1800 bytes per row. We have 600 rows. So in total, we're dealing with ~1M bytes.
    // To reduce our data, we average every pixel together, which reduces us to 600 bytes per row for example. Now, we're dealing with a 600x600 grid. This is still far too large, so we will average every 200x200
    // area together. This would reduce our total output feature space to 9 dimensions.
    // Since we know we're only working with a 600x600 pixel grid, we will hardcode that assumption.

    // Practically, this means that in every row we average each 600 bytes (200 pixels) into 1 byte. This should yield 3 bytes per row. We then inspect the value of each byte and clamp it to the range [0-15] so we can use it as HEX.
    // So, we should get 3 HEX characters per row. Then, we avaerage the first 200 rows in each column, then the 2nd 200 rows, then the 3rd 200, in each column (clamping the same), to yield a 3x3 grid of 9 HEX characters after every move.

    // Output stream
    std::stringstream temp;
    temp << std::hex; // set to hex output
    
    // handle row reducing
    std::vector<int> rowResults;
    for (int row = 0; row < TTT::screenHeight; ++row) {
        for (int j = 0; j < 3; j++) {
            int sum = 0;
            // Each row is 1800 bytes long, average every 600 bytes (200 pixels * 3 bytes per pixel). 
            // Iterate through the rows by multiplying the current row number by the length of each row.
            // j corresponds to which third we're averaging in the row.
            int offset = row * TTT::screenWidth * 3;
            for (int i = 0; i < 600; i++) {
                sum += static_cast<int>(data[offset + i + j * 600]);
            }
            sum /= 600; // divide by 600 to get within the range 0 - 255
            rowResults.push_back(sum);
        }
    }
    if (rowResults.size() != 1800) {
        std::cerr << "Incorrect row reduction." << std::endl;
        return "FAILURE";
    }

    // handle column reduction
    std::vector<int> colResults;
    for (int col = 0; col < 3; col++) {
        std::array<int, 3> sums = {0, 0, 0};
        
        // Iterate over each column
        for (int i = 0; i < 600; i++) {
            int index = i * 3 + col; // Index is column offset + the current i value * 3 since we have 1800 total values
            if (index < 200) sums[0] += rowResults[index]; // If in the first 3rd, add to 1st sum
            else if (index < 400) sums[1] += rowResults[index]; // 2nd
            else sums[2] += rowResults[index]; // 3rd
        }

        for (auto s : sums) {
            colResults.push_back(s / 200);
        }
    }
    if (colResults.size() != 9) {
        std::cerr << "Incorrect col reduction." << std::endl;
        return "FAILURE";
    }

    // adjust column values so they're in the range 0-15
    for (auto iter = colResults.begin(); iter != colResults.end(); iter++) {
        *iter /= 16;
    }
    
    // Output the move's information as a row to the output log
    for (auto val : colResults) {
        temp << val << ",";
    } temp << std::dec << move; // output the move in decimal
    
    // Write to our output string
    std::string result(temp.str());

    // Free the screen data
    free(data);

    // Return
    return result;
}

void CSVHandler::exportMove(const int move) {
    std::string outPath = std::filesystem::path(CSV_PATH).string() + "/out_log.csv";

    std::ofstream file;
    file.exceptions(file.exceptions() | std::ios::failbit);
    try {
        file.open(outPath.c_str(), std::ios::app);
    } catch (std::exception e) {
        std::cerr << "Unable to open CSV log file: " << outPath << " --> " << e.what() << std::endl;
        return;
    }

    // Write to our output file
    std::string result = generateRowData(move);
    if (result != "FAILURE") {
           file << result << std::endl;
    }

    file.close();
}