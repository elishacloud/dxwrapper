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
*/

#include <ddraw.h>
#include "..\Wrappers\wrapper.h"

// ddraw proc typedefs
typedef void(WINAPI *AcquireDDThreadLockProc)();
typedef void(WINAPI *CompleteCreateSysmemSurfaceProc)();
typedef HRESULT(WINAPI *D3DParseUnknownCommandProc)(LPVOID lpCmd, LPVOID *lpRetCmd);
typedef void(WINAPI *DDGetAttachedSurfaceLclProc)();
typedef void(WINAPI *DDInternalLockProc)();
typedef void(WINAPI *DDInternalUnlockProc)();
typedef void(WINAPI *DSoundHelpProc)();
typedef HRESULT(WINAPI *DirectDrawCreateProc)(GUID FAR *lpGUID, LPDIRECTDRAW FAR *lplpDD, IUnknown FAR *pUnkOuter);
typedef HRESULT(WINAPI *DirectDrawCreateClipperProc)(DWORD dwFlags, LPDIRECTDRAWCLIPPER *lplpDDClipper, LPUNKNOWN pUnkOuter);
typedef HRESULT(WINAPI *DirectDrawEnumerateAProc)(LPDDENUMCALLBACKA lpCallback, LPVOID lpContext);
typedef HRESULT(WINAPI *DirectDrawEnumerateExAProc)(LPDDENUMCALLBACKEXA lpCallback, LPVOID lpContext, DWORD dwFlags);
typedef HRESULT(WINAPI *DirectDrawEnumerateExWProc)(LPDDENUMCALLBACKEXW lpCallback, LPVOID lpContext, DWORD dwFlags);
typedef HRESULT(WINAPI *DirectDrawEnumerateWProc)(LPDDENUMCALLBACKW lpCallback, LPVOID lpContext);
typedef HRESULT(WINAPI *DirectDrawCreateExProc)(GUID FAR *lpGUID, LPVOID *lplpDD, REFIID riid, IUnknown FAR *pUnkOuter);
typedef HRESULT(WINAPI *DllCanUnloadNowProc)();
typedef HRESULT(WINAPI *DllGetClassObjectProc)(REFCLSID rclsid, REFIID riid, LPVOID *ppv);
typedef void(WINAPI *GetDDSurfaceLocalProc)();
typedef HANDLE(WINAPI *GetOLEThunkDataProc)(int i1);
typedef HRESULT(WINAPI *GetSurfaceFromDCProc)(HDC hdc, LPDIRECTDRAWSURFACE7 *lpDDS);
typedef void(WINAPI *RegisterSpecialCaseProc)();
typedef void(WINAPI *ReleaseDDThreadLockProc)();
typedef HRESULT(WINAPI *SetAppCompatDataProc)(DWORD, DWORD);

namespace DdrawWrapper
{
	char dllname[MAX_PATH];

#define INITIALIZE_WRAPPED_PROC(procName, unused) \
	procName ## Proc m_p ## procName = nullptr;

	VISIT_PROCS_DDRAW(INITIALIZE_WRAPPED_PROC);
	VISIT_PROCS_DDRAW_SHARED(INITIALIZE_WRAPPED_PROC);

#undef INITIALIZE_WRAPPED_PROC
}

using namespace DdrawWrapper;

void InitDdraw()
{
	static bool RunOnce = true;

	if (RunOnce)
	{
		// Load dll
		HMODULE ddrawdll = LoadLibraryA(dllname);

		// Get function addresses
#define INITIALIZE_WRAPPED_PROC(procName, unused) \
	m_p ## procName = (procName ## Proc)GetProcAddress(ddrawdll, #procName);

		VISIT_PROCS_DDRAW(INITIALIZE_WRAPPED_PROC);
		VISIT_PROCS_DDRAW_SHARED(INITIALIZE_WRAPPED_PROC);

#undef INITIALIZE_WRAPPED_PROC

		RunOnce = false;
	}
}

void WINAPI dd_AcquireDDThreadLock()
{
	InitDdraw();

	if (!m_pAcquireDDThreadLock)
	{
		return;
	}

	return m_pAcquireDDThreadLock();
}

void WINAPI dd_CompleteCreateSysmemSurface()
{
	InitDdraw();

	if (!m_pCompleteCreateSysmemSurface)
	{
		return;
	}

	return m_pCompleteCreateSysmemSurface();
}

HRESULT WINAPI dd_D3DParseUnknownCommand(LPVOID lpCmd, LPVOID *lpRetCmd)
{
	InitDdraw();

	if (!m_pD3DParseUnknownCommand)
	{
		return DDERR_GENERIC;
	}

	return m_pD3DParseUnknownCommand(lpCmd, lpRetCmd);
}

void WINAPI dd_DDGetAttachedSurfaceLcl()
{
	InitDdraw();

	if (!m_pDDGetAttachedSurfaceLcl)
	{
		return;
	}

	return m_pDDGetAttachedSurfaceLcl();
}

void WINAPI dd_DDInternalLock()
{
	InitDdraw();

	if (!m_pDDInternalLock)
	{
		return;
	}

	return m_pDDInternalLock();
}

void WINAPI dd_DDInternalUnlock()
{
	InitDdraw();

	if (!m_pDDInternalUnlock)
	{
		return;
	}

	return m_pDDInternalUnlock();
}

void WINAPI dd_DSoundHelp()
{
	InitDdraw();

	if (!m_pDSoundHelp)
	{
		return;
	}

	return m_pDSoundHelp();
}

