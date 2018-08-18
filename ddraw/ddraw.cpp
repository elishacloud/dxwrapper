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
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return;
	}

	if (!Wrapper::ValidProcAddress(AcquireDDThreadLock_out))
	{
		return;
	}
	return ((AcquireDDThreadLockProc)AcquireDDThreadLock_out)();
}

void WINAPI dd_CompleteCreateSystemSurface()
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return;
	}

	if (!Wrapper::ValidProcAddress(CompleteCreateSystemSurface_out))
	{
		return;
	}
	return ((CompleteCreateSystemSurfaceProc)CompleteCreateSystemSurface_out)();
}

HRESULT WINAPI dd_D3DParseUnknownCommand(LPVOID lpCmd, LPVOID *lpRetCmd)
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	if (!Wrapper::ValidProcAddress(D3DParseUnknownCommand_out))
	{
		return DDERR_INVALIDOBJECT;
	}
	return ((D3DParseUnknownCommandProc)D3DParseUnknownCommand_out)(lpCmd, lpRetCmd);
}

void WINAPI dd_DDGetAttachedSurfaceLcl()
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return;
	}

	if (!Wrapper::ValidProcAddress(DDGetAttachedSurfaceLcl_out))
	{
		return;
	}
	((DDGetAttachedSurfaceLclProc)DDGetAttachedSurfaceLcl_out)();
}

void WINAPI dd_DDInternalLock()
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return;
	}

	if (!Wrapper::ValidProcAddress(DDInternalLock_out))
	{
		return;
	}
	((DDInternalLockProc)DDInternalLock_out)();
}

void WINAPI dd_DDInternalUnlock()
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return;
	}

	if (!Wrapper::ValidProcAddress(DDInternalUnlock_out))
	{
		return;
	}
	((DDInternalUnlockProc)DDInternalUnlock_out)();
}

void WINAPI dd_DSoundHelp()
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return;
	}

	if (!Wrapper::ValidProcAddress(DSoundHelp_out))
	{
		return;
	}
	((DSoundHelpProc)DSoundHelp_out)();
}

HRESULT WINAPI dd_DirectDrawCreate(GUID FAR *lpGUID, LPDIRECTDRAW FAR *lplpDD, IUnknown FAR *pUnkOuter)
{
	if (!Wrapper::ValidProcAddress(DirectDrawCreate_out))
	{
		return DDERR_INVALIDOBJECT;
	}

	if (Config.ConvertToDirect3D7 && Config.ConvertToDirectDraw7)
	{
		return dd_DirectDrawCreateEx(lpGUID, (LPVOID*)lplpDD, IID_IDirectDraw, pUnkOuter);
	}

	HRESULT hr = ((DirectDrawCreateProc)DirectDrawCreate_out)(lpGUID, lplpDD, pUnkOuter);

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
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	if (!Wrapper::ValidProcAddress(DirectDrawCreateClipper_out))
	{
		return DDERR_INVALIDOBJECT;
	}

	HRESULT hr = ((DirectDrawCreateClipperProc)DirectDrawCreateClipper_out)(dwFlags, lplpDDClipper, pUnkOuter);

	if (SUCCEEDED(hr) && lplpDDClipper)
	{
		*lplpDDClipper = ProxyAddressLookupTable.FindAddress<m_IDirectDrawClipper>(*lplpDDClipper);
	}

	return hr;
}

HRESULT WINAPI dd_DirectDrawCreateEx(GUID FAR *lpGUID, LPVOID *lplpDD, REFIID riid, IUnknown FAR *pUnkOuter)
{
	if (!Wrapper::ValidProcAddress(DirectDrawCreateEx_out))
	{
		return DDERR_INVALIDOBJECT;
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

	HRESULT hr = ((DDrawCreateExProc)DirectDrawCreateEx_out)(lpGUID, lplpDD, IID_IDirectDraw7, pUnkOuter);

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
				// Error creation directdraw
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
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	if (!Wrapper::ValidProcAddress(DirectDrawEnumerateA_out))
	{
		return DDERR_INVALIDOBJECT;
	}
	return ((DDrawEnumerateAProc)DirectDrawEnumerateA_out)(lpCallback, lpContext);
}

HRESULT WINAPI dd_DirectDrawEnumerateExA(LPDDENUMCALLBACKEXA lpCallback, LPVOID lpContext, DWORD dwFlags)
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	if (!Wrapper::ValidProcAddress(DirectDrawEnumerateExA_out))
	{
		return DDERR_INVALIDOBJECT;
	}
	return ((DDrawEnumerateExAProc)DirectDrawEnumerateExA_out)(lpCallback, lpContext, dwFlags);
}

