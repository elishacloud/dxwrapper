/**
* Copyright (C) 2017 Elisha Riedlinger
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

#include "Settings\Settings.h"
#include "d3d8.h"
#include "d3d9.h"
#include "ddraw.h"
#include "dplayx.h"
#include "dsound.h"
#include "dinput.h"
#include "dxgi.h"
#include "winmm.h"
#include "bcrypt.h"
#include "uxtheme.h"
#include "dwmapi.h"

struct custom_dll
{
	bool Flag = false;
	HMODULE dll = nullptr;
} custom[256];

custom_dll dllhandle[dtypeArraySize];

// Load real dll file that is being wrapped
HMODULE LoadDll(DWORD dlltype)
{
	// Check for valid dlltype
	if (dlltype == 0 || dlltype >= dtypeArraySize) return nullptr;

	// Check if dll is already loaded
	if (dllhandle[dlltype].Flag) return dllhandle[dlltype].dll;
	dllhandle[dlltype].Flag = true;

	// Load dll from ini, if DllPath is not '0'
	if (Config.szDllPath[0] != '\0' && Config.RealWrapperMode == dlltype)
	{
		Compat::Log() << "Loading " << Config.szDllPath << " library";
		dllhandle[dlltype].dll = LoadLibrary(Config.szDllPath);
		if (!dllhandle[dlltype].dll) Compat::Log() << "Cannot load " << Config.szDllPath << " library";
	}

	// Load current dll
	if (!dllhandle[dlltype].dll && Config.RealWrapperMode != dlltype)
	{
		Compat::Log() << "Loading " << dtypename[dlltype] << " library";
		dllhandle[dlltype].dll = LoadLibrary(dtypename[dlltype]);
		if (!dllhandle[dlltype].dll) Compat::Log() << "Cannot load " << dtypename[dlltype] << " library";
	}

	// Load default system dll
	if (!dllhandle[dlltype].dll)
	{
		char path[MAX_PATH];
		GetSystemDirectory(path, MAX_PATH);
		strcat_s(path, MAX_PATH, "\\");
		strcat_s(path, MAX_PATH, dtypename[dlltype]);
		Compat::Log() << "Loading " << path << " library";
		dllhandle[dlltype].dll = LoadLibrary(path);
	}

	// Cannot load dll
	if (!dllhandle[dlltype].dll)
	{
		Compat::Log() << "Cannot load " << dtypename[dlltype] << " library";
		if (Config.WrapperMode != 0 && Config.WrapperMode != 255) ExitProcess(0);
	}

	// Return dll handle
	return dllhandle[dlltype].dll;
}

// Load custom dll files
void LoadCustomDll()
{
	for (UINT x = 1; x <= Config.CustomDllCount; ++x)
	{
		if (Config.szCustomDllPath[x] != '\0')
		{
			Compat::Log() << "Loading custom " << Config.szCustomDllPath[x] << " library";
			// Load dll from ini
			custom[x].dll = LoadLibrary(Config.szCustomDllPath[x]);
			// Load from system
			if (!custom[x].dll)
			{
				char path[MAX_PATH];
				GetSystemDirectory(path, MAX_PATH);
				strcat_s(path, MAX_PATH, "\\");
				strcat_s(path, MAX_PATH, Config.szCustomDllPath[x]);
				custom[x].dll = LoadLibrary(path);
			}
			// Cannot load dll
			if (!custom[x].dll)
			{
				Compat::Log() << "Cannot load custom " << Config.szCustomDllPath[x] << " library";
			}
			else {
				custom[x].Flag = true;
			}
		}
	}
}

// Unload custom dll files
void FreeCustomLibrary()
{
	for (UINT x = 1; x <= Config.CustomDllCount; ++x)
	{
		// If dll was loaded
		if (custom[x].Flag)
		{
			// Unload dll
			FreeLibrary(custom[x].dll);
		}
	}
}

// Load wrapper dll files
void DllAttach()
{
	if (Config.WrapperMode == dtype.bcrypt || Config.WrapperMode == 0 || Config.WrapperMode == 255) LoadBcrypt();
	if (Config.WrapperMode == dtype.dplayx || Config.WrapperMode == 0 || Config.WrapperMode == 255) LoadDplayx();
	if (Config.WrapperMode == dtype.d3d8 || Config.WrapperMode == 0 || Config.WrapperMode == 255 || Config.D3d8to9) LoadD3d8();
	if (Config.WrapperMode == dtype.dxgi || Config.WrapperMode == 0 || Config.WrapperMode == 255) LoadDxgi();
	if (Config.WrapperMode == dtype.winmm || Config.WrapperMode == 0 || Config.WrapperMode == 255) LoadWinmm();
	if (Config.WrapperMode == dtype.dsound || Config.WrapperMode == 0 || Config.WrapperMode == 255 || Config.DSoundCtrl) LoadDsound();
	if (Config.WrapperMode == dtype.dinput || Config.WrapperMode == 0 || Config.WrapperMode == 255) LoadDinput();
	if (Config.WrapperMode == dtype.d3d9 || Config.WrapperMode == 0 || Config.WrapperMode == 255) LoadD3d9();
	if (Config.WrapperMode == dtype.ddraw || Config.WrapperMode == 0 || Config.WrapperMode == 255 || Config.DDrawCompat) LoadDdraw();
	if (Config.CustomDllCount > 0) LoadCustomDll();
}

// Unload all dll files loaded by the wrapper
void DllDetach()
{
	// Unload custom libraries
	FreeCustomLibrary();

	// Unload wrapper libraries
	for (UINT x = 1; x < dtypeArraySize; ++x)
	{
		// If dll was loaded
		if (dllhandle[x].dll)
		{
			// Unload dll
			FreeLibrary(dllhandle[x].dll);
		}
	}

	// Unload dynmaic libraries
	UnLoaddwmapi();
	UnLoadUxtheme();
}