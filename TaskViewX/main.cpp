#include "TaskViewUiaClient.h"
#include <QtWidgets/QApplication>
#include <QMainWindow>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

	auto client = MakeComPtr(new TaskViewUiaClient());

	auto* win = new QWidget();
	win->setWindowFlags(Qt::Widget | Qt::FramelessWindowHint | Qt::ToolTip | Qt::WindowTransparentForInput);

	win->setParent(0); // Create TopLevel-Widget
	win->setAttribute(Qt::WA_NoSystemBackground);
	win->setAttribute(Qt::WA_TranslucentBackground);
	win->setAttribute(Qt::WA_AlwaysStackOnTop);

	win->setGeometry(1500, 100, 100, 100);

	QObject::connect(client.get(), &TaskViewUiaClient::TaskViewShown, [=]()
	{
		qDebug() << "*** TaskViewShown";
		win->show();
	});

	QObject::connect(client.get(), &TaskViewUiaClient::TaskViewDisappeared, [=]()
	{
		qDebug() << "*** TaskViewDisappeared";
		win->hide();
	});

    return a.exec();
}
