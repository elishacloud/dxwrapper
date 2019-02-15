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
*
* D3DParseUnknownCommand created from source code found in ReactOS
* https://doxygen.reactos.org/d3/d02/dll_2directx_2ddraw_2main_8c.html#af9a1eb1ced046770ad6f79838cc8517d
*/

#include "ddraw.h"
#include "d3dhal.h"
#include "ddrawExternal.h"
#include "Dllmain\Dllmain.h"

AddressLookupTableDdraw<void> ProxyAddressLookupTable = AddressLookupTableDdraw<void>();
m_IDirect3DDeviceX *lpCurrentD3DDevice = nullptr;
bool ThreadSyncFlag = false;

CRITICAL_SECTION ddcs;

#define INITIALIZE_WRAPPED_PROC(procName, unused) \
	FARPROC procName ## _out = nullptr;

namespace DdrawWrapper
{
	VISIT_PROCS_DDRAW(INITIALIZE_WRAPPED_PROC);
	FARPROC DllCanUnloadNow_out = nullptr;
	FARPROC DllGetClassObject_out = nullptr;
	FARPROC Direct3DCreate9 = nullptr;
}

using namespace DdrawWrapper;

bool IsInitialized = false;

void InitDDraw()
{
	InitializeCriticalSection(&ddcs);
	IsInitialized = true;
}

void ExitDDraw()
{
	IsInitialized = false;
	DeleteCriticalSection(&ddcs);
}

void WINAPI dd_AcquireDDThreadLock()
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		if (IsInitialized)
		{
			EnterCriticalSection(&ddcs);
		}
		return;
	}

	static AcquireDDThreadLockProc m_pAcquireDDThreadLock = (Wrapper::ValidProcAddress(AcquireDDThreadLock_out)) ? (AcquireDDThreadLockProc)AcquireDDThreadLock_out : nullptr;

	if (!m_pAcquireDDThreadLock)
	{
		return;
	}

	return m_pAcquireDDThreadLock();
}

void WINAPI dd_CompleteCreateSysmemSurface()
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return;
	}

	static CompleteCreateSysmemSurfaceProc m_pCompleteCreateSysmemSurface = (Wrapper::ValidProcAddress(CompleteCreateSysmemSurface_out)) ? (CompleteCreateSysmemSurfaceProc)CompleteCreateSysmemSurface_out : nullptr;

	if (!m_pCompleteCreateSysmemSurface)
	{
		return;
	}

	return m_pCompleteCreateSysmemSurface();
}

HRESULT WINAPI dd_D3DParseUnknownCommand(LPVOID lpCmd, LPVOID *lpRetCmd)
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		if (!lpCmd || !lpRetCmd)
		{
			return E_FAIL;
		}

		LPD3DHAL_DP2COMMAND dp2command = (LPD3DHAL_DP2COMMAND)lpCmd;

		switch (dp2command->bCommand)
		{
			/* check for valid command, only 3 commands are valid */
		case D3DDP2OP_VIEWPORTINFO:
			*lpRetCmd = (LPVOID)((DWORD)lpCmd + (dp2command->wStateCount * sizeof(D3DHAL_DP2VIEWPORTINFO)) + sizeof(D3DHAL_DP2COMMAND));
			break;

		case D3DDP2OP_WINFO:
			*lpRetCmd = (LPVOID)((DWORD)lpCmd + (dp2command->wStateCount * sizeof(D3DHAL_DP2WINFO)) + sizeof(D3DHAL_DP2COMMAND));
			break;

		case 0x0d: /* Undocumented in MSDN */
			*lpRetCmd = (LPVOID)((DWORD)lpCmd + (dp2command->wStateCount * dp2command->bReserved) + sizeof(D3DHAL_DP2COMMAND));
			break;

			/* set the error code */
		default:
			if ((dp2command->bCommand <= D3DDP2OP_INDEXEDTRIANGLELIST) || // dp2command->bCommand  <= with 0 to 3
				(dp2command->bCommand == D3DDP2OP_RENDERSTATE) ||  // dp2command->bCommand  == with 8
				(dp2command->bCommand >= D3DDP2OP_LINELIST))  // dp2command->bCommand  >= with 15 to 255
			{
				/* set error code for command 0 to 3, 8 and 15 to 255 */
				return DDERR_INVALIDPARAMS;
			}
			else
			{   /* set error code for 4 - 7, 9 - 12, 14  */
				return D3DERR_COMMAND_UNPARSED;
			}
		}

		return DD_OK;
	}

	static D3DParseUnknownCommandProc m_pD3DParseUnknownCommand = (Wrapper::ValidProcAddress(D3DParseUnknownCommand_out)) ? (D3DParseUnknownCommandProc)D3DParseUnknownCommand_out : nullptr;

	if (!m_pD3DParseUnknownCommand)
	{
		return E_FAIL;
	}

	return m_pD3DParseUnknownCommand(lpCmd, lpRetCmd);
}

