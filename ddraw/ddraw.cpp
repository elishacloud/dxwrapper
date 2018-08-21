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
IDirectDraw7 *CurrentDDInterface = nullptr;

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
	static AcquireDDThreadLockProc m_pAcquireDDThreadLock = (Wrapper::ValidProcAddress(AcquireDDThreadLock_out)) ? (AcquireDDThreadLockProc)AcquireDDThreadLock_out : nullptr;

	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return;
	}

	if (!m_pAcquireDDThreadLock)
	{
		return;
	}

	return m_pAcquireDDThreadLock();
}

void WINAPI dd_CompleteCreateSystemSurface()
{
	static CompleteCreateSystemSurfaceProc m_pCompleteCreateSystemSurface = (Wrapper::ValidProcAddress(CompleteCreateSystemSurface_out)) ? (CompleteCreateSystemSurfaceProc)CompleteCreateSystemSurface_out : nullptr;

	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return;
	}

	if (!m_pCompleteCreateSystemSurface)
	{
		return;
	}

	return m_pCompleteCreateSystemSurface();
}

HRESULT WINAPI dd_D3DParseUnknownCommand(LPVOID lpCmd, LPVOID *lpRetCmd)
{
	static D3DParseUnknownCommandProc m_pD3DParseUnknownCommand = (Wrapper::ValidProcAddress(D3DParseUnknownCommand_out)) ? (D3DParseUnknownCommandProc)D3DParseUnknownCommand_out : nullptr;

	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	if (!m_pD3DParseUnknownCommand)
	{
		return E_FAIL;
	}

	return m_pD3DParseUnknownCommand(lpCmd, lpRetCmd);
}

void WINAPI dd_DDGetAttachedSurfaceLcl()
{
	static DDGetAttachedSurfaceLclProc m_pDDGetAttachedSurfaceLcl = (Wrapper::ValidProcAddress(DDGetAttachedSurfaceLcl_out)) ? (DDGetAttachedSurfaceLclProc)DDGetAttachedSurfaceLcl_out : nullptr;

	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return;
	}

	if (!m_pDDGetAttachedSurfaceLcl)
	{
		return;
	}

	return m_pDDGetAttachedSurfaceLcl();
}

void WINAPI dd_DDInternalLock()
{
	static DDInternalLockProc m_pDDInternalLock = (Wrapper::ValidProcAddress(DDInternalLock_out)) ? (DDInternalLockProc)DDInternalLock_out : nullptr;

	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return;
	}

	if (!m_pDDInternalLock)
	{
		return;
	}

	return m_pDDInternalLock();
}

void WINAPI dd_DDInternalUnlock()
{
	static DDInternalUnlockProc m_pDDInternalUnlock = (Wrapper::ValidProcAddress(DDInternalUnlock_out)) ? (DDInternalUnlockProc)DDInternalUnlock_out : nullptr;

	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return;
	}

	if (!m_pDDInternalUnlock)
	{
		return;
	}

	return m_pDDInternalUnlock();
}

void WINAPI dd_DSoundHelp()
{
	static DSoundHelpProc m_pDSoundHelp = (Wrapper::ValidProcAddress(DSoundHelp_out)) ? (DSoundHelpProc)DSoundHelp_out : nullptr;

	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return;
	}

	if (!m_pDSoundHelp)
	{
		return;
	}

	return m_pDSoundHelp();
}

