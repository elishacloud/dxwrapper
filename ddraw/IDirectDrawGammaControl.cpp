/**
* Copyright (C) 2020 Elisha Riedlinger
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

HRESULT m_IDirectDrawGammaControl::QueryInterface(REFIID riid, LPVOID FAR * ppvObj)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ") " << riid;

	if (!ppvObj)
	{
		return E_POINTER;
	}

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

	if (!ProxyInterface)
	{
		return InterlockedIncrement(&RefCount);
	}

	return ProxyInterface->AddRef();
}

ULONG m_IDirectDrawGammaControl::Release()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

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
		delete this;
	}

	return ref;
}

HRESULT m_IDirectDrawGammaControl::GetGammaRamp(DWORD dwFlags, LPDDGAMMARAMP lpRampData)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface)
	{
		if (!lpRampData)
		{
			return DDERR_INVALIDPARAMS;
		}

		if (dwFlags & DDSGR_CALIBRATE)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Calibrating gamma ramps is not Implemented");
		}

		ConvertGammaRamp(*lpRampData, RampData);

		return DD_OK;
	}

	return ProxyInterface->GetGammaRamp(dwFlags, lpRampData);
}

HRESULT m_IDirectDrawGammaControl::SetGammaRamp(DWORD dwFlags, LPDDGAMMARAMP lpRampData)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface)
	{
		if (!(dwFlags || dwFlags == DDSGR_CALIBRATE) || !lpRampData)
		{
			return DDERR_INVALIDPARAMS;
		}

		ConvertGammaRamp(RampData, *lpRampData);

		// Present new gamma setting
		if (ddrawParent)
		{
			ddrawParent->SetVsync();

			m_IDirectDrawSurfaceX *lpDDSrcSurfaceX = ddrawParent->GetPrimarySurface();
			if (lpDDSrcSurfaceX)
			{
				lpDDSrcSurfaceX->PresentSurface();
			}
		}

		return DD_OK;
	}

	return ProxyInterface->SetGammaRamp(dwFlags, lpRampData);
}

/************************/
/*** Helper functions ***/
/************************/

void m_IDirectDrawGammaControl::InitGammaControl()
{
	// To add later
}

void m_IDirectDrawGammaControl::ReleaseGammaControl()
{
	if (ddrawParent && !Config.Exiting)
	{
		ddrawParent->ClearGammaInterface();
	}
}