void WINAPI dd_DDGetAttachedSurfaceLcl()
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return;
	}

	static DDGetAttachedSurfaceLclProc m_pDDGetAttachedSurfaceLcl = (Wrapper::ValidProcAddress(DDGetAttachedSurfaceLcl_out)) ? (DDGetAttachedSurfaceLclProc)DDGetAttachedSurfaceLcl_out : nullptr;

	if (!m_pDDGetAttachedSurfaceLcl)
	{
		return;
	}

	return m_pDDGetAttachedSurfaceLcl();
}

void WINAPI dd_DDInternalLock()
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return;
	}

	static DDInternalLockProc m_pDDInternalLock = (Wrapper::ValidProcAddress(DDInternalLock_out)) ? (DDInternalLockProc)DDInternalLock_out : nullptr;

	if (!m_pDDInternalLock)
	{
		return;
	}

	return m_pDDInternalLock();
}

void WINAPI dd_DDInternalUnlock()
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return;
	}

	static DDInternalUnlockProc m_pDDInternalUnlock = (Wrapper::ValidProcAddress(DDInternalUnlock_out)) ? (DDInternalUnlockProc)DDInternalUnlock_out : nullptr;

	if (!m_pDDInternalUnlock)
	{
		return;
	}

	return m_pDDInternalUnlock();
}

void WINAPI dd_DSoundHelp()
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return;
	}

	static DSoundHelpProc m_pDSoundHelp = (Wrapper::ValidProcAddress(DSoundHelp_out)) ? (DSoundHelpProc)DSoundHelp_out : nullptr;

	if (!m_pDSoundHelp)
	{
		return;
	}

	return m_pDSoundHelp();
}

HRESULT WINAPI dd_DirectDrawCreate(GUID FAR *lpGUID, LPDIRECTDRAW FAR *lplpDD, IUnknown FAR *pUnkOuter)
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9 || (Config.ConvertToDirect3D7 && Config.ConvertToDirectDraw7))
	{
		return dd_DirectDrawCreateEx(lpGUID, (LPVOID*)lplpDD, IID_IDirectDraw, pUnkOuter);
	}

	static DirectDrawCreateProc m_pDirectDrawCreate = (Wrapper::ValidProcAddress(DirectDrawCreate_out)) ? (DirectDrawCreateProc)DirectDrawCreate_out : nullptr;

	if (!m_pDirectDrawCreate)
	{
		return E_FAIL;
	}

	HRESULT hr = m_pDirectDrawCreate(lpGUID, lplpDD, pUnkOuter);

	if (SUCCEEDED(hr) && lplpDD && *lplpDD)
	{
		// Convert to new DirectDraw version
		if (ConvertREFIID(IID_IDirectDraw) != IID_IDirectDraw)
		{
			LPDIRECTDRAW lpDD = (LPDIRECTDRAW)*lplpDD;

			hr = lpDD->QueryInterface(ConvertREFIID(IID_IDirectDraw), (LPVOID*)lplpDD);

			if (SUCCEEDED(hr))
			{
				genericQueryInterface(IID_IDirectDraw, (LPVOID*)lplpDD);

				lpDD->Release();
			}
		}
		else
		{
			*lplpDD = ProxyAddressLookupTable.FindAddress<m_IDirectDraw>(*lplpDD);
		}
	}

	return hr;
}

HRESULT WINAPI dd_DirectDrawCreateClipper(DWORD dwFlags, LPDIRECTDRAWCLIPPER *lplpDDClipper, LPUNKNOWN pUnkOuter)
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		if (!lplpDDClipper)
		{
			return DDERR_INVALIDPARAMS;
		}

		*lplpDDClipper = new m_IDirectDrawClipper(dwFlags);

		return DD_OK;
	}

	static DirectDrawCreateClipperProc m_pDirectDrawCreateClipper = (Wrapper::ValidProcAddress(DirectDrawCreateClipper_out)) ? (DirectDrawCreateClipperProc)DirectDrawCreateClipper_out : nullptr;

	if (!m_pDirectDrawCreateClipper)
	{
		return E_FAIL;
	}

	HRESULT hr = m_pDirectDrawCreateClipper(dwFlags, lplpDDClipper, pUnkOuter);

	if (SUCCEEDED(hr) && lplpDDClipper)
	{
		*lplpDDClipper = ProxyAddressLookupTable.FindAddress<m_IDirectDrawClipper>(*lplpDDClipper);
	}

	return hr;
}

