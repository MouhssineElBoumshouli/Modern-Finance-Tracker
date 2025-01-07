#include "mainwindow.h"
#include <QApplication>
#include <QIcon>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Set application information
    QApplication::setApplicationName("Modern Finance Tracker");
    QApplication::setApplicationVersion("0.1");
    QApplication::setWindowIcon(QIcon(":/icons/app.ico"));

    MainWindow w;
    w.show();
    return a.exec();
}
