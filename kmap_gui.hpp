#ifndef KMAP_GUI_HPP
#define KMAP_GUI_HPP

#include "kmap_solver.hpp"
#include <QMainWindow>
#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTabWidget>
#include <QPainter>
#include <QWidget>
#include <QDir>
#include <QKeyEvent>
#include <QTimer>

// Qt3D includes
#include <Qt3DCore/QEntity>
#include <Qt3DRender/QCamera>
#include <Qt3DRender/QCameraLens>
#include <Qt3DCore/QTransform>
#include <Qt3DExtras/QTorusMesh>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DExtras/QDiffuseMapMaterial>
#include <Qt3DExtras/Qt3DWindow>
#include <Qt3DExtras/QOrbitCameraController>
#include <Qt3DRender/QPointLight>
#include <Qt3DExtras/QCuboidMesh>
#include <Qt3DExtras/QSphereMesh>
#include <Qt3DRender/QRenderAspect>
#include <Qt3DInput/QInputAspect>
#include <Qt3DRender/QFrameGraphNode>
#include <Qt3DRender/QViewport>
#include <Qt3DRender/QTextureImage>
#include <QPropertyAnimation>

class KMapGUI : public QMainWindow {
    Q_OBJECT

public:
    KMapGUI(QWidget *parent = nullptr);
    ~KMapGUI();

protected:
    // Handle key events for WASD controls
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

private slots:
    void solveEquation();
    void updateTorusRotation();

private:
    QWidget* centralWidget;
    QVBoxLayout* mainLayout;
    QHBoxLayout* inputLayout;
    QLineEdit* equationInput;
    QPushButton* solveButton;
    QTabWidget* tabWidget;
    
    // Table view tab
    QWidget* tableTab;
    QVBoxLayout* tableLayout;
    QTableWidget* kmapTable;
    
    // 3D torus view tab
    QWidget* torusTab;
    QVBoxLayout* torusLayout;
    Qt3DExtras::Qt3DWindow* torus3DWindow;
    QWidget* torus3DContainer;
    Qt3DCore::QEntity* rootEntity;
    QList<Qt3DCore::QEntity*> cellEntities;
    Qt3DCore::QTransform* torusTransform; // Transform for the torus entity
    
    QLabel* minimizedLabel;
    
    KMapSolver* solver;
    
    // Keyboard control variables
    QTimer* rotationTimer;
    bool keyW, keyA, keyS, keyD;
    float rotationSpeed;
    
    // UI and initialization methods
    void setupUI();
    void createKMapTable();
    void createTorusView();
    void clearResults();
    
    // Table view methods
    void updateKMapTable(const std::vector<std::vector<bool>>& kmap, const std::vector<char>& variables);
    QColor getCellColor(int row, int col, const std::vector<KMapGroup>& groups);
    
    // Torus view methods
    void updateTorusView(const std::vector<std::vector<bool>>& kmap, const std::vector<char>& variables);
};

#endif // KMAP_GUI_HPP 