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

#include "d3d9.h"
#include "d3d9External.h"

#define INITIALIZE_WRAPPED_PROC(procName, unused) \
	FARPROC procName ## _out = nullptr;

namespace D3d9Wrapper
{
	VISIT_PROCS_D3D9(INITIALIZE_WRAPPED_PROC);
	FARPROC DebugSetMute_out = nullptr;
}

using namespace D3d9Wrapper;

HRESULT WINAPI d9_Direct3DShaderValidatorCreate9()
{
	static Direct3DShaderValidatorCreate9Proc m_pDirect3DShaderValidatorCreate9 = (Wrapper::ValidProcAddress(Direct3DShaderValidatorCreate9_out)) ? (Direct3DShaderValidatorCreate9Proc)Direct3DShaderValidatorCreate9_out : nullptr;

	if (!m_pDirect3DShaderValidatorCreate9)
	{
		return E_FAIL;
	}

	return m_pDirect3DShaderValidatorCreate9();
}

HRESULT WINAPI d9_PSGPError()
{
	static PSGPErrorProc m_pPSGPError = (Wrapper::ValidProcAddress(PSGPError_out)) ? (PSGPErrorProc)PSGPError_out : nullptr;

	if (!m_pPSGPError)
	{
		return E_FAIL;
	}

	return m_pPSGPError();
}

HRESULT WINAPI d9_PSGPSampleTexture()
{
	static PSGPSampleTextureProc m_pPSGPSampleTexture = (Wrapper::ValidProcAddress(PSGPSampleTexture_out)) ? (PSGPSampleTextureProc)PSGPSampleTexture_out : nullptr;

	if (!m_pPSGPSampleTexture)
	{
		return E_FAIL;
	}

	return m_pPSGPSampleTexture();
}

int WINAPI d9_D3DPERF_BeginEvent(D3DCOLOR col, LPCWSTR wszName)
{
	static D3DPERF_BeginEventProc m_pD3DPERF_BeginEvent = (Wrapper::ValidProcAddress(D3DPERF_BeginEvent_out)) ? (D3DPERF_BeginEventProc)D3DPERF_BeginEvent_out : nullptr;

	if (!m_pD3DPERF_BeginEvent)
	{
		return NULL;
	}

	return m_pD3DPERF_BeginEvent(col, wszName);
}

int WINAPI d9_D3DPERF_EndEvent()
{
	static D3DPERF_EndEventProc m_pD3DPERF_EndEvent = (Wrapper::ValidProcAddress(D3DPERF_EndEvent_out)) ? (D3DPERF_EndEventProc)D3DPERF_EndEvent_out : nullptr;

	if (!m_pD3DPERF_EndEvent)
	{
		return NULL;
	}

	return m_pD3DPERF_EndEvent();
}

DWORD WINAPI d9_D3DPERF_GetStatus()
{
	static D3DPERF_GetStatusProc m_pD3DPERF_GetStatus = (Wrapper::ValidProcAddress(D3DPERF_GetStatus_out)) ? (D3DPERF_GetStatusProc)D3DPERF_GetStatus_out : nullptr;

	if (!m_pD3DPERF_GetStatus)
	{
		return NULL;
	}

	return m_pD3DPERF_GetStatus();
}

BOOL WINAPI d9_D3DPERF_QueryRepeatFrame()
{
	static D3DPERF_QueryRepeatFrameProc m_pD3DPERF_QueryRepeatFrame = (Wrapper::ValidProcAddress(D3DPERF_QueryRepeatFrame_out)) ? (D3DPERF_QueryRepeatFrameProc)D3DPERF_QueryRepeatFrame_out : nullptr;

	if (!m_pD3DPERF_QueryRepeatFrame)
	{
		return FALSE;
	}

	return m_pD3DPERF_QueryRepeatFrame();
}

void WINAPI d9_D3DPERF_SetMarker(D3DCOLOR col, LPCWSTR wszName)
{
	static D3DPERF_SetMarkerProc m_pD3DPERF_SetMarker = (Wrapper::ValidProcAddress(D3DPERF_SetMarker_out)) ? (D3DPERF_SetMarkerProc)D3DPERF_SetMarker_out : nullptr;

	if (!m_pD3DPERF_SetMarker)
	{
		return;
	}

	return m_pD3DPERF_SetMarker(col, wszName);
}

