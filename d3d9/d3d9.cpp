/**
* Copyright (C) 2025 Elisha Riedlinger
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
* Code for 'Direct3D9SetSwapEffectUpgradeShim' taken from here:  https://github.com/crosire/reshade/commit/3fe0b050706fb9f3510ed48d619cad71f7cb28f2
*/

#include "d3d9.h"
#include "External\Hooking\Hook.h"

namespace D3d9Wrapper
{
	VISIT_PROCS_D3D9(INITIALIZE_OUT_WRAPPED_PROC);
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

FARPROC GetD3d9UnnamedOrdinal(WORD Ordinal)
{
	FARPROC proc = nullptr;

	HMODULE dll = GetSystemD3d9();
	if (!dll)
	{
		Logging::Log() << __FUNCTION__ << " System32 d3d9.dll is not loaded!";
		return nullptr;
	}

	proc = GetProcAddress(dll, reinterpret_cast<LPCSTR>(Ordinal));

	bool FuncNameExists = Hook::CheckExportAddress(dll, proc);

	if (!proc || FuncNameExists)
	{
		Logging::Log() << __FUNCTION__ << " cannot find unnamed ordinal '" << Ordinal << "' in System32 d3d9.dll!";
		return nullptr;
	}

	return proc;
}

int WINAPI d9_D3DPERF_BeginEvent(D3DCOLOR col, LPCWSTR wszName)
{
	LOG_LIMIT(1, __FUNCTION__);

	DEFINE_STATIC_PROC_ADDRESS(D3DPERF_BeginEventProc, D3DPERF_BeginEvent, D3DPERF_BeginEvent_out);

	if (!D3DPERF_BeginEvent)
	{
		return NULL;
	}

	return D3DPERF_BeginEvent(col, wszName);
}

int WINAPI d9_D3DPERF_EndEvent()
{
	LOG_LIMIT(1, __FUNCTION__);

	DEFINE_STATIC_PROC_ADDRESS(D3DPERF_EndEventProc, D3DPERF_EndEvent, D3DPERF_EndEvent_out);

	if (!D3DPERF_EndEvent)
	{
		return NULL;
	}

	return D3DPERF_EndEvent();
}

DWORD WINAPI d9_D3DPERF_GetStatus()
{
	LOG_LIMIT(1, __FUNCTION__);

	DEFINE_STATIC_PROC_ADDRESS(D3DPERF_GetStatusProc, D3DPERF_GetStatus, D3DPERF_GetStatus_out);

	if (!D3DPERF_GetStatus)
	{
		return NULL;
	}

	return D3DPERF_GetStatus();
}

BOOL WINAPI d9_D3DPERF_QueryRepeatFrame()
{
	LOG_LIMIT(1, __FUNCTION__);

	DEFINE_STATIC_PROC_ADDRESS(D3DPERF_QueryRepeatFrameProc, D3DPERF_QueryRepeatFrame, D3DPERF_QueryRepeatFrame_out);

	if (!D3DPERF_QueryRepeatFrame)
	{
		return FALSE;
	}

	return D3DPERF_QueryRepeatFrame();
}

void WINAPI d9_D3DPERF_SetMarker(D3DCOLOR col, LPCWSTR wszName)
{
	LOG_LIMIT(1, __FUNCTION__);

	DEFINE_STATIC_PROC_ADDRESS(D3DPERF_SetMarkerProc, D3DPERF_SetMarker, D3DPERF_SetMarker_out);

	if (!D3DPERF_SetMarker)
	{
		return;
	}

	return D3DPERF_SetMarker(col, wszName);
}

void WINAPI d9_D3DPERF_SetOptions(DWORD dwOptions)
{
	LOG_LIMIT(1, __FUNCTION__);

	DEFINE_STATIC_PROC_ADDRESS(D3DPERF_SetOptionsProc, D3DPERF_SetOptions, D3DPERF_SetOptions_out);

	if (!D3DPERF_SetOptions)
	{
		return;
	}

	return D3DPERF_SetOptions(dwOptions);
}

void WINAPI d9_D3DPERF_SetRegion(D3DCOLOR col, LPCWSTR wszName)
{
	LOG_LIMIT(1, __FUNCTION__);

	DEFINE_STATIC_PROC_ADDRESS(D3DPERF_SetRegionProc, D3DPERF_SetRegion, D3DPERF_SetRegion_out);

	if (!D3DPERF_SetRegion)
	{
		return;
	}

	return D3DPERF_SetRegion(col, wszName);
}

void WINAPI Direct3D9ForceHybridEnumeration(UINT Mode)
{
	const WORD Ordinal = 16;

	static FARPROC proc = nullptr;
	if (!proc)
	{
		FARPROC addr = GetD3d9UnnamedOrdinal(Ordinal);

		if (!addr)
		{
			Logging::Log() << __FUNCTION__ << " Error: Failed to get address!";
			return;
		}

		if (memcmp(addr, "\x8B\xFF\x55\x8B\xEC\x8B\x45\x08\xA3", 9) != S_OK)
		{
			Logging::Log() << __FUNCTION__ << " Error: Failed to vaidate memory address!";
			return;
		}

		proc = addr;
	}

	Logging::Log() << __FUNCTION__ << " Calling 'Direct3D9ForceHybridEnumeration' ... " << Mode;

	reinterpret_cast<decltype(&Direct3D9ForceHybridEnumeration)>(proc)(Mode);
}

void WINAPI Direct3D9SetSwapEffectUpgradeShim(int Unknown)
{
	const WORD Ordinal = 18;

	static FARPROC proc = nullptr;
	if (!proc)
	{
		FARPROC addr = GetD3d9UnnamedOrdinal(Ordinal);

		if (!addr)
		{
			Logging::Log() << __FUNCTION__ << " Error: Failed to get address!";
			return;
		}

		if (memcmp(addr, "\x8B\xFF\x55\x8B\xEC\x8B\x0D", 7) != S_OK)
		{
			Logging::Log() << __FUNCTION__ << " Error: Failed to vaidate memory address!";
			return;
		}

		proc = addr;
	}

	Logging::Log() << __FUNCTION__ << " Calling 'Direct3D9SetSwapEffectUpgradeShim' ... " << Unknown;

	reinterpret_cast<decltype(&Direct3D9SetSwapEffectUpgradeShim)>(proc)(Unknown);
}

void WINAPI Direct3D9DisableMaximizedWindowedMode()
{
	static FARPROC proc = nullptr;

	if (!proc)
	{
		HMODULE dll = GetSystemD3d9();
		if (!dll)
		{
			Logging::Log() << __FUNCTION__ << " System32 d3d9.dll is not loaded!";
			return;
		}

		FARPROC addr = GetProcAddress(dll, "Direct3D9EnableMaximizedWindowedModeShim");
		if (!addr)
		{
			Logging::Log() << __FUNCTION__ << " Error: Failed to get address!";
			return;
		}

		if (memcmp(addr, "\x8B\xFF\x55\x8B\xEC\x6A\x01\xFF\x75\x08\xE8", 11) != S_OK)
		{
			Logging::Log() << __FUNCTION__ << " Error: Failed to vaidate memory address!";
			return;
		}

		DWORD Protect;
		if (VirtualProtect((LPVOID)((BYTE*)addr + 6), 1, PAGE_EXECUTE_READWRITE, &Protect) == FALSE)
		{
			Logging::Log() << __FUNCTION__ << " Error: Failed to VirtualProtect memory!";
			return;
		}
		*(BYTE*)((BYTE*)addr + 6) = 0;
		VirtualProtect((LPVOID)((BYTE*)addr + 6), 1, Protect, &Protect);

		proc = addr;
	}

	Logging::Log() << __FUNCTION__ << " Calling 'Direct3D9EnableMaximizedWindowedModeShim' to disable MaximizedWindowedMode ...";

	reinterpret_cast<decltype(&Direct3D9DisableMaximizedWindowedMode)>(proc)();

	return;
}

IDirect3D9* WINAPI d9_Direct3DCreate9(UINT SDKVersion)
{
	LOG_LIMIT(1, __FUNCTION__);

	DEFINE_STATIC_PROC_ADDRESS(Direct3DCreate9Proc, Direct3DCreate9, Direct3DCreate9_out);

	if (!Direct3DCreate9)
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
		Direct3D9ForceHybridEnumeration(Config.GraphicsHybridAdapter);
	}

