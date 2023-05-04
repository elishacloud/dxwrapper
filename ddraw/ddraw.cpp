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
*
* SetAppCompatData code created based on information from here:
* http://web.archive.org/web/20170418171908/http://www.blitzbasic.com/Community/posts.php?topic=99477
*
* D3DParseUnknownCommand created from source code found in ReactOS
* https://doxygen.reactos.org/d3/d02/dll_2directx_2ddraw_2main_8c.html#af9a1eb1ced046770ad6f79838cc8517d
*/

#include "ddraw.h"
#include "d3dhal.h"
#include "ddrawExternal.h"
#include "DDrawCompat\DDrawCompatExternal.h"
#include "Dllmain\Dllmain.h"
#include "IClassFactory\IClassFactory.h"
#include "d3d9\d3d9External.h"

AddressLookupTableDdraw<void> ProxyAddressLookupTable = AddressLookupTableDdraw<void>();

// Store a list of clipper
std::vector<m_IDirectDrawClipper*> BaseClipperVector;

CRITICAL_SECTION ddcs;
bool IsInitialized = false;

struct ENUMMONITORS
{
	LPSTR lpName;
	HMONITOR hm;
};

#define INITIALIZE_WRAPPED_PROC(procName, unused) \
	FARPROC procName ## _out = nullptr;

namespace DdrawWrapper
{
	VISIT_PROCS_DDRAW(INITIALIZE_WRAPPED_PROC);
	VISIT_PROCS_DDRAW_SHARED(INITIALIZE_WRAPPED_PROC);
	FARPROC Direct3DCreate9_out = nullptr;
}

using namespace DdrawWrapper;

void InitDDraw()
{
	if (!IsInitialized)
	{
		InitializeCriticalSection(&ddcs);
		IsInitialized = true;
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
// http://web.archive.org/web/20170418171908/http://www.blitzbasic.com/Community/posts.php?topic=99477
void SetAllAppCompatData()
{
	static SetAppCompatDataProc m_pSetAppCompatData = (Wrapper::ValidProcAddress(SetAppCompatData_out)) ? (SetAppCompatDataProc)SetAppCompatData_out : nullptr;

	if (!m_pSetAppCompatData)
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
				m_pSetAppCompatData(x, Config.LockColorkey);
			}
			// For all the other items
			else
			{
				m_pSetAppCompatData(x, 0);
			}
		}
	}
	return;
}

HRESULT WINAPI dd_AcquireDDThreadLock()
{
	LOG_LIMIT(1, __FUNCTION__);

	if (Config.Dd7to9)
	{
		return SetCriticalSection();
	}

	static AcquireDDThreadLockProc m_pAcquireDDThreadLock = (Wrapper::ValidProcAddress(AcquireDDThreadLock_out)) ? (AcquireDDThreadLockProc)AcquireDDThreadLock_out : nullptr;

	if (!m_pAcquireDDThreadLock)
	{
		return DDERR_UNSUPPORTED;
	}

	return m_pAcquireDDThreadLock();
}

DWORD WINAPI dd_CompleteCreateSysmemSurface(DWORD arg)
{
	LOG_LIMIT(1, __FUNCTION__);

	if (Config.Dd7to9)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");
		return NULL;
	}

	static CompleteCreateSysmemSurfaceProc m_pCompleteCreateSysmemSurface = (Wrapper::ValidProcAddress(CompleteCreateSysmemSurface_out)) ? (CompleteCreateSysmemSurfaceProc)CompleteCreateSysmemSurface_out : nullptr;

	if (!m_pCompleteCreateSysmemSurface)
	{
		return NULL;
	}

	return m_pCompleteCreateSysmemSurface(arg);
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

	static D3DParseUnknownCommandProc m_pD3DParseUnknownCommand = (Wrapper::ValidProcAddress(D3DParseUnknownCommand_out)) ? (D3DParseUnknownCommandProc)D3DParseUnknownCommand_out : nullptr;

	if (!m_pD3DParseUnknownCommand)
	{
		return D3DERR_COMMAND_UNPARSED;
	}

	return m_pD3DParseUnknownCommand(lpCmd, lpRetCmd);
}

