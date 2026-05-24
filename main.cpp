// main.cpp
#include <QApplication>
#include "mainwindow.h"
#include "settings.h"
#include "stylesheet.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("PPTX课件分类器");
    app.setOrganizationName("YourCompany");
    app.setApplicationDisplayName("PPTX课件分类器");
    app.setStyle("Fusion");
    app.setStyleSheet(Material3StyleSheet::getStyle());
    // 加载设置
    Settings::instance();
    
    MainWindow window;
    window.show();
    
    return app.exec();
}