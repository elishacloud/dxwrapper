#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

class __declspec(uuid("1DD9E8DA-1C77-4D40-B0CF-98FEFDFF9512")) Direct3D8;

extern "C" Direct3D8 *WINAPI _Direct3DCreate8(UINT);

void Loadd3dx9();

extern FARPROC p_D3DXAssembleShader;
extern FARPROC p_D3DXDisassembleShader;
extern FARPROC p_D3DXLoadSurfaceFromSurface;

namespace D3d8to9
{
	constexpr FARPROC Direct3DCreate8 = (FARPROC)*_Direct3DCreate8;
	extern FARPROC Direct3DCreate9;
}
