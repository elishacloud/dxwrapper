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

#include "d3d9.h"
#include "d3d9External.h"

namespace D3d9Wrapper
{
	FARPROC Direct3DShaderValidatorCreate9_out = nullptr;
	FARPROC PSGPError_out = nullptr;
	FARPROC PSGPSampleTexture_out = nullptr;
	FARPROC D3DPERF_BeginEvent_out = nullptr;
	FARPROC D3DPERF_EndEvent_out = nullptr;
	FARPROC D3DPERF_GetStatus_out = nullptr;
	FARPROC D3DPERF_QueryRepeatFrame_out = nullptr;
	FARPROC D3DPERF_SetMarker_out = nullptr;
	FARPROC D3DPERF_SetOptions_out = nullptr;
	FARPROC D3DPERF_SetRegion_out = nullptr;
	FARPROC DebugSetLevel_out = nullptr;
	FARPROC DebugSetMute_out = nullptr;
	FARPROC Direct3D9EnableMaximizedWindowedModeShim_out = nullptr;
	FARPROC Direct3DCreate9_out = nullptr;
	FARPROC Direct3DCreate9Ex_out = nullptr;
}

using namespace D3d9Wrapper;

HRESULT WINAPI dd_Direct3DShaderValidatorCreate9()
{
	return ((Direct3DShaderValidatorCreate9Proc)Direct3DShaderValidatorCreate9_out)();
}

HRESULT WINAPI dd_PSGPError()
{
	return ((PSGPErrorProc)PSGPError_out)();
}

HRESULT WINAPI dd_PSGPSampleTexture()
{
	return ((PSGPSampleTextureProc)PSGPSampleTexture_out)();
}

int WINAPI dd_D3DPERF_BeginEvent(D3DCOLOR col, LPCWSTR wszName)
{
	return ((D3DPERF_BeginEventProc)D3DPERF_BeginEvent_out)(col, wszName);
}

int WINAPI dd_D3DPERF_EndEvent()
{
	return ((D3DPERF_EndEventProc)D3DPERF_EndEvent_out)();
}

DWORD WINAPI dd_D3DPERF_GetStatus()
{
	return ((D3DPERF_GetStatusProc)D3DPERF_GetStatus_out)();
}

BOOL WINAPI dd_D3DPERF_QueryRepeatFrame()
{
	return ((D3DPERF_QueryRepeatFrameProc)D3DPERF_QueryRepeatFrame_out)();
}

void WINAPI dd_D3DPERF_SetMarker(D3DCOLOR col, LPCWSTR wszName)
{
	return ((D3DPERF_SetMarkerProc)D3DPERF_SetMarker_out)(col, wszName);
}

void WINAPI dd_D3DPERF_SetOptions(DWORD dwOptions)
{
	return ((D3DPERF_SetOptionsProc)D3DPERF_SetOptions_out)(dwOptions);
}

void WINAPI dd_D3DPERF_SetRegion(D3DCOLOR col, LPCWSTR wszName)
{
	return ((D3DPERF_SetRegionProc)D3DPERF_SetRegion_out)(col, wszName);
}

HRESULT WINAPI dd_DebugSetLevel(DWORD dw1)
{
	return ((DebugSetLevelProc)DebugSetLevel_out)(dw1);
}

void WINAPI dd_DebugSetMute()
{
	return ((DebugSetMuteProc)DebugSetMute_out)();
}

void WINAPI dd_Direct3D9EnableMaximizedWindowedModeShim()
{
	return ((Direct3D9EnableMaximizedWindowedModeShimProc)Direct3D9EnableMaximizedWindowedModeShim_out)();
}

IDirect3D9 *WINAPI dd_Direct3DCreate9(UINT SDKVersion)
{
	return new m_IDirect3D9(((Direct3DCreate9Proc)Direct3DCreate9_out)(SDKVersion));
}

HRESULT WINAPI dd_Direct3DCreate9Ex(UINT SDKVersion, IDirect3D9Ex **ppD3D)
{
	HRESULT hr = ((Direct3DCreate9ExProc)Direct3DCreate9Ex_out)(SDKVersion, ppD3D);

	if (SUCCEEDED(hr))
	{
		*ppD3D = new m_IDirect3D9Ex(*ppD3D);
	}

	return hr;
}