HRESULT WINAPI dd_DirectDrawCreate(GUID FAR *lpGUID, LPDIRECTDRAW FAR *lplpDD, IUnknown FAR *pUnkOuter)
{
	static DirectDrawCreateProc m_pDirectDrawCreate = (Wrapper::ValidProcAddress(DirectDrawCreate_out)) ? (DirectDrawCreateProc)DirectDrawCreate_out : nullptr;

	if (!m_pDirectDrawCreate)
	{
		return E_FAIL;
	}

	if (Config.ConvertToDirect3D7 && Config.ConvertToDirectDraw7)
	{
		return dd_DirectDrawCreateEx(lpGUID, (LPVOID*)lplpDD, IID_IDirectDraw, pUnkOuter);
	}

	HRESULT hr = m_pDirectDrawCreate(lpGUID, lplpDD, pUnkOuter);

	if (SUCCEEDED(hr))
	{
		// Convert to new DirectDraw version
		if (ConvertREFIID(IID_IDirectDraw) != IID_IDirectDraw)
		{
			LPDIRECTDRAW lpDD = (LPDIRECTDRAW)*lplpDD;

			if (SUCCEEDED(hr))
			{
				hr = lpDD->QueryInterface(ConvertREFIID(IID_IDirectDraw), (LPVOID*)lplpDD);

				if (SUCCEEDED(hr))
				{
					genericQueryInterface(IID_IDirectDraw, (LPVOID*)lplpDD);
				}

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
	static DirectDrawCreateClipperProc m_pDirectDrawCreateClipper = (Wrapper::ValidProcAddress(DirectDrawCreateClipper_out)) ? (DirectDrawCreateClipperProc)DirectDrawCreateClipper_out : nullptr;

	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

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
	static DirectDrawCreateExProc m_pDirectDrawCreateEx = (Wrapper::ValidProcAddress(DirectDrawCreateEx_out)) ? (DirectDrawCreateExProc)DirectDrawCreateEx_out : nullptr;

	if (!m_pDirectDrawCreateEx)
	{
		return E_FAIL;
	}

	if (riid != CLSID_DirectDraw &&
		riid != IID_IDirectDraw &&
		riid != IID_IDirectDraw2 &&
		riid != IID_IDirectDraw3 &&
		riid != IID_IDirectDraw4 &&
		riid != IID_IDirectDraw7 &&
		riid != CLSID_DirectDraw7)
	{
		return DDERR_INVALIDPARAMS;
	}

	HRESULT hr = m_pDirectDrawCreateEx(lpGUID, lplpDD, IID_IDirectDraw7, pUnkOuter);

	if (SUCCEEDED(hr))
	{
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

			if (d3d9Object == nullptr)
			{
				Logging::Log() << __FUNCTION__ << " Failed to create Direct3D9 object";
				// Error creating directdraw
				return DDERR_GENERIC;
			}

			if (riid == IID_IDirectDraw || riid == CLSID_DirectDraw)
			{
				m_IDirectDraw *lpDD = new m_IDirectDraw((IDirectDraw *)d3d9Object);
				*lplpDD = lpDD;
			}
			else if (riid == IID_IDirectDraw2)
			{
				m_IDirectDraw2 *lpDD = new m_IDirectDraw2((IDirectDraw2 *)d3d9Object);
				*lplpDD = lpDD;
			}
			else if	(riid == IID_IDirectDraw3)
			{
				m_IDirectDraw3 *lpDD = new m_IDirectDraw3((IDirectDraw3 *)d3d9Object);
				*lplpDD = lpDD;
			}
			else if	(riid == IID_IDirectDraw4)
			{
				m_IDirectDraw4 *lpDD = new m_IDirectDraw4((IDirectDraw4 *)d3d9Object);
				*lplpDD = lpDD;
			}
			else
			{
				m_IDirectDraw7 *lpDD = new m_IDirectDraw7((IDirectDraw7 *)d3d9Object);
				*lplpDD = lpDD;
			}

			// Success
			return DD_OK;
		}

		genericQueryInterface(riid, lplpDD);
	}

	return hr;
}

HRESULT WINAPI dd_DirectDrawEnumerateA(LPDDENUMCALLBACKA lpCallback, LPVOID lpContext)
{
	static DirectDrawEnumerateAProc m_pDirectDrawEnumerateA = (Wrapper::ValidProcAddress(DirectDrawEnumerateA_out)) ? (DirectDrawEnumerateAProc)DirectDrawEnumerateA_out : nullptr;

	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	if (!m_pDirectDrawEnumerateA)
	{
		return E_FAIL;
	}

	return m_pDirectDrawEnumerateA(lpCallback, lpContext);
}

HRESULT WINAPI dd_DirectDrawEnumerateExA(LPDDENUMCALLBACKEXA lpCallback, LPVOID lpContext, DWORD dwFlags)
{
	static DirectDrawEnumerateExAProc m_pDirectDrawEnumerateExA = (Wrapper::ValidProcAddress(DirectDrawEnumerateExA_out)) ? (DirectDrawEnumerateExAProc)DirectDrawEnumerateExA_out : nullptr;

	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	if (!m_pDirectDrawEnumerateExA)
	{
		return E_FAIL;
	}

	return m_pDirectDrawEnumerateExA(lpCallback, lpContext, dwFlags);
}

HRESULT WINAPI dd_DirectDrawEnumerateExW(LPDDENUMCALLBACKEXW lpCallback, LPVOID lpContext, DWORD dwFlags)
{
	static DirectDrawEnumerateExWProc m_pDirectDrawEnumerateExW = (Wrapper::ValidProcAddress(DirectDrawEnumerateExW_out)) ? (DirectDrawEnumerateExWProc)DirectDrawEnumerateExW_out : nullptr;

	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	if (!m_pDirectDrawEnumerateExW)
	{
		return E_FAIL;
	}

	return m_pDirectDrawEnumerateExW(lpCallback, lpContext, dwFlags);
}

HRESULT WINAPI dd_DirectDrawEnumerateW(LPDDENUMCALLBACKW lpCallback, LPVOID lpContext)
{
	static DirectDrawEnumerateWProc m_pDirectDrawEnumerateW = (Wrapper::ValidProcAddress(DirectDrawEnumerateW_out)) ? (DirectDrawEnumerateWProc)DirectDrawEnumerateW_out : nullptr;

	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	if (!m_pDirectDrawEnumerateW)
	{
		return E_FAIL;
	}

	return m_pDirectDrawEnumerateW(lpCallback, lpContext);
}

HRESULT WINAPI dd_DllCanUnloadNow()
{
	static DllCanUnloadNowProc m_pDllCanUnloadNow = (Wrapper::ValidProcAddress(DllCanUnloadNow_out)) ? (DllCanUnloadNowProc)DllCanUnloadNow_out : nullptr;

	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	if (!m_pDllCanUnloadNow)
	{
		return E_FAIL;
	}

	return m_pDllCanUnloadNow();
}

HRESULT WINAPI dd_DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID *ppv)
{
	static DllGetClassObjectProc m_pDllGetClassObject = (Wrapper::ValidProcAddress(DllGetClassObject_out)) ? (DllGetClassObjectProc)DllGetClassObject_out : nullptr;

	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	if (!m_pDllGetClassObject)
	{
		return E_FAIL;
	}

	HRESULT hr = m_pDllGetClassObject(rclsid, ConvertREFIID(riid), ppv);

	if (SUCCEEDED(hr))
	{
		genericQueryInterface(riid, ppv);
	}

	return hr;
}

void WINAPI dd_GetDDSurfaceLocal()
{
	static GetDDSurfaceLocalProc m_pGetDDSurfaceLocal = (Wrapper::ValidProcAddress(GetDDSurfaceLocal_out)) ? (GetDDSurfaceLocalProc)GetDDSurfaceLocal_out : nullptr;

	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return;
	}

	if (!m_pGetDDSurfaceLocal)
	{
		return;
	}

	return m_pGetDDSurfaceLocal();
}

