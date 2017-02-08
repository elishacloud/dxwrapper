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

struct ddraw_dll
{
	HMODULE dll;
	FARPROC	Direct3DCreate8;
	FARPROC	DebugSetMute;
	FARPROC	ValidateVertexShader;
	FARPROC	ValidatePixelShader;
} d3d8;

__declspec(naked) void FakeDirect3DCreate8()			{ _asm { jmp [d3d8.Direct3DCreate8] } }
__declspec(naked) void FakeDebugSetMute()				{ _asm { jmp [d3d8.DebugSetMute] } }
__declspec(naked) void FakeValidateVertexShader()		{ _asm { jmp [d3d8.ValidateVertexShader] } }
__declspec(naked) void FakeValidatePixelShader()		{ _asm { jmp [d3d8.ValidatePixelShader] } }

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	char path[MAX_PATH];
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		{
			CopyMemory(path+GetSystemDirectory(path,MAX_PATH-10), "\\d3d8.dll",11);
			d3d8.dll = LoadLibrary(path);
			if (d3d8.dll == false)
			{
				MessageBox(0, "Cannot load original d3d8.dll library", APP_NAME, MB_ICONERROR);
				ExitProcess(0);
			}
			d3d8.Direct3DCreate8			= GetProcAddress(d3d8.dll, "Direct3DCreate8");
			d3d8.DebugSetMute				= GetProcAddress(d3d8.dll, "DebugSetMute");
			d3d8.ValidateVertexShader		= GetProcAddress(d3d8.dll, "ValidateVertexShader");
			d3d8.ValidatePixelShader		= GetProcAddress(d3d8.dll, "ValidatePixelShader");

			InitDxWnd();
			break;
		}
		case DLL_PROCESS_DETACH:
		{
			DxWndEndHook();
			FreeLibrary(d3d8.dll);
		}
		break;
	}
	return TRUE;
}
