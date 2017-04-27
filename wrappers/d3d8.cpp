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

#include "cfg.h"
#include "dllmain.h"
#include "wrapper.h"
#include "d3d9.h"

bool d3d8Flag = false;

struct d3d8_dll
{
	HMODULE dll;
	FARPROC Direct3DCreate8;
	FARPROC DebugSetMute;		 // <---  Shared with d3d9.dll
	FARPROC ValidateVertexShader;
	FARPROC ValidatePixelShader;
} d3d8;

__declspec(naked) void FakeDirect3DCreate8()				{ _asm { jmp [d3d8.Direct3DCreate8] } }
//__declspec(naked) void FakeDebugSetMute()					{ _asm { jmp [d3d8.DebugSetMute] } }		 // <---  Shared with d3d9.dll
__declspec(naked) void FakeValidateVertexShader()			{ _asm { jmp [d3d8.ValidateVertexShader] } }
__declspec(naked) void FakeValidatePixelShader()			{ _asm { jmp [d3d8.ValidatePixelShader] } }

void LoadD3d8()
{
	// Load real dll
	d3d8.dll = LoadDll("d3d8.dll", dtype.d3d8);
	// Load dll functions
	if (d3d8.dll)
	{
		d3d8Flag = true;
		d3d8.Direct3DCreate8 = GetProcAddress(d3d8.dll, "Direct3DCreate8");
		//d3d8.Direct3DCreate8				= GetProcAddress(d3d8.dll, "DebugSetMute");		 // <---  Shared with d3d9.dll
		SetSharedD3d9(d3d8.dll);
		d3d8.ValidateVertexShader = GetProcAddress(d3d8.dll, "ValidateVertexShader");
		d3d8.ValidatePixelShader = GetProcAddress(d3d8.dll, "ValidatePixelShader");
	}
	// Enable d3d8 to d3d9 conversion
	if (Config.D3d8to9)
	{
		Compat::Log() << "Enabling D3d8 to D3d9 function";
		d3d8.Direct3DCreate8 = GetProcAddress(hModule_dll, "_Direct3DCreate8");
	}
}

void FreeD3d8Library()
{
	if (d3d8Flag) FreeLibrary(d3d8.dll);
}