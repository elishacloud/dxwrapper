/**
* Copyright (C) 2024 Elisha Riedlinger
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
* SetAppCompatData code created based on information from here:
* http://web.archive.org/web/20170418171908/http://www.blitzbasic.com/Community/posts.php?topic=99477
*
* D3DParseUnknownCommand created from source code found in ReactOS
* https://doxygen.reactos.org/d3/d02/dll_2directx_2ddraw_2main_8c.html#af9a1eb1ced046770ad6f79838cc8517d
*/

#include "ddraw.h"
#include "Dllmain\Dllmain.h"
#include "IClassFactory\IClassFactory.h"
#include "d3d9\d3d9External.h"
#include "Utils\Utils.h"
#include "GDI\GDI.h"
#include "External\Hooking\Hook.h"

AddressLookupTableDdraw<void> ProxyAddressLookupTable = AddressLookupTableDdraw<void>();

namespace DdrawWrapper
{
	VISIT_PROCS_DDRAW(INITIALIZE_OUT_WRAPPED_PROC);
	VISIT_PROCS_DDRAW_SHARED(INITIALIZE_OUT_WRAPPED_PROC);
	INITIALIZE_OUT_WRAPPED_PROC(Direct3DCreate9, unused);
}

using namespace DdrawWrapper;

namespace {
	CRITICAL_SECTION ddcs;
	bool IsInitialized = false;

	struct ENUMMONITORS
	{
		LPSTR lpName;
		HMONITOR hm;
	};

	std::vector<m_IDirectDrawClipper*> BaseClipperVector;
}

static void SetAllAppCompatData();
static HRESULT DirectDrawEnumerateHandler(LPVOID lpCallback, LPVOID lpContext, DWORD dwFlags, DirectDrawEnumerateTypes DDETType);

// ******************************
// ddraw.dll export functions
// ******************************

HRESULT WINAPI dd_AcquireDDThreadLock()
{
	LOG_LIMIT(1, __FUNCTION__);

	if (Config.Dd7to9)
	{
		if (IsInitialized)
		{
			EnterCriticalSection(&ddcs);
			return DD_OK;
		}
		return DDERR_UNSUPPORTED;
	}

	DEFINE_STATIC_PROC_ADDRESS(AcquireDDThreadLockProc, AcquireDDThreadLock, AcquireDDThreadLock_out);

	if (!AcquireDDThreadLock)
	{
		return DDERR_UNSUPPORTED;
	}

	return AcquireDDThreadLock();
}

DWORD WINAPI dd_CompleteCreateSysmemSurface(DWORD arg)
{
	LOG_LIMIT(1, __FUNCTION__);

	if (Config.Dd7to9)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");
		return NULL;
	}

	DEFINE_STATIC_PROC_ADDRESS(CompleteCreateSysmemSurfaceProc, CompleteCreateSysmemSurface, CompleteCreateSysmemSurface_out);

	if (!CompleteCreateSysmemSurface)
	{
		return NULL;
	}

	return CompleteCreateSysmemSurface(arg);
}

