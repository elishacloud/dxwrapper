//------------------------------------------------------------------------------
// Tiny DirectDraw Proxy DLL v1.0 [28.05.2010]
// Copyright 2010 Evgeny Vrublevsky <veg@tut.by>
//------------------------------------------------------------------------------
#define APP_NAME		"Tiny DirectDraw Proxy"
#define APP_VERSION		"1.0 [28.05.2010]"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <ctype.h>

void InitDxWnd(void);
void DxWndEndHook(void);

struct d3d9_dll
{
	HMODULE dll;
	FARPROC	Direct3DShaderValidatorCreate9;
	FARPROC	PSGPError;
	FARPROC	PSGPSampleTexture;
	FARPROC	D3DPERF_BeginEvent;
	FARPROC	D3DPERF_EndEvent;
	FARPROC	D3DPERF_GetStatus;
	FARPROC	D3DPERF_QueryRepeatFrame;
	FARPROC	D3DPERF_SetMarker;
	FARPROC	D3DPERF_SetOptions;
	FARPROC	D3DPERF_SetRegion;
	FARPROC	DebugSetLevel;
	FARPROC	DebugSetMute;
	FARPROC	Direct3DCreate9;
	FARPROC	Direct3DCreate9Ex;
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

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	char path[MAX_PATH];
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		{
			CopyMemory(path+GetSystemDirectory(path,MAX_PATH-10), "\\d3d9.dll",11);
			d3d9.dll = LoadLibrary(path);
			if (d3d9.dll == false)
			{
				MessageBox(0, "Cannot load original d3d9.dll library", APP_NAME, MB_ICONERROR);
				ExitProcess(0);
			}
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
			InitDxWnd();
			break;
		}
		case DLL_PROCESS_DETACH:
		{
			DxWndEndHook();
			FreeLibrary(d3d9.dll);
		}
		break;
	}
	return TRUE;
}
