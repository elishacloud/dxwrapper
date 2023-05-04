#pragma once

#include <ddraw.h>
#include "Wrappers\wrapper.h"

HRESULT WINAPI dd_AcquireDDThreadLock();
DWORD WINAPI dd_CompleteCreateSysmemSurface(DWORD arg);
HRESULT WINAPI dd_D3DParseUnknownCommand(LPVOID lpCmd, LPVOID *lpRetCmd);
HRESULT WINAPI dd_DDGetAttachedSurfaceLcl(DWORD arg1, DWORD arg2, DWORD arg3);
DWORD WINAPI dd_DDInternalLock(DWORD arg1, DWORD arg2);
DWORD WINAPI dd_DDInternalUnlock(DWORD arg);
HRESULT WINAPI dd_DSoundHelp(DWORD arg1, DWORD arg2, DWORD arg3);
HRESULT WINAPI dd_DirectDrawCreate(GUID FAR *lpGUID, LPDIRECTDRAW FAR *lplpDD, IUnknown FAR *pUnkOuter);
HRESULT WINAPI dd_DirectDrawCreateClipper(DWORD dwFlags, LPDIRECTDRAWCLIPPER *lplpDDClipper, LPUNKNOWN pUnkOuter);
HRESULT WINAPI dd_DirectDrawCreateEx(GUID FAR *lpGUID, LPVOID *lplpDD, REFIID riid, IUnknown FAR *pUnkOuter);
HRESULT WINAPI dd_DirectDrawEnumerateA(LPDDENUMCALLBACKA lpCallback, LPVOID lpContext);
HRESULT WINAPI dd_DirectDrawEnumerateExA(LPDDENUMCALLBACKEXA lpCallback, LPVOID lpContext, DWORD dwFlags);
HRESULT WINAPI dd_DirectDrawEnumerateExW(LPDDENUMCALLBACKEXW lpCallback, LPVOID lpContext, DWORD dwFlags);
HRESULT WINAPI dd_DirectDrawEnumerateW(LPDDENUMCALLBACKW lpCallback, LPVOID lpContext);
HRESULT WINAPI dd_DllCanUnloadNow();
HRESULT WINAPI dd_DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID *ppv);
HRESULT WINAPI dd_GetDDSurfaceLocal(DWORD arg1, DWORD arg2, DWORD arg3);
DWORD WINAPI dd_GetOLEThunkData(DWORD index);
HRESULT WINAPI dd_GetSurfaceFromDC(HDC hdc, LPDIRECTDRAWSURFACE7 *lpDDS, DWORD arg);
HRESULT WINAPI dd_RegisterSpecialCase(DWORD arg1, DWORD arg2, DWORD arg3, DWORD arg4);
HRESULT WINAPI dd_ReleaseDDThreadLock();
HRESULT WINAPI dd_SetAppCompatData(DWORD Type, DWORD Value);

class m_IDirectDrawClipper;

void AddBaseClipperToVetor(m_IDirectDrawClipper* lpClipper);
void RemoveBaseClipperFromVector(m_IDirectDrawClipper* lpClipper);
bool DoesBaseClipperExist(m_IDirectDrawClipper* lpClipper);

DWORD GetDDrawBitsPixel();
DWORD GetDDrawWidth();
DWORD GetDDrawHeight();

void InitDDraw();
void ExitDDraw();

#define DECLARE_IN_WRAPPED_PROC(procName, unused) \
	const FARPROC procName ## _in = (FARPROC)*dd_ ## procName;

#define EXPORT_OUT_WRAPPED_PROC(procName, unused) \
	extern FARPROC procName ## _out;

namespace DdrawWrapper
{
	VISIT_PROCS_DDRAW(DECLARE_IN_WRAPPED_PROC);
	VISIT_PROCS_DDRAW_SHARED(DECLARE_IN_WRAPPED_PROC);

	VISIT_PROCS_DDRAW(EXPORT_OUT_WRAPPED_PROC);
	VISIT_PROCS_DDRAW_SHARED(EXPORT_OUT_WRAPPED_PROC);
	extern FARPROC Direct3DCreate9_out;
}

#undef DECLARE_IN_WRAPPED_PROC
#undef EXPORT_OUT_WRAPPED_PROC