HRESULT WINAPI dd_D3DParseUnknownCommand(LPVOID lpCmd, LPVOID *lpRetCmd)
{
	LOG_LIMIT(1, __FUNCTION__);

	if (Config.Dd7to9)
	{
		if (!lpCmd || !lpRetCmd)
		{
			return DDERR_INVALIDPARAMS;
		}

		LPD3DHAL_DP2COMMAND dp2command = (LPD3DHAL_DP2COMMAND)lpCmd;

		/* check for valid command, only 3 commands are valid */
		switch (dp2command->bCommand)
		{
		case D3DDP2OP_VIEWPORTINFO:
			*lpRetCmd = (LPVOID)((DWORD)lpCmd + (dp2command->wStateCount * sizeof(D3DHAL_DP2VIEWPORTINFO)) + sizeof(D3DHAL_DP2COMMAND));
			break;

		case D3DDP2OP_WINFO:
			*lpRetCmd = (LPVOID)((DWORD)lpCmd + (dp2command->wStateCount * sizeof(D3DHAL_DP2WINFO)) + sizeof(D3DHAL_DP2COMMAND));
			break;

		case 0x0d: /* Undocumented in MSDN */
			*lpRetCmd = (LPVOID)((DWORD)lpCmd + (dp2command->wStateCount * dp2command->bReserved) + sizeof(D3DHAL_DP2COMMAND));
			break;

		default:   /* set the error code */
			if ((dp2command->bCommand <= D3DDP2OP_INDEXEDTRIANGLELIST) || // dp2command->bCommand  <= with 0 to 3
				(dp2command->bCommand == D3DDP2OP_RENDERSTATE) ||  // dp2command->bCommand  == with 8
				(dp2command->bCommand >= D3DDP2OP_LINELIST))  // dp2command->bCommand  >= with 15 to 255
			{
				/* set error code for command 0 to 3, 8 and 15 to 255 */
				return DDERR_INVALIDPARAMS;
			}
			else
			{
				/* set error code for 4 - 7, 9 - 12, 14  */
				return D3DERR_COMMAND_UNPARSED;
			}
		}

		return DD_OK;
	}

	DEFINE_STATIC_PROC_ADDRESS(D3DParseUnknownCommandProc, D3DParseUnknownCommand, D3DParseUnknownCommand_out);

	if (!D3DParseUnknownCommand)
	{
		return D3DERR_COMMAND_UNPARSED;
	}

	return D3DParseUnknownCommand(lpCmd, lpRetCmd);
}

HRESULT WINAPI dd_DDGetAttachedSurfaceLcl(DWORD arg1, DWORD arg2, DWORD arg3)
{
	LOG_LIMIT(1, __FUNCTION__);

	if (Config.Dd7to9)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	DEFINE_STATIC_PROC_ADDRESS(DDGetAttachedSurfaceLclProc, DDGetAttachedSurfaceLcl, DDGetAttachedSurfaceLcl_out);

	if (!DDGetAttachedSurfaceLcl)
	{
		return DDERR_UNSUPPORTED;
	}

	return DDGetAttachedSurfaceLcl(arg1, arg2, arg3);
}

DWORD WINAPI dd_DDInternalLock(DWORD arg1, DWORD arg2)
{
	LOG_LIMIT(1, __FUNCTION__);

	if (Config.Dd7to9)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");
		return 0xFFFFFFFF;
	}

	DEFINE_STATIC_PROC_ADDRESS(DDInternalLockProc, DDInternalLock, DDInternalLock_out);

	if (!DDInternalLock)
	{
		return 0xFFFFFFFF;
	}

	return DDInternalLock(arg1, arg2);
}

DWORD WINAPI dd_DDInternalUnlock(DWORD arg)
{
	LOG_LIMIT(1, __FUNCTION__);

	if (Config.Dd7to9)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");
		return 0xFFFFFFFF;
	}

	DEFINE_STATIC_PROC_ADDRESS(DDInternalUnlockProc, DDInternalUnlock, DDInternalUnlock_out);

	if (!DDInternalUnlock)
	{
		return 0xFFFFFFFF;
	}

	return DDInternalUnlock(arg);
}

HRESULT WINAPI dd_DSoundHelp(DWORD arg1, DWORD arg2, DWORD arg3)
{
	LOG_LIMIT(1, __FUNCTION__);

	if (Config.Dd7to9)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	DEFINE_STATIC_PROC_ADDRESS(DSoundHelpProc, DSoundHelp, DSoundHelp_out);

	if (!DSoundHelp)
	{
		return DDERR_UNSUPPORTED;
	}

	return DSoundHelp(arg1, arg2, arg3);
}

