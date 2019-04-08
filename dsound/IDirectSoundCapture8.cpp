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

HRESULT m_IDirectSoundCapture8::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
	if ((riid == IID_IDirectSoundCapture || riid == IID_IDirectSoundCapture8 || riid == IID_IUnknown) && ppvObj)
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

ULONG m_IDirectSoundCapture8::AddRef()
{
	return ProxyInterface->AddRef();
}

ULONG m_IDirectSoundCapture8::Release()
{
	ULONG x = ProxyInterface->Release();

	if (x == 0)
	{
		delete this;
	}

	return x;
}

// IDirectSoundCapture methods
HRESULT m_IDirectSoundCapture8::CreateCaptureBuffer(LPCDSCBUFFERDESC pcDSCBufferDesc, LPDIRECTSOUNDCAPTUREBUFFER *ppDSCBuffer, LPUNKNOWN pUnkOuter)
{
	HRESULT hr = ProxyInterface->CreateCaptureBuffer(pcDSCBufferDesc, ppDSCBuffer, pUnkOuter);

	if (SUCCEEDED(hr) && ppDSCBuffer)
	{
		*ppDSCBuffer = ProxyAddressLookupTableDsound.FindAddress<m_IDirectSoundCaptureBuffer8>(*ppDSCBuffer);
	}

	return hr;
}

HRESULT m_IDirectSoundCapture8::GetCaps(LPDSCCAPS pDSCCaps)
{
	return ProxyInterface->GetCaps(pDSCCaps);
}

HRESULT m_IDirectSoundCapture8::Initialize(LPCGUID pcGuidDevice)
{
	return ProxyInterface->Initialize(pcGuidDevice);
}
