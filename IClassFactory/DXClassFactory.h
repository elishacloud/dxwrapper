#pragma once

#include <new>
#include "IClassFactory\IClassFactory.h"
#include "External\dinputto8\ClassFactory.h"

template<typename WrapperClass>
class DXClassFactory final : public ClassFactoryBase
{
public:
	explicit DXClassFactory(IClassFactory* dinput8Factory)
		: ClassFactoryBase(dinput8Factory)
	{
	}

	/*** IClassFactory methods ***/
	IFACEMETHOD(CreateInstance)(THIS_ IUnknown* pUnkOuter, REFIID riid, void** ppvObject) override
	{
		if (ppvObject == nullptr)
		{
			return E_POINTER;
		}

		HRESULT hr = E_OUTOFMEMORY;
		*ppvObject = nullptr;

		typename WrapperClass::proxy_type* proxyObject = nullptr;
		HRESULT proxyHr = m_dinput8Factory->CreateInstance(pUnkOuter, WrapperClass::GetCompatibleIID(riid), reinterpret_cast<void**>(&proxyObject));
		if (FAILED(proxyHr))
		{
			return proxyHr;
		}

		// Check if interface is already wrapped
		if (void* p = nullptr; SUCCEEDED(proxyObject->QueryInterface(IID_GetInterfaceX, &p)))
		{
			hr = proxyObject->QueryInterface(riid, ppvObject);
			proxyObject->Release();
			return hr;
		}

		WrapperClass* wrapper = new (std::nothrow) WrapperClass(proxyObject);
		if (wrapper != nullptr)
		{
			hr = wrapper->QueryInterface(riid, ppvObject);
			wrapper->Release();
		}
		else
		{
			proxyObject->Release();
		}
		return hr;
	}
};
