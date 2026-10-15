// ============================================================================
// Program Name: Assignemnt4.cpp
// Description : A recursive, object-oriented C++ program that solves Sudoku 
//               puzzles using a depth-first search brute-force algorithm with 
//               backtracking. It processes multiple puzzle files and displays 
//               all valid solutions.
// Inputs      : puzzle1.txt, puzzle2.txt, puzzle3.txt, puzzle4.txt, puzzle5.txt
// Outputs     : Displays the file name, original unsolved puzzle, and all valid 
//               solutions (or "No solution found" if none exist).
// Collaborators: GenAI Source: Claude (Anthropic), prompt written by the author. Code improvement made by Brayden Nelson
// Author      : Brayden Nelson
// Creation Date: October 15, 2026
// ============================================================================

#include <iostream>   // Include standard I/O library for console output
#include <fstream>    // Include file stream library for reading puzzle files
#include <string>     // Include string library to handle file names and tokens
#include <vector>     // Include vector library to store dynamic lists of solutions
#include <array>      // Include array library to hold fixed 9x9 grids efficiently

// Alias for a 9x9 grid of integers to ensure type declarations are readable
using Grid = std::array<std::array<int, 9>, 9>;

// ============================================================================
// CLASS: SudokuPuzzle
// Purpose: Owns one puzzle's grid, solves it recursively using backtracking,
//          and stores/prints every valid solution it discovers.
// ============================================================================
class SudokuPuzzle {
private: // Internal state variables hidden from outside execution
    Grid sudokuGrid;                    // Current working grid (0 indicates a blank cell)
    Grid originalGrid;                  // Untouched copy of the initial puzzle layout
    std::vector<Grid> foundSolutions;   // Dynamic list storing every completed grid found
    std::string fileName;               // String holding the name of the source puzzle file
    bool isLoaded;                      // Boolean flag tracking if the file was read correctly

    // ------------------------------------------------------------------------
    // MAJOR BLOCK: Constraint Verification
    // Returns true if placing 'value' at (row, col) breaks no row, column,
    // or 3x3 block rule given the current contents of the grid.
    // ------------------------------------------------------------------------
    bool isSafe(int row, int col, int value) const {
        for (int i = 0; i < 9; ++i) {                               // Loop through 9 positions
            if (sudokuGrid[row][i] == value) return false;          // Reject if value already exists in the row
            if (sudokuGrid[i][col] == value) return false;          // Reject if value already exists in the column
        }
        int blockRow = (row / 3) * 3;                               // Calculate the top row index of this 3x3 block
        int blockCol = (col / 3) * 3;                               // Calculate the left column index of this 3x3 block
        for (int r = 0; r < 3; ++r) {                               // Iterate through the 3 rows of the sub-block
            for (int c = 0; c < 3; ++c) {                           // Iterate through the 3 columns of the sub-block
                if (sudokuGrid[blockRow + r][blockCol + c] == value) return false; // Reject if value exists in the block
            }
        }
        return true;                                                // Return true as no conflicts were found
    }

    // ------------------------------------------------------------------------
    // MAJOR BLOCK: Initial Clue Consistency Check
    // Verifies the starting clues do not contradict each other before solving.
    // ------------------------------------------------------------------------
    bool cluesAreConsistent() {
        for (int r = 0; r < 9; ++r) {                               // Iterate through every row
            for (int c = 0; c < 9; ++c) {                           // Iterate through every column
                int cellValue = sudokuGrid[r][c];                   // Extract the current clue at this cell
                if (cellValue == 0) continue;                       // Skip verification for blank cells
                sudokuGrid[r][c] = 0;                               // Temporarily remove the clue from the grid
                bool isLegal = isSafe(r, c, cellValue);             // Check if placing the clue back is mathematically legal
                sudokuGrid[r][c] = cellValue;                       // Restore the clue to the grid
                if (!isLegal) return false;                         // Return false if the initial puzzle has duplicate clues
            }
        }
        return true;                                                // Return true indicating all initial clues are valid
    }

    // ------------------------------------------------------------------------
    // MAJOR BLOCK: Recursive Depth-First Search
    // Advances through the grid and exhaustively tests numbers with backtracking.
    // ------------------------------------------------------------------------
    void search(int pos) {
        while (pos < 81 && sudokuGrid[pos / 9][pos % 9] != 0) ++pos; // Skip past any already filled cells
        if (pos == 81) {                                            // Base case: check if every cell is filled
            foundSolutions.push_back(sudokuGrid);                   // Record this completed grid as a valid solution
            return;                                                 // Backtrack immediately to find more potential solutions
        }
        int row = pos / 9;                                          // Calculate the row index of the blank cell
        int col = pos % 9;                                          // Calculate the column index of the blank cell
        for (int candidateValue = 1; candidateValue <= 9; ++candidateValue) { // Try each candidate number from 1 to 9
            if (isSafe(row, col, candidateValue)) {                 // Check if the candidate number is legally allowed
                sudokuGrid[row][col] = candidateValue;              // Place the valid candidate number into the cell
                search(pos + 1);                                    // Recursively call search for the next grid position
                sudokuGrid[row][col] = 0;                           // Undo the placement (backtrack) to explore other paths
            }
        }
    }

