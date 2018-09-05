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

HRESULT CALLBACK m_IDirectDrawEnumDisplayModes::ConvertCallback(LPDDSURFACEDESC2 lpDDSurfaceDesc2, LPVOID lpContext)
{
	if (!lpContext || !lpDDSurfaceDesc2)
	{
		Logging::Log() << __FUNCTION__ << " Error: invaid context!";
		return DDENUMRET_CANCEL;
	}

	ENUMDISPLAYMODES *lpCallbackContext = (ENUMDISPLAYMODES*)lpContext;

	if (!lpCallbackContext->lpCallback)
	{
		Logging::Log() << __FUNCTION__ << " Error: invaid callback!";
		return DDENUMRET_CANCEL;
	}

	DDSURFACEDESC Desc;
	ConvertSurfaceDesc(Desc, *lpDDSurfaceDesc2);

	return lpCallbackContext->lpCallback(&Desc, lpCallbackContext->lpContext);
}

HRESULT CALLBACK m_IDirectDrawEnumSurface::ConvertCallback(LPDIRECTDRAWSURFACE lpDDSurface, LPDDSURFACEDESC lpDDSurfaceDesc, LPVOID lpContext)
{
	if (!lpContext)
	{
		Logging::Log() << __FUNCTION__ << " Error: invaid context!";
		return DDENUMRET_CANCEL;
	}

	ENUMSURFACE *lpCallbackContext = (ENUMSURFACE*)lpContext;

	if (!lpCallbackContext->lpCallback)
	{
		Logging::Log() << __FUNCTION__ << " Error: invaid callback!";
		return DDENUMRET_CANCEL;
	}

	if (lpDDSurface)
	{
		lpDDSurface = (LPDIRECTDRAWSURFACE)ProxyAddressLookupTable.FindAddress<m_IDirectDrawSurface7>(lpDDSurface, lpCallbackContext->DirectXVersion);
	}

	return lpCallbackContext->lpCallback(lpDDSurface, lpDDSurfaceDesc, lpCallbackContext->lpContext);
}

HRESULT CALLBACK m_IDirectDrawEnumSurface::ConvertCallback2(LPDIRECTDRAWSURFACE7 lpDDSurface, LPDDSURFACEDESC2 lpDDSurfaceDesc2, LPVOID lpContext)
{
	if (!lpContext)
	{
		Logging::Log() << __FUNCTION__ << " Error: invaid context!";
		return DDENUMRET_CANCEL;
	}

	ENUMSURFACE *lpCallbackContext = (ENUMSURFACE*)lpContext;

	if (!lpCallbackContext->lpCallback7)
	{
		Logging::Log() << __FUNCTION__ << " Error: invaid callback!";
		return DDENUMRET_CANCEL;
	}

	if (lpDDSurface)
	{
		lpDDSurface = ProxyAddressLookupTable.FindAddress<m_IDirectDrawSurface7>(lpDDSurface, lpCallbackContext->DirectXVersion);
	}

	// Game using old DirectX, Convert back to LPDDSURFACEDESC
	if (lpDDSurfaceDesc2 && lpCallbackContext->ConvertSurfaceDescTo2)
	{
		DDSURFACEDESC Desc;
		ConvertSurfaceDesc(Desc, *lpDDSurfaceDesc2);

		return ((LPDDENUMSURFACESCALLBACK)lpCallbackContext->lpCallback7)((LPDIRECTDRAWSURFACE)lpDDSurface, &Desc, lpCallbackContext->lpContext);
	}

	return lpCallbackContext->lpCallback7(lpDDSurface, lpDDSurfaceDesc2, lpCallbackContext->lpContext);
}
