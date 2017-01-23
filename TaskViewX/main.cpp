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
	QColor _bg;
	QColor _fg;
public:
	ShitWin() : QWidget(nullptr, Qt::Widget | Qt::FramelessWindowHint | Qt::ToolTip | Qt::WindowTransparentForInput)
	{
		_bg = QColor(255, 255, 255, 240);
		_fg = QColor(255, 95, 10);

		setParent(0); // Create TopLevel-Widget
		setAttribute(Qt::WA_NoSystemBackground);
		setAttribute(Qt::WA_TranslucentBackground);
		setAttribute(Qt::WA_AlwaysStackOnTop);
		setAttribute(Qt::WA_DeleteOnClose);

		//auto* effect = new QGraphicsDropShadowEffect();
		//effect->setOffset(0, SHADOW_RADIUS / 4);
		//effect->setBlurRadius(SHADOW_RADIUS);
		//setGraphicsEffect(effect);
	}

	void setText(const QString& s)
	{
		_text = s;
	}

	void setColors(const QColor& bg, const QColor& fg, bool repaintNow=true)
	{
		_bg = bg;
		_fg = fg;
		if (repaintNow)
			repaint();
		else 
			update();
	}

protected:
	void paintEvent(QPaintEvent* e) override
	{
		auto rec = rect();
		auto shadowOffsetY = 0;//((QGraphicsDropShadowEffect*)graphicsEffect())->yOffset();
		rec.adjust(SHADOW_RADIUS, SHADOW_RADIUS - shadowOffsetY, -SHADOW_RADIUS, -SHADOW_RADIUS - shadowOffsetY);

		QPainter painter(this);
		painter.setRenderHint(QPainter::Antialiasing);

		QPainterPath path;
		//path.addRoundedRect(rec, corner, corner);
		path.addEllipse(rec);
		QPen pen(_fg, 2);
		painter.setPen(pen);
		painter.fillPath(path, _bg);
		painter.drawPath(path);

		painter.setPen(QColor(255, 95, 10));
		painter.setFont(QFont(QString::fromWCharArray(L"Consolas"), rec.height() / 3));

		painter.setPen(QColor(255, 255, 255));
		painter.drawText(rec.adjusted(0,2,0,2), Qt::AlignCenter, _text);

		painter.setPen(_fg);
		painter.drawText(rec, Qt::AlignCenter, _text);
	}
};

const QString LETTERS = "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
const QString LETTERS_OVERFLOW = QString::fromWCharArray(L"窗口太多了囧");

static HHOOK gKbdHook;
static ComPtr<TaskViewUiaClient> gClient;
static std::shared_ptr<std::vector<ShitWin*>> gLabelWindows;

static bool gBlockingInput;

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (gBlockingInput)
	{
		qDebug() << "** Blocking Input";
		return 0;
	}

	if (nCode < 0 || ! gClient->IsShowing())
		return CallNextHookEx(gKbdHook, nCode, wParam, lParam);

	auto* data = (KBDLLHOOKSTRUCT*)lParam;

	switch (wParam)
	{
		case WM_KEYDOWN:
		{
			auto key = (char)data->vkCode;
			qDebug() << "WM_KEYDOWN: " << key;
			auto index = LETTERS.indexOf(QChar(key));
			qDebug() << "Index= " << index;
			if (index >= 0)
			{
				gBlockingInput = true;
				auto* label = gLabelWindows->at(index);
				label->setColors(QColor(255, 95, 10), Qt::white);
				QTimer::singleShot(150, [=]() 
				{
					gClient->SwitchTo(index);
					gClient->Dismiss();

					QTimer::singleShot(500, []()
					{
						gBlockingInput = false;
					});
				});
			}
				break;
		}

		case WM_KEYUP:
			qDebug() << "WM_KEYUP";
			break;
		case WM_SYSKEYDOWN:
			qDebug() << "WM_SYSKEYDOWN";
			break;
		case WM_SYSKEYUP:
			qDebug() << "WM_SYSKEYUP";
			break;
	}

	return CallNextHookEx(gKbdHook, nCode, wParam, lParam);
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
	gLabelWindows = std::shared_ptr<std::vector<ShitWin*>>(new std::vector<ShitWin*>());
	gClient = MakeComPtr(new TaskViewUiaClient());

	//todo: ...
	gKbdHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, nullptr, 0);

	if (gKbdHook == NULL)
	{
		auto err = GetLastError();

		LPWSTR messageBuffer = nullptr;
		auto size = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR) &messageBuffer, 0, NULL);

		qDebug() << QString::fromWCharArray(messageBuffer, size);

		//Free the buffer.
		LocalFree(messageBuffer);

		system("PAUSE");
	}

	QObject::connect(gClient.get(), &TaskViewUiaClient::TaskViewShown, [=]()
	{
		qDebug() << "*** TaskViewShown";
	});

	QObject::connect(gClient.get(), &TaskViewUiaClient::TaskViewDisappeared, [=]()
	{
		qDebug() << "*** TaskViewDisappeared";

		for (auto* l : *gLabelWindows)
		{
			l->close();
		}
		gLabelWindows->clear();
	});

	//todo: 多退少补策略
	QObject::connect(gClient.get(), &TaskViewUiaClient::TaskViewChanged, [=](const std::vector<TaskViewItem>& items)
	{
		qDebug() << "*** TaskViewChanged: items=" << items.size();
		for (auto* l : *gLabelWindows)
		{
			l->close();
		}
		gLabelWindows->clear();

		int i = 0;

		for (auto& item : items)
		{
			const int W = 200;
			const int H = 200;

			auto lb = new ShitWin();
			auto rect = QRect((item.rect.left + item.rect.right) / 2 - W/2, (item.rect.bottom + item.rect.top) / 2 - H/2, W, H);

			lb->setGeometry(rect);
			lb->setText( i < LETTERS.length() ? LETTERS[i++] : LETTERS_OVERFLOW[(i++ - LETTERS.length()) % LETTERS_OVERFLOW.length()]);

			gLabelWindows->push_back(lb);
			lb->show();
		}
	});

    return a.exec();
}

