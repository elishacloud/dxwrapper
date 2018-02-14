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

AcquireDDThreadLockProc m_pAcquireDDThreadLock;
FARPROC m_pCompleteCreateSysmemSurface;
D3DParseUnknownCommandProc m_pD3DParseUnknownCommand;
FARPROC m_pDDGetAttachedSurfaceLcl;
FARPROC m_pDDInternalLock;
FARPROC m_pDDInternalUnlock;
FARPROC m_pDSoundHelp;
DDrawCreateProc m_pDDrawCreate;
DirectDrawCreateClipperProc m_pDirectDrawCreateClipper;
DDrawCreateExProc m_pDDrawCreateEx;
DDrawEnumerateAProc m_pDDrawEnumerateA;
DDrawEnumerateExAProc m_pDDrawEnumerateExA;
DDrawEnumerateExWProc m_pDDrawEnumerateExW;
DDrawEnumerateWProc m_pDDrawEnumerateW;
DllCanUnloadNowProc m_pDllCanUnloadNow;
DllGetClassObjectProc m_pDllGetClassObject;
FARPROC m_pGetDDSurfaceLocal;
FARPROC m_pGetOLEThunkData;
GetSurfaceFromDCProc m_pGetSurfaceFromDC;
FARPROC m_pRegisterSpecialCase;
ReleaseDDThreadLockProc m_pReleaseDDThreadLock;
SetAppCompatDataProc m_pSetAppCompatData;

namespace DdrawWrapper
{
	FARPROC out_DirectDrawCreate = nullptr;
	FARPROC out_DirectDrawCreateEx = nullptr;
}

using namespace DdrawWrapper;

bool _stdcall dd_DllMain(HANDLE, DWORD, LPVOID)
{
	//Get function addresses
	HMODULE ddrawdll = nullptr;
	m_pAcquireDDThreadLock = (AcquireDDThreadLockProc)GetProcAddress(ddrawdll, "AcquireDDThreadLock");
	m_pCompleteCreateSysmemSurface = GetProcAddress(ddrawdll, "CompleteCreateSysmemSurface");
	m_pD3DParseUnknownCommand = (D3DParseUnknownCommandProc)GetProcAddress(ddrawdll, "D3DParseUnknownCommand");
	m_pDDGetAttachedSurfaceLcl = GetProcAddress(ddrawdll, "DDGetAttachedSurfaceLcl");
	m_pDDInternalLock = GetProcAddress(ddrawdll, "DDInternalLock");
	m_pDDInternalUnlock = GetProcAddress(ddrawdll, "DDInternalUnlock");
	m_pDSoundHelp = GetProcAddress(ddrawdll, "DSoundHelp");
	m_pDirectDrawCreateClipper = (DirectDrawCreateClipperProc)GetProcAddress(ddrawdll, "DirectDrawCreateClipper");
	m_pDDrawEnumerateA = (DDrawEnumerateAProc)GetProcAddress(ddrawdll, "DirectDrawEnumerateA");
	m_pDDrawEnumerateExA = (DDrawEnumerateExAProc)GetProcAddress(ddrawdll, "DirectDrawEnumerateExA");
	m_pDDrawEnumerateExW = (DDrawEnumerateExWProc)GetProcAddress(ddrawdll, "DirectDrawEnumerateExW");
	m_pDDrawEnumerateW = (DDrawEnumerateWProc)GetProcAddress(ddrawdll, "DirectDrawEnumerateW");
	m_pDllCanUnloadNow = (DllCanUnloadNowProc)GetProcAddress(ddrawdll, "DllCanUnloadNow");
	m_pDllGetClassObject = (DllGetClassObjectProc)GetProcAddress(ddrawdll, "DllGetClassObject");
	m_pGetDDSurfaceLocal = GetProcAddress(ddrawdll, "GetDDSurfaceLocal");
	m_pGetOLEThunkData = GetProcAddress(ddrawdll, "GetOLEThunkData");
	m_pGetSurfaceFromDC = (GetSurfaceFromDCProc)GetProcAddress(ddrawdll, "GetSurfaceFromDC");
	m_pRegisterSpecialCase = GetProcAddress(ddrawdll, "RegisterSpecialCase");
	m_pReleaseDDThreadLock = (ReleaseDDThreadLockProc)GetProcAddress(ddrawdll, "ReleaseDDThreadLock");
	m_pSetAppCompatData = (SetAppCompatDataProc)GetProcAddress(ddrawdll, "SetAppCompatData");
	return true;
}

void WINAPI _AcquireDDThreadLock()
{
	return m_pAcquireDDThreadLock();
}