HRESULT WINAPI dd_DirectDrawCreate(GUID FAR *lpGUID, LPDIRECTDRAW FAR *lplpDD, IUnknown FAR *pUnkOuter)
{
	LOG_LIMIT(1, __FUNCTION__);

	if (Config.Dd7to9)
	{
		LOG_LIMIT(3, "Redirecting 'DirectDrawCreate' to --> 'Direct3DCreate9'");

		if (Config.SetSwapEffectShim < 2)
		{
			Direct3D9SetSwapEffectUpgradeShim(Config.SetSwapEffectShim);
		}

		ScopedDDCriticalSection ThreadLockDD;

		m_IDirectDrawX* p_IDirectDrawX = new m_IDirectDrawX(1, false);

		*lplpDD = reinterpret_cast<LPDIRECTDRAW>(p_IDirectDrawX->GetWrapperInterfaceX(1));

		// Success
		return DD_OK;
	}

	DEFINE_STATIC_PROC_ADDRESS(DirectDrawCreateProc, DirectDrawCreate, DirectDrawCreate_out);

	if (!DirectDrawCreate)
	{
		return DDERR_UNSUPPORTED;
	}

	// Set AppCompatData
	if (Config.isAppCompatDataSet)
	{
		SetAllAppCompatData();
	}

	LOG_LIMIT(3, "Redirecting 'DirectDrawCreate' ...");

	HRESULT hr = DirectDrawCreate(lpGUID, lplpDD, pUnkOuter);

	if (SUCCEEDED(hr) && lplpDD && *lplpDD)
	{
		m_IDirectDrawX* Interface = new m_IDirectDrawX((IDirectDraw7*)*lplpDD, 1);

		*lplpDD = (LPDIRECTDRAW)Interface->GetWrapperInterfaceX(1);
	}

	return hr;
}

HRESULT WINAPI dd_DirectDrawCreateClipper(DWORD dwFlags, LPDIRECTDRAWCLIPPER *lplpDDClipper, LPUNKNOWN pUnkOuter)
{
	LOG_LIMIT(1, __FUNCTION__);

	if (Config.Dd7to9)
	{
		if (!lplpDDClipper || pUnkOuter)
		{
			return DDERR_INVALIDPARAMS;
		}

		ScopedDDCriticalSection ThreadLockDD;

		m_IDirectDrawClipper* ClipperX = m_IDirectDrawClipper::CreateDirectDrawClipper(nullptr, nullptr, dwFlags);

		AddBaseClipper(ClipperX);

		*lplpDDClipper = ClipperX;

		return DD_OK;
	}

	DEFINE_STATIC_PROC_ADDRESS(DirectDrawCreateClipperProc, DirectDrawCreateClipper, DirectDrawCreateClipper_out);

	if (!DirectDrawCreateClipper)
	{
		return DDERR_UNSUPPORTED;
	}

	HRESULT hr = DirectDrawCreateClipper(dwFlags, lplpDDClipper, pUnkOuter);

	if (SUCCEEDED(hr) && lplpDDClipper)
	{
		*lplpDDClipper = m_IDirectDrawClipper::CreateDirectDrawClipper(*lplpDDClipper, nullptr, dwFlags);
	}

	return hr;
}

HRESULT WINAPI dd_DirectDrawCreateEx(GUID FAR *lpGUID, LPVOID *lplpDD, REFIID riid, IUnknown FAR *pUnkOuter)
{
	LOG_LIMIT(1, __FUNCTION__);

	if (Config.Dd7to9)
	{
		if (!lplpDD || pUnkOuter)
		{
			return DDERR_INVALIDPARAMS;
		}

		if (riid != IID_IDirectDraw7)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: invalid IID " << riid);
			return DDERR_INVALIDPARAMS;
		}

		LOG_LIMIT(3, "Redirecting 'DirectDrawCreateEx' to --> 'Direct3DCreate9'");

		if (Config.SetSwapEffectShim < 2)
		{
			Direct3D9SetSwapEffectUpgradeShim(Config.SetSwapEffectShim);
		}

		ScopedDDCriticalSection ThreadLockDD;

		m_IDirectDrawX *p_IDirectDrawX = new m_IDirectDrawX(7, true);

		*lplpDD = p_IDirectDrawX->GetWrapperInterfaceX(7);

		// Success
		return DD_OK;
	}

	DEFINE_STATIC_PROC_ADDRESS(DirectDrawCreateExProc, DirectDrawCreateEx, DirectDrawCreateEx_out);

	if (!DirectDrawCreateEx)
	{
		return DDERR_UNSUPPORTED;
	}

	// Set AppCompatData
	if (Config.isAppCompatDataSet)
	{
		SetAllAppCompatData();
	}

	LOG_LIMIT(3, "Redirecting 'DirectDrawCreateEx' ...");

	HRESULT hr = DirectDrawCreateEx(lpGUID, lplpDD, IID_IDirectDraw7, pUnkOuter);

	if (SUCCEEDED(hr))
	{
		DWORD DxVersion = GetGUIDVersion(riid);

		m_IDirectDrawX *p_IDirectDrawX = new m_IDirectDrawX((IDirectDraw7*)*lplpDD, DxVersion);

		*lplpDD = p_IDirectDrawX->GetWrapperInterfaceX(DxVersion);
	}

	return hr;
}

