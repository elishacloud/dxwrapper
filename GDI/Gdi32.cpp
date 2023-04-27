/**
* Copyright (C) 2022 Elisha Riedlinger
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
#include "ddraw\ddrawExternal.h"
#include "GDI.h"
#include "Settings\Settings.h"
#include "Logging\Logging.h"

namespace GdiWrapper
{
	FARPROC GetDeviceCaps_out = nullptr;
}

using namespace GdiWrapper;

int WINAPI gdi_GetDeviceCaps(HDC hdc, int index)
{
	Logging::LogDebug() << __FUNCTION__ << " " << hdc << " " << index;

	static GetDeviceCapsProc m_pGetDeviceCaps = (Wrapper::ValidProcAddress(GetDeviceCaps_out)) ? (GetDeviceCapsProc)GetDeviceCaps_out : nullptr;

	if (index == BITSPIXEL)
	{
		switch (GetDDrawBitsPixel())
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

	if (!m_pGetDeviceCaps)
	{
		return 0;
	}

	return m_pGetDeviceCaps(hdc, index);
}
