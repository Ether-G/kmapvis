#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/common.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "KMapVisualizer.hpp"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <set>
#include <cctype>

// Static member initialization
KMapVisualizer* KMapVisualizer::instance = nullptr;

KMapVisualizer::KMapVisualizer(int width, int height)
    : windowWidth(width), windowHeight(height), window(nullptr),
      VAO(0), VBO(0), EBO(0), isDrawingLoop(false),
      zoom(1.0f), pan(0.0f, 0.0f),
      showEquationInput(false), showMintermInput(false) {
    instance = this;
}

KMapVisualizer::~KMapVisualizer() {
    if (window) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        glfwDestroyWindow(window);
        glfwTerminate();
    }
}

bool KMapVisualizer::initialize() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }

    // Configure GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create window
    window = glfwCreateWindow(windowWidth, windowHeight, "K-Map Visualizer", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetKeyCallback(window, keyCallback);

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return false;
    }

    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Initialize K-map
    kmap = std::make_unique<KMapGrid>(2); // Start with 2 variables

    return true;
}

void KMapVisualizer::run() {
    while (!glfwWindowShouldClose(window)) {
        processInput();
        update();
        render();
    }
}

void KMapVisualizer::processInput() {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
}

void KMapVisualizer::update() {
    // Update projection and view matrices
    projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
    view = glm::translate(glm::mat4(1.0f), glm::vec3(pan, 0.0f)) * 
           glm::scale(glm::mat4(1.0f), glm::vec3(zoom));
}

