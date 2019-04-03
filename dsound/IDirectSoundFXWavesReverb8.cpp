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

HRESULT m_IDirectSoundFXWavesReverb8::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
	if ((riid == IID_IDirectSoundFXWavesReverb || riid == IID_IDirectSoundFXWavesReverb8 || riid == IID_IUnknown) && ppvObj)
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

ULONG m_IDirectSoundFXWavesReverb8::AddRef()
{
	return ProxyInterface->AddRef();
}

ULONG m_IDirectSoundFXWavesReverb8::Release()
{
	ULONG x = ProxyInterface->Release();

	if (x == 0)
	{
		delete this;
	}

	return x;
}

// IDirectSoundFXWavesReverb methods
HRESULT m_IDirectSoundFXWavesReverb8::SetAllParameters(LPCDSFXWavesReverb pcDsFxWavesReverb)
{
	return ProxyInterface->SetAllParameters(pcDsFxWavesReverb);
}

HRESULT m_IDirectSoundFXWavesReverb8::GetAllParameters(LPDSFXWavesReverb pDsFxWavesReverb)
{
	return ProxyInterface->GetAllParameters(pDsFxWavesReverb);
}
