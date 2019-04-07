#pragma once

#include <mmeapi.h>
#include <dsound.h>
#include "Wrappers\wrapper.h"

HRESULT WINAPI ds_DirectSoundCreate(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter);
HRESULT WINAPI ds_DirectSoundCreate8(LPCGUID pcGuidDevice, LPDIRECTSOUND8 *ppDS8, LPUNKNOWN pUnkOuter);
HRESULT WINAPI ds_GetDeviceID(LPCGUID pGuidSrc, LPGUID pGuidDest);
HRESULT WINAPI ds_DirectSoundEnumerateA(LPDSENUMCALLBACKA pDSEnumCallback, LPVOID pContext);
HRESULT WINAPI ds_DirectSoundEnumerateW(LPDSENUMCALLBACKW pDSEnumCallback, LPVOID pContext);
HRESULT WINAPI ds_DirectSoundCaptureEnumerateA(LPDSENUMCALLBACKA pDSEnumCallback, LPVOID pContext);
HRESULT WINAPI ds_DirectSoundCaptureEnumerateW(LPDSENUMCALLBACKW pDSEnumCallback, LPVOID pContext);
HRESULT WINAPI ds_DirectSoundCaptureCreate(LPCGUID pcGuidDevice, LPDIRECTSOUNDCAPTURE *ppDSC, LPUNKNOWN pUnkOuter);
HRESULT WINAPI ds_DirectSoundCaptureCreate8(LPCGUID pcGuidDevice, LPDIRECTSOUNDCAPTURE8 *ppDSC8, LPUNKNOWN pUnkOuter);
HRESULT WINAPI ds_DirectSoundFullDuplexCreate(LPCGUID pcGuidCaptureDevice, LPCGUID pcGuidRenderDevice,
	LPCDSCBUFFERDESC pcDSCBufferDesc, LPCDSBUFFERDESC pcDSBufferDesc, HWND hWnd,
	DWORD dwLevel, LPDIRECTSOUNDFULLDUPLEX* ppDSFD, LPDIRECTSOUNDCAPTUREBUFFER8 *ppDSCBuffer8,
	LPDIRECTSOUNDBUFFER8 *ppDSBuffer8, LPUNKNOWN pUnkOuter);
HRESULT WINAPI ds_DllGetClassObject(IN REFCLSID rclsid, IN REFIID riid, OUT LPVOID FAR* ppv);
HRESULT WINAPI ds_DllCanUnloadNow();

#define DECLARE_IN_WRAPPED_PROC(procName, unused) \
	const FARPROC procName ## _in = (FARPROC)*ds_ ## procName;

#define EXPORT_OUT_WRAPPED_PROC(procName, unused) \
	extern FARPROC procName ## _out;

namespace DsoundWrapper
{
	VISIT_PROCS_DSOUND(DECLARE_IN_WRAPPED_PROC);
	VISIT_PROCS_DSOUND_SHARED(DECLARE_IN_WRAPPED_PROC);

	VISIT_PROCS_DSOUND(EXPORT_OUT_WRAPPED_PROC);
	VISIT_PROCS_DSOUND_SHARED(EXPORT_OUT_WRAPPED_PROC);
}

#undef DECLARE_IN_WRAPPED_PROC
#undef EXPORT_OUT_WRAPPED_PROC
