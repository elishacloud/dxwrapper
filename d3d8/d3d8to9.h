#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

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

namespace D3d8to9
{
	const FARPROC Direct3DCreate8 = (FARPROC)*d8_Direct3DCreate8;
	extern FARPROC Direct3DCreate9;
}
