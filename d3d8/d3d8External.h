#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "Wrappers\d3d8.h"

struct D3DCAPS8;
class Direct3D8;

HRESULT WINAPI d8_ValidatePixelShader(const DWORD* pPixelShader, const D3DCAPS8* pCaps, BOOL ErrorsFlag, char** Errors);
HRESULT WINAPI d8_ValidateVertexShader(const DWORD* pVertexShader, const DWORD* pDeclaration, const D3DCAPS8* pCaps, BOOL ErrorsFlag, char** Errors);
Direct3D8 *WINAPI d8_Direct3DCreate8(UINT SDKVersion);

#define DECLARE_IN_WRAPPED_PROC(procName, unused) \
	const FARPROC procName ## _in = (FARPROC)*d8_ ## procName;

namespace D3d8Wrapper
{
	VISIT_PROCS_D3D8(DECLARE_IN_WRAPPED_PROC);

	extern FARPROC Direct3DCreate9_out;
}

#undef DECLARE_IN_WRAPPED_PROC
