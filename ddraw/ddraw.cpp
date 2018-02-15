/**
* Copyright (C) 2017 Elisha Riedlinger
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

AddressLookupTable<void> ProxyAddressLookupTable = AddressLookupTable<void>(nullptr);

namespace DdrawWrapper
{
	FARPROC AcquireDDThreadLock_out = nullptr;
	FARPROC CheckFullscreen_out = nullptr;
	FARPROC CompleteCreateSysmemSurface_out = nullptr;
	FARPROC D3DParseUnknownCommand_out = nullptr;
	FARPROC DDGetAttachedSurfaceLcl_out = nullptr;
	FARPROC DDInternalLock_out = nullptr;
	FARPROC DDInternalUnlock_out = nullptr;
	FARPROC DSoundHelp_out = nullptr;
	FARPROC DirectDrawCreate_out = nullptr;
	FARPROC DirectDrawCreateClipper_out = nullptr;
	FARPROC DirectDrawCreateEx_out = nullptr;
	FARPROC DirectDrawEnumerateA_out = nullptr;
	FARPROC DirectDrawEnumerateExA_out = nullptr;
	FARPROC DirectDrawEnumerateExW_out = nullptr;
	FARPROC DirectDrawEnumerateW_out = nullptr;
	FARPROC DllCanUnloadNow_out = nullptr;
	FARPROC DllGetClassObject_out = nullptr;
	FARPROC GetDDSurfaceLocal_out = nullptr;
	FARPROC GetOLEThunkData_out = nullptr;
	FARPROC GetSurfaceFromDC_out = nullptr;
	FARPROC RegisterSpecialCase_out = nullptr;
	FARPROC ReleaseDDThreadLock_out = nullptr;
	FARPROC SetAppCompatData_out = nullptr;
}

using namespace DdrawWrapper;

void WINAPI dd_AcquireDDThreadLock()
{
	return ((AcquireDDThreadLockProc)AcquireDDThreadLock_out)();
}

void __declspec(naked) dd_CheckFullscreen()
{
	_asm jmp CheckFullscreen_out;
}

void __declspec(naked) dd_CompleteCreateSysmemSurface()
{
	_asm jmp CompleteCreateSysmemSurface_out;
}

HRESULT WINAPI dd_D3DParseUnknownCommand(LPVOID lpCmd, LPVOID *lpRetCmd)
{
	return ((D3DParseUnknownCommandProc)D3DParseUnknownCommand_out)(lpCmd, lpRetCmd);
}

void __declspec(naked) dd_DDGetAttachedSurfaceLcl()
{
	_asm jmp DDGetAttachedSurfaceLcl_out;
}

void __declspec(naked) dd_DDInternalLock()
{
	_asm jmp DDInternalLock_out;
}

void __declspec(naked) dd_DDInternalUnlock()
{
	_asm jmp DDInternalUnlock_out;
}

void __declspec(naked) dd_DSoundHelp()
{
	_asm jmp DSoundHelp_out;
}

HRESULT WINAPI dd_DirectDrawCreate(GUID FAR *lpGUID, LPDIRECTDRAW FAR *lplpDD, IUnknown FAR *pUnkOuter)
{
	HRESULT hr = ((DirectDrawCreateProc)DirectDrawCreate_out)(lpGUID, lplpDD, pUnkOuter);

	if (SUCCEEDED(hr))
	{
		*lplpDD = ProxyAddressLookupTable.FindAddress<m_IDirectDraw>(*lplpDD);
	}

	return hr;
}

HRESULT WINAPI dd_DirectDrawCreateClipper(DWORD dwFlags, LPDIRECTDRAWCLIPPER *lplpDDClipper, LPUNKNOWN pUnkOuter)
{
	return ((DirectDrawCreateClipperProc)DirectDrawCreateClipper_out)(dwFlags, lplpDDClipper, pUnkOuter);
}

HRESULT WINAPI dd_DirectDrawCreateEx(GUID FAR *lpGUID, LPVOID *lplpDD, REFIID riid, IUnknown FAR *pUnkOuter)
{
	HRESULT hr = ((DDrawCreateExProc)DirectDrawCreateEx_out)(lpGUID, lplpDD, riid, pUnkOuter);

	if (SUCCEEDED(hr))
	{
		genericQueryInterface(riid, lplpDD);
	}

	return hr;
}

HRESULT WINAPI dd_DirectDrawEnumerateA(LPDDENUMCALLBACKA lpCallback, LPVOID lpContext)
{
	return ((DDrawEnumerateAProc)DirectDrawEnumerateA_out)(lpCallback, lpContext);
}

HRESULT WINAPI dd_DirectDrawEnumerateExA(LPDDENUMCALLBACKEXA lpCallback, LPVOID lpContext, DWORD dwFlags)
{
	return ((DDrawEnumerateExAProc)DirectDrawEnumerateExA_out)(lpCallback, lpContext, dwFlags);
}

HRESULT WINAPI dd_DirectDrawEnumerateExW(LPDDENUMCALLBACKEXW lpCallback, LPVOID lpContext, DWORD dwFlags)
{
	return ((DDrawEnumerateExWProc)DirectDrawEnumerateExW_out)(lpCallback, lpContext, dwFlags);
}

HRESULT WINAPI dd_DirectDrawEnumerateW(LPDDENUMCALLBACKW lpCallback, LPVOID lpContext)
{
	return ((DDrawEnumerateWProc)DirectDrawEnumerateW_out)(lpCallback, lpContext);
}

HRESULT WINAPI dd_DllCanUnloadNow()
{
	return ((DllCanUnloadNowProc)DllCanUnloadNow_out)();
}

HRESULT WINAPI dd_DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID *ppv)
{
	HRESULT hr = ((DllGetClassObjectProc)DllGetClassObject_out)(rclsid, riid, ppv);

	if (SUCCEEDED(hr))
	{
		genericQueryInterface(riid, ppv);
	}

	return hr;
}

void __declspec(naked) dd_GetDDSurfaceLocal()
{
	_asm jmp GetDDSurfaceLocal_out;
}

void __declspec(naked) dd_GetOLEThunkData()
{
	_asm jmp GetOLEThunkData_out;
}

HRESULT WINAPI dd_GetSurfaceFromDC(HDC hdc, LPDIRECTDRAWSURFACE7 *lpDDS)
{
	HRESULT hr = ((GetSurfaceFromDCProc)GetSurfaceFromDC_out)(hdc, lpDDS);

	if (SUCCEEDED(hr))
	{
		*lpDDS = ProxyAddressLookupTable.FindAddress<m_IDirectDrawSurface7>(*lpDDS);
	}

	return hr;
}

void __declspec(naked) dd_RegisterSpecialCase()
{
	_asm jmp RegisterSpecialCase_out;
}

void WINAPI dd_ReleaseDDThreadLock()
{
	return ((ReleaseDDThreadLockProc)ReleaseDDThreadLock_out)();
}

HRESULT WINAPI dd_SetAppCompatData(DWORD Type, DWORD Value)
{
	return ((SetAppCompatDataProc)SetAppCompatData_out)(Type, Value);
}
