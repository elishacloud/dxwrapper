#pragma once

#include "DSoundCtrl\dsound.h"

BOOL APIENTRY DllMain_DSoundCtrl(HMODULE, DWORD, LPVOID);
HRESULT WINAPI ds_DirectSoundCreate(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter);
HRESULT WINAPI ds_DirectSoundCreate8(LPCGUID pcGuidDevice, LPDIRECTSOUND8 *ppDS8, LPUNKNOWN pUnkOuter);
HRESULT WINAPI ds_GetDeviceID(LPCGUID pGuidSrc, LPGUID pGuidDest);
HRESULT WINAPI ds_DirectSoundEnumerateA(LPDSENUMCALLBACKA pDSEnumCallback, LPVOID pContext);
HRESULT WINAPI ds_DirectSoundEnumerateW(LPDSENUMCALLBACKW pDSEnumCallback, LPVOID pContext);
HRESULT WINAPI ds_DirectSoundCaptureEnumerateA(LPDSENUMCALLBACKA pDSEnumCallback, LPVOID pContext);
HRESULT WINAPI ds_DirectSoundCaptureEnumerateW(LPDSENUMCALLBACKW pDSEnumCallback, LPVOID pContext);
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

namespace DSoundCtrl
{
	constexpr FARPROC DirectSoundCreate_in = (FARPROC)*ds_DirectSoundCreate;
	constexpr FARPROC DirectSoundCreate8_in = (FARPROC)*ds_DirectSoundCreate8;
	constexpr FARPROC GetDeviceID_in = (FARPROC)*ds_GetDeviceID;
	constexpr FARPROC DirectSoundEnumerateA_in = (FARPROC)*ds_DirectSoundEnumerateA;
	constexpr FARPROC DirectSoundEnumerateW_in = (FARPROC)*ds_DirectSoundEnumerateW;
	constexpr FARPROC DirectSoundCaptureCreate_in = (FARPROC)*ds_DirectSoundCaptureCreate;
	constexpr FARPROC DirectSoundCaptureEnumerateA_in = (FARPROC)*ds_DirectSoundCaptureEnumerateA;
	constexpr FARPROC DirectSoundCaptureEnumerateW_in = (FARPROC)*ds_DirectSoundCaptureEnumerateW;
	constexpr FARPROC DirectSoundCaptureCreate8_in = (FARPROC)*ds_DirectSoundCaptureCreate8;
	constexpr FARPROC DirectSoundFullDuplexCreate_in = (FARPROC)*ds_DirectSoundFullDuplexCreate;
	constexpr FARPROC DllGetClassObject_in = (FARPROC)*ds_DllGetClassObject;
	constexpr FARPROC DllCanUnloadNow_in = (FARPROC)*ds_DllCanUnloadNow;
	extern FARPROC DirectSoundCreate_out;
	extern FARPROC DirectSoundCreate8_out;
	extern FARPROC GetDeviceID_out;
	extern FARPROC DirectSoundEnumerateA_out;
	extern FARPROC DirectSoundEnumerateW_out;
	extern FARPROC DirectSoundCaptureCreate_out;
	extern FARPROC DirectSoundCaptureEnumerateA_out;
	extern FARPROC DirectSoundCaptureEnumerateW_out;
	extern FARPROC DirectSoundCaptureCreate8_out;
	extern FARPROC DirectSoundFullDuplexCreate_out;
	extern FARPROC DllGetClassObject_out;
	extern FARPROC DllCanUnloadNow_out;
}