    // ------------------------------------------------------------------------
    // MAJOR BLOCK: Grid Printer
    // Prints the 9x9 grid matrix directly to the standard console output.
    // Memory overhead is kept to O(m) by passing the Grid via constant reference.
    // ------------------------------------------------------------------------
    static void printGrid(const Grid& gridToPrint) {
        for (int r = 0; r < 9; ++r) {                               // Iterate through each row of the grid
            for (int c = 0; c < 9; ++c) {                           // Iterate through each column of the grid
                if (gridToPrint[r][c] == 0) std::cout << "_ ";      // Print an underscore for blank cells
                else std::cout << gridToPrint[r][c] << " ";         // Print the numeric value for filled cells
            }
            std::cout << "\n";                                      // Output a newline at the end of the row
        }
    }

public: // Public interface methods exposed to the main execution loop
    SudokuPuzzle() : sudokuGrid{}, originalGrid{}, isLoaded(false) {} // Default constructor initializing an empty state

    // ------------------------------------------------------------------------
    // MAJOR BLOCK: File Loader
    // Parses the text file, populates the grid, and cleanly rejects bad tokens.
    // ------------------------------------------------------------------------
    bool load(const std::string& targetFileName) {
        fileName = targetFileName;                                  // Store the file name in the class instance
        foundSolutions.clear();                                     // Erase any leftover solutions from prior runs
        std::ifstream fileStream(fileName);                         // Open an input file stream for the target file
        
        if (!fileStream.is_open()) {                                // Verify if the stream failed to open the file
            std::cerr << "Error: Could not open " << fileName << ".\n"; // Gracefully log the stream failure
            return false;                                           // Return false to halt processing for this file
        }
        
        for (int rowIdx = 0; rowIdx < 9; ++rowIdx) {                // Loop 9 times to read all grid rows
            for (int colIdx = 0; colIdx < 9; ++colIdx) {            // Loop 9 times to read all cells per row
                std::string token;                                  // Declare a string to hold the extracted file token
                if (!(fileStream >> token)) {                       // Extract token and verify the file did not end prematurely
                    std::cerr << "Error: Malformed file missing entries.\n"; // Log unexpected EOF error
                    return false;                                   // Return false due to invalid grid dimensions
                }
                
                if (token == "_") {                                 // Check if the token represents an empty square
                    sudokuGrid[rowIdx][colIdx] = 0;                 // Assign 0 to the internal grid matrix
                } else if (token.size() == 1 && token[0] >= '1' && token[0] <= '9') { // Verify token is a strict 1-9 digit
                    sudokuGrid[rowIdx][colIdx] = token[0] - '0';    // Convert the character digit to an integer
                } else {                                            // Handle unrecognized tokens safely
                    std::cerr << "Error: Invalid token '" << token << "' found.\n"; // Log the exact invalid token
                    return false;                                   // Return false due to corrupted file data
                }
            }
        }
        originalGrid = sudokuGrid;                                  // Save a backup copy of the unmodified puzzle
        isLoaded = true;                                            // Toggle the boolean flag indicating a successful load
        return true;                                                // Return true to proceed with solving
    }

    // ------------------------------------------------------------------------
    // MAJOR BLOCK: Solver Trigger
    // Validates clues and kicks off the recursive depth-first search.
    // ------------------------------------------------------------------------
    void solve() {
        foundSolutions.clear();                                     // Reset the solutions vector to guarantee a clean state
        if (!isLoaded) return;                                      // Exit immediately if the file was never loaded properly
        if (cluesAreConsistent()) search(0);                        // Trigger recursive search starting at index 0 if clues are valid
    }

    // ------------------------------------------------------------------------
    // MAJOR BLOCK: Output Report Generator
    // Prints the file name, initial board state, and discovered solutions.
    // ------------------------------------------------------------------------
    void report() const {
        std::cout << "File: " << fileName << "\n";                  // Output the name of the processed file
        if (!isLoaded) {                                            // Check if the file failed the load validation phase
            std::cout << "Could not read a valid puzzle from this file.\n\n"; // Print a formatted error message
            return;                                                 // Exit the report method early
        }
        std::cout << "Puzzle:\n";                                   // Output the puzzle header
        printGrid(originalGrid);                                    // Display the initial unsolved puzzle grid
        if (foundSolutions.empty()) {                               // Check if the algorithm failed to find any valid boards
            std::cout << "No solution found\n\n";                   // Print the required no solution fallback message
            return;                                                 // Exit the report method
        }
        for (std::size_t i = 0; i < foundSolutions.size(); ++i) {   // Iterate through every solution matrix found
            std::cout << "Solution " << (i + 1) << ":\n";           // Output the sequentially numbered solution header
            printGrid(foundSolutions[i]);                           // Display the solved grid matrix
        }
        std::cout << "Solutions found: " << foundSolutions.size() << "\n\n"; // Output the final count of discovered solutions
    }
};

// ============================================================================
// MAJOR BLOCK: Main Application Entry Point
// Iterates sequentially through the array of target puzzle files.
// ============================================================================
int main() {
    const std::string targetFiles[5] = {                            // Initialize an array with the five required file names
        "puzzle1.txt", "puzzle2.txt", "puzzle3.txt", "puzzle4.txt", "puzzle5.txt"
    };
    for (const std::string& currentFileName : targetFiles) {        // Iterate through each target file name in the array
        SudokuPuzzle activePuzzle;                                  // Instantiate a fresh SudokuPuzzle object
        activePuzzle.load(currentFileName);                         // Attempt to load and parse the puzzle file
        activePuzzle.solve();                                       // Execute the backtracking solver algorithm
        activePuzzle.report();                                      // Print the final puzzle states and solutions to console
    }
    return 0;                                                       // Return integer zero indicating successful program termination
}