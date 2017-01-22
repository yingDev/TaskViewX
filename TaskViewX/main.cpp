#include "TaskViewX.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    TaskViewX w;
    w.show();
    return a.exec();
}
