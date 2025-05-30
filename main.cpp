#include "kmap_solver.hpp"
#include <iostream>

using std::cout;
using std::cerr;
using std::endl;

void printUsage(const char* programName) {
    cout << "Usage: " << programName << " <boolean_equation> [num_variables]" << endl;
    cout << "Example: " << programName << " \"AB + BC\"" << endl;
    cout << "Example: " << programName << " \"BD + B'D'\" 4   # Force 4 variables (A,B,C,D)" << endl;
    cout << "Note: Use quotes around the equation if it contains spaces" << endl;
    cout << "      If num_variables is specified, variables A,B,C,D,... will be used" << endl;
}

int main(int argc, char* argv[]) {
    if (argc < 2 || argc > 3) {
        printUsage(argv[0]);
        return 1;
    }

    string equation = argv[1];
    
    try {
        KMapSolver* solver = nullptr;
        
        if (argc == 3) {
            // Number of variables specified
            int numVars = std::stoi(argv[2]);
            if (numVars < 2 || numVars > 4) {
                cerr << "Error: Number of variables must be between 2 and 4" << endl;
                return 1;
            }
            solver = new KMapSolver(equation, numVars);
        } else {
            // Auto-detect variables from equation
            solver = new KMapSolver(equation);
        }
        
        // Generate and display the K-map
        vector<vector<bool>> kmap = solver->solve();
        cout << "K-map for equation: " << equation << endl;
        if (argc == 3) {
            cout << "Using " << argv[2] << " variables (A,B,C,D...)" << endl;
        }
        displayKMap(kmap, solver->getVariables());
        
        // Display the minimized expression
        string minimized = solver->getMinimizedExpression();
        displayMinimizedExpression(minimized);
        
        delete solver;
        
    } catch (const std::exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
    
    return 0;
} 