HRESULT WINAPI dd_DDGetAttachedSurfaceLcl(DWORD arg1, DWORD arg2, DWORD arg3)
{
	LOG_LIMIT(1, __FUNCTION__);

	if (Config.Dd7to9)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	static DDGetAttachedSurfaceLclProc m_pDDGetAttachedSurfaceLcl = (Wrapper::ValidProcAddress(DDGetAttachedSurfaceLcl_out)) ? (DDGetAttachedSurfaceLclProc)DDGetAttachedSurfaceLcl_out : nullptr;

	if (!m_pDDGetAttachedSurfaceLcl)
	{
		return DDERR_UNSUPPORTED;
	}

	return m_pDDGetAttachedSurfaceLcl(arg1, arg2, arg3);
}

DWORD WINAPI dd_DDInternalLock(DWORD arg1, DWORD arg2)
{
	LOG_LIMIT(1, __FUNCTION__);

	if (Config.Dd7to9)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");
		return 0xFFFFFFFF;
	}

	static DDInternalLockProc m_pDDInternalLock = (Wrapper::ValidProcAddress(DDInternalLock_out)) ? (DDInternalLockProc)DDInternalLock_out : nullptr;

	if (!m_pDDInternalLock)
	{
		return 0xFFFFFFFF;
	}

	return m_pDDInternalLock(arg1, arg2);
}

DWORD WINAPI dd_DDInternalUnlock(DWORD arg)
{
	LOG_LIMIT(1, __FUNCTION__);

	if (Config.Dd7to9)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");
		return 0xFFFFFFFF;
	}

	static DDInternalUnlockProc m_pDDInternalUnlock = (Wrapper::ValidProcAddress(DDInternalUnlock_out)) ? (DDInternalUnlockProc)DDInternalUnlock_out : nullptr;

	if (!m_pDDInternalUnlock)
	{
		return 0xFFFFFFFF;
	}

	return m_pDDInternalUnlock(arg);
}

HRESULT WINAPI dd_DSoundHelp(DWORD arg1, DWORD arg2, DWORD arg3)
{
	LOG_LIMIT(1, __FUNCTION__);

	if (Config.Dd7to9)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	static DSoundHelpProc m_pDSoundHelp = (Wrapper::ValidProcAddress(DSoundHelp_out)) ? (DSoundHelpProc)DSoundHelp_out : nullptr;

	if (!m_pDSoundHelp)
	{
		return DDERR_UNSUPPORTED;
	}

	return m_pDSoundHelp(arg1, arg2, arg3);
}

