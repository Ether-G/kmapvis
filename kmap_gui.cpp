#include "kmap_gui.hpp"
#include <QHeaderView>
#include <QMessageBox>
#include <QColor>
#include <cmath>

KMapGUI::KMapGUI(QWidget *parent) : QMainWindow(parent) {
    // Initialize keyboard control variables
    keyW = keyA = keyS = keyD = false;
    rotationSpeed = 2.0f;
    torusTransform = nullptr;
    
    // Create rotation timer for smooth animation
    rotationTimer = new QTimer(this);
    connect(rotationTimer, &QTimer::timeout, this, &KMapGUI::updateTorusRotation);
    rotationTimer->start(16); // ~60 FPS
    
    setupUI();
    solver = nullptr;
    rootEntity = nullptr;
    
    // Set focus policy for the main window to ensure it receives key events
    setFocusPolicy(Qt::StrongFocus);
}

KMapGUI::~KMapGUI() {
    delete solver;
    // Qt3D entities are deleted automatically through parent-child relationships
}

// Override keyPressEvent and keyReleaseEvent instead of using eventFilter
void KMapGUI::keyPressEvent(QKeyEvent *event) {
    // Only process key events when the 3D tab is active
    if (tabWidget->currentWidget() == torusTab) {
        switch (event->key()) {
            case Qt::Key_W:
                keyW = true;
                event->accept();
                return;
            case Qt::Key_A:
                keyA = true;
                event->accept();
                return;
            case Qt::Key_S:
                keyS = true;
                event->accept();
                return;
            case Qt::Key_D:
                keyD = true;
                event->accept();
                return;
        }
    }
    QMainWindow::keyPressEvent(event);
}

void KMapGUI::keyReleaseEvent(QKeyEvent *event) {
    // Only process key events when the 3D tab is active
    if (tabWidget->currentWidget() == torusTab) {
        switch (event->key()) {
            case Qt::Key_W:
                keyW = false;
                event->accept();
                return;
            case Qt::Key_A:
                keyA = false;
                event->accept();
                return;
            case Qt::Key_S:
                keyS = false;
                event->accept();
                return;
            case Qt::Key_D:
                keyD = false;
                event->accept();
                return;
        }
    }
    QMainWindow::keyReleaseEvent(event);
}

void KMapGUI::updateTorusRotation() {
    if (!torusTransform)
        return;
    
    // Get current rotation as Euler angles
    QVector3D currentEulerAngles = torusTransform->rotation().toEulerAngles();
    
    // Apply rotation based on which keys are pressed
    if (keyW)
        currentEulerAngles.setX(currentEulerAngles.x() + rotationSpeed);
    if (keyS)
        currentEulerAngles.setX(currentEulerAngles.x() - rotationSpeed);
    if (keyA)
        currentEulerAngles.setY(currentEulerAngles.y() + rotationSpeed);
    if (keyD)
        currentEulerAngles.setY(currentEulerAngles.y() - rotationSpeed);
    
    // Convert Euler angles back to quaternion
    QQuaternion rotation = QQuaternion::fromEulerAngles(currentEulerAngles);
    torusTransform->setRotation(rotation);
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
    
    // Create tabbed view
    tabWidget = new QTabWidget();
    
    // Create table view tab
    tableTab = new QWidget();
    tableLayout = new QVBoxLayout(tableTab);
    createKMapTable();
    tableLayout->addWidget(kmapTable);
    tabWidget->addTab(tableTab, "Table View");
    
    // Create 3D torus view tab
    torusTab = new QWidget();
    torusLayout = new QVBoxLayout(torusTab);
    createTorusView();
    tabWidget->addTab(torusTab, "3D Torus View");
    
    mainLayout->addWidget(tabWidget);
    
    // Create minimized expression label
    minimizedLabel = new QLabel();
    minimizedLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(minimizedLabel);
    
    // Connect signals and slots
    connect(solveButton, &QPushButton::clicked, this, &KMapGUI::solveEquation);
    
    // Connect tab changes to focus handling
    connect(tabWidget, &QTabWidget::currentChanged, this, [this](int index) {
        // If switching to 3D tab, set focus and refresh view
        if (tabWidget->widget(index) == torusTab) {
            this->setFocus();
            
            // Force refresh of the 3D view
            if (torus3DWindow && rootEntity) {
                // Toggle visibility to force a refresh
                rootEntity->setEnabled(false);
                QTimer::singleShot(50, [this]() {
                    if (rootEntity) rootEntity->setEnabled(true);
                });
            }
        }
    });
    
    // Set window properties
    setWindowTitle("K-Map Solver");
    resize(800, 600); // Larger size for 3D view
}

