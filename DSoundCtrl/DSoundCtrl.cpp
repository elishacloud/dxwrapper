/**
* Copyright (c) 2003-2007, Arne Bockholdt, github@bockholdt.info
*
* This file is part of Direct Sound Control.
*
* Direct Sound Control is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* Direct Sound Control is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with Direct Sound Control.  If not, see <http://www.gnu.org/licenses/>.
*
* DSoundCtrl.cpp : Defines the initialization routines for the DLL.
*
* Updated 2017 by Elisha Riedlinger
*/

#include "Settings\Settings.h"
#include <assert.h>
#ifdef _DEBUG
#include <atlstr.h>
#endif //_DEBUG
#include "dsound.h"
#include "DSoundTypes.h"
#include "DSoundCtrl.h"
#include "IDirectSoundClassFactoryEx.h"
#include "DSoundCtrlExternal.h"
#include "Logging\Logging.h"

#define DllMain DllMain_DSoundCtrl

LPDSENUMCALLBACKA	g_pAppDSEnumCallbackA;
LPDSENUMCALLBACKW	g_pAppDSEnumCallbackW;

const char* g_cszClassName = "ExportFunction";

#define INITIALIZE_WRAPPED_PROC(procName, unused) \
	FARPROC procName ## _out = nullptr;

namespace DsoundWrapper
{
	VISIT_PROCS_DSOUND(INITIALIZE_WRAPPED_PROC);
	FARPROC DllGetClassObject_out = nullptr;
	FARPROC DllCanUnloadNow_out = nullptr;
}

using namespace DsoundWrapper;

void LogMessage(const char* szClassName, void* pInstance, char* szMessage)
{
	char buffer[1024];

	if (szClassName)
		sprintf_s(buffer, "%s,%u,%s", szClassName, (unsigned long)pInstance, szMessage);
	else
		sprintf_s(buffer, ",,%s", szMessage);

	Logging::Log() << buffer;

#ifdef _DEBUG
	ATLTRACE(buffer);
#endif  // _DEBUG
}

#ifdef _DEBUG
BOOL CALLBACK DSEnumCallback(LPGUID  lpGuid, LPCSTR  lpcstrDescription, LPCSTR  lpcstrModule, LPVOID  lpContext)
{
	return ((CDSoundCtrlApp*)lpContext)->EnumCallback(lpGuid, lpcstrDescription, lpcstrModule);
}
#endif  // _DEBUG

CDSoundCtrlApp::CDSoundCtrlApp()
{
	m_cszClassName = CDSOUNDCTRLAPP_CLASS_NAME;
}

CDSoundCtrlApp::~CDSoundCtrlApp()
{
#ifdef _DEBUG
	if (g_bLogSystem)
	{
		LogMessage(m_cszClassName, this, "Destructor called....");
	}

	if (g_bDebugBeep)
	{
		Beep(3000, 100);
	}

	if (g_bLogSystem)
	{
		LogMessage(m_cszClassName, this, "Original DSOUND.DLL unloaded....");

		::LogMessage((const char*) nullptr, nullptr, "********* DirectSound Control Logging ended *********");
	}
#endif // _DEBUG
}

BOOL CDSoundCtrlApp::InitInstance()
{
	// Starting DirectSoundControl
	Logging::Log() << "Enabling DSoundCtrl function";

	// Get configuration settings
	g_bLogSystem = true;
	g_bLogDirectSound = true;
	g_bLogDirectSoundBuffer = true;
	g_bLogDirectSound3DBuffer = true;
	g_bLogDirectSound3DListener = true;
	g_bDebugBeep = true;

	g_nNum2DBuffers = Config.Num2DBuffers;
	g_nNum3DBuffers = Config.Num3DBuffers;
	g_bForceCertification = Config.ForceCertification;
	g_bForceExclusiveMode = Config.ForceExclusiveMode;
	g_bForceSoftwareMixing = Config.ForceSoftwareMixing;
	g_bForceHardwareMixing = Config.ForceHardwareMixing;
	g_bPreventSpeakerSetup = Config.PreventSpeakerSetup;
	g_bForceHQSoftware3D = Config.ForceHQ3DSoftMixing;
	g_bForceNonStatic = Config.ForceNonStaticBuffers;
	g_bForceVoiceManagement = Config.ForceVoiceManagement;
	g_bForcePrimaryBufferFormat = Config.ForcePrimaryBufferFormat;
	g_nPrimaryBufferBits = Config.PrimaryBufferBits;
	g_nPrimaryBufferSamples = Config.PrimaryBufferSamples;
	g_nPrimaryBufferChannels = Config.PrimaryBufferChannels;

	g_bForceSpeakerConfig = Config.ForceSpeakerConfig;
	g_nSpeakerConfig = Config.SpeakerConfig;

	g_bStoppedDriverWorkaround = Config.StoppedDriverWorkaround;

#ifdef _DEBUG
	if (g_bDebugBeep)
	{
		Beep(1000, 100);
	}

	if (g_bLogSystem)
	{
		CString sMessage;

		::GetCurrentDirectory(255, sMessage.GetBuffer(255));
		sMessage.ReleaseBuffer();
		sMessage = "Working directory," + sMessage;
		::LogMessage(m_cszClassName, this, sMessage.GetBuffer());

		::LogMessage(m_cszClassName, this, "Beginning device enumeration...");
		((DirectSoundEnumerateAProc)DirectSoundEnumerateA_out)(DSEnumCallback, this);
		::LogMessage(m_cszClassName, this, "Device enumeration complete...");
	}
#endif //_DEBUG

	return TRUE;
}

