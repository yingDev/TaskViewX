#pragma once

#include <memory>
#include <uiautomation.h>

#include <QtWidgets/QMainWindow>
#include "ui_TaskViewX.h"
#include "ComObect.h"
#include "UiaUtil.h"

class TaskViewX : public QMainWindow, public ComObject<IUIAutomationStructureChangedEventHandler, IUIAutomationEventHandler>
{
    Q_OBJECT

public:
    TaskViewX(QWidget *parent = Q_NULLPTR);

private:
	QTimer* _throttleTimer;
	IUIAutomation* _client;
	UiaElemArrPtr _taskViewWindows;
	ComPtr<IUIAutomationElement> _rootElem;
	ComPtr<IUIAutomationCacheRequest> _nameCacheReq;

	Q_SLOT void sycTaskViews();
	HRESULT STDMETHODCALLTYPE HandleAutomationEvent(IUIAutomationElement * pSender, EVENTID eventID) override;
	HRESULT STDMETHODCALLTYPE HandleStructureChangedEvent(IUIAutomationElement* pSender, StructureChangeType changeType, SAFEARRAY* pRuntimeID) override;

    Ui::TaskViewXClass ui;
};
