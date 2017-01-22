#include "TaskViewX.h"
#include <QDebug>
#include <QTimer>

#include "UiaUtil.h"
#include "HRutil.h"

TaskViewX::TaskViewX(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	_throttleTimer = new QTimer(this);
	_throttleTimer->setSingleShot(true);
	connect(_throttleTimer, SIGNAL(timeout()), this, SLOT(sycTaskViews()));

	CoInitialize(nullptr);

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

	//CHECK_HR(_client->AddStructureChangedEventHandler(_rootElem.get(), TreeScope::TreeScope_Children, nullptr, this), { return; });

}

void TaskViewX::sycTaskViews()
{
	qDebug() << __FUNCTION__ << endl;
	DWORD explorerPid;
	if (GetExplorerPid(&explorerPid))
	{
	}

	if (_taskViewWindows != nullptr)
	{
		qDebug() << "- RemoveStructureChangedEventHandlers";

		for (auto& tv : UiaElemArrEnumerator(_taskViewWindows.get()))
		{
			CHECK_HR(_client->RemoveStructureChangedEventHandler(tv.get(), this), { return; });
		}
	}

	_taskViewWindows = GetTopLevelWindowsByClassAndPid(_client, L"MultitaskingViewFrame", explorerPid);
	if (_taskViewWindows == nullptr || _taskViewWindows.size() <= 0)
	{
		qDebug() << "Task Views not Showing.";
		return;
	}

	for (auto& tv : _taskViewWindows)
	{
		qDebug() << "- AddStructureChangedEventHandler";
		CHECK_HR(_client->AddStructureChangedEventHandler(tv.get(), TreeScope::TreeScope_Descendants, _nameCacheReq.get(), this), { return; });
	}

	if (_taskViewWindows.size() > 0)
	{
		ComPtr<IUIAutomationCacheRequest> cacheReq;
		CHECK_HR(_client->CreateCacheRequest(&cacheReq), { return; });
		cacheReq->put_TreeScope(TreeScope::TreeScope_Children);
		cacheReq->AddProperty(UIA_NamePropertyId);

		auto foundLists = GetTaskViewContentElement(_client, _taskViewWindows.get(), cacheReq.get());

		qDebug() << " Task Views ==============";
		for (auto& itemList : foundLists)
		{
			UiaElemArrPtr childrenArr;
			ComPtr<IUIAutomationCondition> condTrue;

			CHECK_HR(_client->CreateTrueCondition(&condTrue), { return; });
			CHECK_HR(itemList->GetCachedChildren(&childrenArr), { return; });

			if (childrenArr == nullptr)
				continue;

			qDebug() << "Running Applications: " << childrenArr.size();

			for (auto& elem : childrenArr)
			{
				BSTR name = nullptr;
				CHECK_HR(elem->get_CachedName(&name), { return; });
				qDebug() << QString::fromWCharArray(name) << "  ";
				SysFreeString(name);
			}
			qDebug() << "";
		}
		qDebug() << "--------------" << endl;
	}
}


HRESULT TaskViewX::HandleAutomationEvent(IUIAutomationElement * pSender, EVENTID eventID)
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
				_throttleTimer->start(2500);
			}
		}
		break;
	}
	case UIA_Window_WindowClosedEventId:
		std::cout << ">> Event WindowClosed!" << eventCount;
		break;

	default:
		wprintf(L">> Event (%d) Received! (count: %d)\n", eventID, eventCount);
		break;
	}

	qDebug() << " Sender = " << QString::fromWCharArray(name);
	return S_OK;
}

HRESULT TaskViewX::HandleStructureChangedEvent(IUIAutomationElement * pSender, StructureChangeType changeType, SAFEARRAY * pRuntimeID)
{
	static int eventCount;
	eventCount++;

	BSTR name = nullptr;
	CHECK_HR(pSender->get_CachedName(&name), { return S_OK; });
	SysFreeString(name);

	switch (changeType)
	{
	case StructureChangeType_ChildAdded:
		wprintf(L">> Structure Changed: ChildAdded! (count: %d) ", eventCount);
		//note: fallthru
	case StructureChangeType_ChildRemoved:
		wprintf(L">> Structure Changed: ChildRemoved! (count: %d) ", eventCount);
		//if (wcsicmp(name, L"Task View") == 0)
		{
			if (!_throttleTimer->isActive())
			{
				_throttleTimer->start(2500);
			}
		}

		break;

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
	qDebug() << " Sender = " << QString::fromWCharArray(name);

	return S_OK;
}
