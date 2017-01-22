#include <QDebug>
#include <QTimer>

#include "UiaUtil.h"
#include "HRutil.h"

class TaskViewUiaClient : public QObject, public ComObject<IUIAutomationStructureChangedEventHandler, IUIAutomationEventHandler>
{
	Q_OBJECT

public:
	TaskViewUiaClient(QObject *parent = Q_NULLPTR);

private:
	QTimer* _throttleTimer;
	IUIAutomation* _client;
	UiaElemArrPtr _taskViewWindows;
	ComPtr<IUIAutomationElement> _rootElem;
	ComPtr<IUIAutomationCacheRequest> _nameCacheReq;

	Q_SLOT void sycTaskViews();
	HRESULT STDMETHODCALLTYPE HandleAutomationEvent(IUIAutomationElement * pSender, EVENTID eventID) override;
	HRESULT STDMETHODCALLTYPE HandleStructureChangedEvent(IUIAutomationElement* pSender, StructureChangeType changeType, SAFEARRAY* pRuntimeID) override;
};