void WINAPI d9_D3DPERF_SetOptions(DWORD dwOptions)
{
	static D3DPERF_SetOptionsProc m_pD3DPERF_SetOptions = (Wrapper::ValidProcAddress(D3DPERF_SetOptions_out)) ? (D3DPERF_SetOptionsProc)D3DPERF_SetOptions_out : nullptr;

	if (!m_pD3DPERF_SetOptions)
	{
		return;
	}

	return m_pD3DPERF_SetOptions(dwOptions);
}

void WINAPI d9_D3DPERF_SetRegion(D3DCOLOR col, LPCWSTR wszName)
{
	static D3DPERF_SetRegionProc m_pD3DPERF_SetRegion = (Wrapper::ValidProcAddress(D3DPERF_SetRegion_out)) ? (D3DPERF_SetRegionProc)D3DPERF_SetRegion_out : nullptr;

	if (!m_pD3DPERF_SetRegion)
	{
		return;
	}

	return m_pD3DPERF_SetRegion(col, wszName);
}

HRESULT WINAPI d9_DebugSetLevel(DWORD dw1)
{
	static DebugSetLevelProc m_pDebugSetLevel = (Wrapper::ValidProcAddress(DebugSetLevel_out)) ? (DebugSetLevelProc)DebugSetLevel_out : nullptr;

	if (!m_pDebugSetLevel)
	{
		return E_FAIL;
	}

	return m_pDebugSetLevel(dw1);
}

void WINAPI d9_DebugSetMute()
{
	static DebugSetMuteProc m_pDebugSetMute = (Wrapper::ValidProcAddress(DebugSetMute_out)) ? (DebugSetMuteProc)DebugSetMute_out : nullptr;

	if (!m_pDebugSetMute)
	{
		return;
	}

	return m_pDebugSetMute();
}

void WINAPI d9_Direct3D9EnableMaximizedWindowedModeShim()
{
	static Direct3D9EnableMaximizedWindowedModeShimProc m_pDirect3D9EnableMaximizedWindowedModeShim = (Wrapper::ValidProcAddress(Direct3D9EnableMaximizedWindowedModeShim_out)) ? (Direct3D9EnableMaximizedWindowedModeShimProc)Direct3D9EnableMaximizedWindowedModeShim_out : nullptr;

	if (!m_pDirect3D9EnableMaximizedWindowedModeShim)
	{
		return;
	}

	return m_pDirect3D9EnableMaximizedWindowedModeShim();
}

IDirect3D9 *WINAPI d9_Direct3DCreate9(UINT SDKVersion)
{
	static Direct3DCreate9Proc m_pDirect3DCreate9 = (Wrapper::ValidProcAddress(Direct3DCreate9_out)) ? (Direct3DCreate9Proc)Direct3DCreate9_out : nullptr;

	if (!m_pDirect3DCreate9)
	{
		return nullptr;
	}

	IDirect3D9 *pD3D9 = m_pDirect3DCreate9(SDKVersion);

	if (pD3D9)
	{
		return new m_IDirect3D9Ex((IDirect3D9Ex*)pD3D9);
	}

	return nullptr;
}

HRESULT WINAPI d9_Direct3DCreate9Ex(UINT SDKVersion, IDirect3D9Ex **ppD3D)
{
	static Direct3DCreate9ExProc m_pDirect3DCreate9Ex = (Wrapper::ValidProcAddress(Direct3DCreate9Ex_out)) ? (Direct3DCreate9ExProc)Direct3DCreate9Ex_out : nullptr;

	if (!m_pDirect3DCreate9Ex)
	{
		return E_FAIL;
	}

	HRESULT hr = m_pDirect3DCreate9Ex(SDKVersion, ppD3D);

	if (SUCCEEDED(hr) && ppD3D)
	{
		*ppD3D = new m_IDirect3D9Ex(*ppD3D);
	}

	return hr;
}