HRESULT WINAPI dd_DirectDrawCreate(GUID FAR *lpGUID, LPDIRECTDRAW FAR *lplpDD, IUnknown FAR *pUnkOuter)
{
	LOG_LIMIT(1, __FUNCTION__);

	if (Config.Dd7to9 || (Config.ConvertToDirect3D7 && Config.ConvertToDirectDraw7))
	{
		return dd_DirectDrawCreateEx(lpGUID, (LPVOID*)lplpDD, IID_IDirectDraw, pUnkOuter);
	}

	static DirectDrawCreateProc m_pDirectDrawCreate = (Wrapper::ValidProcAddress(DirectDrawCreate_out)) ? (DirectDrawCreateProc)DirectDrawCreate_out : nullptr;

	if (!m_pDirectDrawCreate)
	{
		return DDERR_UNSUPPORTED;
	}

	// Set AppCompatData
	if (Config.isAppCompatDataSet)
	{
		SetAllAppCompatData();
	}

	LOG_LIMIT(3, "Redirecting 'DirectDrawCreate' ...");

	HRESULT hr = m_pDirectDrawCreate(lpGUID, lplpDD, pUnkOuter);

	if (SUCCEEDED(hr) && lplpDD && *lplpDD)
	{
		// Convert to new DirectDraw version
		if (Config.ConvertToDirectDraw7)
		{
			LPDIRECTDRAW lpDD = (LPDIRECTDRAW)*lplpDD;

			hr = lpDD->QueryInterface(ConvertREFIID(IID_IDirectDraw7), (LPVOID*)lplpDD);

			if (SUCCEEDED(hr))
			{
				m_IDirectDrawX *Interface = new m_IDirectDrawX((IDirectDraw7*)*lplpDD, 7);

				*lplpDD = (LPDIRECTDRAW)Interface->GetWrapperInterfaceX(1);

				lpDD->Release();
			}
		}
		else
		{
			m_IDirectDrawX *Interface = new m_IDirectDrawX((IDirectDraw7*)*lplpDD, 1);

			*lplpDD = (LPDIRECTDRAW)Interface->GetWrapperInterfaceX(1);
		}
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

		m_IDirectDrawClipper* ClipperX = new m_IDirectDrawClipper(nullptr, dwFlags);

		AddBaseClipperToVetor(ClipperX);

		*lplpDDClipper = ClipperX;

		return DD_OK;
	}

	static DirectDrawCreateClipperProc m_pDirectDrawCreateClipper = (Wrapper::ValidProcAddress(DirectDrawCreateClipper_out)) ? (DirectDrawCreateClipperProc)DirectDrawCreateClipper_out : nullptr;

	if (!m_pDirectDrawCreateClipper)
	{
		return DDERR_UNSUPPORTED;
	}

	HRESULT hr = m_pDirectDrawCreateClipper(dwFlags, lplpDDClipper, pUnkOuter);

	if (SUCCEEDED(hr) && lplpDDClipper)
	{
		*lplpDDClipper = new m_IDirectDrawClipper(*lplpDDClipper);
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

		if (riid != IID_IDirectDraw &&
			riid != IID_IDirectDraw2 &&
			riid != IID_IDirectDraw3 &&
			riid != IID_IDirectDraw4 &&
			riid != IID_IDirectDraw7)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: invalid IID " << riid);
			return DDERR_INVALIDPARAMS;
		}

		// Install DDrawCompat hooks
		if (Config.DDrawCompat)
		{
			DDrawCompat::InstallHooks();
		}

		DWORD DxVersion = GetGUIDVersion(riid);

		LOG_LIMIT(3, "Redirecting 'DirectDrawCreate' " << riid << " to --> 'Direct3DCreate9'");

		m_IDirectDrawX *p_IDirectDrawX = new m_IDirectDrawX(DxVersion);

		*lplpDD = p_IDirectDrawX->GetWrapperInterfaceX(DxVersion);

		// Success
		return DD_OK;
	}

	static DirectDrawCreateExProc m_pDirectDrawCreateEx = (Wrapper::ValidProcAddress(DirectDrawCreateEx_out)) ? (DirectDrawCreateExProc)DirectDrawCreateEx_out : nullptr;

	if (!m_pDirectDrawCreateEx)
	{
		return DDERR_UNSUPPORTED;
	}

	// Set AppCompatData
	if (Config.isAppCompatDataSet)
	{
		SetAllAppCompatData();
	}

	LOG_LIMIT(3, "Redirecting 'DirectDrawCreateEx' ...");

	HRESULT hr = m_pDirectDrawCreateEx(lpGUID, lplpDD, IID_IDirectDraw7, pUnkOuter);

	if (SUCCEEDED(hr))
	{
		DWORD DxVersion = GetGUIDVersion(riid);

		m_IDirectDrawX *p_IDirectDrawX = new m_IDirectDrawX((IDirectDraw7*)*lplpDD, DxVersion);

		*lplpDD = p_IDirectDrawX->GetWrapperInterfaceX(DxVersion);
	}

	return hr;
}

