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
#include "wrapper.h"

struct d3d9_dll
{
	HMODULE dll = nullptr;
	FARPROC Direct3DShaderValidatorCreate9;
	FARPROC PSGPError;
	FARPROC PSGPSampleTexture;
	FARPROC D3DPERF_BeginEvent;
	FARPROC D3DPERF_EndEvent;
	FARPROC D3DPERF_GetStatus;
	FARPROC D3DPERF_QueryRepeatFrame;
	FARPROC D3DPERF_SetMarker;
	FARPROC D3DPERF_SetOptions;
	FARPROC D3DPERF_SetRegion;
	FARPROC DebugSetLevel;
	FARPROC DebugSetMute;
	FARPROC Direct3DCreate9;
	FARPROC Direct3DCreate9Ex;
} d3d9;

__declspec(naked) void FakeDirect3DShaderValidatorCreate9()	{ _asm { jmp [d3d9.Direct3DShaderValidatorCreate9] } }
__declspec(naked) void FakePSGPError()						{ _asm { jmp [d3d9.PSGPError] } }
__declspec(naked) void FakePSGPSampleTexture()				{ _asm { jmp [d3d9.PSGPSampleTexture] } }
__declspec(naked) void FakeD3DPERF_BeginEvent()				{ _asm { jmp [d3d9.D3DPERF_BeginEvent] } }
__declspec(naked) void FakeD3DPERF_EndEvent()				{ _asm { jmp [d3d9.D3DPERF_EndEvent] } }
__declspec(naked) void FakeD3DPERF_GetStatus()				{ _asm { jmp [d3d9.D3DPERF_GetStatus] } }
__declspec(naked) void FakeD3DPERF_QueryRepeatFrame()		{ _asm { jmp [d3d9.D3DPERF_QueryRepeatFrame] } }
__declspec(naked) void FakeD3DPERF_SetMarker()				{ _asm { jmp [d3d9.D3DPERF_SetMarker] } }
__declspec(naked) void FakeD3DPERF_SetOptions()				{ _asm { jmp [d3d9.D3DPERF_SetOptions] } }
__declspec(naked) void FakeD3DPERF_SetRegion()				{ _asm { jmp [d3d9.D3DPERF_SetRegion] } }
__declspec(naked) void FakeDebugSetLevel()					{ _asm { jmp [d3d9.DebugSetLevel] } }
__declspec(naked) void FakeDebugSetMute()					{ _asm { jmp [d3d9.DebugSetMute] } }
__declspec(naked) void FakeDirect3DCreate9()				{ _asm { jmp [d3d9.Direct3DCreate9] } }
__declspec(naked) void FakeDirect3DCreate9Ex()				{ _asm { jmp [d3d9.Direct3DCreate9Ex] } }

void LoadD3d9()
{
	// Load real dll
	d3d9.dll = LoadDll(dtype.d3d9);
	// Load dll functions
	if (d3d9.dll)
	{
		d3d9.Direct3DShaderValidatorCreate9	= GetProcAddress(d3d9.dll, "Direct3DShaderValidatorCreate9");
		d3d9.PSGPError						= GetProcAddress(d3d9.dll, "PSGPError");
		d3d9.PSGPSampleTexture				= GetProcAddress(d3d9.dll, "PSGPSampleTexture");
		d3d9.D3DPERF_BeginEvent				= GetProcAddress(d3d9.dll, "D3DPERF_BeginEvent");
		d3d9.D3DPERF_EndEvent				= GetProcAddress(d3d9.dll, "D3DPERF_EndEvent");
		d3d9.D3DPERF_GetStatus				= GetProcAddress(d3d9.dll, "D3DPERF_GetStatus");
		d3d9.D3DPERF_QueryRepeatFrame		= GetProcAddress(d3d9.dll, "D3DPERF_QueryRepeatFrame");
		d3d9.D3DPERF_SetMarker				= GetProcAddress(d3d9.dll, "D3DPERF_SetMarker");
		d3d9.D3DPERF_SetOptions				= GetProcAddress(d3d9.dll, "D3DPERF_SetOptions");
		d3d9.D3DPERF_SetRegion				= GetProcAddress(d3d9.dll, "D3DPERF_SetRegion");
		d3d9.DebugSetLevel					= GetProcAddress(d3d9.dll, "DebugSetLevel");
		d3d9.DebugSetMute					= GetProcAddress(d3d9.dll, "DebugSetMute");
		d3d9.Direct3DCreate9				= GetProcAddress(d3d9.dll, "Direct3DCreate9");
		d3d9.Direct3DCreate9Ex				= GetProcAddress(d3d9.dll, "Direct3DCreate9Ex");
	}
}

void Set_DebugSetMute(FARPROC ProcAddress)
{
	d3d9.DebugSetMute = ProcAddress;
}

void FreeD3d9Library()
{
	if (d3d9.dll) FreeLibrary(d3d9.dll);
}