#include "kmap_solver.hpp"
#include <iostream>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <stack>
#include <set>
#include <map>
#include <utility>
#include <bitset>

using std::cout;
using std::cerr;
using std::endl;
using std::stringstream;
using std::setw;
using std::stack;
using std::set;
using std::map;

KMapSolver::KMapSolver(const string& equation) : equation(equation) {
    parseEquation();
}

void KMapSolver::parseEquation() {
    // Extract unique variables from the equation
    variables.clear();
    for (char c : equation) {
        if (isalpha(c) && std::find(variables.begin(), variables.end(), c) == variables.end()) {
            variables.push_back(c);
        }
    }
    std::sort(variables.begin(), variables.end());
}

vector<vector<bool>> KMapSolver::solve() const {
    return const_cast<KMapSolver*>(this)->generateKMap();
}

string KMapSolver::getMinimizedExpression() const {
    // Use real minimization logic for up to 4 variables
    return minimizeExpression();
}

vector<string> KMapSolver::findGroups(const vector<vector<bool>>& kmap) const {
    vector<string> groups;
    int rows = kmap.size();
    int cols = kmap[0].size();

    // Example logic to find groups (simplified)
    // In a real implementation, you would use a more sophisticated algorithm
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (kmap[i][j]) {
                // Check for a 2x2 group
                if (i + 1 < rows && j + 1 < cols && kmap[i + 1][j] && kmap[i][j + 1] && kmap[i + 1][j + 1]) {
                    groups.push_back("Group at (" + std::to_string(i) + "," + std::to_string(j) + ")");
                }
            }
        }
    }
    return groups;
}

string KMapSolver::combineTerms(const vector<string>& groups) const {
    // Example logic to combine terms (simplified)
    // In a real implementation, you would generate actual terms
    string result;
    for (const auto& group : groups) {
        result += group + " + ";
    }
    if (!result.empty()) {
        result = result.substr(0, result.length() - 3); // Remove trailing " + "
    }
    return result;
}

int KMapSolver::getVariableCount() const {
    return variables.size();
}

vector<char> KMapSolver::getVariables() const {
    return variables;
}

bool KMapSolver::evaluateExpression(const string& expr) {
    // Split the expression into terms (separated by +)
    stringstream ss(expr);
    string term;
    bool result = false;
    
    while (std::getline(ss, term, '+')) {
        // Remove spaces
        term.erase(std::remove(term.begin(), term.end(), ' '), term.end());
        
        // Evaluate each term
        bool termResult = true;
        for (size_t i = 0; i < term.length(); i++) {
            if (isalpha(term[i])) {
                bool value = variableValues[term[i]];
                // Check for NOT operator
                if (i + 1 < term.length() && term[i + 1] == '\'') {
                    value = !value;
                    i++; // Skip the ' character
                }
                termResult &= value;
            }
        }
        result |= termResult;
    }
    
    return result;
}

vector<vector<bool>> KMapSolver::generateKMap() {
    int varCount = variables.size();
    
    // Determine dimensions based on number of variables
    int rows, cols;
    if (varCount == 2) {
        rows = 2;
        cols = 2;
    } else if (varCount == 3) {
        rows = 4;
        cols = 2;
    } else if (varCount == 4) {
        rows = 4;
        cols = 4;
    } else {
        throw std::runtime_error("Only 2, 3, or 4 variables are supported");
    }
    
    vector<vector<bool>> kmap(rows, vector<bool>(cols, false));
    
    // Generate all possible combinations using Gray code
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            // Convert to Gray code
            int gray_i = i ^ (i >> 1);  // Convert row index to Gray code
            int gray_j = j ^ (j >> 1);  // Convert column index to Gray code
            
            // Set variable values based on Gray code
            if (varCount > 0) {
                // For 2 variables: both in rows
                // For 3 variables: first two in rows, third in columns
                // For 4 variables: first two in rows, last two in columns
                variableValues[variables[0]] = (gray_i & 2) != 0;
                if (varCount > 1) variableValues[variables[1]] = (gray_i & 1) != 0;
                if (varCount > 2) variableValues[variables[2]] = (gray_j & 1) != 0;
                if (varCount > 3) variableValues[variables[3]] = (gray_j & 2) != 0;
            }
            
            kmap[i][j] = evaluateExpression(equation);
        }
    }
    
    return kmap;
}

