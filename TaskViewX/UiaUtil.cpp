#include "UiaUtil.h"
#include "HRutil.h"

#include <uiautomation.h>
#include<comdef.h> 
#include <tlhelp32.h>

auto GetTaskViewContentElement(IUIAutomation* client, IUIAutomationElementArray* taskViews, IUIAutomationCacheRequest* cacheReq = nullptr) -> std::vector<ComPtr<IUIAutomationElement>>
{
	if (taskViews == nullptr)
		return{};

	auto n = 0;
	CHECK_HR(taskViews->get_Length(&n), return{});
	if (n < 1)
		return{};

	ComPtr<IUIAutomationCondition> condName, condControlType, condAll;

	VARIANT_GUARD varName(SysAllocString(L"Running Applications"));
	CHECK_HR(client->CreatePropertyCondition(UIA_NamePropertyId, varName.var, &condName), return{});

	VARIANT_GUARD varControlType((int)UIA_ListControlTypeId);
	CHECK_HR(client->CreatePropertyCondition(UIA_ControlTypePropertyId, varControlType.var, &condControlType), return{});

	CHECK_HR(client->CreateAndCondition(condName.get(), condControlType.get(), &condAll), return{});

	auto foundElems = std::vector<ComPtr<IUIAutomationElement>>();
	foundElems.reserve(n);

	for (auto& elem : UiaElemArrEnumerator(taskViews))
	{
		IUIAutomationElement* tmp = nullptr;
		if (cacheReq != nullptr)
		{
			CHECK_HR(elem->FindFirstBuildCache(TreeScope::TreeScope_Descendants, condAll.get(), cacheReq, &tmp), { return{}; });
		}
		else
			CHECK_HR(elem->FindFirst(TreeScope::TreeScope_Descendants, condAll.get(), &tmp), return{});

		if (tmp != nullptr)
			foundElems.emplace_back(tmp);
	}

	return foundElems;
}

auto GetTopLevelWindowsByClassAndPid(IUIAutomation* client, LPWSTR className, DWORD pid) -> UiaElemArrPtr
{
	if (className == nullptr || pid < 0)
		return nullptr;

	ComPtr<IUIAutomationElement> rootElem;
	ComPtr<IUIAutomationCondition> condPid, condClsName, condAll;

	CHECK_HR(client->GetRootElement(&rootElem), return nullptr);

	if (rootElem == nullptr)
		return nullptr;

	auto varPid = VARIANT_GUARD((int)pid);
	CHECK_HR(client->CreatePropertyCondition(UIA_ProcessIdPropertyId, varPid.var, &condPid), return nullptr);

	auto varClassName = VARIANT_GUARD(SysAllocString(className));
	CHECK_HR(client->CreatePropertyCondition(UIA_ClassNamePropertyId, varClassName.var, &condClsName), return nullptr);
	CHECK_HR(client->CreateAndCondition(condPid.get(), condClsName.get(), &condAll), return nullptr);

	UiaElemArrPtr outArr;
	CHECK_HR(rootElem->FindAll(TreeScope::TreeScope_Children, condAll.get(), &outArr), return nullptr);
	return outArr;
}

auto GetExplorerPid(DWORD* outPid) -> BOOL
{
	const LPWSTR EXPLORER_EXE = L"explorer.exe";

	TCHAR winDir[MAX_PATH];
	if (!GetWindowsDirectory(winDir, MAX_PATH))
	{
		std::cerr << __LINE__ << '@' << __FUNCTION__ << ": " << "Failed to GetWindowsDirectory" << std::endl;
		return FALSE;
	}

	BOOL success = FALSE;
	TCHAR queriedPath[MAX_PATH];
	auto winDirLen = (DWORD)wcslen(winDir);
	auto queriedPathLen = (DWORD)MAX_PATH;// (DWORD)wcslen(winDir);

	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	if (Process32First(snapshot, &entry) == TRUE)
	{
		while (Process32Next(snapshot, &entry) == TRUE)
		{
			if (wcsicmp(entry.szExeFile, EXPLORER_EXE) == 0)
			{
				auto hProc = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, entry.th32ProcessID);
				QueryFullProcessImageName(hProc, 0, queriedPath, &queriedPathLen);
				CloseHandle(hProc);

				//文件名已知，只需比较目录
				queriedPath[winDirLen] = '\0';

				if (wcsicmp(winDir, queriedPath) == 0)
				{
					*outPid = entry.th32ProcessID;
					success = true;
					break;
				}

			}
		}
	}

	CloseHandle(snapshot);

	return success;
}