HRESULT WINAPI dd_DirectDrawCreateEx(GUID FAR *lpGUID, LPVOID *lplpDD, REFIID riid, IUnknown FAR *pUnkOuter)
{
	Logging::LogDebug() << __FUNCTION__;

	if (riid != IID_IDirectDraw &&
		riid != IID_IDirectDraw2 &&
		riid != IID_IDirectDraw3 &&
		riid != IID_IDirectDraw4 &&
		riid != IID_IDirectDraw7)
	{
		Logging::Log() << __FUNCTION__ << " Error: invalid IID " << riid;
		return DDERR_INVALIDPARAMS;
	}

	if (Config.Dd7to9)
	{
		// Declare Direct3DCreate9
		static PFN_Direct3DCreate9 Direct3DCreate9 = reinterpret_cast<PFN_Direct3DCreate9>(DdrawWrapper::Direct3DCreate9);

		if (!Direct3DCreate9)
		{
			Logging::Log() << __FUNCTION__ << "Failed to get 'Direct3DCreate9' ProcAddress of d3d9.dll!";
			return DDERR_GENERIC;
		}

		// Create Direct3D9 device
		LPDIRECT3D9 d3d9Object = Direct3DCreate9(D3D_SDK_VERSION);

		// Error creating Direct3D9
		if (!d3d9Object)
		{
			Logging::Log() << __FUNCTION__ << " Failed to create Direct3D9 object";
			return DDERR_GENERIC;
		}

		DWORD DxVersion = GetIIDVersion(riid);

		m_IDirectDrawX *p_IDirectDrawX = new m_IDirectDrawX(d3d9Object, DxVersion);

		*lplpDD = p_IDirectDrawX->GetWrapperInterfaceX(DxVersion);

		// Success
		return DD_OK;
	}

	static DirectDrawCreateExProc m_pDirectDrawCreateEx = (Wrapper::ValidProcAddress(DirectDrawCreateEx_out)) ? (DirectDrawCreateExProc)DirectDrawCreateEx_out : nullptr;

	if (!m_pDirectDrawCreateEx)
	{
		return E_FAIL;
	}

	HRESULT hr = m_pDirectDrawCreateEx(lpGUID, lplpDD, IID_IDirectDraw7, pUnkOuter);

	if (SUCCEEDED(hr) && lplpDD && *lplpDD)
	{
		genericQueryInterface(riid, lplpDD);
	}

	return hr;
}

struct ENUMMONITORS
{
	LPSTR lpName;
	HMONITOR hm;
};

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

enum DirectDrawEnumerateTypes
{
	DDET_ENUMCALLBACKA,
	DDET_ENUMCALLBACKEXA,
	DDET_ENUMCALLBACKEXW,
	DDET_ENUMCALLBACKW,
};

