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
}

using namespace DdrawWrapper;

void WINAPI dd_AcquireDDThreadLock()
{
	if (!Wrapper::ValidProcAddress(AcquireDDThreadLock_out))
	{
		return;
	}
	return ((AcquireDDThreadLockProc)AcquireDDThreadLock_out)();
}

#pragma warning(suppress: 4740)
void __declspec(naked) dd_CompleteCreateSysmemSurface()
{
	Logging::LogFormat("CompleteCreateSysmemSurface Not Implimented");
	_asm jmp CompleteCreateSysmemSurface_out;
}

HRESULT WINAPI dd_D3DParseUnknownCommand(LPVOID lpCmd, LPVOID *lpRetCmd)
{
	if (!Wrapper::ValidProcAddress(D3DParseUnknownCommand_out))
	{
		return DDERR_INVALIDOBJECT;
	}
	return ((D3DParseUnknownCommandProc)D3DParseUnknownCommand_out)(lpCmd, lpRetCmd);
}

#pragma warning(suppress: 4740)
void __declspec(naked) dd_DDGetAttachedSurfaceLcl()
{
	Logging::LogFormat("DDGetAttachedSurfaceLcl Not Implimented");
	_asm jmp DDGetAttachedSurfaceLcl_out;
}

#pragma warning(suppress: 4740)
void __declspec(naked) dd_DDInternalLock()
{
	Logging::LogFormat("DDInternalLock Not Implimented");
	_asm jmp DDInternalLock_out;
}

#pragma warning(suppress: 4740)
void __declspec(naked) dd_DDInternalUnlock()
{
	Logging::LogFormat("DDInternalUnlock Not Implimented");
	_asm jmp DDInternalUnlock_out;
}

#pragma warning(suppress: 4740)
void __declspec(naked) dd_DSoundHelp()
{
	Logging::LogFormat("DSoundHelp Not Implimented");
	_asm jmp DSoundHelp_out;
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
		genericQueryInterface(riid, lplpDD);
	}

	return hr;
}

HRESULT WINAPI dd_DirectDrawEnumerateA(LPDDENUMCALLBACKA lpCallback, LPVOID lpContext)
{
	if (!Wrapper::ValidProcAddress(DirectDrawEnumerateA_out))
	{
		return DDERR_INVALIDOBJECT;
	}
	return ((DDrawEnumerateAProc)DirectDrawEnumerateA_out)(lpCallback, lpContext);
}

HRESULT WINAPI dd_DirectDrawEnumerateExA(LPDDENUMCALLBACKEXA lpCallback, LPVOID lpContext, DWORD dwFlags)
{
	if (!Wrapper::ValidProcAddress(DirectDrawEnumerateExA_out))
	{
		return DDERR_INVALIDOBJECT;
	}
	return ((DDrawEnumerateExAProc)DirectDrawEnumerateExA_out)(lpCallback, lpContext, dwFlags);
}

HRESULT WINAPI dd_DirectDrawEnumerateExW(LPDDENUMCALLBACKEXW lpCallback, LPVOID lpContext, DWORD dwFlags)
{
	if (!Wrapper::ValidProcAddress(DirectDrawEnumerateExW_out))
	{
		return DDERR_INVALIDOBJECT;
	}
	return ((DDrawEnumerateExWProc)DirectDrawEnumerateExW_out)(lpCallback, lpContext, dwFlags);
}

HRESULT WINAPI dd_DirectDrawEnumerateW(LPDDENUMCALLBACKW lpCallback, LPVOID lpContext)
{
	if (!Wrapper::ValidProcAddress(DirectDrawEnumerateW_out))
	{
		return DDERR_INVALIDOBJECT;
	}
	return ((DDrawEnumerateWProc)DirectDrawEnumerateW_out)(lpCallback, lpContext);
}

HRESULT WINAPI dd_DllCanUnloadNow()
{
	if (!Wrapper::ValidProcAddress(DllCanUnloadNow_out))
	{
		return DDERR_INVALIDOBJECT;
	}
	return ((DllCanUnloadNowProc)DllCanUnloadNow_out)();
}

HRESULT WINAPI dd_DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID *ppv)
{
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

#pragma warning(suppress: 4740)
void __declspec(naked) dd_GetDDSurfaceLocal()
{
	Logging::LogFormat("GetDDSurfaceLocal Not Implimented");
	_asm jmp GetDDSurfaceLocal_out;
}

#pragma warning(suppress: 4740)
void __declspec(naked) dd_GetOLEThunkData()
{
	Logging::LogFormat("GetOLEThunkData Not Implimented");
	_asm jmp GetOLEThunkData_out;
}

HRESULT WINAPI dd_GetSurfaceFromDC(HDC hdc, LPDIRECTDRAWSURFACE7 *lpDDS)
{
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

#pragma warning(suppress: 4740)
void __declspec(naked) dd_RegisterSpecialCase()
{
	Logging::LogFormat("RegisterSpecialCase Not Implimented");
	_asm jmp RegisterSpecialCase_out;
}

void WINAPI dd_ReleaseDDThreadLock()
{
	if (!Wrapper::ValidProcAddress(ReleaseDDThreadLock_out))
	{
		return;
	}
	return ((ReleaseDDThreadLockProc)ReleaseDDThreadLock_out)();
}

HRESULT WINAPI dd_SetAppCompatData(DWORD Type, DWORD Value)
{
	if (!Wrapper::ValidProcAddress(SetAppCompatData_out))
	{
		return DDERR_INVALIDOBJECT;
	}
	return ((SetAppCompatDataProc)SetAppCompatData_out)(Type, Value);
}
