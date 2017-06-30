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
#include "wrapper.h"
#include "d3d9.h"
#include "Utils\Utils.h"
#include "Hook\inithook.h"

struct d3d8_dll
{
	HMODULE dll = nullptr;
	FARPROC Direct3DCreate8 = jmpaddr;
	//FARPROC DebugSetMute = jmpaddr;		 // <---  Shared with d3d9.dll
	FARPROC Direct3D8EnableMaximizedWindowedModeShim = jmpaddr;
	FARPROC ValidateVertexShader = jmpaddr;
	FARPROC ValidatePixelShader = jmpaddr;
} d3d8;

__declspec(naked) void FakeDirect3DCreate8() { _asm { jmp[d3d8.Direct3DCreate8] } }
//__declspec(naked) void FakeDebugSetMute() { _asm { jmp [d3d8.DebugSetMute] } }		 // <---  Shared with d3d9.dll
__declspec(naked) void FakeDirect3D8EnableMaximizedWindowedModeShim() { _asm { jmp[d3d8.Direct3D8EnableMaximizedWindowedModeShim] } }
__declspec(naked) void FakeValidateVertexShader() { _asm { jmp[d3d8.ValidateVertexShader] } }
__declspec(naked) void FakeValidatePixelShader() { _asm { jmp[d3d8.ValidatePixelShader] } }

void LoadD3d8()
{
	// Enable d3d8to9 conversion
	if (Config.D3d8to9 && Config.RealWrapperMode == dtype.d3d8)
	{
		d3d8.Direct3DCreate8 = GetFunctionAddress(hModule_dll, "_Direct3DCreate8", jmpaddr);
	}

	// Load real dll
	else
	{
		d3d8.dll = LoadDll(dtype.d3d8);

		// Load dll functions
		if (d3d8.dll)
		{
			d3d8.Direct3DCreate8 = GetFunctionAddress(d3d8.dll, "Direct3DCreate8", jmpaddr);
			Set_DebugSetMute(GetFunctionAddress(d3d8.dll, "DebugSetMute", jmpaddr));
			d3d8.Direct3D8EnableMaximizedWindowedModeShim = GetFunctionAddress(d3d8.dll, "Direct3D8EnableMaximizedWindowedModeShim", jmpaddr);
			d3d8.ValidateVertexShader = GetFunctionAddress(d3d8.dll, "ValidateVertexShader", jmpaddr);
			d3d8.ValidatePixelShader = GetFunctionAddress(d3d8.dll, "ValidatePixelShader", jmpaddr);

			// Hook APIs for d3d8to9 conversion
			if (Config.D3d8to9)
			{
				Compat::Log() << "Hooking d3d8.dll APIs...";
				d3d8.Direct3DCreate8 = (FARPROC)HookAPI(hModule_dll, dtypename[dtype.d3d8], GetFunctionAddress(d3d8.dll, "Direct3DCreate8"), "Direct3DCreate8", GetFunctionAddress(hModule_dll, "_Direct3DCreate8"));
			}
		}
	}
}