// Helper function to get variable term for a group
static string getGroupTerm(const vector<char>& variables, int rowMask, int colMask, int rowVal, int colVal, int varCount) {
    string term;
    
    // Only include variables that don't change within the group (masked bits)
    // For each variable position, check if it's masked (constant across the group)
    // If not masked, the variable changes within the group and is eliminated
    
    if (varCount == 2) {
        // 2 variables: both in rows
        // For each variable, check if it's masked
        if (!(rowMask & (1 << 1))) { // First variable (constant)
            term += variables[0];
            if (!((rowVal >> 1) & 1)) term += "'"; // Complement if 0
        }
        if (!(rowMask & (1 << 0))) { // Second variable (constant)
            term += variables[1];
            if (!((rowVal >> 0) & 1)) term += "'"; // Complement if 0
        }
    } else if (varCount == 3) {
        // 3 variables: first two in rows, third in column
        if (!(rowMask & (1 << 1))) { // First variable (constant)
            term += variables[0];
            if (!((rowVal >> 1) & 1)) term += "'";
        }
        if (!(rowMask & (1 << 0))) { // Second variable (constant)
            term += variables[1];
            if (!((rowVal >> 0) & 1)) term += "'";
        }
        if (!(colMask & (1 << 0))) { // Third variable (constant)
            term += variables[2];
            if (!((colVal >> 0) & 1)) term += "'";
        }
    } else if (varCount == 4) {
        // 4 variables: first two in rows, last two in columns
        if (!(rowMask & (1 << 1))) { // First variable (constant)
            term += variables[0];
            if (!((rowVal >> 1) & 1)) term += "'";
        }
        if (!(rowMask & (1 << 0))) { // Second variable (constant)
            term += variables[1];
            if (!((rowVal >> 0) & 1)) term += "'";
        }
        if (!(colMask & (1 << 1))) { // Third variable (constant)
            term += variables[2];
            if (!((colVal >> 1) & 1)) term += "'";
        }
        if (!(colMask & (1 << 0))) { // Fourth variable (constant)
            term += variables[3];
            if (!((colVal >> 0) & 1)) term += "'";
        }
    }
    return term;
}

// Real K-map minimization for up to 4 variables
string KMapSolver::minimizeExpression() const {
    // Use getMinimalCoverGroups for consistency
    std::vector<KMapGroup> groups = getMinimalCoverGroups();
    
    if (groups.empty()) return "1"; // Always true or unsupported variable count
    
    // Combine terms
    stringstream result;
    for (size_t i = 0; i < groups.size(); ++i) {
        if (i > 0) result << " + ";
        result << groups[i].term;
    }
    
    return result.str();
}

// Terminal display functions
void displayKMap(const vector<vector<bool>>& kmap, const vector<char>& variables) {
    int rows = kmap.size();
    int cols = kmap[0].size();
    
    // Print column headers with Gray code values
    cout << "    ";
    for (int j = 0; j < cols; j++) {
        // Convert to Gray code
        int gray_j = j ^ (j >> 1);
        cout << setw(4) << gray_j;
    }
    cout << endl;
    
    // Print rows with Gray code values
    for (int i = 0; i < rows; i++) {
        // Convert to Gray code
        int gray_i = i ^ (i >> 1);
        // Convert to binary string
        string row_label = (gray_i & 2 ? "1" : "0") + string(gray_i & 1 ? "1" : "0");
        cout << setw(2) << row_label << " |";
        
        for (int j = 0; j < cols; j++) {
            cout << setw(4) << (kmap[i][j] ? "1" : "0");
        }
        cout << endl;
    }
    
    // Print variable mapping
    cout << "\nVariable Mapping:" << endl;
    if (variables.size() >= 2) {
        cout << "Rows: " << variables[0] << variables[1] << " (in Gray code order)" << endl;
    }
    if (variables.size() >= 3) {
        cout << "Columns: " << variables[2] << endl;
    }
    if (variables.size() >= 4) {
        cout << "Columns: " << variables[2] << variables[3] << " (in Gray code order)" << endl;
    }
}

void displayMinimizedExpression(const string& expression) {
    cout << "Minimized Expression: " << expression << endl;
}

// Helper: generate all possible group sizes for a given K-map
static std::vector<std::pair<int, int>> getGroupSizes(int rows, int cols) {
    std::vector<std::pair<int, int>> sizes;
    for (int h = rows; h >= 1; h /= 2) {
        for (int w = cols; w >= 1; w /= 2) {
            sizes.emplace_back(h, w);
        }
    }
    return sizes;
}

// Helper: check if a group covers only 1s in the K-map
static bool isAllOnes(const std::vector<std::vector<bool>>& kmap, int i, int j, int h, int w) {
    int rows = kmap.size(), cols = kmap[0].size();
    for (int di = 0; di < h; ++di)
        for (int dj = 0; dj < w; ++dj)
            if (!kmap[(i+di)%rows][(j+dj)%cols])
                return false;
    return true;
}

