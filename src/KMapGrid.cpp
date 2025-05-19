#include <algorithm>
#include "KMapGrid.hpp"
#include <stdexcept>
#include <cmath>
#include <sstream>

KMapGrid::KMapGrid(int variables) : variables(variables) {
    if (variables < 2 || variables > 4) {
        throw std::invalid_argument("Number of variables must be between 2 and 4");
    }

    // Calculate grid dimensions based on number of variables
    rows = (variables <= 2) ? 2 : 4;
    cols = (variables <= 2) ? 2 : 4;

    initializeGrid();
}

void KMapGrid::initializeGrid() {
    grid.resize(rows, std::vector<CellState>(cols, CellState::UNMARKED));
    loops.clear();
}

void KMapGrid::setCellState(int row, int col, CellState state) {
    if (row < 0 || row >= rows || col < 0 || col >= cols) {
        throw std::out_of_range("Cell coordinates out of range");
    }
    grid[row][col] = state;
}

KMapGrid::CellState KMapGrid::getCellState(int row, int col) const {
    if (row < 0 || row >= rows || col < 0 || col >= cols) {
        throw std::out_of_range("Cell coordinates out of range");
    }
    return grid[row][col];
}

void KMapGrid::clearGrid() {
    for (auto& row : grid) {
        for (auto& cell : row) {
            cell = CellState::UNMARKED;
        }
    }
}

void KMapGrid::clearLoops() {
    loops.clear();
    // Reset cells that were in loops
    for (auto& row : grid) {
        for (auto& cell : row) {
            if (cell == CellState::IN_LOOP) {
                cell = CellState::UNMARKED;
            }
        }
    }
}

void KMapGrid::setMinterm(int minterm, bool value) {
    if (minterm < 0 || minterm >= (1 << variables)) {
        throw std::out_of_range("Minterm out of range");
    }

    // Convert minterm to grid coordinates
    int row, col;
    if (variables <= 2) {
        row = minterm / 2;
        col = minterm % 2;
    } else {
        // For 3-4 variables, we need to use Gray code ordering
        int gray = binaryToGray(minterm);
        row = gray / 4;
        col = gray % 4;
    }

    setCellState(row, col, value ? CellState::MARKED : CellState::UNMARKED);
}

bool KMapGrid::getMinterm(int minterm) const {
    if (minterm < 0 || minterm >= (1 << variables)) {
        throw std::out_of_range("Minterm out of range");
    }

    // Convert minterm to grid coordinates
    int row, col;
    if (variables <= 2) {
        row = minterm / 2;
        col = minterm % 2;
    } else {
        int gray = binaryToGray(minterm);
        row = gray / 4;
        col = gray % 4;
    }

    return getCellState(row, col) == CellState::MARKED;
}

void KMapGrid::addLoop(const std::vector<std::pair<int, int>>& cells) {
    // Validate cells
    for (const auto& cell : cells) {
        if (cell.first < 0 || cell.first >= rows || cell.second < 0 || cell.second >= cols) {
            throw std::out_of_range("Cell coordinates out of range");
        }
    }

    // Add loop and mark cells
    loops.push_back(cells);
    for (const auto& cell : cells) {
        setCellState(cell.first, cell.second, CellState::IN_LOOP);
    }
}

void KMapGrid::removeLoop(const std::vector<std::pair<int, int>>& cells) {
    // Find and remove the loop
    auto it = std::find(loops.begin(), loops.end(), cells);
    if (it != loops.end()) {
        loops.erase(it);
        // Reset cells that were in this loop
        for (const auto& cell : cells) {
            setCellState(cell.first, cell.second, CellState::UNMARKED);
        }
    }
}

int KMapGrid::grayToBinary(int gray) const {
    int binary = gray;
    while (gray >>= 1) {
        binary ^= gray;
    }
    return binary;
}

int KMapGrid::binaryToGray(int binary) const {
    return binary ^ (binary >> 1);
}

const std::vector<std::vector<std::pair<int, int>>>& KMapGrid::getLoops() const {
    return loops;
}

std::string KMapGrid::getTermForLoop(const std::vector<std::pair<int, int>>& loop) const {
    std::stringstream term;
    bool firstVar = true;

    // For each variable
    for (int varIndex = 0; varIndex < variables; ++varIndex) {
        if (isVariableIncluded(varIndex, loop)) {
            if (!firstVar) {
                term << "·";  // Use · for AND operation
            }
            firstVar = false;

            // Add variable name
            term << getVariableName(varIndex);

            // Add complement if needed
            if (isVariableComplemented(varIndex, loop)) {
                term << "'";
            }
        }
    }

    return term.str();
}

std::string KMapGrid::getVariableName(int index) const {
    return std::string(1, 'A' + index);
}

bool KMapGrid::isVariableIncluded(int varIndex, const std::vector<std::pair<int, int>>& loop) const {
    // Check if the variable's value changes within the loop
    bool hasZero = false;
    bool hasOne = false;

    for (const auto& cell : loop) {
        int row = cell.first;
        int col = cell.second;
        int value = (row << (variables / 2)) | col;  // Combine row and col bits
        
        bool bit = (value >> varIndex) & 1;
        if (bit) hasOne = true;
        else hasZero = true;

        if (hasZero && hasOne) return false;  // Variable changes, so it's not included
    }

    return true;  // Variable doesn't change, so it's included
}

bool KMapGrid::isVariableComplemented(int varIndex, const std::vector<std::pair<int, int>>& loop) const {
    // Check the value of the variable in the first cell of the loop
    if (loop.empty()) return false;
    
    int row = loop[0].first;
    int col = loop[0].second;
    int value = (row << (variables / 2)) | col;  // Combine row and col bits
    
    return !((value >> varIndex) & 1);  // Return true if the bit is 0
} 