#include "KMapVisualizer.hpp"
#include <iostream>

int main() {
    try {
        KMapVisualizer visualizer(1024, 768);
        
        if (!visualizer.initialize()) {
            std::cerr << "Failed to initialize K-map visualizer" << std::endl;
            return -1;
        }

        visualizer.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }

    return 0;
} 