#ifdef _DEBUG
bool CDSoundCtrlApp::EnumCallback(LPGUID  lpGuid, LPCSTR  lpcstrDescription, LPCSTR  lpcstrModule)
{
	if (lpGuid)
	{
		CString sMessage;
		sMessage.Format("Device,%s/%s", lpcstrDescription, lpcstrModule);
		::LogMessage(m_cszClassName, this, sMessage.GetBuffer());
	}
	return TRUE;
}
#endif

HRESULT WINAPI ds_DirectSoundCreate(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter)
{
#ifdef _DEBUG
	if (g_bLogSystem)
	{
		LogMessage(g_cszClassName, nullptr, "DirectSoundCreate called...");
	}
#endif // _DEBUG

	if (!Wrapper::ValidProcAddress(DirectSoundCreate_out))
	{
		return E_FAIL;
	}

	assert(((DirectSoundCreateProc)DirectSoundCreate_out));

#ifdef _DEBUG
	if (g_bDebugBeep)
	{
		Beep(4000, 100);
	}
#endif // _DEBUG

	IDirectSound8Ex* pDSX = new IDirectSound8Ex;

	HRESULT hRes = ((DirectSoundCreateProc)DirectSoundCreate_out)(pcGuidDevice, (LPDIRECTSOUND*)&(pDSX->m_lpDirectSound8) /* ppDS8 */, pUnkOuter);

	if (hRes != S_OK)
	{
		delete pDSX;
		return hRes;
	}

	*ppDS = (LPDIRECTSOUND)pDSX;

	return hRes;
}

BOOL WINAPI ds_DSDLLEnumCallbackA(LPGUID lpGuid, LPCSTR lpcstrDescription, LPCSTR lpcstrModule, LPVOID lpContext)
{
#ifdef _DEBUG
	if (g_bLogSystem)
	{
		CString sMessage;
		sMessage.Format("IDirectSoundEnumCallback,%s/%s", lpcstrDescription, lpcstrModule);
		::LogMessage(g_cszClassName, nullptr, sMessage.GetBuffer());
	}
#endif // _DEBUG

	if (!Wrapper::ValidProcAddress((FARPROC)g_pAppDSEnumCallbackA))
	{
		return FALSE;
	}

	return g_pAppDSEnumCallbackA(lpGuid, lpcstrDescription, lpcstrModule, lpContext);
}

BOOL WINAPI ds_DSDLLEnumCallbackW(LPGUID lpGuid, LPCWSTR lpcstrDescription, LPCWSTR lpcstrModule, LPVOID lpContext)
{
#ifdef _DEBUG
	if (g_bLogSystem)
	{
		CString sMessage;
		sMessage.Format("IDirectSoundEnumCallback,%s/%s", lpcstrDescription, lpcstrModule);
		::LogMessage(g_cszClassName, nullptr, sMessage.GetBuffer());
	}
#endif // _DEBUG

	if (!Wrapper::ValidProcAddress((FARPROC)g_pAppDSEnumCallbackW))
	{
		return FALSE;
	}

	return g_pAppDSEnumCallbackW(lpGuid, lpcstrDescription, lpcstrModule, lpContext);
}

HRESULT WINAPI ds_DirectSoundEnumerateA(LPDSENUMCALLBACKA pDSEnumCallback, LPVOID pContext)
{
#ifdef _DEBUG
	if (g_bLogSystem)
	{
		LogMessage(g_cszClassName, nullptr, "DirectSoundEnumerateA called...");
	}
#endif // _DEBUG

	if (!Wrapper::ValidProcAddress(DirectSoundEnumerateA_out))
	{
		return E_FAIL;
	}

	g_pAppDSEnumCallbackA = pDSEnumCallback;

	return ((DirectSoundEnumerateAProc)DirectSoundEnumerateA_out)((LPDSENUMCALLBACKA)ds_DSDLLEnumCallbackA, pContext);
}