HRESULT DirectDrawEnumerateHandler(LPVOID lpCallback, LPVOID lpContext, DWORD dwFlags, DirectDrawEnumerateTypes DDETType)
{
	UNREFERENCED_PARAMETER(dwFlags);

	// Declare Direct3DCreate9
	static PFN_Direct3DCreate9 Direct3DCreate9 = reinterpret_cast<PFN_Direct3DCreate9>(DdrawWrapper::Direct3DCreate9);

	if (!Direct3DCreate9)
	{
		Logging::Log() << __FUNCTION__ << "Failed to get 'Direct3DCreate9' ProcAddress of d3d9.dll!";
		return DDERR_GENERIC;
	}

	// Create Direct3D9 device
	LPDIRECT3D9 d3d9Object = Direct3DCreate9(D3D_SDK_VERSION);

	// Error creating Direct3D9
	if (!d3d9Object)
	{
		Logging::Log() << __FUNCTION__ << " Failed to create Direct3D9 object";
		return DDERR_GENERIC;
	}

	D3DADAPTER_IDENTIFIER9 Identifier = { NULL };
	int AdapterCount = (int)d3d9Object->GetAdapterCount();
	GUID* lpGUID;
	LPSTR lpDesc, lpName;
	wchar_t lpwName[32] = { '\0' };
	wchar_t lpwDesc[128] = { '\0' };
	HMONITOR hm = nullptr;
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
				d3d9Object->Release();
				return DDERR_GENERIC;
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

		switch (DDETType)
		{
		case DDET_ENUMCALLBACKA:
			if (LPDDENUMCALLBACKA(lpCallback)(lpGUID, lpDesc, lpName, lpContext) == DDENUMRET_CANCEL)
			{
				d3d9Object->Release();
				return D3D_OK;
			}
			break;
		case DDET_ENUMCALLBACKEXA:
			if (LPDDENUMCALLBACKEXA(lpCallback)(lpGUID, lpDesc, lpName, lpContext, hm) == DDENUMRET_CANCEL)
			{
				d3d9Object->Release();
				return D3D_OK;
			}
			break;
		case DDET_ENUMCALLBACKEXW:
			if (LPDDENUMCALLBACKEXW(lpCallback)(lpGUID, lpwDesc, lpwName, lpContext, hm) == DDENUMRET_CANCEL)
			{
				d3d9Object->Release();
				return D3D_OK;
			}
			break;
		case DDET_ENUMCALLBACKW:
			if (LPDDENUMCALLBACKW(lpCallback)(lpGUID, lpwDesc, lpwName, lpContext) == DDENUMRET_CANCEL)
			{
				d3d9Object->Release();
				return D3D_OK;
			}
			break;
		default:
			d3d9Object->Release();
			return DDERR_GENERIC;
		}
	}

	d3d9Object->Release();
	return DD_OK;
}

HRESULT WINAPI dd_DirectDrawEnumerateA(LPDDENUMCALLBACKA lpCallback, LPVOID lpContext)
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		return DirectDrawEnumerateHandler(lpCallback, lpContext, 0, DDET_ENUMCALLBACKA);
	}

	static DirectDrawEnumerateAProc m_pDirectDrawEnumerateA = (Wrapper::ValidProcAddress(DirectDrawEnumerateA_out)) ? (DirectDrawEnumerateAProc)DirectDrawEnumerateA_out : nullptr;

	if (!m_pDirectDrawEnumerateA)
	{
		return E_FAIL;
	}

	return m_pDirectDrawEnumerateA(lpCallback, lpContext);
}

HRESULT WINAPI dd_DirectDrawEnumerateExA(LPDDENUMCALLBACKEXA lpCallback, LPVOID lpContext, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		return DirectDrawEnumerateHandler(lpCallback, lpContext, dwFlags, DDET_ENUMCALLBACKEXA);
	}

	static DirectDrawEnumerateExAProc m_pDirectDrawEnumerateExA = (Wrapper::ValidProcAddress(DirectDrawEnumerateExA_out)) ? (DirectDrawEnumerateExAProc)DirectDrawEnumerateExA_out : nullptr;

	if (!m_pDirectDrawEnumerateExA)
	{
		return E_FAIL;
	}

	return m_pDirectDrawEnumerateExA(lpCallback, lpContext, dwFlags);
}

HRESULT WINAPI dd_DirectDrawEnumerateExW(LPDDENUMCALLBACKEXW lpCallback, LPVOID lpContext, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		return DirectDrawEnumerateHandler(lpCallback, lpContext, dwFlags, DDET_ENUMCALLBACKEXW);
	}

	static DirectDrawEnumerateExWProc m_pDirectDrawEnumerateExW = (Wrapper::ValidProcAddress(DirectDrawEnumerateExW_out)) ? (DirectDrawEnumerateExWProc)DirectDrawEnumerateExW_out : nullptr;

	if (!m_pDirectDrawEnumerateExW)
	{
		return E_FAIL;
	}

	return m_pDirectDrawEnumerateExW(lpCallback, lpContext, dwFlags);
}

HRESULT WINAPI dd_DirectDrawEnumerateW(LPDDENUMCALLBACKW lpCallback, LPVOID lpContext)
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		return DirectDrawEnumerateHandler(lpCallback, lpContext, 0, DDET_ENUMCALLBACKW);
	}

	static DirectDrawEnumerateWProc m_pDirectDrawEnumerateW = (Wrapper::ValidProcAddress(DirectDrawEnumerateW_out)) ? (DirectDrawEnumerateWProc)DirectDrawEnumerateW_out : nullptr;

	if (!m_pDirectDrawEnumerateW)
	{
		return E_FAIL;
	}

	return m_pDirectDrawEnumerateW(lpCallback, lpContext);
}

