#include "kmap_solver.hpp"
#include <iostream>

using std::cout;
using std::cerr;
using std::endl;

void printUsage(const char* programName) {
    cout << "Usage: " << programName << " <boolean_equation>" << endl;
    cout << "Example: " << programName << " \"AB + BC\"" << endl;
    cout << "Note: Use quotes around the equation if it contains spaces" << endl;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printUsage(argv[0]);
        return 1;
    }

    string equation = argv[1];
    
    try {
        KMapSolver solver(equation);
        
        // Generate and display the K-map
        vector<vector<bool>> kmap = solver.solve();
        cout << "K-map for equation: " << equation << endl;
        displayKMap(kmap, solver.getVariables());
        
        // Display the minimized expression
        string minimized = solver.getMinimizedExpression();
        displayMinimizedExpression(minimized);
        
    } catch (const std::exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
    
    return 0;
} 