BOOL CALLBACK DispayEnumeratorProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
	UNREFERENCED_PARAMETER(hdcMonitor);
	UNREFERENCED_PARAMETER(lprcMonitor);

	ENUMMONITORS *lpMonitors = (ENUMMONITORS *)dwData;
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

HRESULT DirectDrawEnumerateHandler(LPVOID lpCallback, LPVOID lpContext, DWORD dwFlags, DirectDrawEnumerateTypes DDETType)
{
	UNREFERENCED_PARAMETER(dwFlags);

	if (!lpCallback)
	{
		return DDERR_INVALIDPARAMS;
	}

	// Declare Direct3DCreate9
	static Direct3DCreate9Proc Direct3DCreate9 = reinterpret_cast<Direct3DCreate9Proc>(Direct3DCreate9_out);

	if (!Direct3DCreate9)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: failed to get 'Direct3DCreate9' ProcAddress of d3d9.dll!");
		return DDERR_UNSUPPORTED;
	}

	// Create Direct3D9 device
	LPDIRECT3D9 d3d9Object = Direct3DCreate9(D3D_SDK_VERSION);

	// Error creating Direct3D9
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
				ENUMMONITORS Monitors;
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

	ULONG ref = d3d9Object->Release();
	if (ref)
	{
		Logging::Log() << __FUNCTION__ << " Error: there is still a reference to 'd3d9Object' " << ref;
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

	static DirectDrawEnumerateAProc m_pDirectDrawEnumerateA = (Wrapper::ValidProcAddress(DirectDrawEnumerateA_out)) ? (DirectDrawEnumerateAProc)DirectDrawEnumerateA_out : nullptr;

	if (!m_pDirectDrawEnumerateA)
	{
		return DDERR_UNSUPPORTED;
	}

	return m_pDirectDrawEnumerateA(lpCallback, lpContext);
}

HRESULT WINAPI dd_DirectDrawEnumerateExA(LPDDENUMCALLBACKEXA lpCallback, LPVOID lpContext, DWORD dwFlags)
{
	LOG_LIMIT(1, __FUNCTION__);

	if (Config.Dd7to9)
	{
		return DirectDrawEnumerateHandler(lpCallback, lpContext, dwFlags, DDET_ENUMCALLBACKEXA);
	}

	static DirectDrawEnumerateExAProc m_pDirectDrawEnumerateExA = (Wrapper::ValidProcAddress(DirectDrawEnumerateExA_out)) ? (DirectDrawEnumerateExAProc)DirectDrawEnumerateExA_out : nullptr;

	if (!m_pDirectDrawEnumerateExA)
	{
		return DDERR_UNSUPPORTED;
	}

	return m_pDirectDrawEnumerateExA(lpCallback, lpContext, dwFlags);
}

HRESULT WINAPI dd_DirectDrawEnumerateExW(LPDDENUMCALLBACKEXW lpCallback, LPVOID lpContext, DWORD dwFlags)
{
	LOG_LIMIT(1, __FUNCTION__);

	if (Config.Dd7to9)
	{
		return DirectDrawEnumerateHandler(lpCallback, lpContext, dwFlags, DDET_ENUMCALLBACKEXW);
	}

	static DirectDrawEnumerateExWProc m_pDirectDrawEnumerateExW = (Wrapper::ValidProcAddress(DirectDrawEnumerateExW_out)) ? (DirectDrawEnumerateExWProc)DirectDrawEnumerateExW_out : nullptr;

	if (!m_pDirectDrawEnumerateExW)
	{
		return DDERR_UNSUPPORTED;
	}

	return m_pDirectDrawEnumerateExW(lpCallback, lpContext, dwFlags);
}

