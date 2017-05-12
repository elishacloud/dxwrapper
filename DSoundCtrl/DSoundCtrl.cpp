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

//////////////////////////////////////////////////////////////////////////////////////////////

LPDSENUMCALLBACKA	g_pAppDSEnumCallbackA;
LPDSENUMCALLBACKW	g_pAppDSEnumCallbackW;

//////////////////////////////////////////////////////////////////////////////////////////////

const char* g_cszClassName = "ExportFunction";

//////////////////////////////////////////////////////////////////////////////////////////////

void LogMessage(const char* szClassName, void* pInstance, char* szMessage)
{
	char buffer[1024];

	if (szClassName != NULL)
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

CString GetDirectXVersionFromReg()
{
	HKEY			RegKey;
    unsigned long	iSize = 255;
	DWORD			dwType;
	CString			sTemp = "";

	if(::RegOpenKeyEx(HKEY_LOCAL_MACHINE,"Software\\Microsoft\\DirectX",0,KEY_ALL_ACCESS,&RegKey) == ERROR_SUCCESS)
	{
		RegQueryValueEx(RegKey, "Version",NULL,&dwType,(BYTE*)  sTemp.GetBuffer(255), &iSize);

		sTemp.ReleaseBuffer();
		RegCloseKey(RegKey);
	}

	return sTemp;
}

//////////////////////////////////////////////////////////////////////////////////////////////

CString GetOSVersion()		//<----TODO: update this function
{
	CString			sTemp = "";
	CString			sOSName = "Unknown platform";

	OSVERSIONINFO oOS_version;
	oOS_version.dwOSVersionInfoSize = sizeof(OSVERSIONINFO); 
	::GetVersionEx( &oOS_version );

	switch( oOS_version.dwPlatformId )
	{
		case VER_PLATFORM_WIN32_WINDOWS : 
			{
				switch(oOS_version.dwMinorVersion)
				{
					case 0   : sOSName = "Windows 95"; break;
					case 10  : sOSName = "Windows 98"; break;
					case 90  : sOSName = "Windows ME"; break;
					default  : sOSName = "Unknown Windows 9x-based version";
				}					
				break;
			}
			break;
		case VER_PLATFORM_WIN32_NT :
			{
				switch( oOS_version.dwMajorVersion )
				{
					case 3  : sOSName = "Windows NT 3.x (not supported)"; break;
					case 4  : sOSName = "Windows NT 4 (not supported)"; break;
					case 5  :
						switch( oOS_version.dwMinorVersion )
						{
							case 0 : sOSName	= "Windows 2000"; break;
							case 1 : sOSName	= "Windows XP"; break;
							case 2 : sOSName	= "Windows Server 2003 (R2) or Windows XP Professional x64"; break;
							default : sOSName	= "Unknown Windows 2k-based version";
						}
						break;
					case 6  : 
						switch( oOS_version.dwMinorVersion )
						{
							case 0 : sOSName	= "Windows Vista or Windows Server 2008 (not tested)"; break;
							default : sOSName	= "Unknown Windows Vista-based version";
						}
						break;
		
					default : sOSName = "Unknown Windows NT-based version";
				}				
				break;
			}
	}

	sTemp.Format(" V%d.%d Build %u %s",oOS_version.dwMajorVersion,oOS_version.dwMinorVersion,(oOS_version.dwBuildNumber&0xffff),oOS_version.szCSDVersion );
	return sOSName + sTemp;
}

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

		::LogMessage( (const char*) NULL, NULL, "********* DirectSound Control Logging ended *********");
	}

#endif // _DEBUG
}

//////////////////////////////////////////////////////////////////////////////////////////////
// CDSoundCtrlApp initialization

