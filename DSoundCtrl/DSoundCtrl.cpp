//////////////////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2003-2007, Arne Bockholdt, github@bockholdt.info
//
// This file is part of Direct Sound Control.
//
// Direct Sound Control is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Direct Sound Control is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Direct Sound Control.  If not, see <http://www.gnu.org/licenses/>.
//
//////////////////////////////////////////////////////////////////////////////////////////////
//
// DSoundCtrl.cpp : Defines the initialization routines for the DLL.
//
//////////////////////////////////////////////////////////////////////////////////////////////
//
// Updated by Elisha Riedlinger 2017
//
//////////////////////////////////////////////////////////////////////////////////////////////

#define WIN32_LEAN_AND_MEAN

#ifndef _DEBUG
//#define new DEBUG_NEW		//<----TODO: fix this line
#endif

//////////////////////////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <stdio.h>
#include <atlstr.h>
#include <assert.h>
#include <atltrace.h>
#include "mmsystem.h"
#include "dsound.h"
#include "DSoundTypes.h"
#include "DSoundCtrl.h"
#include "DSoundCtrlExternal.h"
#include "IDirectSoundClassFactoryEx.h"
#include "cfg.h"
#include "dllmain.h"
#include "wrappers\wrapper.h"

//////////////////////////////////////////////////////////////////////////////////////////////

LPDSENUMCALLBACKA	g_pAppDSEnumCallbackA;
LPDSENUMCALLBACKW	g_pAppDSEnumCallbackW;

//////////////////////////////////////////////////////////////////////////////////////////////

const char* g_cszClassName = "ExportFunction";

//////////////////////////////////////////////////////////////////////////////////////////////

void LogMessage(const char* szClassName, void* pInstance, char* szMessage)
{
	char buffer[1024];

	if ( szClassName )
		sprintf_s(buffer, "%s,%u,%s", szClassName, (unsigned long)pInstance, szMessage);
	else
		sprintf_s(buffer, ",,%s", szMessage);

	Compat::Log() << buffer;

#ifdef _DEBUG
	ATLTRACE(buffer);
#endif  // _DEBUG
}

//////////////////////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG

//////////////////////////////////////////////////////////////////////////////////////////////

BOOL CALLBACK DSEnumCallback( LPGUID  lpGuid,    LPCSTR  lpcstrDescription,  LPCSTR  lpcstrModule,   LPVOID  lpContext  )
{
	return ((CDSoundCtrlApp*)lpContext)->EnumCallback( lpGuid, lpcstrDescription, lpcstrModule );
}

//////////////////////////////////////////////////////////////////////////////////////////////

#endif  // _DEBUG

//////////////////////////////////////////////////////////////////////////////////////////////
// CDSoundCtrlApp
//
// The one and only CDSoundCtrlApp object

CDSoundCtrlApp theApp;

//////////////////////////////////////////////////////////////////////////////////////////////
// CDSoundCtrlApp construction

CDSoundCtrlApp::CDSoundCtrlApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
	m_cszClassName = CDSOUNDCTRLAPP_CLASS_NAME;
}

//////////////////////////////////////////////////////////////////////////////////////////////

CDSoundCtrlApp::~CDSoundCtrlApp()
{
#ifdef _DEBUG
	if( g_bLogSystem == true )
		LogMessage( m_cszClassName, this, "Destructor called....");
#endif // _DEBUG

	::FreeLibrary( g_hDLL );

#ifdef _DEBUG
	if( g_bDebugBeep == true )
		Beep( 3000, 100 );

	if( g_bLogSystem == true )
	{
		LogMessage( m_cszClassName, this, "Original DSOUND.DLL unloaded....");

		::LogMessage( (const char*) nullptr, nullptr, "********* DirectSound Control Logging ended *********");
	}

#endif // _DEBUG
}

//////////////////////////////////////////////////////////////////////////////////////////////
// CDSoundCtrlApp initialization