HRESULT WINAPI ds_DirectSoundEnumerateW(LPDSENUMCALLBACKW pDSEnumCallback, LPVOID pContext)
{
#ifdef _DEBUG
	if (g_bLogSystem)
	{
		LogMessage(g_cszClassName, nullptr, "DirectSoundEnumerateW called...");
	}
#endif // _DEBUG

	if (!Wrapper::ValidProcAddress(DirectSoundEnumerateW_out))
	{
		return E_FAIL;
	}

	g_pAppDSEnumCallbackW = pDSEnumCallback;

	return ((DirectSoundEnumerateWProc)DirectSoundEnumerateW_out)((LPDSENUMCALLBACKW)ds_DSDLLEnumCallbackW, pContext);
}

HRESULT WINAPI ds_DirectSoundCaptureCreate(LPCGUID pcGuidDevice, LPDIRECTSOUNDCAPTURE *ppDSC, LPUNKNOWN pUnkOuter)
{
#ifdef _DEBUG
	if (g_bLogSystem)
	{
		LogMessage(g_cszClassName, nullptr, "DirectSoundCaptureCreate called...");
	}
#endif // _DEBUG

	if (!Wrapper::ValidProcAddress(DirectSoundCaptureCreate_out))
	{
		return E_FAIL;
	}

	return ((DirectSoundCaptureCreateProc)DirectSoundCaptureCreate_out)(pcGuidDevice, ppDSC, pUnkOuter);
}

HRESULT WINAPI ds_DirectSoundCaptureEnumerateA(LPDSENUMCALLBACKA pDSEnumCallback, LPVOID pContext)
{
#ifdef _DEBUG
	if (g_bLogSystem)
	{
		LogMessage(g_cszClassName, nullptr, "DirectSoundCaptureEnumerateA called...");
	}
#endif // _DEBUG

	if (!Wrapper::ValidProcAddress(DirectSoundCaptureEnumerateA_out))
	{
		return E_FAIL;
	}

	return ((DirectSoundCaptureEnumerateAProc)DirectSoundCaptureEnumerateA_out)(pDSEnumCallback, pContext);
}

HRESULT WINAPI ds_DirectSoundCaptureEnumerateW(LPDSENUMCALLBACKW pDSEnumCallback, LPVOID pContext)
{
#ifdef _DEBUG
	if (g_bLogSystem)
	{
		LogMessage(g_cszClassName, nullptr, "DirectSoundCaptureEnumerateW called...");
	}
#endif // _DEBUG

	if (!Wrapper::ValidProcAddress(DirectSoundCaptureEnumerateW_out))
	{
		return E_FAIL;
	}

	return ((DirectSoundCaptureEnumerateWProc)DirectSoundCaptureEnumerateW_out)(pDSEnumCallback, pContext);
}

HRESULT WINAPI ds_DirectSoundCreate8(LPCGUID pcGuidDevice, LPDIRECTSOUND8 *ppDS8, LPUNKNOWN pUnkOuter)
{
#ifdef _DEBUG
	if (g_bLogSystem)
	{
		LogMessage(g_cszClassName, nullptr, "DirectSoundCreate8 called...");
	}

	if (g_bDebugBeep)
	{
		Beep(2000, 100);
	}
#endif // _DEBUG

	if (!Wrapper::ValidProcAddress(DirectSoundCreate8_out))
	{
		return E_FAIL;
	}

	assert(((DirectSoundCreate8Proc)DirectSoundCreate8_out));

	IDirectSound8Ex* pDSX = new IDirectSound8Ex;

	HRESULT hRes = ((DirectSoundCreate8Proc)DirectSoundCreate8_out)(pcGuidDevice, &(pDSX->m_lpDirectSound8) /* ppDS8 */, pUnkOuter);

	if (hRes != S_OK)
	{
		delete pDSX;
		return hRes;
	}

	*ppDS8 = (LPDIRECTSOUND8)pDSX;

	return hRes;
}

HRESULT WINAPI ds_DirectSoundCaptureCreate8(LPCGUID pcGuidDevice, LPDIRECTSOUNDCAPTURE8 *ppDSC8, LPUNKNOWN pUnkOuter)
{
#ifdef _DEBUG
	if (g_bLogSystem)
	{
		LogMessage(g_cszClassName, nullptr, "DirectSoundCaptureCreate8 called...");
	}
#endif // _DEBUG

	if (!Wrapper::ValidProcAddress(DirectSoundCaptureCreate8_out))
	{
		return E_FAIL;
	}

	return ((DirectSoundCaptureCreate8Proc)DirectSoundCaptureCreate8_out)(pcGuidDevice, ppDSC8, pUnkOuter);
}

