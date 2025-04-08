/**
* Copyright (C) 2025 Elisha Riedlinger
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

// ******************************
// IUnknown functions
// ******************************

HRESULT m_IDirectDrawGammaControl::QueryInterface(REFIID riid, LPVOID FAR * ppvObj)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ") " << riid;

	if (!ProxyInterface && !ddrawParent)
	{
		if (ppvObj)
		{
			*ppvObj = nullptr;
		}
		return E_NOINTERFACE;
	}

	if (!ppvObj)
	{
		return E_POINTER;
	}
	*ppvObj = nullptr;

	if (riid == IID_GetRealInterface)
	{
		*ppvObj = ProxyInterface;
		return DD_OK;
	}
	if (ppvObj && riid == IID_GetInterfaceX)
	{
		*ppvObj = this;
		return DD_OK;
	}

	if (riid == IID_IDirectDrawGammaControl || riid == IID_IUnknown)
	{
		AddRef();

		*ppvObj = this;

		return DD_OK;
	}

	return ProxyQueryInterface(ProxyInterface, riid, ppvObj, WrapperID);
}

ULONG m_IDirectDrawGammaControl::AddRef()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface && !ddrawParent)
	{
		return 0;
	}

	if (Config.Dd7to9)
	{
		return InterlockedIncrement(&RefCount);
	}

	return ProxyInterface->AddRef();
}

ULONG m_IDirectDrawGammaControl::Release()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface && !ddrawParent)
	{
		return 0;
	}

	if (Config.Dd7to9)
	{
		ULONG ref = (InterlockedCompareExchange(&RefCount, 0, 0)) ? InterlockedDecrement(&RefCount) : 0;

		if (ref == 0)
		{
			SaveInterfaceAddress(this);
		}

		return ref;
	}

	ULONG ref = ProxyInterface->Release();

	if (ref == 0)
	{
		SaveInterfaceAddress(this);
	}

	return ref;
}

// ******************************
// IDirectDrawGammaControl functions
// ******************************

HRESULT m_IDirectDrawGammaControl::GetGammaRamp(DWORD dwFlags, LPDDGAMMARAMP lpRampData)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface && !ddrawParent)
	{
		return DDERR_INVALIDOBJECT;
	}

	if (Config.Dd7to9)
	{
		if (!(dwFlags == D3DSGR_NO_CALIBRATION || dwFlags == DDSGR_CALIBRATE) || !lpRampData)
		{
			return DDERR_INVALIDPARAMS;
		}

		if (!ddrawParent)
		{
			return DDERR_INVALIDOBJECT;
		}

		return ddrawParent->GetD9Gamma(dwFlags, lpRampData);
	}

	return ProxyInterface->GetGammaRamp(dwFlags, lpRampData);
}

HRESULT m_IDirectDrawGammaControl::SetGammaRamp(DWORD dwFlags, LPDDGAMMARAMP lpRampData)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface && !ddrawParent)
	{
		return DDERR_INVALIDOBJECT;
	}

	if (Config.Dd7to9)
	{
		if (!(dwFlags == D3DSGR_NO_CALIBRATION || dwFlags == DDSGR_CALIBRATE) || !lpRampData)
		{
			return DDERR_INVALIDPARAMS;
		}

		if (!ddrawParent)
		{
			return DDERR_INVALIDOBJECT;
		}

		return ddrawParent->SetD9Gamma(dwFlags, lpRampData);
	}

	return ProxyInterface->SetGammaRamp(dwFlags, lpRampData);
}

// ******************************
// Helper functions
// ******************************

void m_IDirectDrawGammaControl::InitInterface()
{
	ScopedDDCriticalSection ThreadLockDD;

	if (ddrawParent)
	{
		ddrawParent->SetGammaControl(this);
	}
}

void m_IDirectDrawGammaControl::ReleaseInterface()
{
	if (Config.Exiting)
	{
		return;
	}

	ScopedDDCriticalSection ThreadLockDD;

	if (ddrawParent)
	{
		ddrawParent->ClearGammaControl(this);
	}
}

m_IDirectDrawGammaControl* m_IDirectDrawGammaControl::CreateDirectDrawGammaControl(IDirectDrawGammaControl* aOriginal, m_IDirectDrawX* NewParent)
{
	m_IDirectDrawGammaControl* Interface = InterfaceAddressCache<m_IDirectDrawGammaControl>(nullptr);
	if (Interface)
	{
		Interface->SetProxy(aOriginal, NewParent);
	}
	else
	{
		if (aOriginal)
		{
			Interface = new m_IDirectDrawGammaControl(aOriginal);
		}
		else
		{
			Interface = new m_IDirectDrawGammaControl(NewParent);
		}
	}
	return Interface;
}