HRESULT WINAPI dd_DirectDrawEnumerateA(LPDDENUMCALLBACKA lpCallback, LPVOID lpContext)
{
	LOG_LIMIT(1, __FUNCTION__);

	if (Config.Dd7to9)
	{
		return DirectDrawEnumerateHandler(lpCallback, lpContext, 0, DDET_ENUMCALLBACKA);
	}

	DEFINE_STATIC_PROC_ADDRESS(DirectDrawEnumerateAProc, DirectDrawEnumerateA, DirectDrawEnumerateA_out);

	if (!DirectDrawEnumerateA)
	{
		return DDERR_UNSUPPORTED;
	}

	return DirectDrawEnumerateA(lpCallback, lpContext);
}

HRESULT WINAPI dd_DirectDrawEnumerateExA(LPDDENUMCALLBACKEXA lpCallback, LPVOID lpContext, DWORD dwFlags)
{
	LOG_LIMIT(1, __FUNCTION__);

	if (Config.Dd7to9)
	{
		return DirectDrawEnumerateHandler(lpCallback, lpContext, dwFlags, DDET_ENUMCALLBACKEXA);
	}

	DEFINE_STATIC_PROC_ADDRESS(DirectDrawEnumerateExAProc, DirectDrawEnumerateExA, DirectDrawEnumerateExA_out);

	if (!DirectDrawEnumerateExA)
	{
		return DDERR_UNSUPPORTED;
	}

	return DirectDrawEnumerateExA(lpCallback, lpContext, dwFlags);
}

HRESULT WINAPI dd_DirectDrawEnumerateExW(LPDDENUMCALLBACKEXW lpCallback, LPVOID lpContext, DWORD dwFlags)
{
	LOG_LIMIT(1, __FUNCTION__);

	if (Config.Dd7to9)
	{
		return DirectDrawEnumerateHandler(lpCallback, lpContext, dwFlags, DDET_ENUMCALLBACKEXW);
	}

	DEFINE_STATIC_PROC_ADDRESS(DirectDrawEnumerateExWProc, DirectDrawEnumerateExW, DirectDrawEnumerateExW_out);

	if (!DirectDrawEnumerateExW)
	{
		return DDERR_UNSUPPORTED;
	}

	return DirectDrawEnumerateExW(lpCallback, lpContext, dwFlags);
}

HRESULT WINAPI dd_DirectDrawEnumerateW(LPDDENUMCALLBACKW lpCallback, LPVOID lpContext)
{
	LOG_LIMIT(1, __FUNCTION__);

	if (Config.Dd7to9)
	{
		return DirectDrawEnumerateHandler(lpCallback, lpContext, 0, DDET_ENUMCALLBACKW);
	}

	DEFINE_STATIC_PROC_ADDRESS(DirectDrawEnumerateWProc, DirectDrawEnumerateW, DirectDrawEnumerateW_out);

	if (!DirectDrawEnumerateW)
	{
		return DDERR_UNSUPPORTED;
	}

	return DirectDrawEnumerateW(lpCallback, lpContext);
}

