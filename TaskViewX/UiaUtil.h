#pragma once
#include<vector>

#include <uiautomation.h>
#include "ComObect.h"

struct UiaElemArrEnumerator
{
	IUIAutomationElementArray* arr;

	explicit UiaElemArrEnumerator(IUIAutomationElementArray* arr)
	{
		this->arr = arr;
	}

	struct Iterator {
		IUIAutomationElementArray* arr;
		int i;

		ComPtr<IUIAutomationElement> operator *() const
		{
			IUIAutomationElement* outElem = nullptr;
			arr->GetElement(i, &outElem);
			return MakeComPtr(outElem);
		}

		Iterator operator ++()
		{
			Iterator old = *this;
			i++;
			return old;
		}

		bool operator==(const Iterator& other) const
		{
			return i == other.i && arr == other.arr;
		}

		bool operator!=(const Iterator& other) const
		{
			return !operator==(other);
		}


	};

	int size() const
	{
		int n = 0;
		arr->get_Length(&n);
		return n;
	}

	Iterator begin() const
	{
		return Iterator{ arr, 0 };
	}

	Iterator end() const
	{
		return Iterator{ arr, size() };
	}

};

struct UiaElemArrPtr : public ComPtr<IUIAutomationElementArray>
{
	using ComPtr<IUIAutomationElementArray>::ComPtr;

	UiaElemArrPtr() : ComPtr() {}

	UiaElemArrPtr(ComPtr<IUIAutomationElementArray>&& from)
	{
		if (*this == from)
			return;

		reset(from.get());
		from.giveUp();
	}

	int size() const
	{
		int n = 0;
		get()->get_Length(&n);
		return n;
	}

	UiaElemArrEnumerator::Iterator begin() const
	{
		return UiaElemArrEnumerator::Iterator{ get(), 0 };
	}

	UiaElemArrEnumerator::Iterator end() const
	{
		return UiaElemArrEnumerator::Iterator{ get(), size() };
	}
};

inline auto MakeComPtr(IUIAutomationElementArray* elem)
{
	return UiaElemArrPtr(elem);
}

auto GetTopLevelWindowsByClassAndPid(IUIAutomation* client, LPWSTR className)->UiaElemArrPtr;
auto GetExplorerPid(DWORD* outPid)->BOOL;
auto GetTaskViewContentElement(IUIAutomation* client, IUIAutomationElementArray* taskViews, IUIAutomationCacheRequest* cacheReq)->std::vector<ComPtr<IUIAutomationElement>>;