HANDLE WINAPI dd_GetOLEThunkData(int i1)
{
	static GetOLEThunkDataProc m_pGetOLEThunkData = (Wrapper::ValidProcAddress(GetOLEThunkData_out)) ? (GetOLEThunkDataProc)GetOLEThunkData_out : nullptr;

	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return nullptr;
	}

	if (!m_pGetOLEThunkData)
	{
		return nullptr;
	}

	return m_pGetOLEThunkData(i1);
}

HRESULT WINAPI dd_GetSurfaceFromDC(HDC hdc, LPDIRECTDRAWSURFACE7 *lpDDS)
{
	static GetSurfaceFromDCProc m_pGetSurfaceFromDC = (Wrapper::ValidProcAddress(GetSurfaceFromDC_out)) ? (GetSurfaceFromDCProc)GetSurfaceFromDC_out : nullptr;

	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

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
	static RegisterSpecialCaseProc m_pRegisterSpecialCase = (Wrapper::ValidProcAddress(RegisterSpecialCase_out)) ? (RegisterSpecialCaseProc)RegisterSpecialCase_out : nullptr;

	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return;
	}

	if (!m_pRegisterSpecialCase)
	{
		return;
	}

	return m_pRegisterSpecialCase();
}

void WINAPI dd_ReleaseDDThreadLock()
{
	static ReleaseDDThreadLockProc m_pReleaseDDThreadLock = (Wrapper::ValidProcAddress(ReleaseDDThreadLock_out)) ? (ReleaseDDThreadLockProc)ReleaseDDThreadLock_out : nullptr;

	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return;
	}

	if (!m_pReleaseDDThreadLock)
	{
		return;
	}

	return m_pReleaseDDThreadLock();
}

HRESULT WINAPI dd_SetAppCompatData(DWORD Type, DWORD Value)
{
	static SetAppCompatDataProc m_pSetAppCompatData = (Wrapper::ValidProcAddress(SetAppCompatData_out)) ? (SetAppCompatDataProc)SetAppCompatData_out : nullptr;

	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	if (!m_pSetAppCompatData)
	{
		return E_FAIL;
	}

	return m_pSetAppCompatData(Type, Value);
}