HRESULT WINAPI dd_DllCanUnloadNow()
{
	LOG_LIMIT(1, __FUNCTION__);

	if (Config.Dd7to9)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	DEFINE_STATIC_PROC_ADDRESS(DllCanUnloadNowProc, DllCanUnloadNow, DllCanUnloadNow_out);

	if (!DllCanUnloadNow)
	{
		return DDERR_UNSUPPORTED;
	}

	return DllCanUnloadNow();
}

HRESULT WINAPI dd_DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID *ppv)
{
	LOG_LIMIT(1, __FUNCTION__);

	if (Config.Dd7to9)
	{
		if (!ppv)
		{
			return E_POINTER;
		}

		HRESULT hr = ProxyQueryInterface(nullptr, riid, ppv, rclsid);

		if (SUCCEEDED(hr) && ppv && *ppv)
		{
			if (riid == IID_IClassFactory)
			{
				((m_IClassFactory*)(*ppv))->SetCLSID(rclsid);
			}
			((IUnknown*)ppv)->AddRef();
		}

		return hr;
	}

	DEFINE_STATIC_PROC_ADDRESS(DllGetClassObjectProc, DllGetClassObject, DllGetClassObject_out);

	if (!DllGetClassObject)
	{
		return DDERR_UNSUPPORTED;
	}

	HRESULT hr = DllGetClassObject(rclsid, riid, ppv);

	if (SUCCEEDED(hr) && ppv)
	{
		if (riid == IID_IClassFactory)
		{
			*ppv = new m_IClassFactory((IClassFactory*)*ppv, genericQueryInterface);

			((m_IClassFactory*)(*ppv))->SetCLSID(rclsid);

			return DD_OK;
		}

		genericQueryInterface(riid, ppv);
	}

	return hr;
}

HRESULT WINAPI dd_GetDDSurfaceLocal(DWORD arg1, DWORD arg2, DWORD arg3)
{
	LOG_LIMIT(1, __FUNCTION__);

	if (Config.Dd7to9)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	DEFINE_STATIC_PROC_ADDRESS(GetDDSurfaceLocalProc, GetDDSurfaceLocal, GetDDSurfaceLocal_out);

	if (!GetDDSurfaceLocal)
	{
		return DDERR_UNSUPPORTED;
	}

	return GetDDSurfaceLocal(arg1, arg2, arg3);
}

DWORD WINAPI dd_GetOLEThunkData(DWORD index)
{
	LOG_LIMIT(1, __FUNCTION__);

	if (Config.Dd7to9)
	{
		//	switch(index) 
		//	{
		//		case 1: return _dwLastFrameRate;
		//		case 2: return _lpDriverObjectList;
		//		case 3: return _lpAttachedProcesses;
		//		case 4: return 0; // does nothing?
		//		case 5: return _CheckExclusiveMode;
		//		case 6: return 0; // ReleaseExclusiveModeMutex
		//	}

		LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");
		return NULL;
	}

	DEFINE_STATIC_PROC_ADDRESS(GetOLEThunkDataProc, GetOLEThunkData, GetOLEThunkData_out);

	if (!GetOLEThunkData)
	{
		return NULL;
	}

	return GetOLEThunkData(index);
}

HRESULT WINAPI dd_GetSurfaceFromDC(HDC hdc, LPDIRECTDRAWSURFACE7 *lpDDS, DWORD arg)
{
	LOG_LIMIT(1, __FUNCTION__);

	if (Config.Dd7to9)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	DEFINE_STATIC_PROC_ADDRESS(GetSurfaceFromDCProc, GetSurfaceFromDC, GetSurfaceFromDC_out);

	if (!GetSurfaceFromDC)
	{
		return DDERR_UNSUPPORTED;
	}

	return GetSurfaceFromDC(hdc, lpDDS, arg);
}

HRESULT WINAPI dd_RegisterSpecialCase(DWORD arg1, DWORD arg2, DWORD arg3, DWORD arg4)
{
	LOG_LIMIT(1, __FUNCTION__);

	if (Config.Dd7to9)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	DEFINE_STATIC_PROC_ADDRESS(RegisterSpecialCaseProc, RegisterSpecialCase, RegisterSpecialCase_out);

	if (!RegisterSpecialCase)
	{
		return DDERR_UNSUPPORTED;
	}

	return RegisterSpecialCase(arg1, arg2, arg3, arg4);
}

