#pragma once
#include<comdef.h> 

template<class ...IFaces>
class ComObject : public IUnknown, public IFaces...
{
private:
	LONG _refCount;

	template<class Arg1 = void, class ...Args>
	struct QueryInterfaceHelper
	{
		static HRESULT Query(ComObject* thiz, REFIID iid, void** ppv)
		{
			if (iid == __uuidof(Arg1))
			{
				*ppv = static_cast<Arg1*>(thiz);
				thiz->AddRef();
				return S_OK;
			}
			return QueryInterfaceHelper<Args...>::Query(thiz, iid, ppv);
		}
	};

	template<>
	struct QueryInterfaceHelper<void>
	{
		static HRESULT Query(ComObject* thiz, REFIID iid, void** ppv)
		{
			return E_NOINTERFACE;
		}
	};
public:
	ComObject() : _refCount{ 1 }
	{
	}

	// IUnknown methods.
	ULONG STDMETHODCALLTYPE AddRef()
	{
		ULONG ret = InterlockedIncrement(&_refCount);
		//qDebug() << "ComObject: refCount = " << _refCount << endl;

		return ret;
	}

	ULONG STDMETHODCALLTYPE Release()
	{
		ULONG ret = InterlockedDecrement(&_refCount);
		//qDebug() << "ComObject: refCount = " << _refCount << endl;

		if (ret == 0)
		{
			delete this;
			return 0;
		}
		return ret;
	}

	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppInterface)
	{
		return QueryInterfaceHelper<IUnknown, IFaces...>::Query(this, riid, ppInterface);
	}

};


template<class T>
class ComPtr
{
protected:
	T* _ptr;
public:
	ComPtr() : _ptr{ nullptr } {}
	ComPtr(const ComPtr&) = delete;
	ComPtr(ComPtr<T>&& other) : _ptr{ nullptr }
	{
		*this = std::move(other);
	}

	ComPtr<T>& operator=(ComPtr<T>&& other)
	{
		reset(other.get());
		other.giveUp();

		return *this;
	}
	ComPtr<T>& operator=(const ComPtr<T>&) = delete;

	explicit ComPtr(T* elem) : _ptr{ elem }
	{
	}

	ComPtr(std::nullptr_t) : _ptr{ nullptr }
	{
	}

	~ComPtr()
	{
		if (_ptr)
			_ptr->Release();
	}

	T* get() const
	{
		return _ptr;
	}

	void reset(T* ptr)
	{
		if (_ptr)
			_ptr->Release();
		_ptr = ptr;
	}

	void giveUp()
	{
		_ptr = nullptr;
	}

	T* operator ->()
	{
		return _ptr;
	}

	T** operator &()
	{
		if (_ptr)
			throw "Only empty ComPtr can be taken address of";
		return &_ptr;
	}

	operator bool() const
	{
		return bool(_ptr);
	}

	bool operator==(const ComPtr<T>&  b) const
	{
		return _ptr == b._ptr;
	}

	bool operator==(std::nullptr_t) const
	{
		return _ptr == nullptr;
	}

	bool operator!=(std::nullptr_t) const
	{
		return _ptr != nullptr;
	}
};


template<class T>
inline auto MakeComPtr(T* elem)
{
	return ComPtr<T>(elem);
}

struct VARIANT_GUARD
{
	VARIANT var;

	VARIANT_GUARD()
	{
		VariantInit(&var);
	}

	VARIANT_GUARD(int i)
	{
		VariantInit(&var);
		var.vt = VT_I4;
		var.intVal = i;
	}

	VARIANT_GUARD(long l)
	{
		VariantInit(&var);
		var.vt = VT_I8;
		var.lVal = l;
	}

	VARIANT_GUARD(DWORD d)
	{
		VariantInit(&var);
		var.vt = VT_UI8;
		var.ulVal = d;
	}

	VARIANT_GUARD(BSTR s)
	{
		VariantInit(&var);
		var.vt = VT_BSTR;
		var.bstrVal = s;
	}

	VARIANT_GUARD(const VARIANT_GUARD&) = delete;
	VARIANT_GUARD(VARIANT_GUARD&& other)
	{
		var = other.var;
		other.var.vt = VT_EMPTY;
	}

	~VARIANT_GUARD()
	{
		VariantClear(&var);
	}


};
