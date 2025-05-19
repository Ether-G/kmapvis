#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <string>
#include <utility>

class KMapGrid {
public:
    enum class CellState {
        UNMARKED,
        MARKED,
        IN_LOOP,
        ESSENTIAL
    };

    KMapGrid(int variables);
    ~KMapGrid() = default;

    // Grid operations
    void setCellState(int row, int col, CellState state);
    CellState getCellState(int row, int col) const;
    void clearGrid();
    void clearLoops();

    // K-map specific operations
    void setMinterm(int minterm, bool value);
    bool getMinterm(int minterm) const;
    void addLoop(const std::vector<std::pair<int, int>>& cells);
    void removeLoop(const std::vector<std::pair<int, int>>& cells);

    // Getters
    int getVariables() const { return variables; }
    int getRows() const { return rows; }
    int getCols() const { return cols; }
    const std::vector<std::vector<CellState>>& getGrid() const { return grid; }
    const std::vector<std::vector<std::pair<int, int>>>& getLoops() const;
    std::string getTermForLoop(const std::vector<std::pair<int, int>>& loop) const;

private:
    int variables;
    int rows;
    int cols;
    std::vector<std::vector<CellState>> grid;
    std::vector<std::vector<std::pair<int, int>>> loops;

    void initializeGrid();
    int grayToBinary(int gray) const;
    int binaryToGray(int binary) const;

    // Helper functions for term generation
    std::string getVariableName(int index) const;
    bool isVariableIncluded(int varIndex, const std::vector<std::pair<int, int>>& loop) const;
    bool isVariableComplemented(int varIndex, const std::vector<std::pair<int, int>>& loop) const;
}; 