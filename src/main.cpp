#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setWindowIcon(QIcon("./resources/images/icon.ico"));

    MainWindow w;
    w.show();
    int ret = app.exec();
    return ret;
}
