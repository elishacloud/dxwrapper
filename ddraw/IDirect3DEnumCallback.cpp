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

HRESULT CALLBACK m_IDirect3DEnumDevices::ConvertCallback(LPSTR lpDeviceDescription, LPSTR lpDeviceName, LPD3DDEVICEDESC7 lpDeviceDesc, LPVOID lpContext)
{
	if (!lpContext || !lpDeviceDesc)
	{
		Logging::Log() << __FUNCTION__ << " Error: invaid context!";
		return DDENUMRET_CANCEL;
	}

	ENUMDEVICES *lpCallbackContext = (ENUMDEVICES*)lpContext;

	if (!lpCallbackContext->lpCallback)
	{
		Logging::Log() << __FUNCTION__ << " Error: invaid callback!";
		return DDENUMRET_CANCEL;
	}

	D3DDEVICEDESC D3DHWDevDesc, D3DHELDevDesc;
	ConvertDeviceDesc(D3DHWDevDesc, *lpDeviceDesc);
	ConvertDeviceDesc(D3DHELDevDesc, *lpDeviceDesc);

	return lpCallbackContext->lpCallback(&lpDeviceDesc->deviceGUID, lpDeviceDescription, lpDeviceName, &D3DHWDevDesc, &D3DHELDevDesc, lpCallbackContext->lpContext);
}
