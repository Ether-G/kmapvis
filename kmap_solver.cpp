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

KMapSolver::KMapSolver(const string& equation, int expectedVariableCount) : equation(equation) {
    parseEquation(expectedVariableCount);
}

KMapSolver::KMapSolver(const string& equation, const vector<char>& expectedVariables) : equation(equation) {
    parseEquation(expectedVariables);
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

void KMapSolver::parseEquation(int expectedVariableCount) {
    // Extract unique variables from the equation first
    set<char> foundVars;
    for (char c : equation) {
        if (isalpha(c)) {
            foundVars.insert(c);
        }
    }
    
    // Build the variable list with expected count, starting from 'A'
    variables.clear();
    for (int i = 0; i < expectedVariableCount; i++) {
        char var = 'A' + i;
        variables.push_back(var);
    }
    
    // Verify all found variables are within the expected range
    for (char var : foundVars) {
        if (var < 'A' || var >= 'A' + expectedVariableCount) {
            throw std::runtime_error("Variable " + string(1, var) + " is outside expected range A-" + 
                                   string(1, 'A' + expectedVariableCount - 1));
        }
    }
}

void KMapSolver::parseEquation(const vector<char>& expectedVariables) {
    // Extract unique variables from the equation first
    set<char> foundVars;
    for (char c : equation) {
        if (isalpha(c)) {
            foundVars.insert(c);
        }
    }
    
    // Use the provided variable list
    variables = expectedVariables;
    std::sort(variables.begin(), variables.end());
    
    // Verify all found variables are in the expected list
    for (char var : foundVars) {
        if (std::find(variables.begin(), variables.end(), var) == variables.end()) {
            throw std::runtime_error("Variable " + string(1, var) + " is not in the expected variable list");
        }
    }
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
                // Check if this variable exists in our variable map
                if (variableValues.find(term[i]) == variableValues.end()) {
                    throw std::runtime_error("Variable " + string(1, term[i]) + " not found in variable mapping");
                }
                
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
            if (varCount == 2) {
                // For 2 variables: first variable is bit 1 of row, second variable is bit 0 of row
                // Gray code mapping: 00->00, 01->01, 10->11, 11->10
                variableValues[variables[0]] = (gray_i & 1) != 0;  // LSB of row Gray code
                variableValues[variables[1]] = (gray_j & 1) != 0;  // LSB of column Gray code
            } else if (varCount == 3) {
                // For 3 variables: first two in rows, third in columns
                variableValues[variables[0]] = (gray_i & 2) != 0;
                variableValues[variables[1]] = (gray_i & 1) != 0;
                variableValues[variables[2]] = (gray_j & 1) != 0;
            } else if (varCount == 4) {
                // For 4 variables: first two in rows, last two in columns
                variableValues[variables[0]] = (gray_i & 2) != 0;
                variableValues[variables[1]] = (gray_i & 1) != 0;
                variableValues[variables[2]] = (gray_j & 2) != 0;
                variableValues[variables[3]] = (gray_j & 1) != 0;
            }
            
            kmap[i][j] = evaluateExpression(equation);
        }
    }
    
    return kmap;
}

