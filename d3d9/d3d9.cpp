/**
* Copyright (C) 2022 Elisha Riedlinger
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
}

using namespace D3d9Wrapper;

HMODULE GetSystemD3d9()
{
	static HMODULE h_d3d9 = nullptr;

	// Get System d3d9.dll
	if (!h_d3d9)
	{
		char Path[MAX_PATH] = {};
		GetSystemDirectoryA(Path, MAX_PATH);
		strcat_s(Path, "\\d3d9.dll");
		GetModuleHandleExA(NULL, Path, &h_d3d9);
	}

	return h_d3d9;
}

int WINAPI d9_D3DPERF_BeginEvent(D3DCOLOR col, LPCWSTR wszName)
{
	LOG_LIMIT(1, __FUNCTION__);

	static D3DPERF_BeginEventProc m_pD3DPERF_BeginEvent = (Wrapper::ValidProcAddress(D3DPERF_BeginEvent_out)) ? (D3DPERF_BeginEventProc)D3DPERF_BeginEvent_out : nullptr;

	if (!m_pD3DPERF_BeginEvent)
	{
		return NULL;
	}

	return m_pD3DPERF_BeginEvent(col, wszName);
}

int WINAPI d9_D3DPERF_EndEvent()
{
	LOG_LIMIT(1, __FUNCTION__);

	static D3DPERF_EndEventProc m_pD3DPERF_EndEvent = (Wrapper::ValidProcAddress(D3DPERF_EndEvent_out)) ? (D3DPERF_EndEventProc)D3DPERF_EndEvent_out : nullptr;

	if (!m_pD3DPERF_EndEvent)
	{
		return NULL;
	}

	return m_pD3DPERF_EndEvent();
}

DWORD WINAPI d9_D3DPERF_GetStatus()
{
	LOG_LIMIT(1, __FUNCTION__);

	static D3DPERF_GetStatusProc m_pD3DPERF_GetStatus = (Wrapper::ValidProcAddress(D3DPERF_GetStatus_out)) ? (D3DPERF_GetStatusProc)D3DPERF_GetStatus_out : nullptr;

	if (!m_pD3DPERF_GetStatus)
	{
		return NULL;
	}

	return m_pD3DPERF_GetStatus();
}

BOOL WINAPI d9_D3DPERF_QueryRepeatFrame()
{
	LOG_LIMIT(1, __FUNCTION__);

	static D3DPERF_QueryRepeatFrameProc m_pD3DPERF_QueryRepeatFrame = (Wrapper::ValidProcAddress(D3DPERF_QueryRepeatFrame_out)) ? (D3DPERF_QueryRepeatFrameProc)D3DPERF_QueryRepeatFrame_out : nullptr;

	if (!m_pD3DPERF_QueryRepeatFrame)
	{
		return FALSE;
	}

	return m_pD3DPERF_QueryRepeatFrame();
}

void WINAPI d9_D3DPERF_SetMarker(D3DCOLOR col, LPCWSTR wszName)
{
	LOG_LIMIT(1, __FUNCTION__);

	static D3DPERF_SetMarkerProc m_pD3DPERF_SetMarker = (Wrapper::ValidProcAddress(D3DPERF_SetMarker_out)) ? (D3DPERF_SetMarkerProc)D3DPERF_SetMarker_out : nullptr;

	if (!m_pD3DPERF_SetMarker)
	{
		return;
	}

	return m_pD3DPERF_SetMarker(col, wszName);
}

void WINAPI d9_D3DPERF_SetOptions(DWORD dwOptions)
{
	LOG_LIMIT(1, __FUNCTION__);

	static D3DPERF_SetOptionsProc m_pD3DPERF_SetOptions = (Wrapper::ValidProcAddress(D3DPERF_SetOptions_out)) ? (D3DPERF_SetOptionsProc)D3DPERF_SetOptions_out : nullptr;

	if (!m_pD3DPERF_SetOptions)
	{
		return;
	}

	return m_pD3DPERF_SetOptions(dwOptions);
}

void WINAPI d9_D3DPERF_SetRegion(D3DCOLOR col, LPCWSTR wszName)
{
	LOG_LIMIT(1, __FUNCTION__);

	static D3DPERF_SetRegionProc m_pD3DPERF_SetRegion = (Wrapper::ValidProcAddress(D3DPERF_SetRegion_out)) ? (D3DPERF_SetRegionProc)D3DPERF_SetRegion_out : nullptr;

	if (!m_pD3DPERF_SetRegion)
	{
		return;
	}

	return m_pD3DPERF_SetRegion(col, wszName);
}

void SetGraphicsHybridAdapter(UINT Mode)
{
	static Direct3D9ForceHybridEnumerationProc m_pDirect3D9ForceHybridEnumeration = nullptr;

	if (!m_pDirect3D9ForceHybridEnumeration)
	{
		HMODULE dll = GetSystemD3d9();

		if (!dll)
		{
			Logging::Log() << __FUNCTION__ << " d3d9.dll is not loaded!";
			return;
		}

		// Get Direct3D9ForceHybridEnumeration address
		m_pDirect3D9ForceHybridEnumeration = (Direct3D9ForceHybridEnumerationProc)GetProcAddress(dll, reinterpret_cast<LPCSTR>(16));

		if (!m_pDirect3D9ForceHybridEnumeration)
		{
			Logging::Log() << __FUNCTION__ << " Error: Failed to get `Direct3D9ForceHybridEnumeration` address!";
			return;
		}
	}

	Logging::Log() << __FUNCTION__ << " Calling 'Direct3D9ForceHybridEnumeration' ... " << Mode;
	m_pDirect3D9ForceHybridEnumeration(Mode);
}

bool Direct3D9DisableMaximizedWindowedMode()
{
	static Direct3D9EnableMaximizedWindowedModeShimProc m_pDirect3D9EnableMaximizedWindowedModeShim = nullptr;

	if (!m_pDirect3D9EnableMaximizedWindowedModeShim)
	{
		// Load d3d9.dll from System32
		HMODULE dll = GetSystemD3d9();

		if (!dll)
		{
			Logging::Log() << __FUNCTION__ << " d3d9.dll is not loaded!";
			return false;
		}

		// Get function address
		BYTE* addr = (BYTE*)GetProcAddress(dll, "Direct3D9EnableMaximizedWindowedModeShim");
		if (!addr)
		{
			Logging::Log() << __FUNCTION__ << " Error: Failed to get `Direct3D9EnableMaximizedWindowedModeShim` address!";
			return false;
		}

		// Check memory address
		if (*(BYTE*)(addr + 6) != 1)
		{
			Logging::Log() << __FUNCTION__ << " Error: Failed to vaidate memory address!";
			return false;
		}

		// Update function to disable Maximized Windowed Mode
		DWORD Protect;
		BOOL ret = VirtualProtect((LPVOID)(addr + 6), 1, PAGE_EXECUTE_READWRITE, &Protect);
		if (ret == 0)
		{
			Logging::Log() << __FUNCTION__ << " Error: Failed to VirtualProtect memory!";
			return false;
		}
		*(BYTE*)(addr + 6) = 0;
		VirtualProtect((LPVOID)(addr + 6), 1, Protect, &Protect);

		// Set function address
		m_pDirect3D9EnableMaximizedWindowedModeShim = (Direct3D9EnableMaximizedWindowedModeShimProc)addr;
	}

	// Launch function to disable Maximized Windowed Mode
	Logging::Log() << __FUNCTION__ << " Disabling MaximizedWindowedMode for Direct3D9! Ret = " << (void*)m_pDirect3D9EnableMaximizedWindowedModeShim(0);
	return true;
}

IDirect3D9* WINAPI d9_Direct3DCreate9(UINT SDKVersion)
{
	LOG_LIMIT(1, __FUNCTION__);

	static Direct3DCreate9Proc m_pDirect3DCreate9 = (Wrapper::ValidProcAddress(Direct3DCreate9_out)) ? (Direct3DCreate9Proc)Direct3DCreate9_out : nullptr;

	if (!m_pDirect3DCreate9)
	{
		return nullptr;
	}

	if (Config.ForceDirect3D9On12 && Direct3DCreate9On12_out)
	{
		// Setup arguments
		D3D9ON12_ARGS args;
		memset(&args, 0, sizeof(args));
		args.Enable9On12 = TRUE;

		// Call function
		return d9_Direct3DCreate9On12(SDKVersion, &args, 1);
	}

	if (Config.GraphicsHybridAdapter)
	{
		SetGraphicsHybridAdapter(Config.GraphicsHybridAdapter);
	}

	// Disable MaxWindowedMode
	if (Config.DXPrimaryEmulation[AppCompatDataType.DisableMaxWindowedMode])
	{
		Direct3D9DisableMaximizedWindowedMode();
	}

	LOG_LIMIT(3, "Redirecting 'Direct3DCreate9' ...");

	// Create new d3d9 object
	IDirect3D9* pD3D9 = m_pDirect3DCreate9(SDKVersion);

	if (pD3D9)
	{
		return new m_IDirect3D9Ex((IDirect3D9Ex*)pD3D9, IID_IDirect3D9);
	}

	return nullptr;
}

HRESULT WINAPI d9_Direct3DCreate9Ex(UINT SDKVersion, IDirect3D9Ex** ppD3D)
{
	LOG_LIMIT(1, __FUNCTION__);

	static Direct3DCreate9ExProc m_pDirect3DCreate9Ex = (Wrapper::ValidProcAddress(Direct3DCreate9Ex_out)) ? (Direct3DCreate9ExProc)Direct3DCreate9Ex_out : nullptr;

	if (!m_pDirect3DCreate9Ex)
	{
		return D3DERR_INVALIDCALL;
	}

	if (Config.ForceDirect3D9On12 && Direct3DCreate9On12Ex_out)
	{
		// Setup arguments
		D3D9ON12_ARGS args;
		memset(&args, 0, sizeof(args));
		args.Enable9On12 = TRUE;

		// Call function
		return d9_Direct3DCreate9On12Ex(SDKVersion, &args, 1, ppD3D);
	}

	if (Config.GraphicsHybridAdapter)
	{
		SetGraphicsHybridAdapter(Config.GraphicsHybridAdapter);
	}

	// Disable MaxWindowedMode
	if (Config.DXPrimaryEmulation[AppCompatDataType.DisableMaxWindowedMode])
	{
		Direct3D9DisableMaximizedWindowedMode();
	}

	LOG_LIMIT(3, "Redirecting 'Direct3DCreate9Ex' ...");

	HRESULT hr = m_pDirect3DCreate9Ex(SDKVersion, ppD3D);

	if (SUCCEEDED(hr) && ppD3D)
	{
		*ppD3D = new m_IDirect3D9Ex(*ppD3D, IID_IDirect3D9Ex);
	}

	return hr;
}

IDirect3D9* WINAPI d9_Direct3DCreate9On12(UINT SDKVersion, D3D9ON12_ARGS* pOverrideList, UINT NumOverrideEntries)
{
	LOG_LIMIT(1, __FUNCTION__);

	static Direct3DCreate9On12Proc m_pDirect3DCreate9On12 = (Wrapper::ValidProcAddress(Direct3DCreate9On12_out)) ? (Direct3DCreate9On12Proc)Direct3DCreate9On12_out : nullptr;

	if (!m_pDirect3DCreate9On12)
	{
		return d9_Direct3DCreate9(SDKVersion);
	}

	if (Config.GraphicsHybridAdapter)
	{
		SetGraphicsHybridAdapter(Config.GraphicsHybridAdapter);
	}

	// Disable MaxWindowedMode
	if (Config.DXPrimaryEmulation[AppCompatDataType.DisableMaxWindowedMode])
	{
		Direct3D9DisableMaximizedWindowedMode();
	}

	LOG_LIMIT(3, "Redirecting 'Direct3DCreate9On12' ...");

	if (Config.ForceDirect3D9On12 && pOverrideList)
	{
		pOverrideList->Enable9On12 = TRUE;
	}

	// Create new d3d9 object
	IDirect3D9* pD3D9 = m_pDirect3DCreate9On12(SDKVersion, pOverrideList, NumOverrideEntries);

	if (pD3D9)
	{
		return new m_IDirect3D9Ex((IDirect3D9Ex*)pD3D9, IID_IDirect3D9);
	}

	return nullptr;
}

HRESULT WINAPI d9_Direct3DCreate9On12Ex(UINT SDKVersion, D3D9ON12_ARGS* pOverrideList, UINT NumOverrideEntries, IDirect3D9Ex** ppOutputInterface)
{
	LOG_LIMIT(1, __FUNCTION__);

	static Direct3DCreate9On12ExProc m_pDirect3DCreate9On12Ex = (Wrapper::ValidProcAddress(Direct3DCreate9On12Ex_out)) ? (Direct3DCreate9On12ExProc)Direct3DCreate9On12Ex_out : nullptr;

	if (!m_pDirect3DCreate9On12Ex)
	{
		return d9_Direct3DCreate9Ex(SDKVersion, ppOutputInterface);
	}

	if (Config.GraphicsHybridAdapter)
	{
		SetGraphicsHybridAdapter(Config.GraphicsHybridAdapter);
	}

	// Disable MaxWindowedMode
	if (Config.DXPrimaryEmulation[AppCompatDataType.DisableMaxWindowedMode])
	{
		Direct3D9DisableMaximizedWindowedMode();
	}

	LOG_LIMIT(3, "Redirecting 'Direct3DCreate9On12Ex' ...");

	if (Config.ForceDirect3D9On12 && pOverrideList)
	{
		pOverrideList->Enable9On12 = TRUE;
	}

	HRESULT hr = m_pDirect3DCreate9On12Ex(SDKVersion, pOverrideList, NumOverrideEntries, ppOutputInterface);

	if (SUCCEEDED(hr) && ppOutputInterface)
	{
		*ppOutputInterface = new m_IDirect3D9Ex(*ppOutputInterface, IID_IDirect3D9Ex);
	}

	return hr;
}
