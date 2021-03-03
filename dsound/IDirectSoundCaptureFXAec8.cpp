/**
* Copyright (C) 2021 Elisha Riedlinger
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

HRESULT m_IDirectSoundCaptureFXAec8::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if ((riid == IID_IDirectSoundCaptureFXAec || riid == IID_IDirectSoundCaptureFXAec8 || riid == IID_IUnknown) && ppvObj)
	{
		AddRef();

		*ppvObj = this;

		return DS_OK;
	}

	HRESULT hr = ProxyInterface->QueryInterface(riid, ppvObj);

	if (SUCCEEDED(hr))
	{
		genericQueryInterface(riid, ppvObj);
	}

	return hr;
}

ULONG m_IDirectSoundCaptureFXAec8::AddRef()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->AddRef();
}

ULONG m_IDirectSoundCaptureFXAec8::Release()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	ULONG x = ProxyInterface->Release();

	if (x == 0)
	{
		delete this;
	}

	return x;
}

// IDirectSoundCaptureFXAec methods
HRESULT m_IDirectSoundCaptureFXAec8::SetAllParameters(LPCDSCFXAec pDscFxAec)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->SetAllParameters(pDscFxAec);
}

HRESULT m_IDirectSoundCaptureFXAec8::GetAllParameters(LPDSCFXAec pDscFxAec)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->GetAllParameters(pDscFxAec);
}

HRESULT m_IDirectSoundCaptureFXAec8::GetStatus(LPDWORD pdwStatus)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->GetStatus(pdwStatus);
}

HRESULT m_IDirectSoundCaptureFXAec8::Reset()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->Reset();
}
