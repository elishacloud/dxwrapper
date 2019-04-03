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

HRESULT m_IDirectSoundFXI3DL2Reverb8::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
	if ((riid == IID_IDirectSoundFXI3DL2Reverb || riid == IID_IDirectSoundFXI3DL2Reverb8 || riid == IID_IUnknown) && ppvObj)
	{
		AddRef();

		*ppvObj = this;

		return S_OK;
	}

	HRESULT hr = ProxyInterface->QueryInterface(riid, ppvObj);

	if (SUCCEEDED(hr))
	{
		DsoundQueryInterface(riid, ppvObj);
	}

	return hr;
}

ULONG m_IDirectSoundFXI3DL2Reverb8::AddRef()
{
	return ProxyInterface->AddRef();
}

ULONG m_IDirectSoundFXI3DL2Reverb8::Release()
{
	ULONG x = ProxyInterface->Release();

	if (x == 0)
	{
		delete this;
	}

	return x;
}

// IDirectSoundFXI3DL2Reverb methods
HRESULT m_IDirectSoundFXI3DL2Reverb8::SetAllParameters(LPCDSFXI3DL2Reverb pcDsFxI3DL2Reverb)
{
	return ProxyInterface->SetAllParameters(pcDsFxI3DL2Reverb);
}

HRESULT m_IDirectSoundFXI3DL2Reverb8::GetAllParameters(LPDSFXI3DL2Reverb pDsFxI3DL2Reverb)
{
	return ProxyInterface->GetAllParameters(pDsFxI3DL2Reverb);
}

HRESULT m_IDirectSoundFXI3DL2Reverb8::SetPreset(DWORD dwPreset)
{
	return ProxyInterface->SetPreset(dwPreset);
}

HRESULT m_IDirectSoundFXI3DL2Reverb8::GetPreset(LPDWORD pdwPreset)
{
	return ProxyInterface->GetPreset(pdwPreset);
}

HRESULT m_IDirectSoundFXI3DL2Reverb8::SetQuality(LONG lQuality)
{
	return ProxyInterface->SetQuality(lQuality);
}

HRESULT m_IDirectSoundFXI3DL2Reverb8::GetQuality(LONG *plQuality)
{
	return ProxyInterface->GetQuality(plQuality);
}
