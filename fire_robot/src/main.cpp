#include "../include/fire_robot/mainwidget.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWidget w(argc, argv);
    w.show();
    return a.exec();
}