BOOL CDSoundCtrlApp::InitInstance()
{
	// Starting DirectSoundControl
	Compat::Log() << "Enabling DSoundCtrl function";

	//
	// Read configuration from file
	//

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

#ifdef _DEBUG		//<----TODO: update all logging
	::LogMessage((const char*)nullptr, nullptr, "********* DirectSound Control Logging started *********");
#endif //_DEBUG

	if (g_bForceVoiceManagement & (g_bForceSoftwareMixing | g_bForceHardwareMixing))
	{
		g_bForceHardwareMixing = false;
		g_bForceSoftwareMixing = false;

		LogMessage(m_cszClassName, this, "ERROR: Voice Management can't be used together with force of hardware/software mixing ....using Voice Management");

		if (g_bForceSoftwareMixing & g_bForceHardwareMixing)
		{
			g_bForceHardwareMixing = false;

			LogMessage(m_cszClassName, this, "ERROR: Can't force usage of hardware/software mixing together ....using Software Mixing");
		}
	}

	//
	// Load DSOUND.DLL and get entry points
	//

	if (Config.RealWrapperMode == dtype.dsound)
	{
		g_hDLL = LoadDll(dtype.dsound);
	}
	else
	{
		g_hDLL = hModule_dll;
	}

	if ( !g_hDLL )
	{
		LogMessage(m_cszClassName, this, "ERROR: Can't load original dsound.dll...fatal...");

		return FALSE;
	}

	g_pDirectSoundCreate = (DirectSoundCreatefunc)  ::GetProcAddress(g_hDLL, "DirectSoundCreate");
	g_pDirectSoundCreate8 = (DirectSoundCreate8func) ::GetProcAddress(g_hDLL, "DirectSoundCreate8");
	g_pGetDeviceIDfunc = (GetDeviceIDfunc) ::GetProcAddress(g_hDLL, "GetDeviceID");
	g_pDirectSoundEnumerateAfunc = (DirectSoundEnumerateAfunc)  ::GetProcAddress(g_hDLL, "DirectSoundEnumerateA");
	g_pDirectSoundEnumerateWfunc = (DirectSoundEnumerateWfunc) ::GetProcAddress(g_hDLL, "DirectSoundEnumerateW");
	g_pDirectSoundCaptureCreatefunc = (DirectSoundCaptureCreatefunc) ::GetProcAddress(g_hDLL, "DirectSoundCaptureCreate");
	g_pDirectSoundCaptureEnumerateAfunc = (DirectSoundCaptureEnumerateAfunc)  ::GetProcAddress(g_hDLL, "DirectSoundCaptureEnumerateA");
	g_pDirectSoundCaptureEnumerateWfunc = (DirectSoundCaptureEnumerateWfunc)  ::GetProcAddress(g_hDLL, "DirectSoundCaptureEnumerateW");
	g_pDirectSoundCaptureCreate8func = (DirectSoundCaptureCreate8func)  ::GetProcAddress(g_hDLL, "DirectSoundCaptureCreate8");
	g_pDirectSoundFullDuplexCreatefunc = (DirectSoundFullDuplexCreatefunc)  ::GetProcAddress(g_hDLL, "DirectSoundFullDuplexCreate");
	g_pDllGetClassObjectfunc = (DllGetClassObjectfunc) ::GetProcAddress(g_hDLL, "DllGetClassObject");
	g_pDllCanUnloadNowfunc = (DllCanUnloadNowfunc) ::GetProcAddress(g_hDLL, "DllCanUnloadNow");

#ifdef _DEBUG
	if (g_bDebugBeep == true)
		Beep(1000, 100);

	if (g_bLogSystem == true)
	{
		CString sMessage;

		::GetCurrentDirectory(255, sMessage.GetBuffer(255));
		sMessage.ReleaseBuffer();
		sMessage = "Working directory," + sMessage;
		::LogMessage(m_cszClassName, this, sMessage.GetBuffer());

		::LogMessage(m_cszClassName, this, "Beginning device enumeration...");
		g_pDirectSoundEnumerateAfunc(DSEnumCallback, this);
		::LogMessage(m_cszClassName, this, "Device enumeration complete...");
	}
#endif //_DEBUG

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
bool CDSoundCtrlApp::EnumCallback(LPGUID  lpGuid, LPCSTR  lpcstrDescription, LPCSTR  lpcstrModule)
{
	if( lpGuid )
	{
		CString sMessage;
		sMessage.Format("Device,%s/%s", lpcstrDescription, lpcstrModule );
		::LogMessage(  m_cszClassName, this, sMessage.GetBuffer());
	}
	return TRUE;
}
#endif

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT STDMETHODCALLTYPE DirectSoundCreate(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter)
{
#ifdef _DEBUG
	if( g_bLogSystem == true )
		LogMessage( g_cszClassName, nullptr, "DirectSoundCreate called...");
#endif // _DEBUG

	assert( g_pDirectSoundCreate );

#ifdef _DEBUG
	if( g_bDebugBeep == true )
		Beep( 4000, 100 );
#endif _DEBUG

	IDirectSound8Ex* pDSX = new IDirectSound8Ex;

	HRESULT hRes = g_pDirectSoundCreate( pcGuidDevice, (LPDIRECTSOUND*)&( pDSX->m_lpDirectSound8 ) /* ppDS8 */ , pUnkOuter );

	if( hRes != S_OK )
	{
		delete pDSX;
		return hRes;
	}

	*ppDS = (LPDIRECTSOUND) pDSX;
	
	return hRes;
}

//////////////////////////////////////////////////////////////////////////////////////////////

BOOL STDMETHODCALLTYPE DSDLLEnumCallbackA(LPGUID lpGuid, LPCSTR lpcstrDescription, LPCSTR lpcstrModule, LPVOID lpContext)
{
#ifdef _DEBUG
	if( g_bLogSystem == true )
	{
		CString sMessage;
		sMessage.Format("IDirectSoundEnumCallback,%s/%s", lpcstrDescription, lpcstrModule );
		::LogMessage( g_cszClassName, nullptr, sMessage.GetBuffer());
	}
#endif // _DEBUG

	return g_pAppDSEnumCallbackA( lpGuid,   lpcstrDescription,  lpcstrModule,   lpContext );
}

//////////////////////////////////////////////////////////////////////////////////////////////

BOOL STDMETHODCALLTYPE DSDLLEnumCallbackW(LPGUID lpGuid, LPCWSTR lpcstrDescription, LPCWSTR lpcstrModule, LPVOID lpContext)
{
#ifdef _DEBUG
	if (g_bLogSystem == true)
	{
		CString sMessage;
		sMessage.Format("IDirectSoundEnumCallback,%s/%s", lpcstrDescription, lpcstrModule);
		::LogMessage(g_cszClassName, nullptr, sMessage.GetBuffer());
	}
#endif // _DEBUG

	return g_pAppDSEnumCallbackW(lpGuid, lpcstrDescription, lpcstrModule, lpContext);
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT STDMETHODCALLTYPE DirectSoundEnumerateA(LPDSENUMCALLBACKA pDSEnumCallback, LPVOID pContext)
{
#ifdef _DEBUG
	if( g_bLogSystem == true )
		LogMessage( g_cszClassName, nullptr, "DirectSoundEnumerateA called...");
#endif // _DEBUG

	g_pAppDSEnumCallbackA = pDSEnumCallback;

	return g_pDirectSoundEnumerateAfunc( (LPDSENUMCALLBACKA)DSDLLEnumCallbackA, pContext );
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT STDMETHODCALLTYPE DirectSoundEnumerateW(LPDSENUMCALLBACKW pDSEnumCallback, LPVOID pContext)
{
#ifdef _DEBUG
	if( g_bLogSystem == true )
		LogMessage( g_cszClassName, nullptr, "DirectSoundEnumerateW called...");
#endif // _DEBUG

	g_pAppDSEnumCallbackW = pDSEnumCallback;

	return g_pDirectSoundEnumerateWfunc( (LPDSENUMCALLBACKW)DSDLLEnumCallbackW, pContext );
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT STDMETHODCALLTYPE DirectSoundCaptureCreate(LPCGUID pcGuidDevice, LPDIRECTSOUNDCAPTURE *ppDSC, LPUNKNOWN pUnkOuter)
{
#ifdef _DEBUG
	if( g_bLogSystem == true )
		LogMessage( g_cszClassName, nullptr, "DirectSoundCaptureCreate called...");
#endif // _DEBUG

	return g_pDirectSoundCaptureCreatefunc( pcGuidDevice, ppDSC, pUnkOuter );
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT STDMETHODCALLTYPE DirectSoundCaptureEnumerateA(LPDSENUMCALLBACKA pDSEnumCallback, LPVOID pContext)
{
#ifdef _DEBUG
	if( g_bLogSystem == true )
		LogMessage( g_cszClassName, nullptr, "DirectSoundCaptureEnumerateA called...");
#endif // _DEBUG

	return g_pDirectSoundCaptureEnumerateAfunc( pDSEnumCallback, pContext );
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT STDMETHODCALLTYPE DirectSoundCaptureEnumerateW(LPDSENUMCALLBACKW pDSEnumCallback, LPVOID pContext)
{
#ifdef _DEBUG
	if( g_bLogSystem == true )
		LogMessage( g_cszClassName, nullptr, "DirectSoundCaptureEnumerateW called...");
#endif // _DEBUG

	return g_pDirectSoundCaptureEnumerateWfunc( pDSEnumCallback, pContext );
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT STDMETHODCALLTYPE DirectSoundCreate8(LPCGUID pcGuidDevice, LPDIRECTSOUND8 *ppDS8, LPUNKNOWN pUnkOuter)
{
#ifdef _DEBUG
	if( g_bLogSystem == true )
		LogMessage( g_cszClassName, nullptr, "DirectSoundCreate8 called...");

	if( g_bDebugBeep == true )
		Beep(2000, 100);
#endif // _DEBUG

	assert( g_pDirectSoundCreate8 );

	IDirectSound8Ex* pDSX = new IDirectSound8Ex;

	HRESULT hRes = g_pDirectSoundCreate8( pcGuidDevice, &( pDSX->m_lpDirectSound8 ) /* ppDS8 */ , pUnkOuter );

	if( hRes != S_OK )
	{
		delete pDSX;
		return hRes;
	}

	*ppDS8 = (LPDIRECTSOUND8) pDSX;
	
	return hRes;
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT STDMETHODCALLTYPE DirectSoundCaptureCreate8(LPCGUID pcGuidDevice, LPDIRECTSOUNDCAPTURE8 *ppDSC8, LPUNKNOWN pUnkOuter)
{
#ifdef _DEBUG
	if( g_bLogSystem == true )
		LogMessage( g_cszClassName, nullptr, "DirectSoundCaptureCreate8 called...");
#endif // _DEBUG

	return g_pDirectSoundCaptureCreate8func( pcGuidDevice, ppDSC8, pUnkOuter );
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT STDMETHODCALLTYPE DirectSoundFullDuplexCreate(LPCGUID pcGuidCaptureDevice, LPCGUID pcGuidRenderDevice,
        LPCDSCBUFFERDESC pcDSCBufferDesc, LPCDSBUFFERDESC pcDSBufferDesc, HWND hWnd,
        DWORD dwLevel, LPDIRECTSOUNDFULLDUPLEX* ppDSFD, LPDIRECTSOUNDCAPTUREBUFFER8 *ppDSCBuffer8,
        LPDIRECTSOUNDBUFFER8 *ppDSBuffer8, LPUNKNOWN pUnkOuter)
{
#ifdef _DEBUG
	if( g_bLogSystem == true )
		LogMessage( g_cszClassName, nullptr, "DirectSoundFullDuplexCreate called...");
#endif // _DEBUG

	return g_pDirectSoundFullDuplexCreatefunc( pcGuidCaptureDevice, pcGuidRenderDevice,
        pcDSCBufferDesc, pcDSBufferDesc, hWnd,
        dwLevel, ppDSFD, ppDSCBuffer8,
        ppDSBuffer8, pUnkOuter);
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT STDMETHODCALLTYPE GetDeviceID(LPCGUID pGuidSrc, LPGUID pGuidDest)
{
#ifdef _DEBUG
	if( g_bLogSystem == true )
		LogMessage( g_cszClassName, nullptr, "GetDeviceID called...");
#endif // _DEBUG

	return g_pGetDeviceIDfunc( pGuidSrc, pGuidDest );
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDAPI STDMETHODCALLTYPE DllCanUnloadNow_DSoundCtrl(void)
{
#ifdef _DEBUG
	if( g_bLogSystem == true )
		LogMessage( g_cszClassName, nullptr, "DllCanUnloadNow called...");
#endif // _DEBUG

	return g_pDllCanUnloadNowfunc();
}
//////////////////////////////////////////////////////////////////////////////////////////////

STDAPI STDMETHODCALLTYPE DllGetClassObject_DSoundCtrl(IN REFCLSID rclsid, IN REFIID riid, OUT LPVOID FAR* ppv)
{
	*ppv = nullptr;

	if (( rclsid != CLSID_DirectSound ) && ( rclsid != CLSID_DirectSound8 ))
	{
#ifdef _DEBUG
	if( g_bLogSystem == true )
		LogMessage( g_cszClassName, nullptr, "DllGetClassObject with unknown CLSID called...");
#endif // _DEBUG

		return g_pDllGetClassObjectfunc( rclsid, riid, ppv );
	}

	if( riid == IID_IClassFactory )
	{
		IDirectSoundClassFactoryEx* pDSCFX = new IDirectSoundClassFactoryEx;

		HRESULT hRes = g_pDllGetClassObjectfunc( rclsid, riid, (LPVOID*) &(pDSCFX->m_lpClassFactory) );

		if( hRes )
		{
#ifdef _DEBUG
			if( g_bLogSystem == true )
				LogMessage( g_cszClassName, nullptr, "DllGetClassObject for IID_IClassFactory failed...");
#endif // _DEBUG

			delete pDSCFX;
			return hRes;
		}
		
		*ppv = (LPVOID*) pDSCFX;

#ifdef _DEBUG
		if( g_bLogSystem == true )
			LogMessage( g_cszClassName, nullptr, "DllGetClassObject for IID_IClassFactory called...");
#endif // _DEBUG

		return S_OK;
	}

#ifdef _DEBUG
	if( g_bLogSystem == true )
		LogMessage( g_cszClassName, nullptr, "DllGetClassObject for unknown IID interface called...");
#endif // _DEBUG

	return g_pDllGetClassObjectfunc( rclsid, riid, ppv );
}

//////////////////////////////////////////////////////////////////////////////////////////////

void RunDSoundCtrl()
{
	theApp.InitInstance();
}