void KMapVisualizer::render() {
    glClearColor(0.1f, 0.1f, 0.1f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Start ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Render K-map grid using ImGui
    renderGrid();
    renderUI();

    // Render ImGui
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
    glfwPollEvents();
}

void KMapVisualizer::renderGrid() {
    ImGui::Begin("K-Map", nullptr, ImGuiWindowFlags_NoCollapse);
    
    // Get grid dimensions
    int rows = kmap->getRows();
    int cols = kmap->getCols();
    
    // Calculate cell size
    float cellSize = 50.0f;  // Fixed cell size in pixels
    float labelSize = 30.0f; // Size for axis labels
    ImVec2 windowPos = ImGui::GetWindowPos();
    ImVec2 windowSize = ImGui::GetWindowSize();
    
    // Center the grid
    float gridWidth = cols * cellSize;
    float gridHeight = rows * cellSize;
    float startX = (windowSize.x - gridWidth) * 0.5f + labelSize;
    float startY = (windowSize.y - gridHeight) * 0.5f + labelSize;
    
    // Draw grid lines
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    
    // Draw horizontal lines
    for (int i = 0; i <= rows; ++i) {
        float y = startY + i * cellSize;
        drawList->AddLine(
            ImVec2(windowPos.x + startX, windowPos.y + y),
            ImVec2(windowPos.x + startX + gridWidth, windowPos.y + y),
            IM_COL32(200, 200, 200, 255)
        );
    }
    
    // Draw vertical lines
    for (int i = 0; i <= cols; ++i) {
        float x = startX + i * cellSize;
        drawList->AddLine(
            ImVec2(windowPos.x + x, windowPos.y + startY),
            ImVec2(windowPos.x + x, windowPos.y + startY + gridHeight),
            IM_COL32(200, 200, 200, 255)
        );
    }
    
    // Draw cells
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            float x = startX + col * cellSize;
            float y = startY + row * cellSize;
            
            // Get cell state
            auto state = kmap->getCellState(row, col);
            
            // Draw cell background based on state
            ImU32 cellColor;
            switch (state) {
                case KMapGrid::CellState::MARKED:
                    cellColor = IM_COL32(0, 0, 255, 100);  // Blue for marked
                    break;
                case KMapGrid::CellState::IN_LOOP:
                    cellColor = IM_COL32(0, 255, 0, 100);  // Green for in loop
                    break;
                case KMapGrid::CellState::ESSENTIAL:
                    cellColor = IM_COL32(255, 0, 0, 100);  // Red for essential
                    break;
                default:
                    cellColor = IM_COL32(0, 0, 0, 0);  // Transparent for unmarked
            }
            
            drawList->AddRectFilled(
                ImVec2(windowPos.x + x, windowPos.y + y),
                ImVec2(windowPos.x + x + cellSize, windowPos.y + y + cellSize),
                cellColor
            );
            
            // Draw cell border
            drawList->AddRect(
                ImVec2(windowPos.x + x, windowPos.y + y),
                ImVec2(windowPos.x + x + cellSize, windowPos.y + y + cellSize),
                IM_COL32(200, 200, 200, 255)
            );
        }
    }

    // Draw axis labels
    const char* rowLabels[] = {"0", "1", "00", "01", "11", "10"};
    const char* colLabels[] = {"0", "1", "00", "01", "11", "10"};
    
    // Draw row labels (left side)
    for (int i = 0; i < rows; ++i) {
        float y = startY + i * cellSize + cellSize/2;
        const char* label = rowLabels[i];
        ImVec2 textSize = ImGui::CalcTextSize(label);
        drawList->AddText(
            ImVec2(windowPos.x + startX - textSize.x - 10, windowPos.y + y - textSize.y/2),
            IM_COL32(255, 255, 255, 255),
            label
        );
    }
    
    // Draw column labels (top)
    for (int i = 0; i < cols; ++i) {
        float x = startX + i * cellSize + cellSize/2;
        const char* label = colLabels[i];
        ImVec2 textSize = ImGui::CalcTextSize(label);
        drawList->AddText(
            ImVec2(windowPos.x + x - textSize.x/2, windowPos.y + startY - textSize.y - 10),
            IM_COL32(255, 255, 255, 255),
            label
        );
    }

    // Draw variable names based on number of variables
    int numVars = kmap->getVariables();
    std::string rowVar, colVar;
    
    if (numVars == 2) {
        rowVar = "A";
        colVar = "B";
    } else if (numVars == 3) {
        rowVar = "A";
        colVar = "BC";
    } else if (numVars == 4) {
        rowVar = "AB";
        colVar = "CD";
    }
    
    // Draw row variable name
    ImVec2 rowVarSize = ImGui::CalcTextSize(rowVar.c_str());
    drawList->AddText(
        ImVec2(windowPos.x + startX - rowVarSize.x - 30, windowPos.y + startY + gridHeight/2 - rowVarSize.y/2),
        IM_COL32(255, 255, 255, 255),
        rowVar.c_str()
    );
    
    // Draw column variable name
    ImVec2 colVarSize = ImGui::CalcTextSize(colVar.c_str());
    drawList->AddText(
        ImVec2(windowPos.x + startX + gridWidth/2 - colVarSize.x/2, windowPos.y + startY - colVarSize.y - 30),
        IM_COL32(255, 255, 255, 255),
        colVar.c_str()
    );

    // Draw simplified terms
    ImGui::SetCursorPos(ImVec2(startX, startY + gridHeight + 20));
    ImGui::Text("Simplified Expression:");
    
    // Get the loops and their terms
    auto loops = kmap->getLoops();
    if (!loops.empty()) {
        std::string expression;
        for (size_t i = 0; i < loops.size(); ++i) {
            if (i > 0) expression += " + ";
            
            // Get the term for this loop
            std::string term = kmap->getTermForLoop(loops[i]);
            
            // Create a colored button for the term
            ImGui::SameLine();
            ImVec4 color;
            switch (i % 3) {
                case 0: color = ImVec4(0.0f, 1.0f, 0.0f, 1.0f); break;  // Green
                case 1: color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f); break;  // Red
                case 2: color = ImVec4(0.0f, 0.0f, 1.0f, 1.0f); break;  // Blue
            }
            ImGui::PushStyleColor(ImGuiCol_Button, color);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(color.x * 0.8f, color.y * 0.8f, color.z * 0.8f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(color.x * 0.6f, color.y * 0.6f, color.z * 0.6f, 1.0f));
            ImGui::Button(term.c_str());
            ImGui::PopStyleColor(3);
        }
    } else {
        ImGui::SameLine();
        ImGui::Text("No loops drawn");
    }
    
    ImGui::End();
}

void KMapVisualizer::renderLoops() {
    // TODO: Implement loop rendering
}