void __declspec(naked) _CompleteCreateSysmemSurface()
{
	_asm jmp m_pCompleteCreateSysmemSurface;
}

HRESULT WINAPI _D3DParseUnknownCommand(LPVOID lpCmd, LPVOID *lpRetCmd)
{
	return m_pD3DParseUnknownCommand(lpCmd, lpRetCmd);
}

void __declspec(naked) _DDGetAttachedSurfaceLcl()
{
	_asm jmp m_pDDGetAttachedSurfaceLcl;
}

void __declspec(naked) _DDInternalLock()
{
	_asm jmp m_pDDInternalLock;
}

void __declspec(naked) _DDInternalUnlock()
{
	_asm jmp m_pDDInternalUnlock;
}

void __declspec(naked) _DSoundHelp()
{
	_asm jmp m_pDSoundHelp;
}

HRESULT WINAPI dd_DirectDrawCreate(GUID FAR *lpGUID, LPDIRECTDRAW FAR *lplpDD, IUnknown FAR *pUnkOuter)
{
	m_pDDrawCreate = (DDrawCreateProc)out_DirectDrawCreate;
	HRESULT hr = m_pDDrawCreate(lpGUID, lplpDD, pUnkOuter);

	if (SUCCEEDED(hr))
	{
		*lplpDD = ProxyAddressLookupTable.FindAddress<m_IDirectDraw>(*lplpDD);
	}

	return hr;
}

HRESULT WINAPI _DirectDrawCreateClipper(DWORD dwFlags, LPDIRECTDRAWCLIPPER *lplpDDClipper, LPUNKNOWN pUnkOuter)
{
	return m_pDirectDrawCreateClipper(dwFlags, lplpDDClipper, pUnkOuter);
}

HRESULT WINAPI dd_DirectDrawCreateEx(GUID FAR *lpGUID, LPVOID *lplpDD, REFIID riid, IUnknown FAR *pUnkOuter)
{
	m_pDDrawCreateEx = (DDrawCreateExProc)out_DirectDrawCreateEx;
	HRESULT hr = m_pDDrawCreateEx(lpGUID, lplpDD, riid, pUnkOuter);

	if (SUCCEEDED(hr))
	{
		genericQueryInterface(riid, lplpDD);
	}

	return hr;
}

HRESULT WINAPI _DirectDrawEnumerateA(LPDDENUMCALLBACKA lpCallback, LPVOID lpContext)
{
	return m_pDDrawEnumerateA(lpCallback, lpContext);
}

HRESULT WINAPI _DirectDrawEnumerateExA(LPDDENUMCALLBACKEXA lpCallback, LPVOID lpContext, DWORD dwFlags)
{
	return m_pDDrawEnumerateExA(lpCallback, lpContext, dwFlags);
}

HRESULT WINAPI _DirectDrawEnumerateExW(LPDDENUMCALLBACKEXW lpCallback, LPVOID lpContext, DWORD dwFlags)
{
	return m_pDDrawEnumerateExW(lpCallback, lpContext, dwFlags);
}

HRESULT WINAPI _DirectDrawEnumerateW(LPDDENUMCALLBACKW lpCallback, LPVOID lpContext)
{
	return m_pDDrawEnumerateW(lpCallback, lpContext);
}

HRESULT WINAPI _DllCanUnloadNow()
{
	return m_pDllCanUnloadNow();
}

HRESULT WINAPI _DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID *ppv)
{
	HRESULT hr = m_pDllGetClassObject(rclsid, riid, ppv);

	if (SUCCEEDED(hr))
	{
		genericQueryInterface(riid, ppv);
	}

	return hr;
}

void __declspec(naked) _GetDDSurfaceLocal()
{
	_asm jmp m_pGetDDSurfaceLocal;
}

void __declspec(naked) _GetOLEThunkData()
{
	_asm jmp m_pGetOLEThunkData;
}

extern "C" HRESULT WINAPI _GetSurfaceFromDC(HDC hdc, LPDIRECTDRAWSURFACE7 *lpDDS)
{
	HRESULT hr = m_pGetSurfaceFromDC(hdc, lpDDS);

	if (SUCCEEDED(hr))
	{
		*lpDDS = ProxyAddressLookupTable.FindAddress<m_IDirectDrawSurface7>(*lpDDS);
	}

	return hr;
}

void __declspec(naked) _RegisterSpecialCase()
{
	_asm jmp m_pRegisterSpecialCase;
}

void WINAPI _ReleaseDDThreadLock()
{
	return m_pReleaseDDThreadLock();
}

HRESULT WINAPI _SetAppCompatData(DWORD Type, DWORD Value)
{
	return m_pSetAppCompatData(Type, Value);
}
