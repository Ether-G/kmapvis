#pragma once

// glad must be included before GLFW
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <memory>
#include <string>
#include "KMapGrid.hpp"
#include "Shader.hpp"

class KMapVisualizer {
public:
    KMapVisualizer(int width = 800, int height = 600);
    ~KMapVisualizer();

    bool initialize();
    void run();

private:
    // Static instance for callbacks
    static KMapVisualizer* instance;

    // Window properties
    int windowWidth;
    int windowHeight;
    GLFWwindow* window;

    // OpenGL objects
    std::unique_ptr<Shader> shader;
    unsigned int VAO, VBO, EBO;

    // K-map state
    std::unique_ptr<KMapGrid> kmap;
    bool isDrawingLoop;
    std::vector<std::pair<int, int>> currentLoop;

    // Input state
    std::string booleanEquation;
    bool showEquationInput;
    bool showMintermInput;
    std::vector<int> minterms;

    // Camera/view properties
    glm::mat4 projection;
    glm::mat4 view;
    float zoom;
    glm::vec2 pan;

    // Callback functions
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);
    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

    // Helper functions
    void processInput();
    void update();
    void render();
    void renderGrid();
    void renderLoops();
    void renderUI();
    void initializeGrid();
    
    // Input handling
    void handleCellClick(int row, int col);
    void parseBooleanEquation();
    void updateMinterms();
    void toggleCell(int row, int col);
    
    // Coordinate conversion
    glm::vec2 screenToGrid(glm::vec2 screenPos) const;
    glm::vec2 gridToScreen(glm::vec2 gridPos) const;
}; 