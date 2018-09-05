/**
* Copyright (C) 2018 Elisha Riedlinger
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
	if ((riid == IID_IClassFactory || riid == IID_IUnknown) && ppvObj)
	{
		AddRef();

		*ppvObj = this;

		return S_OK;
	}

	return ProxyQueryInterface(nullptr, riid, ppvObj, WrapperID, nullptr);
}

ULONG m_IClassFactory::AddRef()
{
	return InterlockedIncrement(&RefCount);
}

ULONG m_IClassFactory::Release()
{
	ULONG ref = InterlockedDecrement(&RefCount);

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
	if (riid == IID_IDirectDraw)
	{
		return dd_DirectDrawCreate(nullptr, (LPDIRECTDRAW*)ppvObject, pUnkOuter);
	}
	else
	{
		return dd_DirectDrawCreateEx(nullptr, ppvObject, riid, pUnkOuter);
	}
}

HRESULT m_IClassFactory::LockServer(BOOL fLock)
{
	UNREFERENCED_PARAMETER(fLock);

	Logging::Log() << __FUNCTION__ << " Not Implemented";
	return E_NOTIMPL;
}
