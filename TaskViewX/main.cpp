#include <memory>

#include "TaskViewUiaClient.h"
#include <QtWidgets/QApplication>
#include <QMainWindow>
#include <QPainter>
#include <QGraphicsDropShadowEffect> 

class ShitWin : public QWidget
{
	const int SHADOW_RADIUS = 20;
	QString _text;
public:
	ShitWin() : QWidget(nullptr, Qt::Widget | Qt::FramelessWindowHint | Qt::ToolTip | Qt::WindowTransparentForInput)
	{
		setParent(0); // Create TopLevel-Widget
		setAttribute(Qt::WA_NoSystemBackground);
		setAttribute(Qt::WA_TranslucentBackground);
		setAttribute(Qt::WA_AlwaysStackOnTop);
		setAttribute(Qt::WA_DeleteOnClose);

		auto* effect = new QGraphicsDropShadowEffect();
		effect->setOffset(0, SHADOW_RADIUS / 4);
		effect->setBlurRadius(SHADOW_RADIUS);
		setGraphicsEffect(effect);
	}

	void setText(const QString& s)
	{
		_text = s;
	}

protected:
	void paintEvent(QPaintEvent* e) override
	{
		auto rec = rect();
		auto shadowOffsetY = ((QGraphicsDropShadowEffect*)graphicsEffect())->yOffset();
		rec.adjust(SHADOW_RADIUS, SHADOW_RADIUS - shadowOffsetY, -SHADOW_RADIUS, -SHADOW_RADIUS - shadowOffsetY);

		QPainter painter(this);
		painter.setRenderHint(QPainter::Antialiasing);
		
		QPainterPath path;
		//path.addRoundedRect(rec, corner, corner);
		path.addEllipse(rec);
		QPen pen(Qt::white, 2);
		painter.setPen(pen);
		painter.fillPath(path, QColor(255, 255, 255, 240));
		painter.drawPath(path);
		
		painter.setPen(QColor(255, 95, 10));
		painter.setFont(QFont("Consolas", rec.height() / 3));

		painter.setPen(QColor(255, 255, 255));
		painter.drawText(rec.adjusted(0,2,0,2), Qt::AlignCenter, _text);

		painter.setPen(QColor(255, 95, 10));
		painter.drawText(rec, Qt::AlignCenter, _text);
	}
};

const QString LETTERS = "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
	auto labels = std::shared_ptr<std::vector<ShitWin*>>(new std::vector<ShitWin*>());
	auto client = MakeComPtr(new TaskViewUiaClient());


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
		qDebug() << "*** TaskViewChanged: items=" << items.size();
		for (auto* l : *labels)
		{
			l->close();
		}
		labels->clear();

		int i = 0;

		for (auto& item : items)
		{
			const int W = 200;
			const int H = 200;

			auto lb = new ShitWin();
			auto rect = QRect((item.rect.left + item.rect.right) / 2 - W/2, (item.rect.bottom + item.rect.top) / 2 - H/2, W, H);

			lb->setGeometry(rect);
			lb->setText(LETTERS[i++]);

			labels->push_back(lb);
			lb->show();
		}
	});

    return a.exec();
}