void KMapGUI::createKMapTable() {
    kmapTable = new QTableWidget();
    kmapTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    kmapTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    kmapTable->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

void KMapGUI::createTorusView() {
    // Create a 3D window
    torus3DWindow = new Qt3DExtras::Qt3DWindow();
    
    // Create container widget to hold the 3D window
    torus3DContainer = QWidget::createWindowContainer(torus3DWindow, torusTab);
    torus3DContainer->setMinimumSize(600, 400);
    torus3DContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    torusLayout->addWidget(torus3DContainer, 1);
    
    // Setup camera - position it farther away
    Qt3DRender::QCamera *camera = torus3DWindow->camera();
    camera->lens()->setPerspectiveProjection(45.0f, 16.0f/9.0f, 0.1f, 1000.0f);
    camera->setPosition(QVector3D(0, 0, 40.0f)); // Position farther away
    camera->setViewCenter(QVector3D(0, 0, 0));
    
    // Create root entity
    rootEntity = new Qt3DCore::QEntity();
    
    // Set the root entity
    torus3DWindow->setRootEntity(rootEntity);
    
    // Add a placeholder label with instructions
    QLabel* instructionsLabel = new QLabel("Torus View: Use WASD keys to rotate the torus (no need to click)");
    instructionsLabel->setAlignment(Qt::AlignCenter);
    torusLayout->addWidget(instructionsLabel, 0);
    
    // Create a simple placeholder texture for the torus
    int rows = 4;
    int cols = 4;
    int cellSize = 32;
    int gridWidth = 2;
    int texWidth = cols * (cellSize + gridWidth) + gridWidth;
    int texHeight = rows * (cellSize + gridWidth) + gridWidth;
    
    QImage textureImage(texWidth, texHeight, QImage::Format_RGBA8888);
    textureImage.fill(Qt::transparent);
    
    QPainter painter(&textureImage);
    painter.setRenderHint(QPainter::Antialiasing, true);
    
    // Draw grid lines - same style as updateTorusView
    painter.setPen(QPen(QColor(100, 100, 100), gridWidth));
    painter.setBrush(Qt::NoBrush);
    
    // Draw horizontal and vertical grid lines
    for (int i = 0; i <= rows; i++) {
        int y = i * (cellSize + gridWidth);
        painter.drawLine(0, y, texWidth, y);
    }
    
    for (int j = 0; j <= cols; j++) {
        int x = j * (cellSize + gridWidth);
        painter.drawLine(x, 0, x, texHeight);
    }
    
    // Draw a checkerboard pattern with bright colors matching updateTorusView
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            int x = j * (cellSize + gridWidth) + gridWidth;
            int y = i * (cellSize + gridWidth) + gridWidth;
            
            bool isOne = (i + j) % 2 == 0; // Checkerboard pattern
            QColor cellColor = isOne ? QColor(150, 255, 150) : QColor(200, 200, 200); // Match updateTorusView colors
            
            painter.setPen(Qt::NoPen);
            painter.setBrush(QBrush(cellColor));
            painter.drawRect(x, y, cellSize, cellSize);
            
            // Draw cell values
            painter.setPen(Qt::black);
            painter.setFont(QFont("Arial", cellSize / 3, QFont::Bold));
            painter.drawText(QRect(x, y, cellSize, cellSize), Qt::AlignCenter, isOne ? "1" : "0");
        }
    }
    
    painter.end();
    
    QImage flippedImage = textureImage.mirrored(false, true);
    
    // Create a torus with the checkerboard texture
    Qt3DCore::QEntity* torusEntity = new Qt3DCore::QEntity(rootEntity);
    Qt3DExtras::QTorusMesh* torusMesh = new Qt3DExtras::QTorusMesh();
    float majorRadius = 10.0f;
    float minorRadius = 4.0f;
    torusMesh->setRadius(majorRadius);
    torusMesh->setMinorRadius(minorRadius);
    torusMesh->setRings(rows * 8); // Higher resolution matching updateTorusView
    torusMesh->setSlices(cols * 8); 
    
    // Create a transform component for the torus
    torusTransform = new Qt3DCore::QTransform();
    torusTransform->setScale(1.0f);
    torusTransform->setRotation(QQuaternion::fromEulerAngles(30.0f, 30.0f, 0.0f)); // Initial rotation
    
    // Create a completely flat material without lighting effects - exact same as updateTorusView
    Qt3DExtras::QDiffuseMapMaterial* material = new Qt3DExtras::QDiffuseMapMaterial();
    
    // Set properties for a flat, matte look with maximum flatness
    material->setAmbient(QColor(255, 255, 255));  // Maximum ambient light - fully lit
    material->setSpecular(QColor(0, 0, 0));       // No specular highlights
    material->setShininess(1.0f);                 // Minimum shininess
    
    Qt3DRender::QTextureImage* texImage = new Qt3DRender::QTextureImage();
    
    // Save the image to a temporary file
    QString tempImagePath = QDir::tempPath() + "/kmap_texture_init.png";
    flippedImage.save(tempImagePath);
    
    texImage->setSource(QUrl::fromLocalFile(tempImagePath));
    material->diffuse()->addTextureImage(texImage);
    
    // Add components to entity
    torusEntity->addComponent(torusMesh);
    torusEntity->addComponent(material);
    torusEntity->addComponent(torusTransform);
    
    cellEntities.append(torusEntity);
    
    // Add explanation
    QLabel* descLabel = new QLabel(
        "Torus Visualization shows how K-map cells wrap around with Gray code\n"
        "• Each cell on the torus surface represents a K-map value\n"
        "• Green = 1, Gray = 0\n"
        "• The torus shape shows how the K-map wraps around in both dimensions\n"
        "• Use WASD keys to rotate the torus (no need to click)"
    );
    descLabel->setAlignment(Qt::AlignCenter);
    descLabel->setWordWrap(true);
    torusLayout->addWidget(descLabel);
    
    // Ensure the main window has focus for keyboard control
    this->setFocus();
}

