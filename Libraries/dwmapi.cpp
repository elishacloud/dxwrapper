/**
* Copyright (C) 2026 Elisha Riedlinger
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

#include "Utils\Utils.h"
#include "dwmapi.h"
#include "Logging\Logging.h"

typedef HRESULT(WINAPI* PFN_DwmFlush)(void);
typedef HRESULT(WINAPI* PFN_DwmSetWindowAttribute)(HWND hwnd, DWORD dwAttribute, LPCVOID pvAttribute, DWORD cbAttribute);
typedef HRESULT(WINAPI* PFN_DwmGetCompositionTimingInfo)(HWND hwnd, DWM_TIMING_INFO* pTimingInfo);
PFN_DwmFlush DwmFlushPtr = nullptr;
PFN_DwmSetWindowAttribute DwmSetWindowAttributePtr = nullptr;
PFN_DwmGetCompositionTimingInfo DwmGetCompositionTimingInfoPtr = nullptr;
HMODULE dwmapiModule = nullptr;

static void Loaddwmapi()
{
	static bool IsLoaded = false;
	if (IsLoaded)
	{
		return; // Only load the dll once
	}
	IsLoaded = true;
	dwmapiModule = LoadLibrary("dwmapi.dll");
	if (dwmapiModule)
	{
		DwmFlushPtr = reinterpret_cast<PFN_DwmFlush>(GetProcAddress(dwmapiModule, "DwmFlush"));
		if (!DwmFlushPtr)
		{
			Logging::Log() << "Failed to get 'DwmFlush' ProcAddress of dwmapi.dll!";
		}
		DwmSetWindowAttributePtr = reinterpret_cast<PFN_DwmSetWindowAttribute>(GetProcAddress(dwmapiModule, "DwmSetWindowAttribute"));
		if (!DwmSetWindowAttributePtr)
		{
			Logging::Log() << "Failed to get 'DwmSetWindowAttribute' ProcAddress of dwmapi.dll!";
		}
		DwmGetCompositionTimingInfoPtr = reinterpret_cast<PFN_DwmGetCompositionTimingInfo>(GetProcAddress(dwmapiModule, "DwmGetCompositionTimingInfo"));
		if (!DwmGetCompositionTimingInfoPtr)
		{
			Logging::Log() << "Failed to get 'DwmGetCompositionTimingInfo' ProcAddress of dwmapi.dll!";
		}
	}
	else
	{
		Logging::Log() << "Failed to load dwmapi.dll!";
	}
}

HRESULT DwmFlush()
{
	// Load module
	Loaddwmapi();

	// Call function
	if (DwmFlushPtr)
	{
		return DwmFlushPtr();
	}
	return E_FAIL;
}

HRESULT DwmSetWindowAttribute(HWND hwnd, DWORD dwAttribute, LPCVOID pvAttribute, DWORD cbAttribute)
{
	// Load module
	Loaddwmapi();

	// Call function
	if (DwmSetWindowAttributePtr)
	{
		return DwmSetWindowAttributePtr(hwnd, dwAttribute, pvAttribute, cbAttribute);
	}
	return E_FAIL;
}

HRESULT DwmGetCompositionTimingInfo(HWND hwnd, DWM_TIMING_INFO* pTimingInfo)
{
	// Load module
	Loaddwmapi();

	// Call function
	if (DwmGetCompositionTimingInfoPtr)
	{
		return DwmGetCompositionTimingInfoPtr(hwnd, pTimingInfo);
	}
	return E_FAIL;
}
