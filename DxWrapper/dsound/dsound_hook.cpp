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

#include <windows.h>
#include <dsound.h>
#include "dgame.h"

struct dsound_dll_hook
{
	HMODULE dll;
	FARPROC DirectSoundCaptureCreate;
	FARPROC DirectSoundCaptureCreate8;
	FARPROC DirectSoundCaptureEnumerateA;
	FARPROC DirectSoundCaptureEnumerateW;
	FARPROC DirectSoundCreate;
	FARPROC DirectSoundCreate8;
	FARPROC DirectSoundEnumerateA;
	FARPROC DirectSoundEnumerateW;
	FARPROC DirectSoundFullDuplexCreate;
	FARPROC DllCanUnloadNow_dsound;
	FARPROC DllGetClassObject_dsound;
	FARPROC GetDeviceID;
} dsound;

typedef HRESULT(*fn_DirectSoundCaptureCreate)(LPGUID lpGUID, LPDIRECTSOUNDCAPTURE *lplpDSC, LPUNKNOWN pUnkOuter);
void _DirectSoundCaptureCreate()
{
	(fn_DirectSoundCaptureCreate)dsound.DirectSoundCaptureCreate();
}

typedef HRESULT(*fn_DirectSoundCaptureCreate8)(LPCGUID lpcGUID, LPDIRECTSOUNDCAPTURE8 * lplpDSC, LPUNKNOWN pUnkOuter);
void _DirectSoundCaptureCreate8()
{
	(fn_DirectSoundCaptureCreate8)dsound.DirectSoundCaptureCreate8();
}

typedef HRESULT(*fn_DirectSoundCaptureEnumerateA)(LPDSENUMCALLBACKA lpDSEnumCallback, LPVOID lpContext);
void _DirectSoundCaptureEnumerateA()
{
	(fn_DirectSoundCaptureEnumerateA)dsound.DirectSoundCaptureEnumerateA();
}

typedef HRESULT(*fn_DirectSoundCaptureEnumerateW)(LPDSENUMCALLBACKW lpDSEnumCallback, LPVOID lpContext);
void _DirectSoundCaptureEnumerateW()
{
	(fn_DirectSoundCaptureEnumerateW)dsound.DirectSoundCaptureEnumerateW();
}

typedef HRESULT(*fn_DirectSoundCreate)(LPCGUID lpcGUID, LPDIRECTSOUND* ppDS, IUnknown* pUnkOuter);
void _DirectSoundCreate()
{
	(fn_DirectSoundCreate)dsound.DirectSoundCreate();
}

typedef HRESULT(*fn_DirectSoundCreate8)(LPCGUID lpcGUID, LPDIRECTSOUND8* ppDS, IUnknown* pUnkOuter);
void _DirectSoundCreate8()
{
	(fn_DirectSoundCreate8)dsound.DirectSoundCreate8();
}

typedef HRESULT(*fn_DirectSoundEnumerateA)(LPDSENUMCALLBACKA lpDSEnumCallback, LPVOID lpContext);
void _DirectSoundEnumerateA()
{
	(fn_DirectSoundEnumerateA)dsound.DirectSoundEnumerateA();
}

typedef HRESULT(*fn_DirectSoundEnumerateW)(LPDSENUMCALLBACKW lpDSEnumCallback, LPVOID lpContext);
void _DirectSoundEnumerateW()
{
	(fn_DirectSoundEnumerateW)dsound.DirectSoundEnumerateW();
}

typedef HRESULT(*fn_DirectSoundFullDuplexCreate)(const GUID* capture_dev, const GUID* render_dev, const DSCBUFFERDESC* cbufdesc, const DSBUFFERDESC* bufdesc, HWND  hwnd, DWORD level, IDirectSoundFullDuplex**  dsfd, IDirectSoundCaptureBuffer8** dscb8, IDirectSoundBuffer8** dsb8, IUnknown* outer_unk);
void _DirectSoundFullDuplexCreate()
{
	(fn_DirectSoundFullDuplexCreate)dsound.DirectSoundFullDuplexCreate();
}

typedef HRESULT(*fn_DllCanUnloadNow_dsound)();
void _DllCanUnloadNow_dsound()
{
	(fn_DllCanUnloadNow_dsound)dsound.DllCanUnloadNow_dsound();
}

typedef HRESULT(*fn_DllGetClassObject_dsound)(REFCLSID rclsid, REFIID riid, LPVOID *ppv);
void _DllGetClassObject_dsound()
{
	(fn_DllGetClassObject_dsound)dsound.DllGetClassObject_dsound();
}

typedef HRESULT(*fn_GetDeviceID)(LPCGUID pGuidSrc, LPGUID pGuidDest);
void _GetDeviceID()
{
	(fn_GetDeviceID)dsound.GetDeviceID();
}

HMODULE LoadDsoundHook()
{
	//Load real dll
	dsound.dll = LoadDll("dsound.dll", dtype.dsound);
	//Load dll functions
	if (dsound.dll)
	{
		dsound.DirectSoundCaptureCreate			= GetProcAddress(dsound.dll, "DirectSoundCaptureCreate");
		dsound.DirectSoundCaptureCreate8		= GetProcAddress(dsound.dll, "DirectSoundCaptureCreate8");
		dsound.DirectSoundCaptureEnumerateA		= GetProcAddress(dsound.dll, "DirectSoundCaptureEnumerateA");
		dsound.DirectSoundCaptureEnumerateW		= GetProcAddress(dsound.dll, "DirectSoundCaptureEnumerateW");
		dsound.DirectSoundCreate				= GetProcAddress(dsound.dll, "DirectSoundCreate");
		dsound.DirectSoundCreate8				= GetProcAddress(dsound.dll, "DirectSoundCreate8");
		dsound.DirectSoundEnumerateA			= GetProcAddress(dsound.dll, "DirectSoundEnumerateA");
		dsound.DirectSoundEnumerateW			= GetProcAddress(dsound.dll, "DirectSoundEnumerateW");
		dsound.DirectSoundFullDuplexCreate		= GetProcAddress(dsound.dll, "DirectSoundFullDuplexCreate");
		dsound.DllCanUnloadNow_dsound			= GetProcAddress(dsound.dll, "DllCanUnloadNow");
		dsound.DllGetClassObject_dsound			= GetProcAddress(dsound.dll, "DllGetClassObject");
		dsound.GetDeviceID						= GetProcAddress(dsound.dll, "GetDeviceID");
		return dsound.dll;
	}
	else
	{
		return NULL;
	}
}