void KMapGUI::clearResults() {
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
    
    // For 3D entities, we'll handle them separately in updateTorusView
    // Don't clear cellEntities or delete entities here, as it can cause issues
    // The updateTorusView method will properly handle clearing and recreation
}

void KMapGUI::solveEquation() {
    string equation = equationInput->text().toStdString();
    
    try {
        if (solver) {
            delete solver;
        }
        solver = new KMapSolver(equation);
        
        // Generate the K-map
        auto kmap = solver->solve();
        
        // Clear only the table view (not the 3D view)
        if (kmapTable) {
            kmapTable->clear();
            kmapTable->setRowCount(0);
            kmapTable->setColumnCount(0);
        }
        
        if (minimizedLabel) {
            minimizedLabel->clear();
        }
        
        // Update both views
        updateKMapTable(kmap, solver->getVariables());
        
        // Completely clean up old entities before creating new ones
        for (auto entity : cellEntities) {
            if (entity) {
                entity->setEnabled(false);
                entity->deleteLater();
            }
        }
        cellEntities.clear();
        torusTransform = nullptr;
        
        // Now update the torus view with the new data
        updateTorusView(kmap, solver->getVariables());
        
        // Display the minimized expression
        string minimized = solver->getMinimizedExpression();
        minimizedLabel->setText(QString::fromStdString("Minimized Expression: " + minimized));
        
        // Restore focus to ensure keyboard controls work
        this->setFocus();
        
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Error", QString::fromStdString(e.what()));
    }
}

