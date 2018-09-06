#pragma once

#include "Wrappers\wrapper.h"

void WINAPI dd_AcquireDDThreadLock();
void WINAPI dd_CompleteCreateSysmemSurface();
HRESULT WINAPI dd_D3DParseUnknownCommand(LPVOID lpCmd, LPVOID *lpRetCmd);
void WINAPI dd_DDGetAttachedSurfaceLcl();
void WINAPI dd_DDInternalLock();
void WINAPI dd_DDInternalUnlock();
void WINAPI dd_DSoundHelp();
HRESULT WINAPI dd_DirectDrawCreate(GUID FAR *lpGUID, LPDIRECTDRAW FAR *lplpDD, IUnknown FAR *pUnkOuter);
HRESULT WINAPI dd_DirectDrawCreateClipper(DWORD dwFlags, LPDIRECTDRAWCLIPPER *lplpDDClipper, LPUNKNOWN pUnkOuter);
HRESULT WINAPI dd_DirectDrawCreateEx(GUID FAR *lpGUID, LPVOID *lplpDD, REFIID riid, IUnknown FAR *pUnkOuter);
HRESULT WINAPI dd_DirectDrawEnumerateA(LPDDENUMCALLBACKA lpCallback, LPVOID lpContext);
HRESULT WINAPI dd_DirectDrawEnumerateExA(LPDDENUMCALLBACKEXA lpCallback, LPVOID lpContext, DWORD dwFlags);
HRESULT WINAPI dd_DirectDrawEnumerateExW(LPDDENUMCALLBACKEXW lpCallback, LPVOID lpContext, DWORD dwFlags);
HRESULT WINAPI dd_DirectDrawEnumerateW(LPDDENUMCALLBACKW lpCallback, LPVOID lpContext);
HRESULT WINAPI dd_DllCanUnloadNow();
HRESULT WINAPI dd_DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID *ppv);
void WINAPI dd_GetDDSurfaceLocal();
HANDLE WINAPI dd_GetOLEThunkData(int i1);
HRESULT WINAPI dd_GetSurfaceFromDC(HDC hdc, LPDIRECTDRAWSURFACE7 *lpDDS);
void WINAPI dd_RegisterSpecialCase();
void WINAPI dd_ReleaseDDThreadLock();
HRESULT WINAPI dd_SetAppCompatData(DWORD Type, DWORD Value);

#define DECLARE_IN_WRAPPED_PROC(procName, unused) \
	constexpr FARPROC procName ## _in = (FARPROC)*dd_ ## procName;

#define EXPORT_OUT_WRAPPED_PROC(procName, unused) \
	extern FARPROC procName ## _out;

namespace DdrawWrapper
{
	VISIT_PROCS_DDRAW(DECLARE_IN_WRAPPED_PROC);
	constexpr FARPROC DllCanUnloadNow_in = (FARPROC)*dd_DllCanUnloadNow;
	constexpr FARPROC DllGetClassObject_in = (FARPROC)*dd_DllGetClassObject;

	VISIT_PROCS_DDRAW(EXPORT_OUT_WRAPPED_PROC);
	extern FARPROC DllCanUnloadNow_out;
	extern FARPROC DllGetClassObject_out;
	extern FARPROC Direct3DCreate9;
}

#undef DECLARE_IN_WRAPPED_PROC
#undef EXPORT_OUT_WRAPPED_PROC