HRESULT WINAPI dd_ReleaseDDThreadLock()
{
	LOG_LIMIT(1, __FUNCTION__);

	if (Config.Dd7to9)
	{
		if (IsInitialized)
		{
			LeaveCriticalSection(&ddcs);
			return DD_OK;
		}
		return DDERR_UNSUPPORTED;
	}

	DEFINE_STATIC_PROC_ADDRESS(ReleaseDDThreadLockProc, ReleaseDDThreadLock, ReleaseDDThreadLock_out);

	if (!ReleaseDDThreadLock)
	{
		return DDERR_UNSUPPORTED;
	}

	return ReleaseDDThreadLock();
}

HRESULT WINAPI dd_SetAppCompatData(DWORD Type, DWORD Value)
{
	LOG_LIMIT(1, __FUNCTION__);

	if (Config.Dd7to9)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Skipping compatibility flags: " << Type << " " << Value);
		return DD_OK;
	}

	DEFINE_STATIC_PROC_ADDRESS(SetAppCompatDataProc, SetAppCompatData, SetAppCompatData_out);

	if (!SetAppCompatData)
	{
		return DDERR_GENERIC;
	}

	return SetAppCompatData(Type, Value);
}

// ******************************
// Helper functions
// ******************************

void InitDDraw()
{
	if (!IsInitialized)
	{
		InitializeCriticalSection(&ddcs);
		IsInitialized = true;
	}

	// Hook other gdi32 and user32 APIs
	static bool RunOnce = true;
	if (RunOnce)
	{
		Logging::Log() << "Installing GDI & User32 hooks";
		using namespace GdiWrapper;
		if (!GetModuleHandleA("gdi32.dll")) LoadLibrary("gdi32.dll");
		if (!GetModuleHandleA("user32.dll")) LoadLibrary("user32.dll");
		HMODULE gdi32 = GetModuleHandleA("gdi32.dll");
		HMODULE user32 = GetModuleHandleA("user32.dll");
		HMODULE kernel32 = GetModuleHandleA("kernel32.dll");
		if (gdi32)
		{
			GetDeviceCaps_out = (FARPROC)Hook::HotPatch(GetProcAddress(gdi32, "GetDeviceCaps"), "GetDeviceCaps", gdi_GetDeviceCaps);
		}
		if (user32)
		{
			CreateWindowExA_out = (FARPROC)Hook::HotPatch(GetProcAddress(user32, "CreateWindowExA"), "CreateWindowExA", user_CreateWindowExA);
			CreateWindowExW_out = (FARPROC)Hook::HotPatch(GetProcAddress(user32, "CreateWindowExW"), "CreateWindowExW", user_CreateWindowExW);
			DestroyWindow_out = (FARPROC)Hook::HotPatch(GetProcAddress(user32, "DestroyWindow"), "DestroyWindow", user_DestroyWindow);
			GetSystemMetrics_out = (FARPROC)Hook::HotPatch(GetProcAddress(user32, "GetSystemMetrics"), "GetSystemMetrics", user_GetSystemMetrics);
			//GetWindowLongA_out = (FARPROC)Hook::HotPatch(GetProcAddress(user32, "GetWindowLongA"), "GetWindowLongA", user_GetWindowLongA);
			//GetWindowLongW_out = (FARPROC)Hook::HotPatch(GetProcAddress(user32, "GetWindowLongW"), "GetWindowLongW", user_GetWindowLongW);
			//SetWindowLongA_out = (FARPROC)Hook::HotPatch(GetProcAddress(user32, "SetWindowLongA"), "SetWindowLongA", user_SetWindowLongA);
			//SetWindowLongW_out = (FARPROC)Hook::HotPatch(GetProcAddress(user32, "SetWindowLongW"), "SetWindowLongW", user_SetWindowLongW);
		}
		if (kernel32)
		{
			Logging::Log() << "Installing Kernel32 hooks";
			Utils::GetDiskFreeSpaceA_out = (FARPROC)Hook::HotPatch(GetProcAddress(kernel32, "GetDiskFreeSpaceA"), "GetDiskFreeSpaceA", Utils::kernel_GetDiskFreeSpaceA);
			if (!Utils::CreateThread_out)
			{
				Utils::CreateThread_out = (FARPROC)Hook::HotPatch(GetProcAddress(kernel32, "CreateThread"), "CreateThread", Utils::kernel_CreateThread);
			}
			Utils::VirtualAlloc_out = (FARPROC)Hook::HotPatch(GetProcAddress(kernel32, "VirtualAlloc"), "VirtualAlloc", Utils::kernel_VirtualAlloc);
			//Utils::HeapAlloc_out = (FARPROC)Hook::HotPatch(GetProcAddress(kernel32, "HeapAlloc"), "HeapAlloc", Utils::kernel_HeapAlloc);
			Utils::HeapSize_out = (FARPROC)Hook::HotPatch(GetProcAddress(kernel32, "HeapSize"), "HeapSize", Utils::kernel_HeapSize);
		}
		RunOnce = false;
	}
}

