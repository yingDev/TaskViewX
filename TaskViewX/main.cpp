#include "TaskViewUiaClient.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

	auto client = MakeComPtr(new TaskViewUiaClient());


    return a.exec();
}