	// Disable MaxWindowedMode
	if (Config.DXPrimaryEmulation[AppCompatDataType.DisableMaxWindowedMode])
	{
		Direct3D9DisableMaximizedWindowedMode();
	}

	LOG_LIMIT(3, "Redirecting 'Direct3DCreate9' ...");

	// Create new d3d9 object
	IDirect3D9* pD3D9 = Direct3DCreate9(SDKVersion);

	if (pD3D9)
	{
		return new m_IDirect3D9Ex((IDirect3D9Ex*)pD3D9, IID_IDirect3D9);
	}

	return nullptr;
}

HRESULT WINAPI d9_Direct3DCreate9Ex(UINT SDKVersion, IDirect3D9Ex** ppD3D)
{
	LOG_LIMIT(1, __FUNCTION__);

	DEFINE_STATIC_PROC_ADDRESS(Direct3DCreate9ExProc, Direct3DCreate9Ex, Direct3DCreate9Ex_out);

	if (!Direct3DCreate9Ex)
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
		Direct3D9ForceHybridEnumeration(Config.GraphicsHybridAdapter);
	}

	// Disable MaxWindowedMode
	if (Config.DXPrimaryEmulation[AppCompatDataType.DisableMaxWindowedMode])
	{
		Direct3D9DisableMaximizedWindowedMode();
	}

	LOG_LIMIT(3, "Redirecting 'Direct3DCreate9Ex' ...");

	HRESULT hr = Direct3DCreate9Ex(SDKVersion, ppD3D);

	if (SUCCEEDED(hr) && ppD3D)
	{
		*ppD3D = new m_IDirect3D9Ex(*ppD3D, IID_IDirect3D9Ex);
	}

	return hr;
}