void KMapVisualizer::renderUI() {
    ImGui::Begin("K-Map Controls");
    
    // Input method selection
    if (ImGui::Button("Input Boolean Equation")) {
        showEquationInput = true;
        showMintermInput = false;
    }
    if (ImGui::Button("Input Minterms")) {
        showEquationInput = false;
        showMintermInput = true;
    }

    // Boolean equation input
    if (showEquationInput) {
        ImGui::Text("Enter Boolean Equation (e.g., A'B + AB' + C)");
        char buffer[256] = {0};
        strncpy(buffer, booleanEquation.c_str(), sizeof(buffer) - 1);
        if (ImGui::InputText("##equation", buffer, sizeof(buffer))) {
            booleanEquation = buffer;
        }
        if (ImGui::Button("Apply Equation")) {
            parseBooleanEquation();
        }
    }

    // Minterm input
    if (showMintermInput) {
        ImGui::Text("Enter Minterms (comma-separated)");
        static char buffer[256] = {0};
        if (ImGui::InputText("##minterms", buffer, sizeof(buffer))) {
            std::string input = buffer;
            std::stringstream ss(input);
            std::string minterm;
            minterms.clear();
            while (std::getline(ss, minterm, ',')) {
                try {
                    minterms.push_back(std::stoi(minterm));
                } catch (...) {
                    // Invalid input, ignore
                }
            }
            updateMinterms();
        }
    }

    ImGui::Separator();

    // Clear buttons
    if (ImGui::Button("Clear Grid")) {
        kmap->clearGrid();
    }
    if (ImGui::Button("Clear Loops")) {
        kmap->clearLoops();
    }

    ImGui::End();
}

glm::vec2 KMapVisualizer::screenToGrid(glm::vec2 screenPos) const {
    // Convert screen coordinates to normalized device coordinates
    glm::vec2 ndc = (screenPos / glm::vec2(windowWidth, windowHeight)) * 2.0f - 1.0f;
    
    // Apply inverse view transform
    glm::vec4 worldPos = glm::inverse(view) * glm::vec4(ndc, 0.0f, 1.0f);
    
    // Convert to grid coordinates
    return (glm::vec2(worldPos) + 1.0f) * 0.5f * glm::vec2(kmap->getCols(), kmap->getRows());
}

glm::vec2 KMapVisualizer::gridToScreen(glm::vec2 gridPos) const {
    // Convert grid coordinates to normalized device coordinates
    glm::vec2 ndc = (gridPos / glm::vec2(kmap->getCols(), kmap->getRows())) * 2.0f - 1.0f;
    
    // Apply view transform
    glm::vec4 screenPos = view * glm::vec4(ndc, 0.0f, 1.0f);
    
    // Convert to screen coordinates
    return (glm::vec2(screenPos) + 1.0f) * 0.5f * glm::vec2(windowWidth, windowHeight);
}

// Static callback functions
void KMapVisualizer::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    instance->windowWidth = width;
    instance->windowHeight = height;
}

void KMapVisualizer::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    // Check if ImGui is using the mouse
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse) {
        return;
    }

    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            glm::vec2 gridPos = instance->screenToGrid(glm::vec2(xpos, ypos));
            instance->handleCellClick(
                static_cast<int>(gridPos.y),
                static_cast<int>(gridPos.x)
            );
        }
    }
}

void KMapVisualizer::cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    if (instance->isDrawingLoop) {
        glm::vec2 gridPos = instance->screenToGrid(glm::vec2(xpos, ypos));
        instance->currentLoop.push_back(std::make_pair(
            static_cast<int>(gridPos.y),
            static_cast<int>(gridPos.x)
        ));
    }
}

void KMapVisualizer::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    instance->zoom = glm::clamp(static_cast<float>(instance->zoom + yoffset * 0.1f), 0.1f, 5.0f);
}

void KMapVisualizer::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_SPACE:
                instance->kmap->clearLoops();
                break;
            case GLFW_KEY_2:
            case GLFW_KEY_3:
            case GLFW_KEY_4:
                instance->kmap = std::make_unique<KMapGrid>(key - GLFW_KEY_0);
                break;
        }
    }
}

void KMapVisualizer::handleCellClick(int row, int col) {
    if (row >= 0 && row < kmap->getRows() && col >= 0 && col < kmap->getCols()) {
        toggleCell(row, col);
    }
}

void KMapVisualizer::toggleCell(int row, int col) {
    KMapGrid::CellState currentState = kmap->getCellState(row, col);
    if (currentState == KMapGrid::CellState::UNMARKED) {
        kmap->setCellState(row, col, KMapGrid::CellState::MARKED);
    } else if (currentState == KMapGrid::CellState::MARKED) {
        kmap->setCellState(row, col, KMapGrid::CellState::UNMARKED);
    }
}

