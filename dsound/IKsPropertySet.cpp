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

#include "dsound.h"

HRESULT m_IKsPropertySet::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
	if ((riid == IID_IKsPropertySet || riid == IID_IUnknown) && ppvObj)
	{
		AddRef();

		*ppvObj = this;

		return S_OK;
	}

	HRESULT hr = ProxyInterface->QueryInterface(riid, ppvObj);

	if (SUCCEEDED(hr))
	{
		genericQueryInterface(riid, ppvObj);
	}

	return hr;
}

ULONG m_IKsPropertySet::AddRef()
{
	return ProxyInterface->AddRef();
}

ULONG m_IKsPropertySet::Release()
{
	ULONG x = ProxyInterface->Release();

	if (x == 0)
	{
		delete this;
	}

	return x;
}

// IKsPropertySet methods
HRESULT m_IKsPropertySet::Get(REFGUID rguidPropSet, ULONG ulId, LPVOID pInstanceData, ULONG ulInstanceLength, LPVOID pPropertyData, ULONG ulDataLength, PULONG pulBytesReturned)
{
	return ProxyInterface->Get(rguidPropSet, ulId, pInstanceData, ulInstanceLength, pPropertyData, ulDataLength, pulBytesReturned);
}

HRESULT m_IKsPropertySet::Set(REFGUID rguidPropSet, ULONG ulId, LPVOID pInstanceData, ULONG ulInstanceLength, LPVOID pPropertyData, ULONG ulDataLength)
{
	return ProxyInterface->Set(rguidPropSet, ulId, pInstanceData, ulInstanceLength, pPropertyData, ulDataLength);
}

HRESULT m_IKsPropertySet::QuerySupport(REFGUID rguidPropSet, ULONG ulId, PULONG pulTypeSupport)
{
	return ProxyInterface->QuerySupport(rguidPropSet, ulId, pulTypeSupport);
}
