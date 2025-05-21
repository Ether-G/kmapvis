#ifndef KMAP_SOLVER_HPP
#define KMAP_SOLVER_HPP

#include <string>
#include <vector>
#include <map>
#include <set>

using std::string;
using std::vector;
using std::map;
using std::set;

struct KMapGroup {
    std::vector<std::pair<int, int>> cells; // coordinates in the K-map
    std::string term; // Boolean term for this group
};

class KMapSolver {
public:
    KMapSolver(const string& equation);
    
    // Main solving function
    vector<vector<bool>> solve() const;
    
    // Get the minimized boolean expression
    string getMinimizedExpression() const;
    
    // Get the number of variables in the equation
    int getVariableCount() const;
    
    // Get the list of variables used in the equation
    vector<char> getVariables() const;

    std::vector<KMapGroup> getMinimalCoverGroups() const; // For GUI highlighting

private:
    string equation;
    vector<char> variables;
    map<char, bool> variableValues;
    vector<vector<bool>> kmap;
    
    // Helper functions
    void parseEquation();
    bool evaluateExpression(const string& expr);
    vector<vector<bool>> generateKMap();
    string minimizeExpression() const;
    set<string> findPrimeImplicants() const;
    set<string> findEssentialPrimeImplicants(const set<string>& primeImplicants) const;
    vector<string> findGroups(const vector<vector<bool>>& kmap) const;
    string combineTerms(const vector<string>& groups) const;
};

// Terminal display functions
void displayKMap(const vector<vector<bool>>& kmap, const vector<char>& variables);
void displayMinimizedExpression(const string& expression);

#endif // KMAP_SOLVER_HPP 