void ExitDDraw()
{
	if (IsInitialized)
	{
		IsInitialized = false;
		DeleteCriticalSection(&ddcs);
	}
}

// Sets Application Compatibility Toolkit options for DXPrimaryEmulation using SetAppCompatData API
static void SetAllAppCompatData()
{
	DEFINE_STATIC_PROC_ADDRESS(SetAppCompatDataProc, SetAppCompatData, SetAppCompatData_out);

	if (!SetAppCompatData)
	{
		Logging::Log() << __FUNCTION__ << " Error: Failed to get `SetAppCompatData` address!";
		return;
	}

	// Set AppCompatData
	for (DWORD x = 1; x <= 12; x++)
	{
		if (Config.DXPrimaryEmulation[x])
		{
			Logging::Log() << __FUNCTION__ << " SetAppCompatData: " << x << " " << (DWORD)((x == AppCompatDataType.LockColorkey) ? AppCompatDataType.LockColorkey : 0);

			// For LockColorkey, this one uses the second parameter
			if (x == AppCompatDataType.LockColorkey)
			{
				SetAppCompatData(x, Config.LockColorkey);
			}
			// For all the other items
			else
			{
				SetAppCompatData(x, 0);
			}
		}
	}
	return;
}

static BOOL CALLBACK DispayEnumeratorProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
	UNREFERENCED_PARAMETER(hdcMonitor);
	UNREFERENCED_PARAMETER(lprcMonitor);

	ENUMMONITORS* lpMonitors = (ENUMMONITORS*)dwData;
	if (!dwData || !lpMonitors->lpName)
	{
		return DDENUMRET_CANCEL;
	}

	MONITORINFOEX monitorInfo;
	ZeroMemory(&monitorInfo, sizeof(monitorInfo));
	monitorInfo.cbSize = sizeof(monitorInfo);

	if (!GetMonitorInfo(hMonitor, &monitorInfo))
	{
		return DDENUMRET_OK;
	}

	if (strcmp(monitorInfo.szDevice, lpMonitors->lpName) == 0)
	{
		lpMonitors->hm = hMonitor;
		return DDENUMRET_CANCEL;
	}

	return DDENUMRET_OK;
}