IDirect3D9* WINAPI d9_Direct3DCreate9On12(UINT SDKVersion, D3D9ON12_ARGS* pOverrideList, UINT NumOverrideEntries)
{
	LOG_LIMIT(1, __FUNCTION__);

	DEFINE_STATIC_PROC_ADDRESS(Direct3DCreate9On12Proc, Direct3DCreate9On12, Direct3DCreate9On12_out);

	if (!Direct3DCreate9On12)
	{
		return d9_Direct3DCreate9(SDKVersion);
	}

	if (Config.GraphicsHybridAdapter)
	{
		Direct3D9ForceHybridEnumeration(Config.GraphicsHybridAdapter);
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
	IDirect3D9* pD3D9 = Direct3DCreate9On12(SDKVersion, pOverrideList, NumOverrideEntries);

	if (pD3D9)
	{
		return new m_IDirect3D9Ex((IDirect3D9Ex*)pD3D9, IID_IDirect3D9);
	}

	return nullptr;
}

HRESULT WINAPI d9_Direct3DCreate9On12Ex(UINT SDKVersion, D3D9ON12_ARGS* pOverrideList, UINT NumOverrideEntries, IDirect3D9Ex** ppOutputInterface)
{
	LOG_LIMIT(1, __FUNCTION__);

	DEFINE_STATIC_PROC_ADDRESS(Direct3DCreate9On12ExProc, Direct3DCreate9On12Ex, Direct3DCreate9On12Ex_out);

	if (!Direct3DCreate9On12Ex)
	{
		return d9_Direct3DCreate9Ex(SDKVersion, ppOutputInterface);
	}

	if (Config.GraphicsHybridAdapter)
	{
		Direct3D9ForceHybridEnumeration(Config.GraphicsHybridAdapter);
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

	HRESULT hr = Direct3DCreate9On12Ex(SDKVersion, pOverrideList, NumOverrideEntries, ppOutputInterface);

	if (SUCCEEDED(hr) && ppOutputInterface)
	{
		*ppOutputInterface = new m_IDirect3D9Ex(*ppOutputInterface, IID_IDirect3D9Ex);
	}

	return hr;
}

void AdjustWindow(HWND MainhWnd, LONG displayWidth, LONG displayHeight, bool isWindowed, bool EnableWindowMode, bool FullscreenWindowMode)
{
	m_IDirect3D9Ex::AdjustWindow(MainhWnd, displayWidth, displayHeight, isWindowed, EnableWindowMode, FullscreenWindowMode);
}