HRESULT WINAPI dd_DllCanUnloadNow()
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	static DllCanUnloadNowProc m_pDllCanUnloadNow = (Wrapper::ValidProcAddress(DllCanUnloadNow_out)) ? (DllCanUnloadNowProc)DllCanUnloadNow_out : nullptr;

	if (!m_pDllCanUnloadNow)
	{
		return E_FAIL;
	}

	return m_pDllCanUnloadNow();
}

HRESULT WINAPI dd_DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID *ppv)
{
	Logging::LogDebug() << __FUNCTION__;

	if (!ppv)
	{
		return E_INVALIDARG;
	}

	if (Config.Dd7to9)
	{
		return ProxyQueryInterface(nullptr, riid, ppv, rclsid, nullptr);
	}

	static DllGetClassObjectProc m_pDllGetClassObject = (Wrapper::ValidProcAddress(DllGetClassObject_out)) ? (DllGetClassObjectProc)DllGetClassObject_out : nullptr;

	if (!m_pDllGetClassObject)
	{
		return E_INVALIDARG;
	}

	HRESULT hr = m_pDllGetClassObject(rclsid, ConvertREFIID(riid), ppv);

	if (SUCCEEDED(hr) && ppv)
	{
		genericQueryInterface(riid, ppv);

		if (riid == IID_IClassFactory && *ppv)
		{
			((m_IClassFactory*)(*ppv))->SetCLSID(rclsid);
		}
	}

	return hr;
}

void WINAPI dd_GetDDSurfaceLocal()
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return;
	}

	static GetDDSurfaceLocalProc m_pGetDDSurfaceLocal = (Wrapper::ValidProcAddress(GetDDSurfaceLocal_out)) ? (GetDDSurfaceLocalProc)GetDDSurfaceLocal_out : nullptr;

	if (!m_pGetDDSurfaceLocal)
	{
		return;
	}

	return m_pGetDDSurfaceLocal();
}

HANDLE WINAPI dd_GetOLEThunkData(int i1)
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return nullptr;
	}

	static GetOLEThunkDataProc m_pGetOLEThunkData = (Wrapper::ValidProcAddress(GetOLEThunkData_out)) ? (GetOLEThunkDataProc)GetOLEThunkData_out : nullptr;

	if (!m_pGetOLEThunkData)
	{
		return nullptr;
	}

	return m_pGetOLEThunkData(i1);
}

HRESULT WINAPI dd_GetSurfaceFromDC(HDC hdc, LPDIRECTDRAWSURFACE7 *lpDDS)
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	static GetSurfaceFromDCProc m_pGetSurfaceFromDC = (Wrapper::ValidProcAddress(GetSurfaceFromDC_out)) ? (GetSurfaceFromDCProc)GetSurfaceFromDC_out : nullptr;

	if (!m_pGetSurfaceFromDC)
	{
		return E_FAIL;
	}

	HRESULT hr = m_pGetSurfaceFromDC(hdc, lpDDS);

	if (SUCCEEDED(hr))
	{
		*lpDDS = ProxyAddressLookupTable.FindAddress<m_IDirectDrawSurface7>(*lpDDS);
	}

	return hr;
}

void WINAPI dd_RegisterSpecialCase()
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return;
	}

	static RegisterSpecialCaseProc m_pRegisterSpecialCase = (Wrapper::ValidProcAddress(RegisterSpecialCase_out)) ? (RegisterSpecialCaseProc)RegisterSpecialCase_out : nullptr;

	if (!m_pRegisterSpecialCase)
	{
		return;
	}

	return m_pRegisterSpecialCase();
}

void WINAPI dd_ReleaseDDThreadLock()
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		if (IsInitialized)
		{
			LeaveCriticalSection(&ddcs);
		}
		return;
	}

	static ReleaseDDThreadLockProc m_pReleaseDDThreadLock = (Wrapper::ValidProcAddress(ReleaseDDThreadLock_out)) ? (ReleaseDDThreadLockProc)ReleaseDDThreadLock_out : nullptr;

	if (!m_pReleaseDDThreadLock)
	{
		return;
	}

	return m_pReleaseDDThreadLock();
}

HRESULT WINAPI dd_SetAppCompatData(DWORD Type, DWORD Value)
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	static SetAppCompatDataProc m_pSetAppCompatData = (Wrapper::ValidProcAddress(SetAppCompatData_out)) ? (SetAppCompatDataProc)SetAppCompatData_out : nullptr;

	if (!m_pSetAppCompatData)
	{
		return E_FAIL;
	}

	return m_pSetAppCompatData(Type, Value);
}
