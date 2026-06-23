#pragma once

#include <new>
#include "External\dinputto8\ClassFactory.h"

class DDClassFactory final : public ClassFactoryBase
{
private:
	const GUID ClassID;

public:
	explicit DDClassFactory(IClassFactory* dinput8Factory, REFCLSID riid)
		: ClassFactoryBase(dinput8Factory), ClassID(riid)
	{
	}

	/*** IClassFactory methods ***/
	IFACEMETHOD(CreateInstance)(THIS_ IUnknown* pUnkOuter, REFIID riid, void** ppvObject) override
	{
		if (ppvObject == nullptr)
		{
			return E_POINTER;
		}
		*ppvObject = nullptr;

		// Dd7to9
		if (m_dinput8Factory == nullptr)
		{
			if (ClassID == CLSID_DirectDraw || ClassID == CLSID_DirectDraw7)
			{
				IDirectDraw7* wrapper = nullptr;
				if (ClassID == CLSID_DirectDraw7 || riid == IID_IDirectDraw7)
				{
					HRESULT proxyHr = dd_DirectDrawCreateEx(nullptr, reinterpret_cast<void**>(&wrapper), IID_IDirectDraw7, pUnkOuter);
					if (FAILED(proxyHr))
					{
						return proxyHr;
					}
				}
				else
				{
					HRESULT proxyHr = dd_DirectDrawCreate(nullptr, reinterpret_cast<LPDIRECTDRAW*>(&wrapper), pUnkOuter);
					if (FAILED(proxyHr))
					{
						return proxyHr;
					}
				}

				HRESULT hr = wrapper->QueryInterface(riid, ppvObject);
				wrapper->Release();
				return hr;
			}
			else if (ClassID == CLSID_DirectDrawClipper)
			{
				IDirectDrawClipper* wrapper = nullptr;
				HRESULT proxyHr = dd_DirectDrawCreateClipper(0, reinterpret_cast<LPDIRECTDRAWCLIPPER*>(&wrapper), pUnkOuter);
				if (FAILED(proxyHr))
				{
					return proxyHr;
				}

				HRESULT hr = wrapper->QueryInterface(riid, ppvObject);
				wrapper->Release();
				return hr;
			}
			else
			{
				return CLASS_E_CLASSNOTAVAILABLE;
			}
		}

		if (ClassID == CLSID_DirectDraw || ClassID == CLSID_DirectDraw7)
		{
			IDirectDraw7* proxyObject = nullptr;
			HRESULT proxyHr = m_dinput8Factory->CreateInstance(pUnkOuter, riid, reinterpret_cast<void**>(&proxyObject));
			if (FAILED(proxyHr))
			{
				return proxyHr;
			}

			const DWORD DXVersion = ClassID == CLSID_DirectDraw ? 1 : 7;

			m_IDirectDrawX* wrapper = new (std::nothrow) m_IDirectDrawX(proxyObject, DXVersion);
			if (!wrapper)
			{
				proxyObject->Release();
				return E_OUTOFMEMORY;
			}

			HRESULT hr = wrapper->QueryInterface(riid, ppvObject, DXVersion);
			wrapper->Release(DXVersion);
			return hr;
		}
		else if (ClassID == CLSID_DirectDrawClipper)
		{
			IDirectDrawClipper* proxyObject = nullptr;
			HRESULT proxyHr = m_dinput8Factory->CreateInstance(pUnkOuter, riid, reinterpret_cast<void**>(&proxyObject));
			if (FAILED(proxyHr))
			{
				return proxyHr;
			}

			m_IDirectDrawClipper* wrapper = new (std::nothrow) m_IDirectDrawClipper(proxyObject);
			if (!wrapper)
			{
				proxyObject->Release();
				return E_OUTOFMEMORY;
			}

			HRESULT hr = wrapper->QueryInterface(riid, ppvObject);
			wrapper->Release();
			return hr;
		}
		else
		{
			return CLASS_E_CLASSNOTAVAILABLE;
		}
	}
};