static HRESULT DirectDrawEnumerateHandler(LPVOID lpCallback, LPVOID lpContext, DWORD dwFlags, DirectDrawEnumerateTypes DDETType)
{
	UNREFERENCED_PARAMETER(dwFlags);

	if (!lpCallback)
	{
		return DDERR_INVALIDPARAMS;
	}

	// Declare Direct3DCreate9
	DEFINE_STATIC_PROC_ADDRESS(Direct3DCreate9Proc, Direct3DCreate9, Direct3DCreate9_out);

	if (!Direct3DCreate9)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: failed to get 'Direct3DCreate9' ProcAddress of d3d9.dll!");
		return DDERR_UNSUPPORTED;
	}

	// Create Direct3D9 object
	ComPtr<IDirect3D9> d3d9Object(Direct3DCreate9(D3D_SDK_VERSION));

	// Error handling
	if (!d3d9Object)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: failed to create Direct3D9 object");
		return DDERR_UNSUPPORTED;
	}
	D3DADAPTER_IDENTIFIER9 Identifier = {};
	int AdapterCount = (!dwFlags) ? 0 : (int)d3d9Object->GetAdapterCount();

	GUID* lpGUID;
	LPSTR lpDesc, lpName;
	wchar_t lpwName[32] = { '\0' };
	wchar_t lpwDesc[128] = { '\0' };
	HMONITOR hm = nullptr;
	HRESULT hr = DD_OK;
	for (int x = -1; x < AdapterCount; x++)
	{
		if (x == -1)
		{
			lpGUID = nullptr;
			lpDesc = "Primary Display Driver";
			lpName = "display";
			hm = nullptr;
		}
		else
		{
			if (FAILED(d3d9Object->GetAdapterIdentifier(x, 0, &Identifier)))
			{
				hr = DDERR_UNSUPPORTED;
				break;
			}
			lpGUID = &Identifier.DeviceIdentifier;
			lpDesc = Identifier.Description;
			lpName = Identifier.DeviceName;

			if (DDETType == DDET_ENUMCALLBACKEXA || DDETType == DDET_ENUMCALLBACKEXW)
			{
				ENUMMONITORS Monitors = {};
				Monitors.lpName = lpName;
				Monitors.hm = nullptr;
				EnumDisplayMonitors(nullptr, nullptr, DispayEnumeratorProc, (LPARAM)&Monitors);
				hm = Monitors.hm;
			}
		}

		if (DDETType == DDET_ENUMCALLBACKEXW || DDETType == DDET_ENUMCALLBACKW)
		{
			size_t nReturn;
			mbstowcs_s(&nReturn, lpwName, lpName, 32);
			mbstowcs_s(&nReturn, lpwDesc, lpDesc, 128);
		}

		BOOL hr_Callback = DDENUMRET_OK;
		switch (DDETType)
		{
		case DDET_ENUMCALLBACKA:
			hr_Callback = LPDDENUMCALLBACKA(lpCallback)(lpGUID, lpDesc, lpName, lpContext);
			break;
		case DDET_ENUMCALLBACKEXA:
			hr_Callback = LPDDENUMCALLBACKEXA(lpCallback)(lpGUID, lpDesc, lpName, lpContext, hm);
			break;
		case DDET_ENUMCALLBACKEXW:
			hr_Callback = LPDDENUMCALLBACKEXW(lpCallback)(lpGUID, lpwDesc, lpwName, lpContext, hm);
			break;
		case DDET_ENUMCALLBACKW:
			hr_Callback = LPDDENUMCALLBACKW(lpCallback)(lpGUID, lpwDesc, lpwName, lpContext);
			break;
		default:
			hr_Callback = DDENUMRET_CANCEL;
			hr = DDERR_UNSUPPORTED;
		}

		if (hr_Callback == DDENUMRET_CANCEL)
		{
			break;
		}
	}

	return hr;
}

void AddBaseClipper(m_IDirectDrawClipper* lpClipper)
{
	if (!lpClipper || DoesBaseClipperExist(lpClipper))
	{
		return;
	}

	ScopedDDCriticalSection ThreadLockDD;

	BaseClipperVector.push_back(lpClipper);
}

void ClearBaseClipper(m_IDirectDrawClipper* lpClipper)
{
	if (!lpClipper)
	{
		return;
	}

	ScopedDDCriticalSection ThreadLockDD;

	BaseClipperVector.erase(std::remove(BaseClipperVector.begin(), BaseClipperVector.end(), lpClipper), BaseClipperVector.end());
}

bool DoesBaseClipperExist(m_IDirectDrawClipper* lpClipper)
{
	if (!lpClipper)
	{
		return false;
	}

	ScopedDDCriticalSection ThreadLockDD;

	const bool found = (std::find(BaseClipperVector.begin(), BaseClipperVector.end(), lpClipper) != std::end(BaseClipperVector));

	return found;
}
