#include "TaskViewUiaClient.h"
#include <QDebug>
#include <QTimer>

#include "UiaUtil.h"
#include "HRutil.h"

//todo: 鼠标滚轮切换虚拟桌面！
TaskViewUiaClient::TaskViewUiaClient(QObject *parent)
	: QObject(parent)
{
	CoInitialize(nullptr);

	_throttleTimer = new QTimer(this);
	_throttleTimer->setSingleShot(true);
	connect(_throttleTimer, SIGNAL(timeout()), this, SLOT(sycTaskViews()));

	_tvPollTimer = new QTimer(this);
	_tvPollTimer->setSingleShot(false);
	_tvPollTimer->setInterval(33);
	connect(_tvPollTimer, SIGNAL(timeout()), this, SLOT(pollWindowFromPoint()));
	//_tvPollTimer->start();

	HRESULT hr = S_OK;

	hr = CoCreateInstance(CLSID_CUIAutomation, nullptr,
		CLSCTX_INPROC_SERVER, IID_IUIAutomation,
		reinterpret_cast<void**>(&_client));
	ASSERT_HR(hr);
	CHECK_HR(_client->GetRootElement(&_rootElem), { return; });

	qDebug() << "Adding Event Handler" << endl;
	CHECK_HR(_client->CreateCacheRequest(&_nameCacheReq), {});
	_nameCacheReq->AddProperty(UIA_NamePropertyId);

	CHECK_HR(_client->AddAutomationEventHandler(UIA_Window_WindowOpenedEventId, _rootElem.get(), TreeScope::TreeScope_Children, _nameCacheReq.get(), this), { exit(-2); });
	//CHECK_HR(_client->AddAutomationEventHandler(UIA_Window_WindowClosedEventId, _rootElem.get(), TreeScope::TreeScope_Children, nullptr, this), { exit(-2); });

	//CHECK_HR(_client->AddStructureChangedEventHandler(_rootElem.get(), TreeScope::TreeScope_Element, nullptr, this), { return; });

}

Q_SLOT void TaskViewUiaClient::pollWindowFromPoint()
{
	auto hwnd = WindowFromPoint({ 100,100 });
	TCHAR clsName[128];
	GetClassName(hwnd, clsName, 128);

	//qDebug() << "WindowFromPoint = " << QString::fromWCharArray(clsName);
	if (wcsicmp(clsName, L"MultitaskingViewFrame") != 0)
	{
		//qDebug() << "MultitaskingViewFrame not found";
		return notifyDisappearing();
	}
}

void TaskViewUiaClient::sycTaskViews()
{
	if (!IsShowing())
		return notifyDisappearing();

	qDebug() << __FUNCTION__ << endl;
	/*DWORD explorerPid;
	if (! GetExplorerPid(&explorerPid))
	{
		return notifyDisappearing();
	}*/

	_currentItems.clear();

	if (_taskViewWindows != nullptr)
	{
		qDebug() << "- RemoveStructureChangedEventHandlers";

		for (auto& tv : UiaElemArrEnumerator(_taskViewWindows.get()))
		{
			CHECK_HR(_client->RemoveStructureChangedEventHandler(tv.get(), this), { return notifyDisappearing(); });
		}
	}

	auto newTaskViewWindows = GetTopLevelWindowsByClassAndPid(_client, L"MultitaskingViewFrame");
	auto isOldOnesExists = (_taskViewWindows != nullptr);
	_taskViewWindows = std::move(newTaskViewWindows);

	if(!isOldOnesExists)
		emit TaskViewShown();

	if (_taskViewWindows == nullptr || _taskViewWindows.size() <= 0)
	{
		qDebug() << "Task Views not Showing.";
		_taskViewWindows = nullptr;
		return notifyDisappearing();
	}

	for (auto& tv : _taskViewWindows)
	{
		qDebug() << "- AddStructureChangedEventHandler";
		CHECK_HR(_client->AddStructureChangedEventHandler(tv.get(), TreeScope::TreeScope_Descendants, _nameCacheReq.get(), this), { return notifyDisappearing(); });
	}

	if (_taskViewWindows.size() > 0)
	{
		ComPtr<IUIAutomationCacheRequest> cacheReq;
		CHECK_HR(_client->CreateCacheRequest(&cacheReq), { return notifyDisappearing(); });
		cacheReq->put_TreeScope(TreeScope::TreeScope_Children);
		cacheReq->AddProperty(UIA_NamePropertyId);
		cacheReq->AddPattern(UIA_InvokePatternId);
		cacheReq->AddProperty(UIA_BoundingRectanglePropertyId);

		auto foundLists = GetTaskViewContentElement(_client, _taskViewWindows.get(), cacheReq.get());
		if (foundLists.size() <= 0)
			return notifyDisappearing();

		_currentItems.reserve(foundLists.size());
		int index = 0;

		qDebug() << " Task Views ==============";
		for (auto& itemList : foundLists)
		{
			UiaElemArrPtr childrenArr;
			//ComPtr<IUIAutomationCondition> condTrue;

			//CHECK_HR(_client->CreateTrueCondition(&condTrue), { return notifyDisappearing(); });
			CHECK_HR(itemList->GetCachedChildren(&childrenArr), { return notifyDisappearing(); });

			if (childrenArr == nullptr)
				continue;

			qDebug() << "Running Applications: " << childrenArr.size();

			for (auto& elem : childrenArr)
			{
				BSTR name = nullptr;
				CHECK_HR(elem->get_CachedName(&name), { return notifyDisappearing(); });
				SysFreeString(name);

				QString nameStr = QString::fromWCharArray(name);
				qDebug() << nameStr << "  ";

				RECT rect;
				//CHECK_HR(elem->get_CachedBoundingRectangle(&rect), { return notifyDisappearing(); });

				CHECK_HR(elem->get_CurrentBoundingRectangle(&rect), { return notifyDisappearing(); });

				_currentItems.push_back({index++, nameStr, rect , std::move( elem )});

				//qDebug() << "Rect= " << "[" << rect.top << ", " << rect.right << ", " << rect.bottom << ", " << rect.left << "]";
			}
			qDebug() << "";
		}

		emit TaskViewChanged(_currentItems);


		qDebug() << "--------------" << endl;
	}
	else
		return notifyDisappearing();
}


