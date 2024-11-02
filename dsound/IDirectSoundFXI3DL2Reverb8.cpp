/**
* Copyright (C) 2024 Elisha Riedlinger
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

HRESULT m_IDirectSoundFXI3DL2Reverb8::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ppvObj)
	{
		return E_POINTER;
	}

	if (riid == IID_IDirectSoundFXI3DL2Reverb8 || riid == IID_IUnknown)
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

ULONG m_IDirectSoundFXI3DL2Reverb8::AddRef()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->AddRef();
}

ULONG m_IDirectSoundFXI3DL2Reverb8::Release()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	ULONG x = ProxyInterface->Release();

	if (x == 0)
	{
		delete this;
	}

	return x;
}

// IDirectSoundFXI3DL2Reverb methods
HRESULT m_IDirectSoundFXI3DL2Reverb8::SetAllParameters(_In_ LPCDSFXI3DL2Reverb pcDsFxI3DL2Reverb)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->SetAllParameters(pcDsFxI3DL2Reverb);
}

HRESULT m_IDirectSoundFXI3DL2Reverb8::GetAllParameters(_Out_ LPDSFXI3DL2Reverb pDsFxI3DL2Reverb)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->GetAllParameters(pDsFxI3DL2Reverb);
}

HRESULT m_IDirectSoundFXI3DL2Reverb8::SetPreset(DWORD dwPreset)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->SetPreset(dwPreset);
}

HRESULT m_IDirectSoundFXI3DL2Reverb8::GetPreset(_Out_ LPDWORD pdwPreset)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->GetPreset(pdwPreset);
}

HRESULT m_IDirectSoundFXI3DL2Reverb8::SetQuality(LONG lQuality)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->SetQuality(lQuality);
}

HRESULT m_IDirectSoundFXI3DL2Reverb8::GetQuality(_Out_ LONG *plQuality)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->GetQuality(plQuality);
}