HRESULT WINAPI dd_DirectDrawCreate(GUID FAR *lpGUID, LPDIRECTDRAW FAR *lplpDD, IUnknown FAR *pUnkOuter)
{
	InitDdraw();

	if (!m_pDirectDrawCreate)
	{
		return DDERR_GENERIC;
	}

	return m_pDirectDrawCreate(lpGUID, lplpDD, pUnkOuter);
}

HRESULT WINAPI dd_DirectDrawCreateClipper(DWORD dwFlags, LPDIRECTDRAWCLIPPER *lplpDDClipper, LPUNKNOWN pUnkOuter)
{
	InitDdraw();

	if (!m_pDirectDrawCreateClipper)
	{
		return DDERR_GENERIC;
	}

	return m_pDirectDrawCreateClipper(dwFlags, lplpDDClipper, pUnkOuter);
}

HRESULT WINAPI dd_DirectDrawCreateEx(GUID FAR *lpGUID, LPVOID *lplpDD, REFIID riid, IUnknown FAR *pUnkOuter)
{
	InitDdraw();

	if (!m_pDirectDrawCreateEx)
	{
		return DDERR_GENERIC;
	}

	return m_pDirectDrawCreateEx(lpGUID, lplpDD, riid, pUnkOuter);
}

HRESULT WINAPI dd_DirectDrawEnumerateA(LPDDENUMCALLBACKA lpCallback, LPVOID lpContext)
{
	InitDdraw();

	if (!m_pDirectDrawEnumerateA)
	{
		return DDERR_GENERIC;
	}

	return m_pDirectDrawEnumerateA(lpCallback, lpContext);
}

HRESULT WINAPI dd_DirectDrawEnumerateExA(LPDDENUMCALLBACKEXA lpCallback, LPVOID lpContext, DWORD dwFlags)
{
	InitDdraw();

	if (!m_pDirectDrawEnumerateExA)
	{
		return DDERR_GENERIC;
	}

	return m_pDirectDrawEnumerateExA(lpCallback, lpContext, dwFlags);
}

HRESULT WINAPI dd_DirectDrawEnumerateExW(LPDDENUMCALLBACKEXW lpCallback, LPVOID lpContext, DWORD dwFlags)
{
	InitDdraw();

	if (!m_pDirectDrawEnumerateExW)
	{
		return DDERR_GENERIC;
	}

	return m_pDirectDrawEnumerateExW(lpCallback, lpContext, dwFlags);
}

HRESULT WINAPI dd_DirectDrawEnumerateW(LPDDENUMCALLBACKW lpCallback, LPVOID lpContext)
{
	InitDdraw();

	if (!m_pDirectDrawEnumerateW)
	{
		return DDERR_GENERIC;
	}

	return m_pDirectDrawEnumerateW(lpCallback, lpContext);
}

HRESULT WINAPI dd_DllCanUnloadNow()
{
	InitDdraw();

	if (!m_pDllCanUnloadNow)
	{
		return DDERR_GENERIC;
	}

	return m_pDllCanUnloadNow();
}

HRESULT WINAPI dd_DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID *ppv)
{
	InitDdraw();

	if (!m_pDllGetClassObject)
	{
		return DDERR_GENERIC;
	}

	return m_pDllGetClassObject(rclsid, riid, ppv);
}

void WINAPI dd_GetDDSurfaceLocal()
{
	InitDdraw();

	if (!m_pGetDDSurfaceLocal)
	{
		return;
	}

	return m_pGetDDSurfaceLocal();
}

HANDLE WINAPI dd_GetOLEThunkData(int i1)
{
	InitDdraw();

	if (!m_pGetOLEThunkData)
	{
		return nullptr;
	}

	return m_pGetOLEThunkData(i1);
}

HRESULT WINAPI dd_GetSurfaceFromDC(HDC hdc, LPDIRECTDRAWSURFACE7 *lpDDS)
{
	InitDdraw();

	if (!m_pGetSurfaceFromDC)
	{
		return DDERR_GENERIC;
	}

	return m_pGetSurfaceFromDC(hdc, lpDDS);
}

void WINAPI dd_RegisterSpecialCase()
{
	InitDdraw();

	if (!m_pRegisterSpecialCase)
	{
		return;
	}

	return m_pRegisterSpecialCase();
}

void WINAPI dd_ReleaseDDThreadLock()
{
	InitDdraw();

	if (!m_pReleaseDDThreadLock)
	{
		return;
	}

	return m_pReleaseDDThreadLock();
}

HRESULT WINAPI dd_SetAppCompatData(DWORD Type, DWORD Value)
{
	InitDdraw();

	if (!m_pSetAppCompatData)
	{
		return DDERR_GENERIC;
	}

	return m_pSetAppCompatData(Type, Value);
}

void StartDdraw(const char *name)
{
	if (name)
	{
		strcpy_s(dllname, MAX_PATH, name);
	}
	else
	{
		GetSystemDirectoryA(dllname, MAX_PATH);
		strcat_s(dllname, "\\ddraw.dll");
	}

#define INITIALIZE_WRAPPED_PROC(procName, unused) \
	ddraw::procName ## _var = (FARPROC)*dd_ ## procName;

	VISIT_PROCS_DDRAW(INITIALIZE_WRAPPED_PROC);
	VISIT_PROCS_DDRAW_SHARED(INITIALIZE_WRAPPED_PROC);
}
