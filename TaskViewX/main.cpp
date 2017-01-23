#include <memory>

#include "TaskViewUiaClient.h"
#include <QtWidgets/QApplication>
#include <QMainWindow>
#include <QPainter>


class ShitWin : public QWidget
{
public:
	ShitWin() : QWidget(nullptr, Qt::Widget | Qt::FramelessWindowHint | Qt::ToolTip | Qt::WindowTransparentForInput)
	{
		setParent(0); // Create TopLevel-Widget
		setAttribute(Qt::WA_NoSystemBackground);
		setAttribute(Qt::WA_TranslucentBackground);
		setAttribute(Qt::WA_AlwaysStackOnTop);
		setAttribute(Qt::WA_DeleteOnClose);
	}

protected:
	void paintEvent(QPaintEvent* e) override
	{
		QPainter painter(this);
		painter.fillRect(rect(), QColor(255, 0, 0, 128));

		painter.setPen(Qt::blue);
		painter.setFont(QFont("Arial", 30));
		painter.drawText(rect(), Qt::AlignCenter, "Qt");
	}
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
	auto labels = std::shared_ptr<std::vector<ShitWin*>>(new std::vector<ShitWin*>());

	auto client = MakeComPtr(new TaskViewUiaClient());

	/*auto* win = new ShitWin();

	win->setGeometry(1500, 100, 100, 100);
	*/

	QObject::connect(client.get(), &TaskViewUiaClient::TaskViewShown, [=]()
	{
		qDebug() << "*** TaskViewShown";
	});

	QObject::connect(client.get(), &TaskViewUiaClient::TaskViewDisappeared, [=]()
	{
		qDebug() << "*** TaskViewDisappeared";

		for (auto* l : *labels)
		{
			l->close();
		}
		labels->clear();
	});

	QObject::connect(client.get(), &TaskViewUiaClient::TaskViewChanged, [=](const std::vector<TaskViewItem>& items) 
	{
		qDebug() << "*** TaskViewChanged";
		for (auto* l : *labels)
		{
			l->close();
		}
		labels->clear();

		for (auto& item : items)
		{
			auto lb = new ShitWin();
			auto rect = QRect((item.rect.left + item.rect.right) / 2 - 50, (item.rect.bottom + item.rect.top) / 2 - 25, 100, 50);

			lb->setGeometry(rect);

			labels->push_back(lb);
			lb->show();
		}
	});

    return a.exec();
}