HRESULT WINAPI dd_DirectDrawEnumerateExW(LPDDENUMCALLBACKEXW lpCallback, LPVOID lpContext, DWORD dwFlags)
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	if (!Wrapper::ValidProcAddress(DirectDrawEnumerateExW_out))
	{
		return DDERR_INVALIDOBJECT;
	}
	return ((DDrawEnumerateExWProc)DirectDrawEnumerateExW_out)(lpCallback, lpContext, dwFlags);
}

HRESULT WINAPI dd_DirectDrawEnumerateW(LPDDENUMCALLBACKW lpCallback, LPVOID lpContext)
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	if (!Wrapper::ValidProcAddress(DirectDrawEnumerateW_out))
	{
		return DDERR_INVALIDOBJECT;
	}
	return ((DDrawEnumerateWProc)DirectDrawEnumerateW_out)(lpCallback, lpContext);
}

HRESULT WINAPI dd_DllCanUnloadNow()
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	if (!Wrapper::ValidProcAddress(DllCanUnloadNow_out))
	{
		return DDERR_INVALIDOBJECT;
	}
	return ((DllCanUnloadNowProc)DllCanUnloadNow_out)();
}

HRESULT WINAPI dd_DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID *ppv)
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	if (!Wrapper::ValidProcAddress(DllGetClassObject_out))
	{
		return DDERR_INVALIDOBJECT;
	}

	HRESULT hr = ((DllGetClassObjectProc)DllGetClassObject_out)(rclsid, ConvertREFIID(riid), ppv);

	if (SUCCEEDED(hr))
	{
		genericQueryInterface(riid, ppv);
	}

	return hr;
}

void WINAPI dd_GetDDSurfaceLocal()
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return;
	}

	if (!Wrapper::ValidProcAddress(GetDDSurfaceLocal_out))
	{
		return;
	}
	((GetDDSurfaceLocalProc)GetDDSurfaceLocal_out)();
}

HANDLE WINAPI dd_GetOLEThunkData(int i1)
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return nullptr;
	}

	if (!Wrapper::ValidProcAddress(GetOLEThunkData_out))
	{
		return nullptr;
	}
	return ((GetOLEThunkDataProc)GetOLEThunkData_out)(i1);
}

HRESULT WINAPI dd_GetSurfaceFromDC(HDC hdc, LPDIRECTDRAWSURFACE7 *lpDDS)
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	if (!Wrapper::ValidProcAddress(GetSurfaceFromDC_out))
	{
		return DDERR_INVALIDOBJECT;
	}

	HRESULT hr = ((GetSurfaceFromDCProc)GetSurfaceFromDC_out)(hdc, lpDDS);

	if (SUCCEEDED(hr))
	{
		*lpDDS = ProxyAddressLookupTable.FindAddress<m_IDirectDrawSurface7>(*lpDDS);
	}

	return hr;
}

void WINAPI dd_RegisterSpecialCase()
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return;
	}

	if (!Wrapper::ValidProcAddress(RegisterSpecialCase_out))
	{
		return;
	}
	((RegisterSpecialCaseProc)RegisterSpecialCase_out)();
}

void WINAPI dd_ReleaseDDThreadLock()
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return;
	}

	if (!Wrapper::ValidProcAddress(ReleaseDDThreadLock_out))
	{
		return;
	}
	return ((ReleaseDDThreadLockProc)ReleaseDDThreadLock_out)();
}

HRESULT WINAPI dd_SetAppCompatData(DWORD Type, DWORD Value)
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	if (!Wrapper::ValidProcAddress(SetAppCompatData_out))
	{
		return DDERR_INVALIDOBJECT;
	}
	return ((SetAppCompatDataProc)SetAppCompatData_out)(Type, Value);
}
