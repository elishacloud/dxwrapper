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
// DSoundCtrl.h : main header file for the DSoundCtrl DLL
//
//////////////////////////////////////////////////////////////////////////////////////////////
//
// Updated by Elisha Riedlinger 2017
//
//////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////

#define PROGRAM_VERSION					"V1.82"

//////////////////////////////////////////////////////////////////////////////////////////////

#define INI_FILE_NAME					".\\dsoundctrl.ini"
#define LOG_FILE_NAME					"dsoundctrl.csv"

#define SECTION_TWEAK					"Tweak"

#ifdef ENABLE_LOG
	#define SECTION_DEBUG					"Debug"

	#define KEY_LOGSYSTEM					"LogSystem"
	#define KEY_LOGDIRECTSOUND				"LogIDirectSound"
	#define KEY_LOGDIRECTSOUNDBUFFER		"LogIDirectSoundBuffer"
	#define KEY_LOGDIRECTSOUND3DBUFFER		"LogIDirectSound3DBuffer"
	#define KEY_LOGDIRECTSOUND3DLISTENER	"LogIDirectSound3DListener"
	#define KEY_DEBUGBEEP					"DebugBeep"
#endif // ENABLE_LOG

#define KEY_NUM2DBUFF					"Num2DBuffers"
#define KEY_NUM3DBUFF					"Num3DBuffers"
#define KEY_FORCECERT					"ForceCertification"
#define KEY_FORCEEXCLUSIVEMODE			"ForceExclusiveMode" 
#define KEY_FORCESOFTMIX				"ForceSoftwareMixing"
#define KEY_FORCEHARDMIX				"ForceHardwareMixing"
#define KEY_PREVSPEAKSETUP				"PreventSpeakerSetup"
#define KEY_HQSOFT3D					"ForceHQ3DSoftMixing"
#define KEY_FORCENONSTATIC				"ForceNonStaticBuffers"
#define KEY_FORCEVOICEMANAGEMENT		"ForceVoiceManagement"
#define KEY_FORCEPRIMARYBUFFERFORMAT	"ForcePrimaryBufferFormat"
#define KEY_PRIMARYBUFFERBITS			"PrimaryBufferBits"
#define KEY_PRIMARYBUFFERSAMPLES		"PrimaryBufferSamples"
#define KEY_PRIMARYBUFFERCHANNELS		"PrimaryBufferChannels"

#define KEY_WORKAROUNDSTOPPEDDRIVER		"EnableStoppedDriverWorkaround"

#define KEY_FORCESPEAKERCONFIG			"ForceSpeakerConfig"
#define KEY_SPEAKERCONFIG				"SpeakerConfig"

//////////////////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus

extern "C" {  // only need to export C interface if
              // used by C++ source code
#endif

//////////////////////////////////////////////////////////////////////////////////////////////

#undef DirectSoundCreate

//////////////////////////////////////////////////////////////////////////////////////////////

	DirectSoundCreate8func				g_pDirectSoundCreate8 = NULL;
	DirectSoundCreatefunc				g_pDirectSoundCreate = NULL;
	GetDeviceIDfunc						g_pGetDeviceIDfunc = NULL;
	DirectSoundEnumerateAfunc			g_pDirectSoundEnumerateAfunc = NULL;
	DirectSoundEnumerateWfunc			g_pDirectSoundEnumerateWfunc = NULL;
	DirectSoundCaptureCreatefunc		g_pDirectSoundCaptureCreatefunc = NULL;
	DirectSoundCaptureEnumerateAfunc	g_pDirectSoundCaptureEnumerateAfunc = NULL;
	DirectSoundCaptureEnumerateWfunc	g_pDirectSoundCaptureEnumerateWfunc = NULL;
	DirectSoundCaptureCreate8func		g_pDirectSoundCaptureCreate8func = NULL;
	DirectSoundFullDuplexCreatefunc		g_pDirectSoundFullDuplexCreatefunc = NULL;
	DllGetClassObjectfunc				g_pDllGetClassObjectfunc = NULL;
	DllCanUnloadNowfunc					g_pDllCanUnloadNowfunc = NULL;

//////////////////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif

#include "IDirectSoundEx.h"
#include "IDirectSoundBufferEx.h"

//////////////////////////////////////////////////////////////////////////////////////////////

#define CDSOUNDCTRLAPP_CLASS_NAME	"CDSoundCtrlApp"

//////////////////////////////////////////////////////////////////////////////////////////////

#ifdef ENABLE_LOG
extern void LogMessage(const char* szClassName, void* pInstance, char* szMessage );
#endif //ENABLE_LOG

//////////////////////////////////////////////////////////////////////////////////////////////

HMODULE			g_hDLL					=	NULL;

//////////////////////////////////////////////////////////////////////////////////////////////

int				g_nNum2DBuffers					=	0;
int				g_nNum3DBuffers					=	0;
bool			g_bForceCertification			=	false;
bool			g_bForceSoftwareMixing			=	false;
bool			g_bForceHardwareMixing			=	false;
bool			g_bPreventSpeakerSetup			=	false;
bool			g_bForceExclusiveMode			=	false;
bool			g_bForceHQSoftware3D			=	false;
bool			g_bForceNonStatic				=	false;
bool			g_bForceVoiceManagement			=	false;
bool			g_bForcePrimaryBufferFormat		=	false;
int				g_nPrimaryBufferBits			=	16;
int				g_nPrimaryBufferSamples			=	44100;
int				g_nPrimaryBufferChannels		=	2;

bool			g_bForceSpeakerConfig			=	false;
int				g_nSpeakerConfig				=	DSSPEAKER_5POINT1;

bool			g_bStoppedDriverWorkaround		=	false;

#ifdef ENABLE_LOG
DWORD			g_dwStartTime					=	0;
bool			g_bLogSystem					=	true;
bool			g_bLogDirectSound				=	true;
bool			g_bLogDirectSoundBuffer			=	true;
bool			g_bLogDirectSound3DBuffer		=	true;
bool			g_bLogDirectSound3DListener		=	true;
bool			g_bDebugBeep					=	true;
#endif // ENABLE_LOG

//////////////////////////////////////////////////////////////////////////////////////////////

// CDSoundCtrlApp
// See DSoundCtrl.cpp for the implementation of this class
//

class CDSoundCtrlApp
{
public:
	CDSoundCtrlApp();
	virtual ~CDSoundCtrlApp();

// Overrides
public:
	virtual BOOL InitInstance();

#ifdef ENABLE_LOG
	bool EnumCallback( LPGUID  lpGuid,    LPCSTR  lpcstrDescription,  LPCSTR  lpcstrModule );
#endif // ENABLE_LOG

protected : 
#ifdef ENABLE_LOG
	const char* m_cszClassName;
#endif // ENABLE_LOG
};