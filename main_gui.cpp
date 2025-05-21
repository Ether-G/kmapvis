#include "kmap_gui.hpp"
#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    KMapGUI window;
    window.show();
    return app.exec();
} 