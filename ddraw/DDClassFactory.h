#pragma once

#include <new>
#include "External\dinputto8\ClassFactory.h"

class DDClassFactory final : public ClassFactoryBase
{
private:
	const GUID rclsid;

public:
	explicit DDClassFactory(IClassFactory* dinput8Factory, REFCLSID riid)
		: ClassFactoryBase(dinput8Factory), rclsid(riid)
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

		// Dd7to9
		if (m_dinput8Factory == nullptr)
		{
			if (rclsid == CLSID_DirectDraw || rclsid == CLSID_DirectDraw7)
			{
				IDirectDraw7* wrapper = nullptr;
				if (rclsid == CLSID_DirectDraw)
				{
					HRESULT proxyHr = dd_DirectDrawCreate(nullptr, reinterpret_cast<LPDIRECTDRAW*>(&wrapper), pUnkOuter);
					if (FAILED(proxyHr))
					{
						return proxyHr;
					}
				}
				else
				{
					HRESULT proxyHr = dd_DirectDrawCreateEx(nullptr, reinterpret_cast<void**>(&wrapper), IID_IDirectDraw7, pUnkOuter);
					if (FAILED(proxyHr))
					{
						return proxyHr;
					}
				}

				if (wrapper != nullptr)
				{
					hr = wrapper->QueryInterface(riid, ppvObject);
					wrapper->Release();
				}
				return hr;
			}
			else if (rclsid == CLSID_DirectDrawClipper)
			{
				IDirectDrawClipper* wrapper = nullptr;
				HRESULT proxyHr = dd_DirectDrawCreateClipper(0, reinterpret_cast<LPDIRECTDRAWCLIPPER*>(&wrapper), pUnkOuter);
				if (FAILED(proxyHr))
				{
					return proxyHr;
				}

				if (wrapper != nullptr)
				{
					hr = wrapper->QueryInterface(riid, ppvObject);
					wrapper->Release();
				}
				return hr;
			}
			else
			{
				return CLASS_E_CLASSNOTAVAILABLE;
			}
		}

		if (rclsid == CLSID_DirectDraw || rclsid == CLSID_DirectDraw7)
		{
			IDirectDraw7* proxyObject = nullptr;
			HRESULT proxyHr = m_dinput8Factory->CreateInstance(pUnkOuter, riid, reinterpret_cast<void**>(&proxyObject));
			if (FAILED(proxyHr))
			{
				return proxyHr;
			}

			const DWORD DXVersion = rclsid == CLSID_DirectDraw ? 1 : 7;

			m_IDirectDrawX* wrapper = new (std::nothrow) m_IDirectDrawX(proxyObject, DXVersion);
			if (wrapper != nullptr)
			{
				hr = wrapper->QueryInterface(riid, ppvObject, DXVersion);
				wrapper->Release();
			}
			else
			{
				proxyObject->Release();
			}
			return hr;
		}
		else if (rclsid == CLSID_DirectDrawClipper)
		{
			IDirectDrawClipper* proxyObject = nullptr;
			HRESULT proxyHr = m_dinput8Factory->CreateInstance(pUnkOuter, riid, reinterpret_cast<void**>(&proxyObject));
			if (FAILED(proxyHr))
			{
				return proxyHr;
			}

			m_IDirectDrawClipper* wrapper = new (std::nothrow) m_IDirectDrawClipper(proxyObject);
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
		else
		{
			return CLASS_E_CLASSNOTAVAILABLE;
		}
	}
};
