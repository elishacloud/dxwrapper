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

#include "IClassFactory.h"

namespace DdrawWrapper
{
	REFIID ConvertREFIID(REFIID riid);
}

namespace dinputto8
{
	REFIID ConvertREFIID(REFIID riid);
}

REFIID ConvertAllREFIID(REFIID riid)
{
	if (Config.Dinputto8)
	{
		return DdrawWrapper::ConvertREFIID(dinputto8::ConvertREFIID(riid));
	}
	else
	{
		return DdrawWrapper::ConvertREFIID(riid);
	}
}

/************************/
/*** IUnknown methods ***/
/************************/

HRESULT m_IClassFactory::QueryInterface(REFIID riid, LPVOID FAR * ppvObj)
{
	Logging::LogDebug() << __FUNCTION__;

	if (!ppvObj)
	{
		return E_FAIL;
	}

	if ((riid == IID_IClassFactory || riid == IID_IUnknown) && ppvObj)
	{
		AddRef();

		*ppvObj = this;

		return S_OK;
	}

	Logging::LogDebug() << "Query for " << riid << " from " << WrapperID;

	if (!ProxyInterface)
	{
		if (ppvObj)
		{
			*ppvObj = nullptr;

			IQueryInterface(riid, ppvObj);

			if (*ppvObj)
			{
				return S_OK;
			}
		}

		Logging::Log() << __FUNCTION__ << " Query Not Implemented for " << riid << " from " << WrapperID;

		return E_NOINTERFACE;
	}

	HRESULT hr = ProxyInterface->QueryInterface(ConvertAllREFIID(riid), ppvObj);

	if (SUCCEEDED(hr))
	{
		IQueryInterface(riid, ppvObj);
	}
	else
	{
		if (ppvObj)
		{
			*ppvObj = nullptr;

			IQueryInterface(riid, ppvObj);

			if (*ppvObj)
			{
				return S_OK;
			}
		}

		Logging::LogDebug() << "Query failed for " << riid << " Error " << hr;
	}

	return hr;
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
	Logging::LogDebug() << __FUNCTION__ << " " << ClassID << " --> " << riid;

	if (!ProxyInterface)
	{
		if (ppvObject)
		{
			*ppvObject = nullptr;

			IQueryInterface(riid, ppvObject);

			if (*ppvObject)
			{
				return S_OK;
			}
		}

		Logging::Log() << __FUNCTION__ << " Not Implemented for IID " << riid;

		return E_FAIL;
	}

	HRESULT hr = ProxyInterface->CreateInstance(pUnkOuter, ConvertAllREFIID(riid), ppvObject);

	if (SUCCEEDED(hr))
	{
		IQueryInterface(riid, ppvObject);
	}
	else
	{
		if (ppvObject)
		{
			*ppvObject = nullptr;

			IQueryInterface(riid, ppvObject);

			if (*ppvObject)
			{
				return S_OK;
			}
		}

		Logging::LogDebug() << "Query failed for " << riid << " Error " << hr;
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
