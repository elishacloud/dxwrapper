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
#include "Dllmain\Dllmain.h"
#include "Hook\inithook.h"
#include "Utils\Utils.h"
#include "wrapper.h"
#include "d3d8.h"

d3d8_dll d3d8;

#define module d3d8

VISIT_D3D8_PROCS(CREATE_PROC_STUB)

HRESULT WINAPI _ValidateVertexShader(DWORD* vertexshader, DWORD* reserved1, DWORD* reserved2, BOOL flag, DWORD* toto)
{
	UNREFERENCED_PARAMETER(flag);
	UNREFERENCED_PARAMETER(toto);

	if (!vertexshader)
	{
		return E_FAIL;
	}

	if (reserved1 || reserved2)
	{
		return E_FAIL;
	}

	switch (*vertexshader)
	{
	case 0xFFFE0100:
	case 0xFFFE0101:
		return S_OK;
		break;
	default:
		return E_FAIL;
	}
}

HRESULT WINAPI _ValidatePixelShader(DWORD* pixelshader, DWORD* reserved1, BOOL flag, DWORD* toto)
{
	UNREFERENCED_PARAMETER(flag);
	UNREFERENCED_PARAMETER(toto);

	if (!pixelshader)
	{
		return E_FAIL;
	}

	if (reserved1)
	{
		return E_FAIL;
	}

	switch (*pixelshader)
	{
	case 0xFFFF0100:
	case 0xFFFF0101:
	case 0xFFFF0102:
	case 0xFFFF0103:
	case 0xFFFF0104:
		return S_OK;
		break;
	default:
		return E_FAIL;
	}
}

void d3d8_dll::Load()
{
	// Enable d3d8to9 conversion
	if (Config.D3d8to9 && Config.RealWrapperMode == dtype.d3d8)
	{
		Direct3DCreate8 = GetFunctionAddress(hModule_dll, "_Direct3DCreate8", jmpaddr);
		ValidateVertexShader = (FARPROC)*_ValidateVertexShader;
		ValidatePixelShader = (FARPROC)*_ValidatePixelShader;
	}

	// Load normal dll
	else
	{
		// Load real dll
		dll = Wrapper.LoadDll(dtype.d3d8);

		// Load dll functions
		if (dll)
		{
			VISIT_D3D8_PROCS(LOAD_ORIGINAL_PROC);

			// Hook APIs for d3d8to9 conversion
			if (Config.D3d8to9)
			{
				LOG << "Hooking d3d8.dll APIs...";
				Direct3DCreate8 = (FARPROC)HookAPI(hModule_dll, dtypename[dtype.d3d8], GetFunctionAddress(dll, "Direct3DCreate8"), "Direct3DCreate8", GetFunctionAddress(hModule_dll, "_Direct3DCreate8"));
			}
		}
	}
}
