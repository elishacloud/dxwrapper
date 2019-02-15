/**
* Copyright (C) 2019 Elisha Riedlinger
*
* This software is  provided 'as-is', without any express  or implied  warranty. In no event will the
* authors be held liable for any damages arising from the use of this software.
* Permission  is granted  to anyone  to use  this software  for  any  purpose,  including  commercial
* applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*   1. The origin of this software must not be misrepresented; you must not claim that you  wrote the
*      original  software. If you use this  software  in a product, an  acknowledgment in the product
*      documentation would be appreciated but is not required.
*   2. Altered source versions must  be plainly  marked as such, and  must not be  misrepresented  as
*      being the original software.
*   3. This notice may not be removed or altered from any source distribution.
*/

#include "ddraw.h"

/************************/
/*** IUnknown methods ***/
/************************/

HRESULT m_IClassFactory::QueryInterface(REFIID riid, LPVOID FAR * ppvObj)
{
	Logging::LogDebug() << __FUNCTION__;

	if (!ProxyInterface)
	{
		if ((riid == IID_IClassFactory || riid == IID_IUnknown) && ppvObj)
		{
			AddRef();

			*ppvObj = this;

			return S_OK;
		}
	}

	return ProxyQueryInterface(ProxyInterface, riid, ppvObj, WrapperID, this);
}

ULONG m_IClassFactory::AddRef()
{
	Logging::LogDebug() << __FUNCTION__;

	if (!ProxyInterface)
	{
		return InterlockedIncrement(&RefCount);
	}

	return ProxyInterface->AddRef();
}

ULONG m_IClassFactory::Release()
{
	Logging::LogDebug() << __FUNCTION__;

	ULONG ref;

	if (!ProxyInterface)
	{
		ref = InterlockedDecrement(&RefCount);
	}
	else
	{
		ref = ProxyInterface->Release();
	}

	if (ref == 0)
	{
		delete this;
	}

	return ref;
}

/*****************************/
/*** IClassFactory methods ***/
/*****************************/

HRESULT m_IClassFactory::CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppvObject)
{
	Logging::LogDebug() << __FUNCTION__;

	if (!ProxyInterface)
	{
		if ((riid == IID_IDirectDraw ||
			riid == IID_IDirectDraw2 ||
			riid == IID_IDirectDraw3 ||
			riid == IID_IDirectDraw4 ||
			riid == IID_IDirectDraw7) && ppvObject)
		{
			if (Config.Dd7to9 || (Config.ConvertToDirect3D7 && Config.ConvertToDirectDraw7) || ClassID == CLSID_DirectDraw7)
			{
				return dd_DirectDrawCreateEx(nullptr, ppvObject, riid, pUnkOuter);
			}

			HRESULT hr = dd_DirectDrawCreate(nullptr, (LPDIRECTDRAW*)ppvObject, pUnkOuter);

			if (FAILED(hr))
			{
				*ppvObject = nullptr;
				return E_FAIL;
			}

			// Convert to new DirectDraw version
			if (ConvertREFIID(riid) != IID_IDirectDraw)
			{
				LPDIRECTDRAW lpDD = (LPDIRECTDRAW)*ppvObject;

				hr = lpDD->QueryInterface(riid, (LPVOID*)ppvObject);

				lpDD->Release();

				if (FAILED(hr))
				{
					*ppvObject = nullptr;
					return E_FAIL;
				}
			}

			return DD_OK;
		}

		Logging::Log() << __FUNCTION__ << " Not Implemented for IID " << riid;
		return E_FAIL;
	}

	HRESULT hr = ProxyInterface->CreateInstance(pUnkOuter, ConvertREFIID(riid), ppvObject);

	if (SUCCEEDED(hr))
	{
		genericQueryInterface(riid, ppvObject);
	}

	return hr;
}

HRESULT m_IClassFactory::LockServer(BOOL fLock)
{
	Logging::LogDebug() << __FUNCTION__;

	if (!ProxyInterface)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	return ProxyInterface->LockServer(fLock);
}
