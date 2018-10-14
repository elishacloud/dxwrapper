#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "Wrappers\d3d8.h"

class __declspec(uuid("1DD9E8DA-1C77-4D40-B0CF-98FEFDFF9512")) Direct3D8;

void WINAPI d8_Direct3D8EnableMaximizedWindowedModeShim();
HRESULT WINAPI d8_ValidatePixelShader(DWORD* pixelshader, DWORD* reserved1, BOOL flag, DWORD* toto);
HRESULT WINAPI d8_ValidateVertexShader(DWORD* vertexshader, DWORD* reserved1, DWORD* reserved2, BOOL flag, DWORD* toto);
void WINAPI d8_DebugSetMute();
Direct3D8 *WINAPI d8_Direct3DCreate8(UINT SDKVersion);

void Loadd3dx9();

extern FARPROC p_D3DXAssembleShader;
extern FARPROC p_D3DXDisassembleShader;
extern FARPROC p_D3DXLoadSurfaceFromSurface;

#define DECLARE_IN_WRAPPED_PROC(procName, unused) \
	const FARPROC procName ## _in = (FARPROC)*d8_ ## procName;

namespace D3d8Wrapper
{
	VISIT_PROCS_D3D8(DECLARE_IN_WRAPPED_PROC);
	const FARPROC DebugSetMute_in = (FARPROC)*d8_DebugSetMute;

	extern FARPROC Direct3DCreate9;
}

#undef DECLARE_IN_WRAPPED_PROC