void KMapVisualizer::parseBooleanEquation() {
    // Clear current minterms
    minterms.clear();
    
    // Convert to uppercase and remove spaces
    std::string eq = booleanEquation;
    std::transform(eq.begin(), eq.end(), eq.begin(), ::toupper);
    eq.erase(std::remove_if(eq.begin(), eq.end(), ::isspace), eq.end());
    
    if (eq.empty()) return;

    // Count variables in the equation
    std::set<char> variables;
    for (char c : eq) {
        if (c >= 'A' && c <= 'D') {
            variables.insert(c);
        }
    }

    // Create new K-map with correct number of variables
    int numVars = variables.size();
    if (numVars < 2 || numVars > 4) {
        std::cerr << "Error: Equation must use 2-4 variables (A-D)" << std::endl;
        return;
    }
    kmap = std::make_unique<KMapGrid>(numVars);

    // Helper function to evaluate a term
    auto evaluateTerm = [this](const std::string& term) -> std::vector<int> {
        std::vector<int> result;
        std::vector<bool> varValues(kmap->getVariables(), false);
        std::vector<bool> varComplements(kmap->getVariables(), false);
        
        // Parse the term
        for (size_t i = 0; i < term.length(); ++i) {
            char c = term[i];
            if (c >= 'A' && c <= 'D') {
                int varIndex = c - 'A';
                if (varIndex < kmap->getVariables()) {
                    varValues[varIndex] = true;
                    // Check for complement
                    if (i + 1 < term.length() && term[i + 1] == '\'') {
                        varComplements[varIndex] = true;
                        i++; // Skip the ' character
                    }
                }
            }
        }
        
        // Convert to minterms
        for (int i = 0; i < (1 << kmap->getVariables()); ++i) {
            bool matches = true;
            for (int j = 0; j < kmap->getVariables(); ++j) {
                if (varValues[j]) {
                    bool bit = (i >> j) & 1;
                    if (varComplements[j]) bit = !bit;
                    if (!bit) {
                        matches = false;
                        break;
                    }
                }
            }
            if (matches) {
                result.push_back(i);
            }
        }
        
        return result;
    };
    
    // Split by OR operations
    std::stringstream ss(eq);
    std::string term;
    std::set<int> uniqueMinterms;
    
    while (std::getline(ss, term, '+')) {
        // Handle parentheses
        if (term.front() == '(' && term.back() == ')') {
            term = term.substr(1, term.length() - 2);
        }
        
        // Split by AND operations
        std::stringstream termStream(term);
        std::string factor;
        std::vector<std::string> factors;
        
        while (std::getline(termStream, factor, '*')) {
            factors.push_back(factor);
        }
        
        // Evaluate the term
        std::vector<int> termMinterms = evaluateTerm(term);
        uniqueMinterms.insert(termMinterms.begin(), termMinterms.end());
    }
    
    // Convert to vector and update K-map
    minterms.assign(uniqueMinterms.begin(), uniqueMinterms.end());
    updateMinterms();
}

void KMapVisualizer::updateMinterms() {
    // Clear the current K-map
    kmap->clearGrid();
    
    // Set the new minterms
    for (int minterm : minterms) {
        kmap->setMinterm(minterm, true);
    }

    // Debug: Print K-map state
    std::cout << "\nK-Map State (" << kmap->getVariables() << " variables):\n";
    for (int row = 0; row < kmap->getRows(); ++row) {
        for (int col = 0; col < kmap->getCols(); ++col) {
            auto state = kmap->getCellState(row, col);
            char symbol = '0';
            switch (state) {
                case KMapGrid::CellState::MARKED:
                    symbol = '1';
                    break;
                case KMapGrid::CellState::IN_LOOP:
                    symbol = 'L';
                    break;
                case KMapGrid::CellState::ESSENTIAL:
                    symbol = 'E';
                    break;
                default:
                    symbol = '0';
            }
            std::cout << symbol << " ";
        }
        std::cout << "\n";
    }
    std::cout << "Minterms: ";
    for (int minterm : minterms) {
        std::cout << minterm << " ";
    }
    std::cout << "\n\n";
} 