BOOL CDSoundCtrlApp::InitInstance()
{
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

	g_bStoppedDriverWorkaround = Config.EnableStoppedDriverWorkaround;

#ifdef _DEBUG		//<----TODO: update all logging
	::LogMessage((const char*)NULL, NULL, "********* DirectSound Control Logging started *********");
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
	// Get path to original DSOUND.DLL
	//

	char buffer[1024];

	::GetSystemDirectory(buffer, 1024);
	::PathAddBackslash(buffer);
	strcat_s(buffer, 1024, "dsound.dll");
	//strcpy_s( buffer + strlen( buffer ), 1024, "dsound.dll" );


	//
	// Load DLL and get entry points
	//

	g_hDLL = ::LoadLibrary(buffer);

	if (g_hDLL == NULL)
	{
		LogMessage(m_cszClassName, this, "ERROR: Can't load original DLL...fatal...");

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
		::LogMessage(m_cszClassName, this, "Original DSOUND.DLL loaded....");

		CString sMessage;

		sMessage.Format("Location,%s", buffer);
		::LogMessage(m_cszClassName, this, sMessage.GetBuffer());

		sMessage = "OS," + ::GetOSVersion();
		::LogMessage(m_cszClassName, this, sMessage.GetBuffer());

		sMessage = "DirectX," + ::GetDirectXVersionFromReg();
		::LogMessage(m_cszClassName, this, sMessage.GetBuffer());

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
	if( lpGuid != NULL )
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
		LogMessage( g_cszClassName, NULL, "DirectSoundCreate called...");
#endif // _DEBUG

	assert( g_pDirectSoundCreate != NULL );

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
		::LogMessage( g_cszClassName, NULL, sMessage.GetBuffer());
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
		::LogMessage(g_cszClassName, NULL, sMessage.GetBuffer());
	}
#endif // _DEBUG

	return g_pAppDSEnumCallbackW(lpGuid, lpcstrDescription, lpcstrModule, lpContext);
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT STDMETHODCALLTYPE DirectSoundEnumerateA(LPDSENUMCALLBACKA pDSEnumCallback, LPVOID pContext)
{
#ifdef _DEBUG
	if( g_bLogSystem == true )
		LogMessage( g_cszClassName, NULL, "DirectSoundEnumerateA called...");
#endif // _DEBUG

	g_pAppDSEnumCallbackA = pDSEnumCallback;

	return g_pDirectSoundEnumerateAfunc( (LPDSENUMCALLBACKA)DSDLLEnumCallbackA, pContext );
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT STDMETHODCALLTYPE DirectSoundEnumerateW(LPDSENUMCALLBACKW pDSEnumCallback, LPVOID pContext)
{
#ifdef _DEBUG
	if( g_bLogSystem == true )
		LogMessage( g_cszClassName, NULL, "DirectSoundEnumerateW called...");
#endif // _DEBUG

	g_pAppDSEnumCallbackW = pDSEnumCallback;

	return g_pDirectSoundEnumerateWfunc( (LPDSENUMCALLBACKW)DSDLLEnumCallbackW, pContext );
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT STDMETHODCALLTYPE DirectSoundCaptureCreate(LPCGUID pcGuidDevice, LPDIRECTSOUNDCAPTURE *ppDSC, LPUNKNOWN pUnkOuter)
{
#ifdef _DEBUG
	if( g_bLogSystem == true )
		LogMessage( g_cszClassName, NULL, "DirectSoundCaptureCreate called...");
#endif // _DEBUG

	return g_pDirectSoundCaptureCreatefunc( pcGuidDevice, ppDSC, pUnkOuter );
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT STDMETHODCALLTYPE DirectSoundCaptureEnumerateA(LPDSENUMCALLBACKA pDSEnumCallback, LPVOID pContext)
{
#ifdef _DEBUG
	if( g_bLogSystem == true )
		LogMessage( g_cszClassName, NULL, "DirectSoundCaptureEnumerateA called...");
#endif // _DEBUG

	return g_pDirectSoundCaptureEnumerateAfunc( pDSEnumCallback, pContext );
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT STDMETHODCALLTYPE DirectSoundCaptureEnumerateW(LPDSENUMCALLBACKW pDSEnumCallback, LPVOID pContext)
{
#ifdef _DEBUG
	if( g_bLogSystem == true )
		LogMessage( g_cszClassName, NULL, "DirectSoundCaptureEnumerateW called...");
#endif // _DEBUG

	return g_pDirectSoundCaptureEnumerateWfunc( pDSEnumCallback, pContext );
}

//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT STDMETHODCALLTYPE DirectSoundCreate8(LPCGUID pcGuidDevice, LPDIRECTSOUND8 *ppDS8, LPUNKNOWN pUnkOuter)
{
#ifdef _DEBUG
	if( g_bLogSystem == true )
		LogMessage( g_cszClassName, NULL, "DirectSoundCreate8 called...");

	if( g_bDebugBeep == true )
		Beep(2000, 100);
#endif // _DEBUG

	assert( g_pDirectSoundCreate8 != NULL );

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
		LogMessage( g_cszClassName, NULL, "DirectSoundCaptureCreate8 called...");
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
		LogMessage( g_cszClassName, NULL, "DirectSoundFullDuplexCreate called...");
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
		LogMessage( g_cszClassName, NULL, "GetDeviceID called...");
#endif // _DEBUG

	return g_pGetDeviceIDfunc( pGuidSrc, pGuidDest );
}

//////////////////////////////////////////////////////////////////////////////////////////////

STDAPI STDMETHODCALLTYPE DllCanUnloadNow_DSoundCtrl(void)
{
#ifdef _DEBUG
	if( g_bLogSystem == true )
		LogMessage( g_cszClassName, NULL, "DllCanUnloadNow called...");
#endif // _DEBUG

	return g_pDllCanUnloadNowfunc();
}
//////////////////////////////////////////////////////////////////////////////////////////////

STDAPI STDMETHODCALLTYPE DllGetClassObject_DSoundCtrl(IN REFCLSID rclsid, IN REFIID riid, OUT LPVOID FAR* ppv)
{
	*ppv = NULL;

	if (( rclsid != CLSID_DirectSound ) && ( rclsid != CLSID_DirectSound8 ))
	{
#ifdef _DEBUG
	if( g_bLogSystem == true )
		LogMessage( g_cszClassName, NULL, "DllGetClassObject with unknown CLSID called...");
#endif // _DEBUG

		return g_pDllGetClassObjectfunc( rclsid, riid, ppv );
	}

	if( riid == IID_IClassFactory )
	{
		IDirectSoundClassFactoryEx* pDSCFX = new IDirectSoundClassFactoryEx;

		HRESULT hRes = g_pDllGetClassObjectfunc( rclsid, riid, (LPVOID*) &(pDSCFX->m_lpClassFactory) );

		if( hRes != NULL )
		{
#ifdef _DEBUG
			if( g_bLogSystem == true )
				LogMessage( g_cszClassName, NULL, "DllGetClassObject for IID_IClassFactory failed...");
#endif // _DEBUG

			delete pDSCFX;
			return hRes;
		}
		
		*ppv = (LPVOID*) pDSCFX;

#ifdef _DEBUG
		if( g_bLogSystem == true )
			LogMessage( g_cszClassName, NULL, "DllGetClassObject for IID_IClassFactory called...");
#endif // _DEBUG

		return S_OK;
	}

#ifdef _DEBUG
	if( g_bLogSystem == true )
		LogMessage( g_cszClassName, NULL, "DllGetClassObject for unknown IID interface called...");
#endif // _DEBUG

	return g_pDllGetClassObjectfunc( rclsid, riid, ppv );
}

//////////////////////////////////////////////////////////////////////////////////////////////

void RunDSoundCtrl()
{
	theApp.InitInstance();
}
