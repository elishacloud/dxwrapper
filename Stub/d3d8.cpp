/**
* Copyright (C) 2019 Elisha Riedlinger
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

#include <windows.h>
#include "..\Wrappers\wrapper.h"

// d3d8 proc typedefs
typedef void(WINAPI *Direct3D8EnableMaximizedWindowedModeShimProc)();
typedef HRESULT(WINAPI *ValidatePixelShaderProc)(DWORD* pixelshader, DWORD* reserved1, BOOL flag, DWORD* toto);
typedef HRESULT(WINAPI *ValidateVertexShaderProc)(DWORD* vertexshader, DWORD* reserved1, DWORD* reserved2, BOOL flag, DWORD* toto);
typedef void(WINAPI *DebugSetMuteProc)();
typedef LPVOID(WINAPI *Direct3DCreate8Proc)(UINT SDKVersion);

namespace D3d8Wrapper
{
	char dllname[MAX_PATH];

#define INITIALIZE_WRAPPED_PROC(procName, unused) \
	procName ## Proc m_p ## procName = nullptr;

	VISIT_PROCS_D3D8(INITIALIZE_WRAPPED_PROC);
	VISIT_PROCS_D3D8_SHARED(INITIALIZE_WRAPPED_PROC);

#undef INITIALIZE_WRAPPED_PROC
}

using namespace D3d8Wrapper;

void InitD3d8()
{
	static bool RunOnce = true;

	if (RunOnce)
	{
		// Load dll
		HMODULE d3d8dll = LoadLibraryA(dllname);

		// Get function addresses
#define INITIALIZE_WRAPPED_PROC(procName, unused) \
	m_p ## procName = (procName ## Proc)GetProcAddress(d3d8dll, #procName);

		VISIT_PROCS_D3D8(INITIALIZE_WRAPPED_PROC);
		VISIT_PROCS_D3D8_SHARED(INITIALIZE_WRAPPED_PROC);

#undef INITIALIZE_WRAPPED_PROC

		RunOnce = false;
	}
}


void WINAPI d8_Direct3D8EnableMaximizedWindowedModeShim()
{
	InitD3d8();

	if (!m_pDirect3D8EnableMaximizedWindowedModeShim)
	{
		return;
	}

	return m_pDirect3D8EnableMaximizedWindowedModeShim();
}

HRESULT WINAPI d8_ValidatePixelShader(DWORD* pixelshader, DWORD* reserved1, BOOL flag, DWORD* toto)
{
	InitD3d8();

	if (!m_pValidatePixelShader)
	{
		return E_FAIL;
	}

	return m_pValidatePixelShader(pixelshader, reserved1, flag, toto);
}

HRESULT WINAPI d8_ValidateVertexShader(DWORD* vertexshader, DWORD* reserved1, DWORD* reserved2, BOOL flag, DWORD* toto)
{
	InitD3d8();

	if (!m_pValidateVertexShader)
	{
		return E_FAIL;
	}

	return m_pValidateVertexShader(vertexshader, reserved1, reserved2, flag, toto);
}

void WINAPI d8_DebugSetMute()
{
	InitD3d8();

	if (!m_pDebugSetMute)
	{
		return;
	}

	return m_pDebugSetMute();
}

LPVOID WINAPI d8_Direct3DCreate8(UINT SDKVersion)
{
	InitD3d8();

	if (!m_pDirect3DCreate8)
	{
		return nullptr;
	}

	return m_pDirect3DCreate8(SDKVersion);
}

void StartD3d8(const char *name)
{
	if (name)
	{
		strcpy_s(dllname, MAX_PATH, name);
	}
	else
	{
		GetSystemDirectoryA(dllname, MAX_PATH);
		strcat_s(dllname, "\\d3d8.dll");
	}

#define INITIALIZE_WRAPPED_PROC(procName, unused) \
	d3d8::procName ## _var = (FARPROC)*d8_ ## procName;

	VISIT_PROCS_D3D8(INITIALIZE_WRAPPED_PROC);
	VISIT_PROCS_D3D8_SHARED(INITIALIZE_WRAPPED_PROC);
}
