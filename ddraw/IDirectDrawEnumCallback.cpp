/**
* Copyright (C) 2018 Elisha Riedlinger
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

//************************************************************************
// IDirectDraw EnumDisplayModes
//************************************************************************

LPDDENUMMODESCALLBACK2 m_IDirectDrawEnumDisplayModes::lpCallback;
DWORD m_IDirectDrawEnumDisplayModes::DirectXVersion;
DWORD m_IDirectDrawEnumDisplayModes::ProxyDirectXVersion;

void m_IDirectDrawEnumDisplayModes::SetCallback(LPDDENUMMODESCALLBACK2 a, DWORD Version, DWORD ProxyVersion)
{
	EnterCriticalSection(&critSec);
	lpCallback = a;
	DirectXVersion = Version;
	ProxyDirectXVersion = ProxyVersion;
}

void m_IDirectDrawEnumDisplayModes::ReleaseCallback()
{
	ProxyDirectXVersion = 0;
	DirectXVersion = 0;
	lpCallback = nullptr;
	LeaveCriticalSection(&critSec);
}

HRESULT CALLBACK m_IDirectDrawEnumDisplayModes::ConvertCallback(LPDDSURFACEDESC2 lpDDSurfaceDesc, LPVOID lpContext)
{
	DDSURFACEDESC Desc;
	if (lpDDSurfaceDesc != nullptr && ProxyDirectXVersion > 3 && DirectXVersion < 4)
	{
		ConvertSurfaceDesc(Desc, *lpDDSurfaceDesc);
		lpDDSurfaceDesc = (LPDDSURFACEDESC2)&Desc;
	}

	return lpCallback(lpDDSurfaceDesc, lpContext);
}

//************************************************************************
// IDirectDraw EnumSurface
//************************************************************************

LPDDENUMSURFACESCALLBACK7 m_IDirectDrawEnumSurface::lpCallback;
DWORD m_IDirectDrawEnumSurface::DirectXVersion;
DWORD m_IDirectDrawEnumSurface::ProxyDirectXVersion;

void m_IDirectDrawEnumSurface::SetCallback(LPDDENUMSURFACESCALLBACK7 a, DWORD Version, DWORD ProxyVersion)
{
	EnterCriticalSection(&critSec);
	lpCallback = a;
	DirectXVersion = Version;
	ProxyDirectXVersion = ProxyVersion;
}

void m_IDirectDrawEnumSurface::ReleaseCallback()
{
	ProxyDirectXVersion = 0;
	DirectXVersion = 0;
	lpCallback = nullptr;
	LeaveCriticalSection(&critSec);
}

HRESULT CALLBACK m_IDirectDrawEnumSurface::ConvertCallback(LPDIRECTDRAWSURFACE7 lpDDSurface, LPDDSURFACEDESC2 lpDDSurfaceDesc, LPVOID lpContext)
{
	if (lpDDSurface)
	{
		lpDDSurface = ProxyAddressLookupTable.FindAddress<m_IDirectDrawSurface7>(lpDDSurface, DirectXVersion);
	}

	DDSURFACEDESC Desc;
	if (lpDDSurfaceDesc != nullptr && ProxyDirectXVersion > 3 && DirectXVersion < 4)
	{
		ConvertSurfaceDesc(Desc, *(LPDDSURFACEDESC2)lpDDSurfaceDesc);
		lpDDSurfaceDesc = (LPDDSURFACEDESC2)&Desc;
	}

	return lpCallback(lpDDSurface, lpDDSurfaceDesc, lpContext);
}