HRESULT TaskViewUiaClient::HandleAutomationEvent(IUIAutomationElement * pSender, EVENTID eventID)
{
	static int eventCount;
	eventCount++;

	BSTR name = nullptr;
	CHECK_HR(pSender->get_CachedName(&name), { return S_OK; });
	SysFreeString(name);

	switch (eventID)
	{
	case UIA_Window_WindowOpenedEventId:
	{
		std::cout << ">> Event WindowOpened!" << eventCount;

		if (wcsicmp(name, L"Task View") == 0)
		{
			if (!_throttleTimer->isActive())
			{
				_throttleTimer->start(100);
			}
		}
		break;
	}
	case UIA_Window_WindowClosedEventId:
		std::cout << ">> Event WindowClosed!" << eventCount;
		break;

	default:
		//wprintf(L">> Event (%d) Received! (count: %d)\n", eventID, eventCount);
		break;
	}

	qDebug() << " Sender = " << QString::fromWCharArray(name);
	return S_OK;
}

HRESULT TaskViewUiaClient::HandleStructureChangedEvent(IUIAutomationElement * pSender, StructureChangeType changeType, SAFEARRAY * pRuntimeID)
{
	static int eventCount;
	eventCount++;
	//qDebug() << "pSender = " << pSender;

	//BSTR name = nullptr;
	//CHECK_HR(pSender->get_CachedName(&name), { /*return S_OK;*/ });

	switch (changeType)
	{
	case StructureChangeType_ChildAdded:
		wprintf(L">> Structure Changed: ChildAdded! (count: %d) \n", eventCount);

		if (!_throttleTimer->isActive())
		{
			_throttleTimer->start(60);
		}

		break;
	case StructureChangeType_ChildRemoved:
	{
		wprintf(L">> Structure Changed: ChildRemoved! (count: %d) \n", eventCount);

		/*qDebug() << " Sender = " << QString::fromWCharArray(name);
		if (name == nullptr || wcsicmp(name, L"Task View") == 0)
		{
			notifyDisappearing();
			break;
		}*/

		if (_taskViewWindows != nullptr)
		{
			for (auto& tv : _taskViewWindows)
			{
				BOOL same = false;
				CHECK_HR(_client->CompareElements(pSender, tv.get(), &same), {});
				if (same)
				{
					qDebug() << "sender is a TaskView, so ... notifyDisappearing()";
					notifyDisappearing();
					break;
				}
			}
		}


		if (!_throttleTimer->isActive())
		{
			_throttleTimer->start(260);
		}

		break;
	}

	case StructureChangeType_ChildrenInvalidated:
	{
		std::cout << "*";
		//sycTaskViews();
		break;
		/*wprintf(L">> Structure Changed: ChildrenInvalidated! (count: %d)\n", _eventCount);
		BOOL isSameElem = false;
		CHECK_HR(_client->CompareElements(_rootElem.get(), pSender, &isSameElem), { });
		if (isSameElem)
		{
		qDebug() << "Sender is RootElement";
		//sycTaskViews();
		}*/
	}
	break;
	case StructureChangeType_ChildrenBulkAdded:
		wprintf(L">> Structure Changed: ChildrenBulkAdded! (count: %d) ", eventCount);
		break;
	case StructureChangeType_ChildrenBulkRemoved:
		wprintf(L">> Structure Changed: ChildrenBulkRemoved! (count: %d) ", eventCount);
		break;
	case StructureChangeType_ChildrenReordered:
		wprintf(L">> Structure Changed: ChildrenReordered! (count: %d) ", eventCount);
		break;
	}

	//if (name)
	{
		//SysFreeString(name);
	}
	return S_OK;
}

void TaskViewUiaClient::notifyDisappearing()
{
	_taskViewWindows = nullptr;

	emit TaskViewDisappeared();
}


bool TaskViewUiaClient::IsShowing()
{
	auto hwnd = WindowFromPoint({ 100,100 });
	TCHAR clsName[128];
	GetClassName(hwnd, clsName, 128);

	if (wcscmp(clsName, L"MultitaskingViewFrame") == 0)
	{
		return true;
	}

	return false;
}

void TaskViewUiaClient::SwitchTo(int index)
{
	if (index >= 0 && index < _currentItems.size())
	{
		if (!IsShowing())
			return;
		ComPtr<IUIAutomationInvokePattern> pattern;
		CHECK_HR(_currentItems[index].element->GetCachedPatternAs(UIA_InvokePatternId, IID_IUIAutomationInvokePattern, (void**)&pattern), { return; });
		CHECK_HR(pattern->Invoke(), {});
	}

}

void TaskViewUiaClient::Dismiss()
{
	notifyDisappearing();
}
