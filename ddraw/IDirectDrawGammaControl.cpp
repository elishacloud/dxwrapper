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

#include "ddraw.h"

// Cached wrapper interface
namespace {
	m_IDirectDrawGammaControl* WrapperInterfaceBackup = nullptr;
}

inline static void SaveInterfaceAddress(m_IDirectDrawGammaControl* Interface, m_IDirectDrawGammaControl*& InterfaceBackup)
{
	if (Interface)
	{
		SetCriticalSection();
		Interface->SetProxy(nullptr, nullptr);
		if (InterfaceBackup)
		{
			InterfaceBackup->DeleteMe();
			InterfaceBackup = nullptr;
		}
		InterfaceBackup = Interface;
		ReleaseCriticalSection();
	}
}

m_IDirectDrawGammaControl* CreateDirectDrawGammaControl(IDirectDrawGammaControl* aOriginal, m_IDirectDrawX* NewParent)
{
	SetCriticalSection();
	m_IDirectDrawGammaControl* Interface = nullptr;
	if (WrapperInterfaceBackup)
	{
		Interface = WrapperInterfaceBackup;
		WrapperInterfaceBackup = nullptr;
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
	ReleaseCriticalSection();
	return Interface;
}

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

	if (!ProxyInterface)
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
		SaveInterfaceAddress(this, WrapperInterfaceBackup);
	}

	return ref;
}

HRESULT m_IDirectDrawGammaControl::GetGammaRamp(DWORD dwFlags, LPDDGAMMARAMP lpRampData)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface && !ddrawParent)
	{
		return DDERR_INVALIDOBJECT;
	}

	if (!ProxyInterface)
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

	if (!ProxyInterface)
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

/************************/
/*** Helper functions ***/
/************************/

void m_IDirectDrawGammaControl::InitInterface()
{
	// Initialize gamma control
}

void m_IDirectDrawGammaControl::ReleaseInterface()
{
	if (Config.Exiting)
	{
		return;
	}

	if (ddrawParent)
	{
		ddrawParent->ClearGammaInterface();
	}
}