HRESULT WINAPI ds_DirectSoundFullDuplexCreate(LPCGUID pcGuidCaptureDevice, LPCGUID pcGuidRenderDevice,
	LPCDSCBUFFERDESC pcDSCBufferDesc, LPCDSBUFFERDESC pcDSBufferDesc, HWND hWnd,
	DWORD dwLevel, LPDIRECTSOUNDFULLDUPLEX* ppDSFD, LPDIRECTSOUNDCAPTUREBUFFER8 *ppDSCBuffer8,
	LPDIRECTSOUNDBUFFER8 *ppDSBuffer8, LPUNKNOWN pUnkOuter)
{
#ifdef _DEBUG
	if (g_bLogSystem)
	{
		LogMessage(g_cszClassName, nullptr, "DirectSoundFullDuplexCreate called...");
	}
#endif // _DEBUG

	if (!Wrapper::ValidProcAddress(DirectSoundFullDuplexCreate_out))
	{
		return E_FAIL;
	}

	return ((DirectSoundFullDuplexCreateProc)DirectSoundFullDuplexCreate_out)(pcGuidCaptureDevice, pcGuidRenderDevice,
		pcDSCBufferDesc, pcDSBufferDesc, hWnd,
		dwLevel, ppDSFD, ppDSCBuffer8,
		ppDSBuffer8, pUnkOuter);
}

HRESULT WINAPI ds_GetDeviceID(LPCGUID pGuidSrc, LPGUID pGuidDest)
{
#ifdef _DEBUG
	if (g_bLogSystem)
	{
		LogMessage(g_cszClassName, nullptr, "GetDeviceID called...");
	}
#endif // _DEBUG

	if (!Wrapper::ValidProcAddress(GetDeviceID_out))
	{
		return E_FAIL;
	}

	return ((GetDeviceIDProc)GetDeviceID_out)(pGuidSrc, pGuidDest);
}

HRESULT WINAPI ds_DllCanUnloadNow()
{
#ifdef _DEBUG
	if (g_bLogSystem)
	{
		LogMessage(g_cszClassName, nullptr, "DllCanUnloadNow called...");
	}
#endif // _DEBUG

	if (!Wrapper::ValidProcAddress(DllCanUnloadNow_out))
	{
		return E_FAIL;
	}

	return ((DllCanUnloadNowProc)DllCanUnloadNow_out)();
}

HRESULT WINAPI ds_DllGetClassObject(IN REFCLSID rclsid, IN REFIID riid, OUT LPVOID FAR* ppv)
{
	if (!Wrapper::ValidProcAddress(DllGetClassObject_out))
	{
		return E_FAIL;
	}

	*ppv = nullptr;

	if ((rclsid != CLSID_DirectSound) && (rclsid != CLSID_DirectSound8))
	{
#ifdef _DEBUG
		if (g_bLogSystem)
		{
			LogMessage(g_cszClassName, nullptr, "DllGetClassObject with unknown CLSID called...");
		}
#endif // _DEBUG

		return ((DllGetClassObjectProc)DllGetClassObject_out)(rclsid, riid, ppv);
	}

	if (riid == IID_IClassFactory)
	{
		IDirectSoundClassFactoryEx* pDSCFX = new IDirectSoundClassFactoryEx;

		HRESULT hRes = ((DllGetClassObjectProc)DllGetClassObject_out)(rclsid, riid, (LPVOID*) &(pDSCFX->m_lpClassFactory));

		if (SUCCEEDED(hRes))
		{
#ifdef _DEBUG
			if (g_bLogSystem)
			{
				LogMessage(g_cszClassName, nullptr, "DllGetClassObject for IID_IClassFactory failed...");
			}
#endif // _DEBUG

			delete pDSCFX;
			return hRes;
		}

		*ppv = (LPVOID*)pDSCFX;

#ifdef _DEBUG
		if (g_bLogSystem)
		{
			LogMessage(g_cszClassName, nullptr, "DllGetClassObject for IID_IClassFactory called...");
		}
#endif // _DEBUG

		return S_OK;
	}

#ifdef _DEBUG
	if (g_bLogSystem)
	{
		LogMessage(g_cszClassName, nullptr, "DllGetClassObject for unknown IID interface called...");
	}
#endif // _DEBUG

	return ((DllGetClassObjectProc)DllGetClassObject_out)(rclsid, riid, ppv);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved)
{
	UNREFERENCED_PARAMETER(hModule);
	UNREFERENCED_PARAMETER(lpReserved);

	static CDSoundCtrlApp *IDSoundCtrl = nullptr;

	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		Logging::Log() << "Loading DSoundCtrl";
		IDSoundCtrl = new CDSoundCtrlApp;
		IDSoundCtrl->InitInstance();
		break;
	case DLL_PROCESS_DETACH:
		if (IDSoundCtrl)
		{
			Logging::Log() << "Unloading DSoundCtrl";
			delete IDSoundCtrl;
		}
		break;
	}

	return TRUE;
}