// Helper: get all cell coordinates for a group
static std::vector<std::pair<int, int>> getGroupCells(int i, int j, int h, int w, int rows, int cols) {
    std::vector<std::pair<int, int>> cells;
    for (int di = 0; di < h; ++di)
        for (int dj = 0; dj < w; ++dj)
            cells.emplace_back((i+di)%rows, (j+dj)%cols);
    return cells;
}

// Helper: check if a group is already in the list (by its cells)
static bool groupExists(const std::vector<KMapGroup>& groups, const std::vector<std::pair<int, int>>& cells) {
    for (const auto& g : groups) {
        if (g.cells == cells) return true;
    }
    return false;
}

std::vector<KMapGroup> KMapSolver::getMinimalCoverGroups() const {
    int varCount = variables.size();
    if (varCount < 2 || varCount > 4) return {};
    std::vector<std::vector<bool>> kmap = const_cast<KMapSolver*>(this)->generateKMap();
    int rows = kmap.size(), cols = kmap[0].size();
    // 1. Find all prime implicants (all possible groups of 1s)
    std::vector<KMapGroup> primes;
    auto groupSizes = getGroupSizes(rows, cols);
    for (auto sz : groupSizes) {
        int h = sz.first, w = sz.second;
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                if (isAllOnes(kmap, i, j, h, w)) {
                    auto cells = getGroupCells(i, j, h, w, rows, cols);
                    std::sort(cells.begin(), cells.end());
                    if (!groupExists(primes, cells)) {
                        // Determine which variables are constant in this group
                        int rowMask = 0, colMask = 0, rowVal = 0, colVal = 0;
                        if (varCount == 2) {
                            rowMask = (h == 2) ? 3 : (1 << (1 - (i%2)));
                            rowVal = i%2;
                            colMask = (w == 2) ? 3 : (1 << (1 - (j%2)));
                            colVal = j%2;
                        } else if (varCount == 3) {
                            rowMask = (h == 4) ? 3 : (h == 2 ? (1 << (1 - ((i/2)%2))) : (1 << (1 - (i%2))));
                            rowVal = i%4;
                            colMask = (w == 2) ? 1 : 0;
                            colVal = j%2;
                        } else if (varCount == 4) {
                            rowMask = (h == 4) ? 3 : (h == 2 ? (1 << (1 - ((i/2)%2))) : (1 << (1 - (i%2))));
                            rowVal = i%4;
                            colMask = (w == 4) ? 3 : (w == 2 ? (1 << (1 - ((j/2)%2))) : (1 << (1 - (j%2))));
                            colVal = j%4;
                        }
                        KMapGroup group;
                        group.cells = cells;
                        group.term = getGroupTerm(variables, rowMask, colMask, rowVal, colVal, varCount);
                        primes.push_back(group);
                    }
                }
            }
        }
    }
    // 2. Build a table: which group covers which minterms
    std::vector<std::pair<int, int>> minterms;
    for (int i = 0; i < rows; ++i)
        for (int j = 0; j < cols; ++j)
            if (kmap[i][j]) minterms.emplace_back(i, j);
    // 3. Find essential prime implicants
    std::vector<KMapGroup> cover;
    std::set<std::pair<int, int>> covered;
    for (const auto& m : minterms) {
        int count = 0, idx = -1;
        for (size_t g = 0; g < primes.size(); ++g) {
            if (std::find(primes[g].cells.begin(), primes[g].cells.end(), m) != primes[g].cells.end()) {
                count++;
                idx = g;
            }
        }
        if (count == 1 && covered.find(m) == covered.end()) {
            // Only one group covers this minterm: essential
            for (auto& cell : primes[idx].cells) covered.insert(cell);
            cover.push_back(primes[idx]);
        }
    }
    // 4. Cover remaining minterms with as few groups as possible (greedy for small K-maps)
    while (covered.size() < minterms.size()) {
        int best = -1, bestCount = 0;
        for (size_t g = 0; g < primes.size(); ++g) {
            int cnt = 0;
            for (auto& cell : primes[g].cells)
                if (kmap[cell.first][cell.second] && covered.find(cell) == covered.end())
                    cnt++;
            if (cnt > bestCount) {
                best = g;
                bestCount = cnt;
            }
        }
        if (best == -1) break; // Should not happen
        for (auto& cell : primes[best].cells) covered.insert(cell);
        cover.push_back(primes[best]);
    }
    // Remove duplicate groups
    std::sort(cover.begin(), cover.end(), [](const KMapGroup& a, const KMapGroup& b){ return a.term < b.term; });
    cover.erase(std::unique(cover.begin(), cover.end(), [](const KMapGroup& a, const KMapGroup& b){ return a.term == b.term; }), cover.end());
    return cover;
} 