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

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "ddraw\ddraw.h"
#include "GDI.h"
#include "Settings\Settings.h"
#include "Logging\Logging.h"

namespace GdiWrapper
{
	INITIALIZE_OUT_WRAPPED_PROC(GetDeviceCaps, unused);
	INITIALIZE_OUT_WRAPPED_PROC(SetDeviceGammaRamp, unused);
}

using namespace GdiWrapper;

int WINAPI gdi_GetDeviceCaps(HDC hdc, int index)
{
	Logging::LogDebug() << __FUNCTION__ << " " << WindowFromDC(hdc) << " " << index;

	DEFINE_STATIC_PROC_ADDRESS(GetDeviceCapsProc, GetDeviceCaps, GetDeviceCaps_out);

	if (index == BITSPIXEL)
	{
		switch (m_IDirectDrawX::GetDDrawBitsPixel(WindowFromDC(hdc)))
		{
		case 8:
			return 8;
		case 15:
		case 16:
			return 16;		// When nIndex is BITSPIXEL and the device has 15bpp or 16bpp, the return value is 16.
		case 24:
			return 24;
		case 32:
			return 32;
		}
	}

	if (!GetDeviceCaps)
	{
		return 0;
	}

	return GetDeviceCaps(hdc, index);
}

BOOL WINAPI gdi_SetDeviceGammaRamp(HDC hdc, LPVOID lpRamp)
{
	Logging::LogDebug() << __FUNCTION__ << " " << WindowFromDC(hdc) << " " << lpRamp;

	DEFINE_STATIC_PROC_ADDRESS(SetDeviceGammaRampProc, SetDeviceGammaRamp, SetDeviceGammaRamp_out);

	if (Config.DisableGDIGammaRamp)
	{
		return TRUE;
	}

	return SetDeviceGammaRamp(hdc, lpRamp);
}
