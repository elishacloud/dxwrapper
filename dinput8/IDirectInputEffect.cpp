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

#include "dinput8.h"

HRESULT m_IDirectInputEffect8::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
	if ((riid == IID_IDirectInputEffect || riid == IID_IUnknown) && ppvObj)
	{
		AddRef();

		*ppvObj = this;

		return S_OK;
	}

	HRESULT hr = ProxyInterface->QueryInterface(riid, ppvObj);

	if (SUCCEEDED(hr))
	{
		dinput8QueryInterface(riid, ppvObj);
	}

	return hr;
}

ULONG m_IDirectInputEffect8::AddRef()
{
	return ProxyInterface->AddRef();
}

ULONG m_IDirectInputEffect8::Release()
{
	ULONG x = ProxyInterface->Release();

	if (x == 0)
	{
		delete this;
	}

	return x;
}

HRESULT m_IDirectInputEffect8::Initialize(HINSTANCE hinst, DWORD dwVersion, REFGUID rguid)
{
	return ProxyInterface->Initialize(hinst, dwVersion, rguid);
}

HRESULT m_IDirectInputEffect8::GetEffectGuid(LPGUID pguid)
{
	return ProxyInterface->GetEffectGuid(pguid);
}

HRESULT m_IDirectInputEffect8::GetParameters(LPDIEFFECT peff, DWORD dwFlags)
{
	return ProxyInterface->GetParameters(peff, dwFlags);
}

HRESULT m_IDirectInputEffect8::SetParameters(LPCDIEFFECT peff, DWORD dwFlags)
{
	return ProxyInterface->SetParameters(peff, dwFlags);
}

HRESULT m_IDirectInputEffect8::Start(DWORD dwIterations, DWORD dwFlags)
{
	return ProxyInterface->Start(dwIterations, dwFlags);
}

HRESULT m_IDirectInputEffect8::Stop()
{
	return ProxyInterface->Stop();
}

HRESULT m_IDirectInputEffect8::GetEffectStatus(LPDWORD pdwFlags)
{
	return ProxyInterface->GetEffectStatus(pdwFlags);
}

HRESULT m_IDirectInputEffect8::Download()
{
	return ProxyInterface->Download();
}

HRESULT m_IDirectInputEffect8::Unload()
{
	return ProxyInterface->Unload();
}

HRESULT m_IDirectInputEffect8::Escape(LPDIEFFESCAPE pesc)
{
	return ProxyInterface->Escape(pesc);
}
