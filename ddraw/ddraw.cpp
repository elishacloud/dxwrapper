/**
* Copyright (C) 2018 Elisha Riedlinger
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

#include "ddraw.h"
#include "ddrawExternal.h"
#include "Dllmain\Dllmain.h"

AddressLookupTableDdraw<void> ProxyAddressLookupTable = AddressLookupTableDdraw<void>();
m_IDirect3DDeviceX *lpCurrentD3DDevice = nullptr;
m_IDirectDrawX *CurrentDDInterface = nullptr;

#define INITUALIZE_WRAPPED_PROC(procName, unused) \
	FARPROC procName ## _out = (FARPROC)*(ddraw::procName);

namespace DdrawWrapper
{
	VISIT_PROCS_DDRAW(INITUALIZE_WRAPPED_PROC);
	FARPROC DllCanUnloadNow_out = (FARPROC)*(ShardProcs::DllCanUnloadNow);
	FARPROC DllGetClassObject_out = (FARPROC)*(ShardProcs::DllGetClassObject);
	FARPROC Direct3DCreate9;
}

using namespace DdrawWrapper;

void WINAPI dd_AcquireDDThreadLock()
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
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
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
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

	if (SUCCEEDED(hr))
	{
		// Convert to new DirectDraw version
		if (ConvertREFIID(IID_IDirectDraw) != IID_IDirectDraw)
		{
			LPDIRECTDRAW lpDD = (LPDIRECTDRAW)*lplpDD;

			hr = lpDD->QueryInterface(ConvertREFIID(IID_IDirectDraw), (LPVOID*)lplpDD);

			if (SUCCEEDED(hr))
			{
				hr = genericQueryInterface(IID_IDirectDraw, (LPVOID*)lplpDD);

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
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
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
			Logging::Log() << "Failed to get 'Direct3DCreate9' ProcAddress of d3d9.dll!";
			return DDERR_GENERIC;
		}

		// Create Direct3D9 device
		LPDIRECT3D9 d3d9Object = Direct3DCreate9(D3D_SDK_VERSION);

		// Error creating Direct3D9
		if (d3d9Object == nullptr)
		{
			Logging::Log() << __FUNCTION__ << " Failed to create Direct3D9 object";
			return DDERR_GENERIC;
		}

		if (riid == IID_IDirectDraw)
		{
			*lplpDD = ProxyAddressLookupTable.FindAddress<m_IDirectDraw>(d3d9Object);
		}
		else if (riid == IID_IDirectDraw2)
		{
			*lplpDD = ProxyAddressLookupTable.FindAddress<m_IDirectDraw2>(d3d9Object);
		}
		else if (riid == IID_IDirectDraw3)
		{
			*lplpDD = ProxyAddressLookupTable.FindAddress<m_IDirectDraw3>(d3d9Object);
		}
		else if (riid == IID_IDirectDraw4)
		{
			*lplpDD = ProxyAddressLookupTable.FindAddress<m_IDirectDraw4>(d3d9Object);
		}
		else
		{
			*lplpDD = ProxyAddressLookupTable.FindAddress<m_IDirectDraw7>(d3d9Object);
		}

		// Success
		return DD_OK;
	}

	static DirectDrawCreateExProc m_pDirectDrawCreateEx = (Wrapper::ValidProcAddress(DirectDrawCreateEx_out)) ? (DirectDrawCreateExProc)DirectDrawCreateEx_out : nullptr;

	if (!m_pDirectDrawCreateEx)
	{
		return E_FAIL;
	}

	HRESULT hr = m_pDirectDrawCreateEx(lpGUID, lplpDD, IID_IDirectDraw7, pUnkOuter);

	if (SUCCEEDED(hr))
	{
		hr = genericQueryInterface(riid, lplpDD);
	}

	return hr;
}

HRESULT WINAPI dd_DirectDrawEnumerateA(LPDDENUMCALLBACKA lpCallback, LPVOID lpContext)
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
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
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
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
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
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
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
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

	if (SUCCEEDED(hr))
	{
		hr = genericQueryInterface(riid, ppv);

		if (SUCCEEDED(hr) && riid == IID_IClassFactory && ppv)
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
		Logging::Log() << __FUNCTION__ << " Not Implemented";
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
