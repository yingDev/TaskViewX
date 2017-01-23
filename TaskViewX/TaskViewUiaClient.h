#pragma once

#include <vector>

#include <QDebug>
#include <QTimer>

#include "UiaUtil.h"
#include "HRutil.h"

class TaskViewItem
{
public:
	int index;
	QString name;
	RECT rect;

	ComPtr<IUIAutomationElement> element;
};


class TaskViewUiaClient : public QObject, public ComObject<IUIAutomationStructureChangedEventHandler, IUIAutomationEventHandler>
{
	Q_OBJECT

public:
	TaskViewUiaClient(QObject *parent = Q_NULLPTR);

	bool IsShowing();
	void Show();
	void SwitchTo(int index);
	void Dismiss();

signals:
	void TaskViewShown();
	void TaskViewChanged(const std::vector<TaskViewItem>& items);
	void TaskViewDisappeared();

private:
	QTimer* _throttleTimer;
	IUIAutomation* _client;
	UiaElemArrPtr _taskViewWindows;
	ComPtr<IUIAutomationElement> _rootElem;
	ComPtr<IUIAutomationCacheRequest> _nameCacheReq;
	std::vector<TaskViewItem> _currentItems;

	Q_SLOT void sycTaskViews();

	HRESULT STDMETHODCALLTYPE HandleAutomationEvent(IUIAutomationElement * pSender, EVENTID eventID) override;
	HRESULT STDMETHODCALLTYPE HandleStructureChangedEvent(IUIAutomationElement* pSender, StructureChangeType changeType, SAFEARRAY* pRuntimeID) override;

	void notifyDisappearing();
};
