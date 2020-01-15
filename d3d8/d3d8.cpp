/**
* Copyright (C) 2020 Elisha Riedlinger
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
*
* ValidatePixelShader and ValidateVertexShader created from source code found in Wine
* https://github.com/alexhenrie/wine/tree/master/dlls/d3d8
*/

#include "d3d8External.h"
#include "d3d9\d3d9External.h"
#include "External\d3d8to9\source\d3d8to9.hpp"
#include "External\d3d8to9\source\d3dx9.hpp"
#include "Settings\Settings.h"
#include "Logging\Logging.h"
#include "BuildNo.rc"

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define APP_VERSION TOSTRING(FILEVERSION)

typedef LPDIRECT3D9(WINAPI *PFN_Direct3DCreate9)(UINT SDKVersion);
typedef void(WINAPI *DebugSetMuteProc)();
typedef void(WINAPI *Direct3D8EnableMaximizedWindowedModeShimProc)();

namespace D3d8Wrapper
{
	FARPROC Direct3DCreate9_out = nullptr;
	FARPROC Direct3D8EnableMaximizedWindowedModeShim_out = nullptr;
	FARPROC DebugSetMute_out = nullptr;
}

PFN_D3DXAssembleShader D3DXAssembleShader = nullptr;
PFN_D3DXDisassembleShader D3DXDisassembleShader = nullptr;
PFN_D3DXLoadSurfaceFromSurface D3DXLoadSurfaceFromSurface = nullptr;

using namespace D3d8Wrapper;

void WINAPI d8_Direct3D8EnableMaximizedWindowedModeShim()
{
	LOG_LIMIT(1, __FUNCTION__);

	static Direct3D8EnableMaximizedWindowedModeShimProc m_pDirect3D8EnableMaximizedWindowedModeShim = (Wrapper::ValidProcAddress(Direct3D8EnableMaximizedWindowedModeShim_out)) ? (Direct3D8EnableMaximizedWindowedModeShimProc)Direct3D8EnableMaximizedWindowedModeShim_out : nullptr;

	if (!m_pDirect3D8EnableMaximizedWindowedModeShim)
	{
		return;
	}

	return m_pDirect3D8EnableMaximizedWindowedModeShim();
}

HRESULT WINAPI d8_ValidatePixelShader(DWORD* pixelshader, DWORD* reserved1, BOOL flag, DWORD* toto)
{
	UNREFERENCED_PARAMETER(flag);
	UNREFERENCED_PARAMETER(toto);

	LOG_LIMIT(1, __FUNCTION__);

	if (!pixelshader)
	{
		return D3DERR_INVALIDCALL;
	}
	if (reserved1)
	{
		return D3DERR_INVALIDCALL;
	}
	switch (*pixelshader)
	{
	case 0xFFFF0100:
	case 0xFFFF0101:
	case 0xFFFF0102:
	case 0xFFFF0103:
	case 0xFFFF0104:
		return D3D_OK;
		break;
	default:
		return D3DERR_INVALIDCALL;
	}
}

HRESULT WINAPI d8_ValidateVertexShader(DWORD* vertexshader, DWORD* reserved1, DWORD* reserved2, BOOL flag, DWORD* toto)
{
	UNREFERENCED_PARAMETER(flag);
	UNREFERENCED_PARAMETER(toto);

	LOG_LIMIT(1, __FUNCTION__);

	if (!vertexshader)
	{
		return D3DERR_INVALIDCALL;
	}
	if (reserved1 || reserved2)
	{
		return D3DERR_INVALIDCALL;
	}
	switch (*vertexshader)
	{
	case 0xFFFE0100:
	case 0xFFFE0101:
		return D3D_OK;
		break;
	default:
		return D3DERR_INVALIDCALL;
	}
}

void WINAPI d8_DebugSetMute()
{
	LOG_LIMIT(1, __FUNCTION__);

	static DebugSetMuteProc m_pDebugSetMute = (Wrapper::ValidProcAddress(DebugSetMute_out)) ? (DebugSetMuteProc)DebugSetMute_out : nullptr;

	if (!m_pDebugSetMute)
	{
		return;
	}

	return m_pDebugSetMute();
}

Direct3D8 *WINAPI d8_Direct3DCreate8(UINT SDKVersion)
{
	LOG_LIMIT(1, __FUNCTION__);

	if (!Config.D3d8to9)
	{
		return nullptr;
	}

	LOG_ONCE("Starting D3d8to9 v" << APP_VERSION);

	LOG_LIMIT(3, "Redirecting 'Direct3DCreate8' to --> 'Direct3DCreate9' (" << SDKVersion << ")");

	// Declare Direct3DCreate9
	static PFN_Direct3DCreate9 Direct3DCreate9 = reinterpret_cast<PFN_Direct3DCreate9>(Direct3DCreate9_out);
	if (!Direct3DCreate9)
	{
		LOG_LIMIT(100, "Failed to get 'Direct3DCreate9' ProcAddress of d3d9.dll!");
		return nullptr;
	}

	IDirect3D9 *const d3d = Direct3DCreate9(D3D_SDK_VERSION);

	if (!d3d)
	{
		return nullptr;
	}

	// Load D3DX
	if (!D3DXAssembleShader || !D3DXDisassembleShader || !D3DXLoadSurfaceFromSurface)
	{
		Loadd3dx9();
		D3DXAssembleShader = (PFN_D3DXAssembleShader)p_D3DXAssembleShader;
		D3DXDisassembleShader = (PFN_D3DXDisassembleShader)p_D3DXDisassembleShader;
		D3DXLoadSurfaceFromSurface = (PFN_D3DXLoadSurfaceFromSurface)p_D3DXLoadSurfaceFromSurface;
	}

	return new Direct3D8(d3d);
}