// Helper function to get variable term for a group
static string getGroupTerm(const vector<char>& variables, int rowMask, int colMask, int rowVal, int colVal, int varCount) {
    string term;
    
    // This function is called with cells already identified as belonging to the same group
    // We need to look at the actual coordinates to determine which variables are constant
    
    if (varCount == 2) {
        // 2 variables: first variable in rows, second variable in columns
        // rowMask == 0 means first variable is constant (include it)
        // colMask == 0 means second variable is constant (include it)
        if (rowMask == 0) { // First variable is constant
            term += variables[0];
            if ((rowVal & 1) == 0) term += "'"; // Complement if value is 0
        }
        if (colMask == 0) { // Second variable is constant  
            term += variables[1];
            if ((colVal & 1) == 0) term += "'"; // Complement if value is 0
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
        // This is a placeholder - the real logic will be done by analyzing cells
        // For now, return empty string to signal we need cell-based analysis
        return "";
    }
    return term;
}

// Helper function specifically for 4-variable term generation using cell coordinates
static string getGroupTermFromCells(const vector<char>& variables, const vector<std::pair<int,int>>& cells) {
    if (variables.size() != 4) return "";
    
    // Get the Gray code values for all cells in the group
    set<int> rowGrays, colGrays;
    for (const auto& cell : cells) {
        int r = cell.first, c = cell.second;
        int gray_r = r ^ (r >> 1);
        int gray_c = c ^ (c >> 1);
        rowGrays.insert(gray_r);
        colGrays.insert(gray_c);
    }
    
    string term;
    
    // Analyze row variables (A, B)
    if (rowGrays.size() == 1) {
        // Both A and B are constant
        int grayVal = *rowGrays.begin();
        // A = bit 1, B = bit 0 in Gray code
        term += variables[0]; // A
        if ((grayVal & 2) == 0) term += "'"; // A is 0
        term += variables[1]; // B  
        if ((grayVal & 1) == 0) term += "'"; // B is 0
    } else if (rowGrays.size() == 2) {
        // One of A or B is constant
        vector<int> grays(rowGrays.begin(), rowGrays.end());
        int diff = grays[0] ^ grays[1];
        if (diff == 1) {
            // Only B varies (bit 0), A is constant (bit 1)
            int aVal = grays[0] & 2; // A's value from bit 1
            term += variables[0]; // A
            if (aVal == 0) term += "'";
        } else if (diff == 2) {
            // Only A varies (bit 1), B is constant (bit 0)
            int bVal = grays[0] & 1; // B's value from bit 0
            term += variables[1]; // B
            if (bVal == 0) term += "'";
        }
        // If diff == 3, both A and B vary, so neither is included
    }
    // If rowGrays.size() > 2, both A and B vary, so neither is included
    
    // Analyze column variables (C, D)
    if (colGrays.size() == 1) {
        // Both C and D are constant
        int grayVal = *colGrays.begin();
        // C = bit 1, D = bit 0 in Gray code
        term += variables[2]; // C
        if ((grayVal & 2) == 0) term += "'"; // C is 0
        term += variables[3]; // D
        if ((grayVal & 1) == 0) term += "'"; // D is 0
    } else if (colGrays.size() == 2) {
        // One of C or D is constant
        vector<int> grays(colGrays.begin(), colGrays.end());
        int diff = grays[0] ^ grays[1];
        if (diff == 1) {
            // Only D varies (bit 0), C is constant (bit 1)
            int cVal = grays[0] & 2; // C's value from bit 1
            term += variables[2]; // C
            if (cVal == 0) term += "'";
        } else if (diff == 2) {
            // Only C varies (bit 1), D is constant (bit 0)
            int dVal = grays[0] & 1; // D's value from bit 0
            term += variables[3]; // D
            if (dVal == 0) term += "'";
        }
        // If diff == 3, both C and D vary, so neither is included
    }
    // If colGrays.size() > 2, both C and D vary, so neither is included
    
    // If no variables are included, this means all variables vary and the term is "1"
    if (term.empty()) {
        return "1";
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
        // If any group is "1" (always true), the entire expression is "1"
        if (groups[i].term == "1") {
            return "1";
        }
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
        if (variables.size() == 2) {
            // For 2 variables, show single bit for row
            cout << setw(2) << (gray_i & 1);
        } else {
            // For 3+ variables, show 2 bits for row
            string row_label = (gray_i & 2 ? "1" : "0") + string(gray_i & 1 ? "1" : "0");
            cout << setw(2) << row_label;
        }
        cout << " |";
        
        for (int j = 0; j < cols; j++) {
            cout << setw(4) << (kmap[i][j] ? "1" : "0");
        }
        cout << endl;
    }
    
    // Print variable mapping
    cout << "\nVariable Mapping:" << endl;
    if (variables.size() == 2) {
        cout << "Rows: " << variables[0] << " (in Gray code order)" << endl;
        cout << "Columns: " << variables[1] << " (in Gray code order)" << endl;
    } else if (variables.size() >= 3) {
        cout << "Rows: " << variables[0] << variables[1] << " (in Gray code order)" << endl;
        if (variables.size() == 3) {
            cout << "Columns: " << variables[2] << endl;
        } else if (variables.size() >= 4) {
            cout << "Columns: " << variables[2] << variables[3] << " (in Gray code order)" << endl;
        }
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
                            // For 2 variables: first var in rows, second var in columns
                            // h=1 means row is constant, h=2 means row varies (first var eliminated)
                            // w=1 means col is constant, w=2 means col varies (second var eliminated)
                            rowMask = (h == 2) ? 1 : 0; // If h==2, first variable is eliminated (mask = 1)
                            colMask = (w == 2) ? 1 : 0; // If w==2, second variable is eliminated (mask = 1)
                            rowVal = i; // Row index gives first variable value
                            colVal = j; // Column index gives second variable value
                        } else if (varCount == 3) {
                            rowMask = (h == 4) ? 3 : (h == 2 ? (1 << (1 - ((i/2)%2))) : (1 << (1 - (i%2))));
                            rowVal = i%4;
                            colMask = (w == 2) ? 1 : 0;
                            colVal = j%2;
                        } else if (varCount == 4) {
                            // For 4 variables: first two in rows, last two in columns
                            rowMask = 0; // Will be analyzed in getGroupTerm
                            colMask = 0; // Will be analyzed in getGroupTerm  
                            rowVal = i;
                            colVal = j;
                        }
                        KMapGroup group;
                        group.cells = cells;
                        if (varCount == 4) {
                            group.term = getGroupTermFromCells(variables, cells);
                        } else {
                            group.term = getGroupTerm(variables, rowMask, colMask, rowVal, colVal, varCount);
                        }
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