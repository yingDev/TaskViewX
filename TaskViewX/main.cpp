#include <memory>

#include <QtWidgets/QApplication>
#include <QMainWindow>
#include <QPainter>
#include <QGraphicsDropShadowEffect>

#include "TaskViewUiaClient.h"
#include "LabelWindow.h"

const QString LETTERS = "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
const QString LETTERS_OVERFLOW = QString::fromWCharArray(L"窗口太多了囧");

static HHOOK gKbdHook;
static ComPtr<TaskViewUiaClient> gClient;
static std::vector<LabelWindow*> gLabelWindows;
static bool gBlockingInput;

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

	gLabelWindows = std::vector<LabelWindow*>();
	gClient = MakeComPtr(new TaskViewUiaClient());


	//todo: ...
	gKbdHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, nullptr, 0);

	//todo: ...
	if (gKbdHook == NULL)
	{
		auto err = GetLastError();
		LPWSTR messageBuffer = nullptr;
		auto size = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR) &messageBuffer, 0, NULL);

		qDebug() << QString::fromWCharArray(messageBuffer, size);

		LocalFree(messageBuffer);
		system("PAUSE");
	}


	//todo: note: 目前不能准确检测 Task View 的状态，所以需要一些hack，来减少奇怪的现象。。。
	QObject::connect(gClient.get(), &TaskViewUiaClient::TaskViewShown, [=]()
	{
		qDebug() << "*** TaskViewShown";
	});

	QObject::connect(gClient.get(), &TaskViewUiaClient::TaskViewDisappeared, [=]()
	{
		qDebug() << "*** TaskViewDisappeared";
		for (auto* l : gLabelWindows)
		{
			l->close();
			delete l;
		}
		gLabelWindows.clear();
	});

	
	QObject::connect(gClient.get(), &TaskViewUiaClient::TaskViewChanged, [=](const std::vector<TaskViewItem>& items)
	{
		auto itemCount = items.size();
		auto labelCount = gLabelWindows.size();

		qDebug() << "*** TaskViewChanged: items=" << itemCount;

		//多退少补
		auto delta = int( itemCount - labelCount);
		for (auto i = 0; i < std::abs(delta); i++)
		{
			if (delta > 0)
				gLabelWindows.push_back(new LabelWindow());
			else
			{
				auto last = gLabelWindows.back();
				last->close();
				delete last;

				gLabelWindows.pop_back();
			}
		}

		const int W = 200;
		const int H = 200;

		for (auto i=0; i<itemCount; i++)
		{
			auto* lb = gLabelWindows[i];
			const auto& item = items[i];

			auto rect = QRect((item.rect.left + item.rect.right) / 2 - W/2, (item.rect.bottom + item.rect.top) / 2 - H/2, W, H);

			if( rect != lb->rect())
				lb->setGeometry(rect);

			lb->setText( i < LETTERS.length() ? LETTERS[i] : LETTERS_OVERFLOW[(i - LETTERS.length()) % LETTERS_OVERFLOW.length()]);
			lb->setColors(QColor(255, 255, 255, 240), QColor(255, 95, 10));

			if(!lb->isVisible())
				lb->show();
		}
	});

    return a.exec();
}


LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (gBlockingInput)
	{
		qDebug() << "** Blocking Input";
		return 0;
	}

	if (nCode < 0 || !gClient->IsShowing())
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
			auto* label = gLabelWindows.at(index);
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