QColor KMapGUI::getCellColor(int row, int col, const std::vector<KMapGroup>& groups) {
    // List of distinct colors for groups
    static const QList<QColor> colors = {
        QColor(255, 200, 200), // Light red
        QColor(200, 255, 200), // Light green
        QColor(200, 200, 255), // Light blue
        QColor(255, 255, 200), // Light yellow
        QColor(255, 200, 255), // Light purple
        QColor(200, 255, 255), // Light cyan
        QColor(255, 220, 180), // Light orange
        QColor(220, 180, 255)  // Light violet
    };
    
    std::vector<int> matchingGroups;
    // Find all groups that contain this cell
    for (size_t i = 0; i < groups.size(); ++i) {
        if (std::find(groups[i].cells.begin(), groups[i].cells.end(), std::make_pair(row, col)) != groups[i].cells.end()) {
            matchingGroups.push_back(i);
        }
    }
    
    if (matchingGroups.empty()) {
        // Not in any group - white or gray if 0
        return QColor(240, 240, 240);
    } else if (matchingGroups.size() == 1) {
        // In exactly one group - use that group's color
        return colors[matchingGroups[0] % colors.size()];
    } else {
        // In multiple groups - blend colors
        QColor blended(255, 255, 255);
        for (int groupIdx : matchingGroups) {
            QColor groupColor = colors[groupIdx % colors.size()];
            blended = QColor(
                (blended.red() + groupColor.red()) / 2,
                (blended.green() + groupColor.green()) / 2,
                (blended.blue() + groupColor.blue()) / 2
            );
        }
        return blended;
    }
}

void KMapGUI::updateKMapTable(const std::vector<std::vector<bool>>& kmap, const std::vector<char>& variables) {
    int rows = kmap.size();
    int cols = kmap[0].size();
    
    // Get the groups for highlighting
    std::vector<KMapGroup> groups = solver->getMinimalCoverGroups();
    
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
            
            // Highlight cell based on its groups
            if (kmap[i][j]) {
                QColor color = getCellColor(i, j, groups);
                item->setBackground(color);
            }
        }
    }
    
    // Remove any existing custom widgets from table tab (except kmapTable)
    QLayoutItem* item;
    for (int i = tableLayout->count() - 1; i >= 0; i--) {
        item = tableLayout->itemAt(i);
        if (item && item->widget() && item->widget() != kmapTable) {
            QWidget* widget = item->widget();
            tableLayout->removeWidget(widget);
            widget->deleteLater();
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
    tableLayout->addWidget(varMappingLabel);
    
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
            colorBox->setStyleSheet(QString("background-color: %1").arg(QColor(getCellColor(groups[i].cells[0].first, groups[i].cells[0].second, groups)).name()));
            
            QLabel* termLabel = new QLabel(QString::fromStdString(groups[i].term));
            
            legendLayout->addWidget(colorBox, i+1, 0);
            legendLayout->addWidget(termLabel, i+1, 1);
        }
        
        QWidget* legendWidget = new QWidget();
        legendWidget->setLayout(legendLayout);
        tableLayout->addWidget(legendWidget);
    }
}

