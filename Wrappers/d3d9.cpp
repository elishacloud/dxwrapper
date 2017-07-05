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
#include "wrapper.h"
#include "Utils\Utils.h"

struct d3d9_dll
{
	HMODULE dll = nullptr;
	FARPROC D3DPERF_BeginEvent = jmpaddr;
	FARPROC D3DPERF_EndEvent = jmpaddr;
	FARPROC D3DPERF_GetStatus = jmpaddr;
	FARPROC D3DPERF_QueryRepeatFrame = jmpaddr;
	FARPROC D3DPERF_SetMarker = jmpaddr;
	FARPROC D3DPERF_SetOptions = jmpaddr;
	FARPROC D3DPERF_SetRegion = jmpaddr;
	FARPROC DebugSetLevel = jmpaddr;
	FARPROC DebugSetMute = jmpaddr;
	FARPROC Direct3DCreate9 = jmpaddr;
	FARPROC Direct3DCreate9Ex = jmpaddr;
	FARPROC Direct3DShaderValidatorCreate9 = jmpaddr;
	FARPROC PSGPError = jmpaddr;
	FARPROC PSGPSampleTexture = jmpaddr;
} d3d9;

__declspec(naked) void FakeD3DPERF_BeginEvent() { _asm { jmp[d3d9.D3DPERF_BeginEvent] } }
__declspec(naked) void FakeD3DPERF_EndEvent() { _asm { jmp[d3d9.D3DPERF_EndEvent] } }
__declspec(naked) void FakeD3DPERF_GetStatus() { _asm { jmp[d3d9.D3DPERF_GetStatus] } }
__declspec(naked) void FakeD3DPERF_QueryRepeatFrame() { _asm { jmp[d3d9.D3DPERF_QueryRepeatFrame] } }
__declspec(naked) void FakeD3DPERF_SetMarker() { _asm { jmp[d3d9.D3DPERF_SetMarker] } }
__declspec(naked) void FakeD3DPERF_SetOptions() { _asm { jmp[d3d9.D3DPERF_SetOptions] } }
__declspec(naked) void FakeD3DPERF_SetRegion() { _asm { jmp[d3d9.D3DPERF_SetRegion] } }
__declspec(naked) void FakeDebugSetLevel() { _asm { jmp[d3d9.DebugSetLevel] } }
__declspec(naked) void FakeDebugSetMute() { _asm { jmp[d3d9.DebugSetMute] } }
__declspec(naked) void FakeDirect3DCreate9() { _asm { jmp[d3d9.Direct3DCreate9] } }
__declspec(naked) void FakeDirect3DCreate9Ex() { _asm { jmp[d3d9.Direct3DCreate9Ex] } }
__declspec(naked) void FakeDirect3DShaderValidatorCreate9() { _asm { jmp[d3d9.Direct3DShaderValidatorCreate9] } }
__declspec(naked) void FakePSGPError() { _asm { jmp[d3d9.PSGPError] } }
__declspec(naked) void FakePSGPSampleTexture() { _asm { jmp[d3d9.PSGPSampleTexture] } }

void LoadD3d9()
{
	// Load real dll
	d3d9.dll = LoadDll(dtype.d3d9);

	// Load dll functions
	if (d3d9.dll)
	{
		d3d9.D3DPERF_BeginEvent = GetFunctionAddress(d3d9.dll, "D3DPERF_BeginEvent", jmpaddr);
		d3d9.D3DPERF_EndEvent = GetFunctionAddress(d3d9.dll, "D3DPERF_EndEvent", jmpaddr);
		d3d9.D3DPERF_GetStatus = GetFunctionAddress(d3d9.dll, "D3DPERF_GetStatus", jmpaddr);
		d3d9.D3DPERF_QueryRepeatFrame = GetFunctionAddress(d3d9.dll, "D3DPERF_QueryRepeatFrame", jmpaddr);
		d3d9.D3DPERF_SetMarker = GetFunctionAddress(d3d9.dll, "D3DPERF_SetMarker", jmpaddr);
		d3d9.D3DPERF_SetOptions = GetFunctionAddress(d3d9.dll, "D3DPERF_SetOptions", jmpaddr);
		d3d9.D3DPERF_SetRegion = GetFunctionAddress(d3d9.dll, "D3DPERF_SetRegion", jmpaddr);
		d3d9.DebugSetLevel = GetFunctionAddress(d3d9.dll, "DebugSetLevel", jmpaddr);
		d3d9.DebugSetMute = GetFunctionAddress(d3d9.dll, "DebugSetMute", jmpaddr);
		d3d9.Direct3DCreate9 = GetFunctionAddress(d3d9.dll, "Direct3DCreate9", jmpaddr);
		d3d9.Direct3DCreate9Ex = GetFunctionAddress(d3d9.dll, "Direct3DCreate9Ex", jmpaddr);
		d3d9.Direct3DShaderValidatorCreate9 = GetFunctionAddress(d3d9.dll, "Direct3DShaderValidatorCreate9", jmpaddr);
		d3d9.PSGPError = GetFunctionAddress(d3d9.dll, "PSGPError", jmpaddr);
		d3d9.PSGPSampleTexture = GetFunctionAddress(d3d9.dll, "PSGPSampleTexture", jmpaddr);
	}
}

void Set_DebugSetMute(FARPROC ProcAddress)
{
	d3d9.DebugSetMute = ProcAddress;
}