HRESULT WINAPI dd_DirectDrawEnumerateW(LPDDENUMCALLBACKW lpCallback, LPVOID lpContext)
{
	LOG_LIMIT(1, __FUNCTION__);

	if (Config.Dd7to9)
	{
		return DirectDrawEnumerateHandler(lpCallback, lpContext, 0, DDET_ENUMCALLBACKW);
	}

	static DirectDrawEnumerateWProc m_pDirectDrawEnumerateW = (Wrapper::ValidProcAddress(DirectDrawEnumerateW_out)) ? (DirectDrawEnumerateWProc)DirectDrawEnumerateW_out : nullptr;

	if (!m_pDirectDrawEnumerateW)
	{
		return DDERR_UNSUPPORTED;
	}

	return m_pDirectDrawEnumerateW(lpCallback, lpContext);
}

HRESULT WINAPI dd_DllCanUnloadNow()
{
	LOG_LIMIT(1, __FUNCTION__);

	if (Config.Dd7to9)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	static DllCanUnloadNowProc m_pDllCanUnloadNow = (Wrapper::ValidProcAddress(DllCanUnloadNow_out)) ? (DllCanUnloadNowProc)DllCanUnloadNow_out : nullptr;

	if (!m_pDllCanUnloadNow)
	{
		return DDERR_UNSUPPORTED;
	}

	return m_pDllCanUnloadNow();
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

	static DllGetClassObjectProc m_pDllGetClassObject = (Wrapper::ValidProcAddress(DllGetClassObject_out)) ? (DllGetClassObjectProc)DllGetClassObject_out : nullptr;

	if (!m_pDllGetClassObject)
	{
		return DDERR_UNSUPPORTED;
	}

	HRESULT hr = m_pDllGetClassObject(ConvertREFCLSID(rclsid), ConvertREFIID(riid), ppv);

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

	static GetDDSurfaceLocalProc m_pGetDDSurfaceLocal = (Wrapper::ValidProcAddress(GetDDSurfaceLocal_out)) ? (GetDDSurfaceLocalProc)GetDDSurfaceLocal_out : nullptr;

	if (!m_pGetDDSurfaceLocal)
	{
		return DDERR_UNSUPPORTED;
	}

	return m_pGetDDSurfaceLocal(arg1, arg2, arg3);
}

DWORD WINAPI dd_GetOLEThunkData(DWORD index)
{
	LOG_LIMIT(1, __FUNCTION__);

	if (Config.Dd7to9)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");
		return NULL;
	}

	//	switch(index) 
	//	{
	//		case 1: return _dwLastFrameRate;
	//		case 2: return _lpDriverObjectList;
	//		case 3: return _lpAttachedProcesses;
	//		case 4: return 0; // does nothing?
	//		case 5: return _CheckExclusiveMode;
	//		case 6: return 0; // ReleaseExclusiveModeMutex
	//	}

	static GetOLEThunkDataProc m_pGetOLEThunkData = (Wrapper::ValidProcAddress(GetOLEThunkData_out)) ? (GetOLEThunkDataProc)GetOLEThunkData_out : nullptr;

	if (!m_pGetOLEThunkData)
	{
		return NULL;
	}

	return m_pGetOLEThunkData(index);
}

HRESULT WINAPI dd_GetSurfaceFromDC(HDC hdc, LPDIRECTDRAWSURFACE7 *lpDDS, DWORD arg)
{
	LOG_LIMIT(1, __FUNCTION__);

	if (Config.Dd7to9)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	static GetSurfaceFromDCProc m_pGetSurfaceFromDC = (Wrapper::ValidProcAddress(GetSurfaceFromDC_out)) ? (GetSurfaceFromDCProc)GetSurfaceFromDC_out : nullptr;

	if (!m_pGetSurfaceFromDC)
	{
		return DDERR_UNSUPPORTED;
	}

	return m_pGetSurfaceFromDC(hdc, lpDDS, arg);
}

