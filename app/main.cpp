#include "log_manager.hpp"
#include "ui/main_window.hpp"
#include <QApplication>
#include <QFile>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    //QApplication::setStyle(QStyleFactory::create("Fusion"));

    { // load stylesheet
        QFile style(":/style.qss");
        style.open(QFile::ReadOnly);
        app.setStyleSheet(QLatin1String(style.readAll()));
        style.close();
    }

    LogManager::instance().init();

    MainWindow w;
    w.show();

    // User can specify a DCB file while running the executable
    if (argc > 1) {
        QString dcbFile = argv[1];
        w.openDCB(dcbFile);
    }
    return app.exec();
}
