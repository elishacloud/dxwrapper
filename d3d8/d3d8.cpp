/**
* Copyright (C) 2023 Elisha Riedlinger
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
*
* ValidatePixelShader and ValidateVertexShader created from source code found in Wine
* https://gitlab.winehq.org/wine/wine/-/tree/master/dlls/d3d8
*/

#include "d3d8External.h"
#include "d3d9\d3d9External.h"
#include "External\d3d8to9\source\d3d8to9.hpp"
#include "External\d3d8to9\source\d3dx9.hpp"
#include "Settings\Settings.h"
#include "Logging\Logging.h"
#include "BuildNo.rc"

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define APP_VERSION TOSTRING(FILEVERSION)

typedef int(WINAPI* Direct3D8EnableMaximizedWindowedModeShimProc)(BOOL);

extern FARPROC f_D3DXAssembleShader;
extern FARPROC f_D3DXDisassembleShader;
extern FARPROC f_D3DXLoadSurfaceFromSurface;

PFN_D3DXAssembleShader D3DXAssembleShader = (PFN_D3DXAssembleShader)f_D3DXAssembleShader;
PFN_D3DXDisassembleShader D3DXDisassembleShader = (PFN_D3DXDisassembleShader)f_D3DXDisassembleShader;
PFN_D3DXLoadSurfaceFromSurface D3DXLoadSurfaceFromSurface = (PFN_D3DXLoadSurfaceFromSurface)f_D3DXLoadSurfaceFromSurface;

namespace D3d8Wrapper
{
	FARPROC Direct3DCreate9_out = nullptr;
}

using namespace D3d8Wrapper;

HMODULE GetSystemD3d8()
{
	static HMODULE h_d3d8 = nullptr;

	// Get System d3d8.dll
	if (!h_d3d8)
	{
		char Path[MAX_PATH] = {};
		GetSystemDirectoryA(Path, MAX_PATH);
		strcat_s(Path, "\\d3d8.dll");
		GetModuleHandleExA(NULL, Path, &h_d3d8);
	}

	return h_d3d8;
}

HRESULT WINAPI d8_ValidatePixelShader(const DWORD* pPixelShader, const D3DCAPS8* pCaps, BOOL ErrorsFlag, char** Errors)
{
	LOG_LIMIT(1, __FUNCTION__);

	HRESULT hr = E_FAIL;
	char* message = "";

	if (!pPixelShader)
	{
		message = "Invalid code pointer.\n";
	}
	else
	{
		switch (*pPixelShader)
		{
		case D3DPS_VERSION(1, 0):
		case D3DPS_VERSION(1, 1):
		case D3DPS_VERSION(1, 2):
		case D3DPS_VERSION(1, 3):
		case D3DPS_VERSION(1, 4):
			if (pCaps && *pPixelShader > pCaps->PixelShaderVersion)
			{
				message = "Shader version not supported by caps.\n";
				break;
			}
			hr = D3D_OK;
			break;
		default:
			message = "Unsupported shader version.\n";
		}
	}

	if (!ErrorsFlag)
	{
		message = "";
	}

	const size_t size = strlen(message) + 1;
	if (Errors)
	{
		*Errors = (char*)HeapAlloc(GetProcessHeap(), 0, size);
		if (*Errors)
		{
			memcpy(*Errors, message, size);
		}
	}

	return hr;
}

HRESULT WINAPI d8_ValidateVertexShader(const DWORD* pVertexShader, const DWORD* pDeclaration, const D3DCAPS8* pCaps, BOOL ErrorsFlag, char** Errors)
{
	LOG_LIMIT(1, __FUNCTION__);

	UNREFERENCED_PARAMETER(pDeclaration);

	HRESULT hr = E_FAIL;
	char* message = "";

	if (!pVertexShader)
	{
		message = "Invalid code pointer.\n";
	}
	else
	{
		switch (*pVertexShader)
		{
		case D3DVS_VERSION(1, 0):
		case D3DVS_VERSION(1, 1):
			if (pCaps && *pVertexShader > pCaps->VertexShaderVersion)
			{
				message = "Shader version not supported by caps.\n";
				break;
			}
			hr = D3D_OK;
			break;
		default:
			message = "Unsupported shader version.\n";
		}
	}

	if (!ErrorsFlag)
	{
		message = "";
	}

	const size_t size = strlen(message) + 1;
	if (Errors)
	{
		*Errors = (char*)HeapAlloc(GetProcessHeap(), 0, size);
		if (*Errors)
		{
			memcpy(*Errors, message, size);
		}
	}

	return hr;
}

BOOL Direct3D8DisableMaximizedWindowedMode()
{
	static Direct3D8EnableMaximizedWindowedModeShimProc d8_Direct3D8EnableMaximizedWindowedModeShim = nullptr;

	if (!d8_Direct3D8EnableMaximizedWindowedModeShim)
	{
		// Load d3d8.dll from System32
		static HMODULE dll = GetSystemD3d8();

		if (!dll)
		{
			Logging::Log() << __FUNCTION__ << " d3d8.dll is not loaded!";
			return FALSE;
		}

		// Get function address
		BYTE* addr = (BYTE*)GetProcAddress(dll, "Direct3D8EnableMaximizedWindowedModeShim");
		if (!addr)
		{
			Logging::Log() << __FUNCTION__ << " Error: Failed to get `Direct3D8EnableMaximizedWindowedModeShim` address!";
			return FALSE;
		}

		// Check memory address
		if (addr[0] != 0xC7 || addr[1] != 0x05 || *(DWORD*)(addr + 6) != 1)
		{
			Logging::Log() << __FUNCTION__ << " Error: Failed to vaidate memory address!";
			return FALSE;
		}

		// Update function to disable Maximized Windowed Mode
		DWORD Protect;
		BOOL ret = VirtualProtect((LPVOID)(addr + 6), 4, PAGE_EXECUTE_READWRITE, &Protect);
		if (ret == 0)
		{
			Logging::Log() << __FUNCTION__ << " Error: Failed to VirtualProtect memory!";
			return FALSE;
		}
		*(DWORD*)(addr + 6) = 0;
		VirtualProtect((LPVOID)(addr + 6), 4, Protect, &Protect);

		// Set function address
		d8_Direct3D8EnableMaximizedWindowedModeShim = (Direct3D8EnableMaximizedWindowedModeShimProc)addr;
	}

	// Launch function to disable Maximized Windowed Mode
	Logging::Log() << __FUNCTION__ << " Disabling MaximizedWindowedMode for Direct3D8! Ret = " << (void*)d8_Direct3D8EnableMaximizedWindowedModeShim(0);
	return TRUE;
}

Direct3D8 *WINAPI d8_Direct3DCreate8(UINT SDKVersion)
{
	LOG_LIMIT(1, __FUNCTION__);

	if (!Config.D3d8to9)
	{
		return nullptr;
	}

	LOG_ONCE("Starting D3d8to9 v" << APP_VERSION);

	LOG_LIMIT(3, "Redirecting 'Direct3DCreate8' to --> 'Direct3DCreate9' (" << SDKVersion << ")");

	// Declare Direct3DCreate9
	static Direct3DCreate9Proc Direct3DCreate9 = reinterpret_cast<Direct3DCreate9Proc>(Direct3DCreate9_out);
	if (!Direct3DCreate9)
	{
		LOG_LIMIT(100, "Failed to get 'Direct3DCreate9' ProcAddress of d3d9.dll!");
		return nullptr;
	}

	IDirect3D9 *const d3d = Direct3DCreate9(D3D_SDK_VERSION);

	return new Direct3D8(d3d);
}