HRESULT WINAPI dd_RegisterSpecialCase(DWORD arg1, DWORD arg2, DWORD arg3, DWORD arg4)
{
	LOG_LIMIT(1, __FUNCTION__);

	if (Config.Dd7to9)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	static RegisterSpecialCaseProc m_pRegisterSpecialCase = (Wrapper::ValidProcAddress(RegisterSpecialCase_out)) ? (RegisterSpecialCaseProc)RegisterSpecialCase_out : nullptr;

	if (!m_pRegisterSpecialCase)
	{
		return DDERR_UNSUPPORTED;
	}

	return m_pRegisterSpecialCase(arg1, arg2, arg3, arg4);
}

HRESULT WINAPI dd_ReleaseDDThreadLock()
{
	LOG_LIMIT(1, __FUNCTION__);

	if (Config.Dd7to9)
	{
		return ReleaseCriticalSection();
	}

	static ReleaseDDThreadLockProc m_pReleaseDDThreadLock = (Wrapper::ValidProcAddress(ReleaseDDThreadLock_out)) ? (ReleaseDDThreadLockProc)ReleaseDDThreadLock_out : nullptr;

	if (!m_pReleaseDDThreadLock)
	{
		return DDERR_UNSUPPORTED;
	}

	return m_pReleaseDDThreadLock();
}

HRESULT WINAPI dd_SetAppCompatData(DWORD Type, DWORD Value)
{
	LOG_LIMIT(1, __FUNCTION__);

	if (Config.Dd7to9)
	{
		if (Type == AppCompatDataType.DisableMaxWindowedMode)
		{
			if (Direct3D9DisableMaximizedWindowedMode())
			{
				return DD_OK;
			}
			else
			{
				return DDERR_GENERIC;
			}
		}

		LOG_LIMIT(100, __FUNCTION__ << " Not Implemented: " << Type << " " << Value);
		return DDERR_UNSUPPORTED;
	}

	static SetAppCompatDataProc m_pSetAppCompatData = (Wrapper::ValidProcAddress(SetAppCompatData_out)) ? (SetAppCompatDataProc)SetAppCompatData_out : nullptr;

	if (!m_pSetAppCompatData)
	{
		return DDERR_GENERIC;
	}

	return m_pSetAppCompatData(Type, Value);
}

void AddBaseClipperToVetor(m_IDirectDrawClipper* lpClipper)
{
	if (!lpClipper || DoesBaseClipperExist(lpClipper))
	{
		return;
	}

	SetCriticalSection();

	// Store clipper
	BaseClipperVector.push_back(lpClipper);

	ReleaseCriticalSection();
}

void RemoveBaseClipperFromVector(m_IDirectDrawClipper* lpClipper)
{
	if (!lpClipper)
	{
		return;
	}

	SetCriticalSection();

	auto it = std::find_if(BaseClipperVector.begin(), BaseClipperVector.end(),
		[=](auto pClipper) -> bool { return pClipper == lpClipper; });

	// Remove clipper from vector
	if (it != std::end(BaseClipperVector))
	{
		BaseClipperVector.erase(it);
	}

	ReleaseCriticalSection();
}

bool DoesBaseClipperExist(m_IDirectDrawClipper* lpClipper)
{
	if (!lpClipper)
	{
		return false;
	}

	bool hr = false;

	SetCriticalSection();

	auto it = std::find_if(BaseClipperVector.begin(), BaseClipperVector.end(),
		[=](auto pSurface) -> bool { return pSurface == lpClipper; });

	if (it != std::end(BaseClipperVector))
	{
		hr = true;
	}

	ReleaseCriticalSection();

	return hr;
}

HRESULT DdrawWrapper::SetCriticalSection()
{
	if (IsInitialized)
	{
		EnterCriticalSection(&ddcs);
		return DD_OK;
	}
	return DDERR_UNSUPPORTED;
}

HRESULT DdrawWrapper::ReleaseCriticalSection()
{
	if (IsInitialized)
	{
		LeaveCriticalSection(&ddcs);
		return DD_OK;
	}
	return DDERR_UNSUPPORTED;
}
