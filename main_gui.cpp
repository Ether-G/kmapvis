#include "kmap_gui.hpp"
#include <QApplication>
#include <QSurfaceFormat>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    // Set up a surface format for 3D rendering
    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    format.setSamples(4);  // Enable anti-aliasing
    format.setStencilBufferSize(8);
    format.setVersion(3, 3);  // OpenGL 3.3
    format.setProfile(QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat(format);
    
    KMapGUI gui;
    gui.show();
    
    return app.exec();
} 