void KMapGUI::updateTorusView(const std::vector<std::vector<bool>>& kmap, const std::vector<char>& variables) {
    // Make sure rootEntity exists and is properly set
    if (!rootEntity) {
        rootEntity = new Qt3DCore::QEntity();
        torus3DWindow->setRootEntity(rootEntity);
    }
    
    int rows = kmap.size();
    int cols = kmap[0].size();
    
    // Get the groups for coloring
    std::vector<KMapGroup> groups = solver->getMinimalCoverGroups();
    
    // Create a mapping of cells to groups
    std::map<std::pair<int, int>, std::vector<int>> cellGroups;
    for (size_t groupIdx = 0; groupIdx < groups.size(); ++groupIdx) {
        for (const auto& cell : groups[groupIdx].cells) {
            cellGroups[cell].push_back(groupIdx);
        }
    }
    
    // Create a mapping of group indices to colors - use brighter colors
    QList<QColor> groupColors;
    groupColors << QColor(255, 120, 120)  // Brighter red
               << QColor(120, 255, 120)  // Brighter green
               << QColor(120, 120, 255)  // Brighter blue
               << QColor(255, 255, 120)  // Brighter yellow
               << QColor(255, 120, 255)  // Brighter purple
               << QColor(120, 255, 255)  // Brighter cyan
               << QColor(255, 200, 120)  // Brighter orange
               << QColor(200, 120, 255); // Brighter violet
    
    // Create the texture image for the K-map
    // We'll create a texture with rows*cols cells, plus grid lines
    int cellSize = 32; // Pixels per cell
    int gridWidth = 2; // Width of grid lines
    int texWidth = cols * (cellSize + gridWidth) + gridWidth;
    int texHeight = rows * (cellSize + gridWidth) + gridWidth;
    
    QImage kmapImage(texWidth, texHeight, QImage::Format_RGBA8888);
    kmapImage.fill(Qt::transparent);
    
    QPainter painter(&kmapImage);
    painter.setRenderHint(QPainter::Antialiasing, true);
    
    // Draw grid lines - make them more visible
    painter.setPen(QPen(QColor(100, 100, 100), gridWidth));
    painter.setBrush(Qt::NoBrush);
    
    // Draw horizontal grid lines
    for (int i = 0; i <= rows; i++) {
        int y = i * (cellSize + gridWidth);
        painter.drawLine(0, y, texWidth, y);
    }
    
    // Draw vertical grid lines
    for (int j = 0; j <= cols; j++) {
        int x = j * (cellSize + gridWidth);
        painter.drawLine(x, 0, x, texHeight);
    }
    
    // Draw cells with brighter colors for better visibility on matte surface
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            int x = j * (cellSize + gridWidth) + gridWidth;
            int y = i * (cellSize + gridWidth) + gridWidth;
            
            QColor cellColor;
            if (kmap[i][j]) {
                // Cell is 1 - check if it's in any groups
                std::pair<int, int> cell(i, j);
                if (cellGroups.count(cell) > 0 && !cellGroups[cell].empty()) {
                    // Average colors if in multiple groups
                    QColor blended(255, 255, 255);
                    for (int groupIdx : cellGroups[cell]) {
                        QColor groupColor = groupColors[groupIdx % groupColors.size()];
                        blended = QColor(
                            (blended.red() + groupColor.red()) / 2,
                            (blended.green() + groupColor.green()) / 2,
                            (blended.blue() + groupColor.blue()) / 2
                        );
                    }
                    cellColor = blended;
                } else {
                    // No group - use bright green for 1's
                    cellColor = QColor(150, 255, 150); 
                }
            } else {
                // Cell is 0 - use light gray
                cellColor = QColor(200, 200, 200); 
            }
            
            painter.setPen(Qt::NoPen);
            painter.setBrush(QBrush(cellColor));
            painter.drawRect(x, y, cellSize, cellSize);
            
            // Draw the cell value as text
            painter.setPen(Qt::black);
            painter.setFont(QFont("Arial", cellSize / 3, QFont::Bold));
            painter.drawText(QRect(x, y, cellSize, cellSize), 
                             Qt::AlignCenter, 
                             kmap[i][j] ? "1" : "0");
            
            // Draw Gray codes for the first row and column
            if (i == 0) {
                int gray_j = j ^ (j >> 1);
                QString label = QString::number(gray_j);
                painter.setPen(Qt::white);
                painter.drawText(QRect(x, y - gridWidth - cellSize/2, cellSize, cellSize/2),
                                Qt::AlignCenter, label);
            }
            
            if (j == 0) {
                int gray_i = i ^ (i >> 1);
                QString label = QString::number(gray_i);
                painter.setPen(Qt::white);
                painter.drawText(QRect(x - gridWidth - cellSize/2, y, cellSize/2, cellSize),
                                Qt::AlignCenter, label);
            }
        }
    }
    
    painter.end();
    
    // Create texture
    QImage flippedImage = kmapImage.mirrored(false, true); // Qt is y-inverted compared to OpenGL
    
    // Create the torus entity with the K-map texture
    Qt3DCore::QEntity* torusEntity = new Qt3DCore::QEntity(rootEntity);
    
    // Create torus mesh
    Qt3DExtras::QTorusMesh* torusMesh = new Qt3DExtras::QTorusMesh();
    float majorRadius = 10.0f;
    float minorRadius = 4.0f;
    torusMesh->setRadius(majorRadius);
    torusMesh->setMinorRadius(minorRadius);
    torusMesh->setRings(rows * 8); // Higher resolution for smoother appearance
    torusMesh->setSlices(cols * 8); 
    
    // Create a transform component for rotation
    torusTransform = new Qt3DCore::QTransform();
    torusTransform->setScale(1.0f);
    torusTransform->setRotation(QQuaternion::fromEulerAngles(30.0f, 30.0f, 0.0f)); // Initial rotation
    
    // Create a completely flat material without lighting effects
    Qt3DExtras::QDiffuseMapMaterial* material = new Qt3DExtras::QDiffuseMapMaterial();
    
    // Set properties for a flat, matte look with maximum flatness
    material->setAmbient(QColor(255, 255, 255));  // Maximum ambient light - fully lit
    material->setSpecular(QColor(0, 0, 0));       // No specular highlights
    material->setShininess(1.0f);                 // Minimum shininess
    
    Qt3DRender::QTextureImage* texImageObj = new Qt3DRender::QTextureImage();
    
    // Save the image to a temporary file
    QString tempImagePath = QDir::tempPath() + "/kmap_texture.png";
    flippedImage.save(tempImagePath);
    
    texImageObj->setSource(QUrl::fromLocalFile(tempImagePath));
    material->diffuse()->addTextureImage(texImageObj);
    
    // Add components to entity
    torusEntity->addComponent(torusMesh);
    torusEntity->addComponent(material);
    torusEntity->addComponent(torusTransform);
    
    cellEntities.append(torusEntity);
    
    // IMPORTANT: Properly clear existing widgets from torusLayout except the container
    QLayoutItem* item;
    QList<QWidget*> torusWidgetsToKeep;
    torusWidgetsToKeep.append(torus3DContainer);
    
    // Use a safer approach to remove widgets
    for (int i = torusLayout->count() - 1; i >= 0; i--) {
        item = torusLayout->itemAt(i);
        if (item && item->widget() && !torusWidgetsToKeep.contains(item->widget())) {
            QWidget* widget = item->widget();
            torusLayout->removeWidget(widget);
            widget->deleteLater();
        }
    }
    
    // Re-add the torus container to the layout
    torusLayout->addWidget(torus3DContainer, 1);
    
    // Add variable labels
    QLabel* varLabel = new QLabel(QString("Variables: Row=%1, Col=%2")
                                 .arg(QString::fromStdString(std::string(1, variables[0]) + 
                                                            (variables.size() > 1 ? std::string(1, variables[1]) : "")))
                                 .arg(QString::fromStdString(std::string(1, variables[variables.size() > 2 ? 2 : 0]) + 
                                                            (variables.size() > 3 ? std::string(1, variables[3]) : ""))));
    varLabel->setAlignment(Qt::AlignCenter);
    torusLayout->addWidget(varLabel);
    
    // Add explanation
    QLabel* descLabel = new QLabel(
        "Torus Visualization shows how K-map cells wrap around with Gray code\n"
        "• Green/Colored cells = 1's, Gray cells = 0's\n"
        "• Colored cells show prime implicants/groups\n"
        "• The torus shape shows how K-map cells wrap around according to Gray code\n"
        "• Use WASD keys to rotate the torus (no need to click)"
    );
    descLabel->setAlignment(Qt::AlignCenter);
    descLabel->setWordWrap(true);
    torusLayout->addWidget(descLabel);
    
    // Add color legend for the groups
    if (!groups.empty()) {
        QGridLayout* legendLayout = new QGridLayout();
        QLabel* legendTitle = new QLabel("Groups and Terms:");
        legendTitle->setAlignment(Qt::AlignCenter);
        legendTitle->setStyleSheet("font-weight: bold;");
        legendLayout->addWidget(legendTitle, 0, 0, 1, 2);
        
        for (size_t i = 0; i < groups.size(); ++i) {
            QLabel* colorBox = new QLabel();
            colorBox->setFixedSize(20, 20);
            colorBox->setStyleSheet(QString("background-color: %1").arg(groupColors[i % groupColors.size()].name()));
            
            QLabel* termLabel = new QLabel(QString::fromStdString(groups[i].term));
            
            legendLayout->addWidget(colorBox, i+1, 0);
            legendLayout->addWidget(termLabel, i+1, 1);
        }
        
        QWidget* legendWidget = new QWidget();
        legendWidget->setLayout(legendLayout);
        torusLayout->addWidget(legendWidget);
    }
    
    // Ensure the main window has focus for keyboard control
    this->setFocus();
} 