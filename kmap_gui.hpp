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

class KMapGUI : public QMainWindow {
    Q_OBJECT

public:
    KMapGUI(QWidget *parent = nullptr);

private slots:
    void solveEquation();

private:
    QWidget* centralWidget;
    QVBoxLayout* mainLayout;
    QHBoxLayout* inputLayout;
    QLineEdit* equationInput;
    QPushButton* solveButton;
    QTableWidget* kmapTable;
    QLabel* minimizedLabel;
    
    KMapSolver* solver;
    
    void setupUI();
    void createKMapTable();
    void updateKMapTable(const vector<vector<bool>>& kmap, const vector<char>& variables);
    void clearResults();
};

#endif // KMAP_GUI_HPP 