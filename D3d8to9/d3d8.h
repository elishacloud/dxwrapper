#pragma once

#include "wrappers\wrapper.h"

class __declspec(uuid("1DD9E8DA-1C77-4D40-B0CF-98FEFDFF9512")) Direct3D8;

extern "C" Direct3D8 *WINAPI Direct3DCreate8(UINT);
HRESULT WINAPI _ValidateVertexShader(DWORD* vertexshader, DWORD* reserved1, DWORD* reserved2, BOOL flag, DWORD* toto);
HRESULT WINAPI _ValidatePixelShader(DWORD* pixelshader, DWORD* reserved1, BOOL flag, DWORD* toto);

namespace Wrapper
{
	namespace D3d8to9
	{
		FARPROC _Direct3DCreate8 = (FARPROC)*Direct3DCreate8;
	}
}