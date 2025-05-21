#include "kmap_gui.hpp"
#include <QHeaderView>
#include <QMessageBox>
#include <QColor>

KMapGUI::KMapGUI(QWidget *parent) : QMainWindow(parent) {
    setupUI();
    solver = nullptr;
}

void KMapGUI::setupUI() {
    // Create central widget and main layout
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    mainLayout = new QVBoxLayout(centralWidget);
    
    // Create input area
    inputLayout = new QHBoxLayout();
    equationInput = new QLineEdit();
    equationInput->setPlaceholderText("Enter boolean equation (e.g., ABC + A'B'C')");
    solveButton = new QPushButton("Solve");
    inputLayout->addWidget(equationInput);
    inputLayout->addWidget(solveButton);
    mainLayout->addLayout(inputLayout);
    
    // Create K-map table
    createKMapTable();
    mainLayout->addWidget(kmapTable);
    
    // Create minimized expression label
    minimizedLabel = new QLabel();
    minimizedLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(minimizedLabel);
    
    // Connect signals and slots
    connect(solveButton, &QPushButton::clicked, this, &KMapGUI::solveEquation);
    
    // Set window properties
    setWindowTitle("K-Map Solver");
    resize(600, 400);
}

void KMapGUI::createKMapTable() {
    kmapTable = new QTableWidget();
    kmapTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    kmapTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    kmapTable->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

void KMapGUI::clearResults() {
    // Only remove dynamically added widgets (e.g., variable mapping labels)
    // Do NOT delete kmapTable or minimizedLabel
    for (int i = mainLayout->count() - 1; i >= 0; --i) {
        QLayoutItem* item = mainLayout->itemAt(i);
        QWidget* widget = item ? item->widget() : nullptr;
        if (widget && widget != kmapTable && widget != minimizedLabel && widget != equationInput && widget != solveButton) {
            mainLayout->removeWidget(widget);
            delete widget;
        }
    }
    // Clear the K-map table contents
    if (kmapTable) {
        kmapTable->clear();
        kmapTable->setRowCount(0);
        kmapTable->setColumnCount(0);
    }
    // Clear the minimized label
    if (minimizedLabel) {
        minimizedLabel->clear();
    }
}

void KMapGUI::solveEquation() {
    clearResults();
    string equation = equationInput->text().toStdString();
    
    try {
        if (solver) {
            delete solver;
        }
        solver = new KMapSolver(equation);
        
        // Generate and display the K-map
        vector<vector<bool>> kmap = solver->solve();
        updateKMapTable(kmap, solver->getVariables());
        
        // Display the minimized expression
        string minimized = solver->getMinimizedExpression();
        minimizedLabel->setText(QString::fromStdString("Minimized Expression: " + minimized));
        
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Error", QString::fromStdString(e.what()));
    }
}

void KMapGUI::updateKMapTable(const vector<vector<bool>>& kmap, const vector<char>& variables) {
    int rows = kmap.size();
    int cols = kmap[0].size();
    
    // Get the groups for highlighting
    std::vector<KMapGroup> groups = solver->getMinimalCoverGroups();
    
    // Generate colors for each group
    QList<QColor> colors;
    colors << QColor(255, 200, 200) // Light red
           << QColor(200, 255, 200) // Light green
           << QColor(200, 200, 255) // Light blue
           << QColor(255, 255, 200) // Light yellow
           << QColor(255, 200, 255) // Light purple
           << QColor(200, 255, 255) // Light cyan
           << QColor(255, 220, 180) // Light orange
           << QColor(220, 180, 255); // Light violet
    
    // Create a map of cell coordinates to groups and colors
    std::map<std::pair<int, int>, std::vector<std::pair<int, QColor>>> cellGroups;
    for (size_t i = 0; i < groups.size(); ++i) {
        QColor color = colors[i % colors.size()];
        for (const auto& cell : groups[i].cells) {
            cellGroups[cell].push_back(std::make_pair(i, color));
        }
    }
    
    // Set table dimensions
    kmapTable->setRowCount(rows);
    kmapTable->setColumnCount(cols);
    
    // Set column headers (Gray code)
    for (int j = 0; j < cols; j++) {
        int gray_j = j ^ (j >> 1);
        kmapTable->setHorizontalHeaderItem(j, new QTableWidgetItem(QString::number(gray_j)));
    }
    
    // Set row headers (Gray code)
    for (int i = 0; i < rows; i++) {
        int gray_i = i ^ (i >> 1);
        string row_label = (gray_i & 2 ? "1" : "0") + string(gray_i & 1 ? "1" : "0");
        kmapTable->setVerticalHeaderItem(i, new QTableWidgetItem(QString::fromStdString(row_label)));
    }
    
    // Fill in the K-map values and highlight groups
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            QTableWidgetItem* item = new QTableWidgetItem(kmap[i][j] ? "1" : "0");
            item->setTextAlignment(Qt::AlignCenter);
            kmapTable->setItem(i, j, item);
            
            // Highlight cell if it belongs to any group
            std::pair<int, int> cell(i, j);
            if (cellGroups.find(cell) != cellGroups.end()) {
                const auto& groupInfos = cellGroups[cell];
                if (groupInfos.size() == 1) {
                    // Cell belongs to only one group
                    item->setBackground(groupInfos[0].second);
                } else {
                    // Cell belongs to multiple groups - blend the colors
                    QColor blended(255, 255, 255); // Start with white
                    for (const auto& groupInfo : groupInfos) {
                        blended = QColor(
                            (blended.red() + groupInfo.second.red()) / 2,
                            (blended.green() + groupInfo.second.green()) / 2,
                            (blended.blue() + groupInfo.second.blue()) / 2
                        );
                    }
                    item->setBackground(blended);
                }
            }
        }
    }
    
    // Add variable mapping labels
    string rowVars = "Rows: " + string(1, variables[0]);
    if (variables.size() > 1) rowVars += variables[1];
    rowVars += " (in Gray code order)";
    
    string colVars = "Columns: " + string(1, variables[2]);
    if (variables.size() > 3) colVars += variables[3];
    colVars += " (in Gray code order)";
    
    QLabel* varMappingLabel = new QLabel(QString::fromStdString(rowVars + "\n" + colVars));
    varMappingLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(varMappingLabel);
    
    // Add group legend
    if (!groups.empty()) {
        QGridLayout* legendLayout = new QGridLayout();
        QLabel* legendTitle = new QLabel("Groups and Terms:");
        legendTitle->setAlignment(Qt::AlignCenter);
        legendTitle->setStyleSheet("font-weight: bold;");
        legendLayout->addWidget(legendTitle, 0, 0, 1, 2);
        
        for (size_t i = 0; i < groups.size(); ++i) {
            QLabel* colorBox = new QLabel();
            colorBox->setFixedSize(20, 20);
            colorBox->setStyleSheet(QString("background-color: %1").arg(colors[i % colors.size()].name()));
            
            QLabel* termLabel = new QLabel(QString::fromStdString(groups[i].term));
            
            legendLayout->addWidget(colorBox, i+1, 0);
            legendLayout->addWidget(termLabel, i+1, 1);
        }
        
        QWidget* legendWidget = new QWidget();
        legendWidget->setLayout(legendLayout);
        mainLayout->addWidget(legendWidget);
    }
} 