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

#include "cfg.h"
#include "wrapper.h"

bool winmmFlag = false;

struct winmm_dll
{
	HMODULE dll;
 	FARPROC CloseDriver;
 	FARPROC DefDriverProc;
 	FARPROC DriverCallback;
 	FARPROC DrvGetModuleHandle;
 	FARPROC GetDriverModuleHandle;
 	FARPROC NotifyCallbackData;
 	FARPROC OpenDriver;
 	FARPROC PlaySound;
 	FARPROC PlaySoundA;
 	FARPROC PlaySoundW;
 	FARPROC SendDriverMessage;
 	FARPROC WOW32DriverCallback;
 	FARPROC WOW32ResolveMultiMediaHandle;
 	FARPROC WOWAppExit;
 	FARPROC aux32Message;
 	FARPROC auxGetDevCapsA;
 	FARPROC auxGetDevCapsW;
 	FARPROC auxGetNumDevs;
 	FARPROC auxGetVolume;
 	FARPROC auxOutMessage;
 	FARPROC auxSetVolume;
 	FARPROC joy32Message;
 	FARPROC joyConfigChanged;
 	FARPROC joyGetDevCapsA;
 	FARPROC joyGetDevCapsW;
 	FARPROC joyGetNumDevs;
 	FARPROC joyGetPos;
 	FARPROC joyGetPosEx;
 	FARPROC joyGetThreshold;
 	FARPROC joyReleaseCapture;
 	FARPROC joySetCapture;
 	FARPROC joySetThreshold;
 	FARPROC mci32Message;
 	FARPROC mciDriverNotify;
 	FARPROC mciDriverYield;
 	FARPROC mciExecute;
 	FARPROC mciFreeCommandResource;
 	FARPROC mciGetCreatorTask;
 	FARPROC mciGetDeviceIDA;
 	FARPROC mciGetDeviceIDFromElementIDA;
 	FARPROC mciGetDeviceIDFromElementIDW;
 	FARPROC mciGetDeviceIDW;
 	FARPROC mciGetDriverData;
 	FARPROC mciGetErrorStringA;
 	FARPROC mciGetErrorStringW;
 	FARPROC mciGetYieldProc;
 	FARPROC mciLoadCommandResource;
 	FARPROC mciSendCommandA;
 	FARPROC mciSendCommandW;
 	FARPROC mciSendStringA;
 	FARPROC mciSendStringW;
 	FARPROC mciSetDriverData;
 	FARPROC mciSetYieldProc;
 	FARPROC mid32Message;
 	FARPROC midiConnect;
 	FARPROC midiDisconnect;
 	FARPROC midiInAddBuffer;
 	FARPROC midiInClose;
 	FARPROC midiInGetDevCapsA;
 	FARPROC midiInGetDevCapsW;
 	FARPROC midiInGetErrorTextA;
 	FARPROC midiInGetErrorTextW;
 	FARPROC midiInGetID;
 	FARPROC midiInGetNumDevs;
 	FARPROC midiInMessage;
 	FARPROC midiInOpen;
 	FARPROC midiInPrepareHeader;
 	FARPROC midiInReset;
 	FARPROC midiInStart;
 	FARPROC midiInStop;
 	FARPROC midiInUnprepareHeader;
 	FARPROC midiOutCacheDrumPatches;
 	FARPROC midiOutCachePatches;
 	FARPROC midiOutClose;
 	FARPROC midiOutGetDevCapsA;
 	FARPROC midiOutGetDevCapsW;
 	FARPROC midiOutGetErrorTextA;
 	FARPROC midiOutGetErrorTextW;
 	FARPROC midiOutGetID;
 	FARPROC midiOutGetNumDevs;
 	FARPROC midiOutGetVolume;
 	FARPROC midiOutLongMsg;
 	FARPROC midiOutMessage;
 	FARPROC midiOutOpen;
 	FARPROC midiOutPrepareHeader;
 	FARPROC midiOutReset;
 	FARPROC midiOutSetVolume;
 	FARPROC midiOutShortMsg;
 	FARPROC midiOutUnprepareHeader;
 	FARPROC midiStreamClose;
 	FARPROC midiStreamOpen;
 	FARPROC midiStreamOut;
 	FARPROC midiStreamPause;
 	FARPROC midiStreamPosition;
 	FARPROC midiStreamProperty;
 	FARPROC midiStreamRestart;
 	FARPROC midiStreamStop;
 	FARPROC mixerClose;
 	FARPROC mixerGetControlDetailsA;
 	FARPROC mixerGetControlDetailsW;
 	FARPROC mixerGetDevCapsA;
 	FARPROC mixerGetDevCapsW;
 	FARPROC mixerGetID;
 	FARPROC mixerGetLineControlsA;
 	FARPROC mixerGetLineControlsW;
 	FARPROC mixerGetLineInfoA;
 	FARPROC mixerGetLineInfoW;
 	FARPROC mixerGetNumDevs;
 	FARPROC mixerMessage;
 	FARPROC mixerOpen;
 	FARPROC mixerSetControlDetails;
 	FARPROC mmDrvInstall;
 	FARPROC mmGetCurrentTask;
 	FARPROC mmTaskBlock;
 	FARPROC mmTaskCreate;
 	FARPROC mmTaskSignal;
 	FARPROC mmTaskYield;
 	FARPROC mmioAdvance;
 	FARPROC mmioAscend;
 	FARPROC mmioClose;
 	FARPROC mmioCreateChunk;
 	FARPROC mmioDescend;
 	FARPROC mmioFlush;
 	FARPROC mmioGetInfo;
 	FARPROC mmioInstallIOProcA;
 	FARPROC mmioInstallIOProcW;
 	FARPROC mmioOpenA;
 	FARPROC mmioOpenW;
 	FARPROC mmioRead;
 	FARPROC mmioRenameA;
 	FARPROC mmioRenameW;
 	FARPROC mmioSeek;
 	FARPROC mmioSendMessage;
 	FARPROC mmioSetBuffer;
 	FARPROC mmioSetInfo;
 	FARPROC mmioStringToFOURCCA;
 	FARPROC mmioStringToFOURCCW;
 	FARPROC mmioWrite;
 	FARPROC mmsystemGetVersion;
 	FARPROC mod32Message;
 	FARPROC mxd32Message;
 	FARPROC sndPlaySoundA;
 	FARPROC sndPlaySoundW;
 	FARPROC tid32Message;
 	FARPROC timeBeginPeriod;
 	FARPROC timeEndPeriod;
 	FARPROC timeGetDevCaps;
 	FARPROC timeGetSystemTime;
 	FARPROC timeGetTime;
 	FARPROC timeKillEvent;
 	FARPROC timeSetEvent;
 	FARPROC waveInAddBuffer;
 	FARPROC waveInClose;
 	FARPROC waveInGetDevCapsA;
 	FARPROC waveInGetDevCapsW;
 	FARPROC waveInGetErrorTextA;
 	FARPROC waveInGetErrorTextW;
 	FARPROC waveInGetID;
 	FARPROC waveInGetNumDevs;
 	FARPROC waveInGetPosition;
 	FARPROC waveInMessage;
 	FARPROC waveInOpen;
 	FARPROC waveInPrepareHeader;
 	FARPROC waveInReset;
 	FARPROC waveInStart;
 	FARPROC waveInStop;
 	FARPROC waveInUnprepareHeader;
 	FARPROC waveOutBreakLoop;
 	FARPROC waveOutClose;
 	FARPROC waveOutGetDevCapsA;
 	FARPROC waveOutGetDevCapsW;
 	FARPROC waveOutGetErrorTextA;
 	FARPROC waveOutGetErrorTextW;
 	FARPROC waveOutGetID;
 	FARPROC waveOutGetNumDevs;
 	FARPROC waveOutGetPitch;
 	FARPROC waveOutGetPlaybackRate;
 	FARPROC waveOutGetPosition;
 	FARPROC waveOutGetVolume;
 	FARPROC waveOutMessage;
 	FARPROC waveOutOpen;
 	FARPROC waveOutPause;
 	FARPROC waveOutPrepareHeader;
 	FARPROC waveOutReset;
 	FARPROC waveOutRestart;
 	FARPROC waveOutSetPitch;
 	FARPROC waveOutSetPlaybackRate;
 	FARPROC waveOutSetVolume;
 	FARPROC waveOutUnprepareHeader;
 	FARPROC waveOutWrite;
 	FARPROC wid32Message;
 	FARPROC winmmDbgOut;
 	FARPROC winmmSetDebugLevel;
 	FARPROC wod32Message;
} winmm;

__declspec(naked) void  FakeCloseDriver()					{ _asm { jmp [winmm.CloseDriver] } }
__declspec(naked) void  FakeDefDriverProc()					{ _asm { jmp [winmm.DefDriverProc] } }
__declspec(naked) void  FakeDriverCallback()				{ _asm { jmp [winmm.DriverCallback] } }
__declspec(naked) void  FakeDrvGetModuleHandle()			{ _asm { jmp [winmm.DrvGetModuleHandle] } }
__declspec(naked) void  FakeGetDriverModuleHandle()			{ _asm { jmp [winmm.GetDriverModuleHandle] } }
__declspec(naked) void  FakeNotifyCallbackData()			{ _asm { jmp [winmm.NotifyCallbackData] } }
__declspec(naked) void  FakeOpenDriver()					{ _asm { jmp [winmm.OpenDriver] } }
__declspec(naked) void  FakePlaySound()						{ _asm { jmp [winmm.PlaySound] } }
__declspec(naked) void  FakePlaySoundA()					{ _asm { jmp [winmm.PlaySoundA] } }
__declspec(naked) void  FakePlaySoundW()					{ _asm { jmp [winmm.PlaySoundW] } }
__declspec(naked) void  FakeSendDriverMessage()				{ _asm { jmp [winmm.SendDriverMessage] } }
__declspec(naked) void  FakeWOW32DriverCallback()			{ _asm { jmp [winmm.WOW32DriverCallback] } }
__declspec(naked) void  FakeWOW32ResolveMultiMediaHandle()	{ _asm { jmp [winmm.WOW32ResolveMultiMediaHandle] } }
__declspec(naked) void  FakeWOWAppExit()					{ _asm { jmp [winmm.WOWAppExit] } }
__declspec(naked) void  Fakeaux32Message()					{ _asm { jmp [winmm.aux32Message] } }
__declspec(naked) void  FakeauxGetDevCapsA()				{ _asm { jmp [winmm.auxGetDevCapsA] } }
__declspec(naked) void  FakeauxGetDevCapsW()				{ _asm { jmp [winmm.auxGetDevCapsW] } }
__declspec(naked) void  FakeauxGetNumDevs()					{ _asm { jmp [winmm.auxGetNumDevs] } }
__declspec(naked) void  FakeauxGetVolume()					{ _asm { jmp [winmm.auxGetVolume] } }
__declspec(naked) void  FakeauxOutMessage()					{ _asm { jmp [winmm.auxOutMessage] } }
__declspec(naked) void  FakeauxSetVolume()					{ _asm { jmp [winmm.auxSetVolume] } }
__declspec(naked) void  Fakejoy32Message()					{ _asm { jmp [winmm.joy32Message] } }
__declspec(naked) void  FakejoyConfigChanged()				{ _asm { jmp [winmm.joyConfigChanged] } }
__declspec(naked) void  FakejoyGetDevCapsA()				{ _asm { jmp [winmm.joyGetDevCapsA] } }
__declspec(naked) void  FakejoyGetDevCapsW()				{ _asm { jmp [winmm.joyGetDevCapsW] } }
__declspec(naked) void  FakejoyGetNumDevs()					{ _asm { jmp [winmm.joyGetNumDevs] } }
__declspec(naked) void  FakejoyGetPos()						{ _asm { jmp [winmm.joyGetPos] } }
__declspec(naked) void  FakejoyGetPosEx()					{ _asm { jmp [winmm.joyGetPosEx] } }
__declspec(naked) void  FakejoyGetThreshold()				{ _asm { jmp [winmm.joyGetThreshold] } }
__declspec(naked) void  FakejoyReleaseCapture()				{ _asm { jmp [winmm.joyReleaseCapture] } }
__declspec(naked) void  FakejoySetCapture()					{ _asm { jmp [winmm.joySetCapture] } }
__declspec(naked) void  FakejoySetThreshold()				{ _asm { jmp [winmm.joySetThreshold] } }
__declspec(naked) void  Fakemci32Message()					{ _asm { jmp [winmm.mci32Message] } }
__declspec(naked) void  FakemciDriverNotify()				{ _asm { jmp [winmm.mciDriverNotify] } }
__declspec(naked) void  FakemciDriverYield()				{ _asm { jmp [winmm.mciDriverYield] } }
__declspec(naked) void  FakemciExecute()					{ _asm { jmp [winmm.mciExecute] } }
__declspec(naked) void  FakemciFreeCommandResource()		{ _asm { jmp [winmm.mciFreeCommandResource] } }
__declspec(naked) void  FakemciGetCreatorTask()				{ _asm { jmp [winmm.mciGetCreatorTask] } }
__declspec(naked) void  FakemciGetDeviceIDA()				{ _asm { jmp [winmm.mciGetDeviceIDA] } }
__declspec(naked) void  FakemciGetDeviceIDFromElementIDA()	{ _asm { jmp [winmm.mciGetDeviceIDFromElementIDA] } }
__declspec(naked) void  FakemciGetDeviceIDFromElementIDW()	{ _asm { jmp [winmm.mciGetDeviceIDFromElementIDW] } }
__declspec(naked) void  FakemciGetDeviceIDW()				{ _asm { jmp [winmm.mciGetDeviceIDW] } }
__declspec(naked) void  FakemciGetDriverData()				{ _asm { jmp [winmm.mciGetDriverData] } }
__declspec(naked) void  FakemciGetErrorStringA()			{ _asm { jmp [winmm.mciGetErrorStringA] } }
__declspec(naked) void  FakemciGetErrorStringW()			{ _asm { jmp [winmm.mciGetErrorStringW] } }
__declspec(naked) void  FakemciGetYieldProc()				{ _asm { jmp [winmm.mciGetYieldProc] } }
__declspec(naked) void  FakemciLoadCommandResource()		{ _asm { jmp [winmm.mciLoadCommandResource] } }
__declspec(naked) void  FakemciSendCommandA()				{ _asm { jmp [winmm.mciSendCommandA] } }
__declspec(naked) void  FakemciSendCommandW()				{ _asm { jmp [winmm.mciSendCommandW] } }
__declspec(naked) void  FakemciSendStringA()				{ _asm { jmp [winmm.mciSendStringA] } }
__declspec(naked) void  FakemciSendStringW()				{ _asm { jmp [winmm.mciSendStringW] } }
__declspec(naked) void  FakemciSetDriverData()				{ _asm { jmp [winmm.mciSetDriverData] } }
__declspec(naked) void  FakemciSetYieldProc()				{ _asm { jmp [winmm.mciSetYieldProc] } }
__declspec(naked) void  Fakemid32Message()					{ _asm { jmp [winmm.mid32Message] } }
__declspec(naked) void  FakemidiConnect()					{ _asm { jmp [winmm.midiConnect] } }
__declspec(naked) void  FakemidiDisconnect()				{ _asm { jmp [winmm.midiDisconnect] } }
__declspec(naked) void  FakemidiInAddBuffer()				{ _asm { jmp [winmm.midiInAddBuffer] } }
__declspec(naked) void  FakemidiInClose()					{ _asm { jmp [winmm.midiInClose] } }
__declspec(naked) void  FakemidiInGetDevCapsA()				{ _asm { jmp [winmm.midiInGetDevCapsA] } }
__declspec(naked) void  FakemidiInGetDevCapsW()				{ _asm { jmp [winmm.midiInGetDevCapsW] } }
__declspec(naked) void  FakemidiInGetErrorTextA()			{ _asm { jmp [winmm.midiInGetErrorTextA] } }
__declspec(naked) void  FakemidiInGetErrorTextW()			{ _asm { jmp [winmm.midiInGetErrorTextW] } }
__declspec(naked) void  FakemidiInGetID()					{ _asm { jmp [winmm.midiInGetID] } }
__declspec(naked) void  FakemidiInGetNumDevs()				{ _asm { jmp [winmm.midiInGetNumDevs] } }
__declspec(naked) void  FakemidiInMessage()					{ _asm { jmp [winmm.midiInMessage] } }
__declspec(naked) void  FakemidiInOpen()					{ _asm { jmp [winmm.midiInOpen] } }
__declspec(naked) void  FakemidiInPrepareHeader()			{ _asm { jmp [winmm.midiInPrepareHeader] } }
__declspec(naked) void  FakemidiInReset()					{ _asm { jmp [winmm.midiInReset] } }
__declspec(naked) void  FakemidiInStart()					{ _asm { jmp [winmm.midiInStart] } }
__declspec(naked) void  FakemidiInStop()					{ _asm { jmp [winmm.midiInStop] } }
__declspec(naked) void  FakemidiInUnprepareHeader()			{ _asm { jmp [winmm.midiInUnprepareHeader] } }
__declspec(naked) void  FakemidiOutCacheDrumPatches()		{ _asm { jmp [winmm.midiOutCacheDrumPatches] } }
__declspec(naked) void  FakemidiOutCachePatches()			{ _asm { jmp [winmm.midiOutCachePatches] } }
__declspec(naked) void  FakemidiOutClose()					{ _asm { jmp [winmm.midiOutClose] } }
__declspec(naked) void  FakemidiOutGetDevCapsA()			{ _asm { jmp [winmm.midiOutGetDevCapsA] } }
__declspec(naked) void  FakemidiOutGetDevCapsW()			{ _asm { jmp [winmm.midiOutGetDevCapsW] } }
__declspec(naked) void  FakemidiOutGetErrorTextA()			{ _asm { jmp [winmm.midiOutGetErrorTextA] } }
__declspec(naked) void  FakemidiOutGetErrorTextW()			{ _asm { jmp [winmm.midiOutGetErrorTextW] } }
__declspec(naked) void  FakemidiOutGetID()					{ _asm { jmp [winmm.midiOutGetID] } }
__declspec(naked) void  FakemidiOutGetNumDevs()				{ _asm { jmp [winmm.midiOutGetNumDevs] } }
__declspec(naked) void  FakemidiOutGetVolume()				{ _asm { jmp [winmm.midiOutGetVolume] } }
__declspec(naked) void  FakemidiOutLongMsg()				{ _asm { jmp [winmm.midiOutLongMsg] } }
__declspec(naked) void  FakemidiOutMessage()				{ _asm { jmp [winmm.midiOutMessage] } }
__declspec(naked) void  FakemidiOutOpen()					{ _asm { jmp [winmm.midiOutOpen] } }
__declspec(naked) void  FakemidiOutPrepareHeader()			{ _asm { jmp [winmm.midiOutPrepareHeader] } }
__declspec(naked) void  FakemidiOutReset()					{ _asm { jmp [winmm.midiOutReset] } }
__declspec(naked) void  FakemidiOutSetVolume()				{ _asm { jmp [winmm.midiOutSetVolume] } }
__declspec(naked) void  FakemidiOutShortMsg()				{ _asm { jmp [winmm.midiOutShortMsg] } }
__declspec(naked) void  FakemidiOutUnprepareHeader()		{ _asm { jmp [winmm.midiOutUnprepareHeader] } }
__declspec(naked) void  FakemidiStreamClose()				{ _asm { jmp [winmm.midiStreamClose] } }
__declspec(naked) void  FakemidiStreamOpen()				{ _asm { jmp [winmm.midiStreamOpen] } }
__declspec(naked) void  FakemidiStreamOut()					{ _asm { jmp [winmm.midiStreamOut] } }
__declspec(naked) void  FakemidiStreamPause()				{ _asm { jmp [winmm.midiStreamPause] } }
__declspec(naked) void  FakemidiStreamPosition()			{ _asm { jmp [winmm.midiStreamPosition] } }
__declspec(naked) void  FakemidiStreamProperty()			{ _asm { jmp [winmm.midiStreamProperty] } }
__declspec(naked) void  FakemidiStreamRestart()				{ _asm { jmp [winmm.midiStreamRestart] } }
__declspec(naked) void  FakemidiStreamStop()				{ _asm { jmp [winmm.midiStreamStop] } }
__declspec(naked) void  FakemixerClose()					{ _asm { jmp [winmm.mixerClose] } }
__declspec(naked) void  FakemixerGetControlDetailsA()		{ _asm { jmp [winmm.mixerGetControlDetailsA] } }
__declspec(naked) void  FakemixerGetControlDetailsW()		{ _asm { jmp [winmm.mixerGetControlDetailsW] } }
__declspec(naked) void  FakemixerGetDevCapsA()				{ _asm { jmp [winmm.mixerGetDevCapsA] } }
__declspec(naked) void  FakemixerGetDevCapsW()				{ _asm { jmp [winmm.mixerGetDevCapsW] } }
__declspec(naked) void  FakemixerGetID()					{ _asm { jmp [winmm.mixerGetID] } }
__declspec(naked) void  FakemixerGetLineControlsA()			{ _asm { jmp [winmm.mixerGetLineControlsA] } }
__declspec(naked) void  FakemixerGetLineControlsW()			{ _asm { jmp [winmm.mixerGetLineControlsW] } }
__declspec(naked) void  FakemixerGetLineInfoA()				{ _asm { jmp [winmm.mixerGetLineInfoA] } }
__declspec(naked) void  FakemixerGetLineInfoW()				{ _asm { jmp [winmm.mixerGetLineInfoW] } }
__declspec(naked) void  FakemixerGetNumDevs()				{ _asm { jmp [winmm.mixerGetNumDevs] } }
__declspec(naked) void  FakemixerMessage()					{ _asm { jmp [winmm.mixerMessage] } }
__declspec(naked) void  FakemixerOpen()						{ _asm { jmp [winmm.mixerOpen] } }
__declspec(naked) void  FakemixerSetControlDetails()		{ _asm { jmp [winmm.mixerSetControlDetails] } }
__declspec(naked) void  FakemmDrvInstall()					{ _asm { jmp [winmm.mmDrvInstall] } }
__declspec(naked) void  FakemmGetCurrentTask()				{ _asm { jmp [winmm.mmGetCurrentTask] } }
__declspec(naked) void  FakemmTaskBlock()					{ _asm { jmp [winmm.mmTaskBlock] } }
__declspec(naked) void  FakemmTaskCreate()					{ _asm { jmp [winmm.mmTaskCreate] } }
__declspec(naked) void  FakemmTaskSignal()					{ _asm { jmp [winmm.mmTaskSignal] } }
__declspec(naked) void  FakemmTaskYield()					{ _asm { jmp [winmm.mmTaskYield] } }
__declspec(naked) void  FakemmioAdvance()					{ _asm { jmp [winmm.mmioAdvance] } }
__declspec(naked) void  FakemmioAscend()					{ _asm { jmp [winmm.mmioAscend] } }
__declspec(naked) void  FakemmioClose()						{ _asm { jmp [winmm.mmioClose] } }
__declspec(naked) void  FakemmioCreateChunk()				{ _asm { jmp [winmm.mmioCreateChunk] } }
__declspec(naked) void  FakemmioDescend()					{ _asm { jmp [winmm.mmioDescend] } }
__declspec(naked) void  FakemmioFlush()						{ _asm { jmp [winmm.mmioFlush] } }
__declspec(naked) void  FakemmioGetInfo()					{ _asm { jmp [winmm.mmioGetInfo] } }
__declspec(naked) void  FakemmioInstallIOProcA()			{ _asm { jmp [winmm.mmioInstallIOProcA] } }
__declspec(naked) void  FakemmioInstallIOProcW()			{ _asm { jmp [winmm.mmioInstallIOProcW] } }
__declspec(naked) void  FakemmioOpenA()						{ _asm { jmp [winmm.mmioOpenA] } }
__declspec(naked) void  FakemmioOpenW()						{ _asm { jmp [winmm.mmioOpenW] } }
__declspec(naked) void  FakemmioRead()						{ _asm { jmp [winmm.mmioRead] } }
__declspec(naked) void  FakemmioRenameA()					{ _asm { jmp [winmm.mmioRenameA] } }
__declspec(naked) void  FakemmioRenameW()					{ _asm { jmp [winmm.mmioRenameW] } }
__declspec(naked) void  FakemmioSeek()						{ _asm { jmp [winmm.mmioSeek] } }
__declspec(naked) void  FakemmioSendMessage()				{ _asm { jmp [winmm.mmioSendMessage] } }
__declspec(naked) void  FakemmioSetBuffer()					{ _asm { jmp [winmm.mmioSetBuffer] } }
__declspec(naked) void  FakemmioSetInfo()					{ _asm { jmp [winmm.mmioSetInfo] } }
__declspec(naked) void  FakemmioStringToFOURCCA()			{ _asm { jmp [winmm.mmioStringToFOURCCA] } }
__declspec(naked) void  FakemmioStringToFOURCCW()			{ _asm { jmp [winmm.mmioStringToFOURCCW] } }
__declspec(naked) void  FakemmioWrite()						{ _asm { jmp [winmm.mmioWrite] } }
__declspec(naked) void  FakemmsystemGetVersion()			{ _asm { jmp [winmm.mmsystemGetVersion] } }
__declspec(naked) void  Fakemod32Message()					{ _asm { jmp [winmm.mod32Message] } }
__declspec(naked) void  Fakemxd32Message()					{ _asm { jmp [winmm.mxd32Message] } }
__declspec(naked) void  FakesndPlaySoundA()					{ _asm { jmp [winmm.sndPlaySoundA] } }
__declspec(naked) void  FakesndPlaySoundW()					{ _asm { jmp [winmm.sndPlaySoundW] } }
__declspec(naked) void  Faketid32Message()					{ _asm { jmp [winmm.tid32Message] } }
__declspec(naked) void  FaketimeBeginPeriod()				{ _asm { jmp [winmm.timeBeginPeriod] } }
__declspec(naked) void  FaketimeEndPeriod()					{ _asm { jmp [winmm.timeEndPeriod] } }
__declspec(naked) void  FaketimeGetDevCaps()				{ _asm { jmp [winmm.timeGetDevCaps] } }
__declspec(naked) void  FaketimeGetSystemTime()				{ _asm { jmp [winmm.timeGetSystemTime] } }
__declspec(naked) void  FaketimeGetTime()					{ _asm { jmp [winmm.timeGetTime] } }
__declspec(naked) void  FaketimeKillEvent()					{ _asm { jmp [winmm.timeKillEvent] } }
__declspec(naked) void  FaketimeSetEvent()					{ _asm { jmp [winmm.timeSetEvent] } }
__declspec(naked) void  FakewaveInAddBuffer()				{ _asm { jmp [winmm.waveInAddBuffer] } }
__declspec(naked) void  FakewaveInClose()					{ _asm { jmp [winmm.waveInClose] } }
__declspec(naked) void  FakewaveInGetDevCapsA()				{ _asm { jmp [winmm.waveInGetDevCapsA] } }
__declspec(naked) void  FakewaveInGetDevCapsW()				{ _asm { jmp [winmm.waveInGetDevCapsW] } }
__declspec(naked) void  FakewaveInGetErrorTextA()			{ _asm { jmp [winmm.waveInGetErrorTextA] } }
__declspec(naked) void  FakewaveInGetErrorTextW()			{ _asm { jmp [winmm.waveInGetErrorTextW] } }
__declspec(naked) void  FakewaveInGetID()					{ _asm { jmp [winmm.waveInGetID] } }
__declspec(naked) void  FakewaveInGetNumDevs()				{ _asm { jmp [winmm.waveInGetNumDevs] } }
__declspec(naked) void  FakewaveInGetPosition()				{ _asm { jmp [winmm.waveInGetPosition] } }
__declspec(naked) void  FakewaveInMessage()					{ _asm { jmp [winmm.waveInMessage] } }
__declspec(naked) void  FakewaveInOpen()					{ _asm { jmp [winmm.waveInOpen] } }
__declspec(naked) void  FakewaveInPrepareHeader()			{ _asm { jmp [winmm.waveInPrepareHeader] } }
__declspec(naked) void  FakewaveInReset()					{ _asm { jmp [winmm.waveInReset] } }
__declspec(naked) void  FakewaveInStart()					{ _asm { jmp [winmm.waveInStart] } }
__declspec(naked) void  FakewaveInStop()					{ _asm { jmp [winmm.waveInStop] } }
__declspec(naked) void  FakewaveInUnprepareHeader()			{ _asm { jmp [winmm.waveInUnprepareHeader] } }
__declspec(naked) void  FakewaveOutBreakLoop()				{ _asm { jmp [winmm.waveOutBreakLoop] } }
__declspec(naked) void  FakewaveOutClose()					{ _asm { jmp [winmm.waveOutClose] } }
__declspec(naked) void  FakewaveOutGetDevCapsA()			{ _asm { jmp [winmm.waveOutGetDevCapsA] } }
__declspec(naked) void  FakewaveOutGetDevCapsW()			{ _asm { jmp [winmm.waveOutGetDevCapsW] } }
__declspec(naked) void  FakewaveOutGetErrorTextA()			{ _asm { jmp [winmm.waveOutGetErrorTextA] } }
__declspec(naked) void  FakewaveOutGetErrorTextW()			{ _asm { jmp [winmm.waveOutGetErrorTextW] } }
__declspec(naked) void  FakewaveOutGetID()					{ _asm { jmp [winmm.waveOutGetID] } }
__declspec(naked) void  FakewaveOutGetNumDevs()				{ _asm { jmp [winmm.waveOutGetNumDevs] } }
__declspec(naked) void  FakewaveOutGetPitch()				{ _asm { jmp [winmm.waveOutGetPitch] } }
__declspec(naked) void  FakewaveOutGetPlaybackRate()		{ _asm { jmp [winmm.waveOutGetPlaybackRate] } }
__declspec(naked) void  FakewaveOutGetPosition()			{ _asm { jmp [winmm.waveOutGetPosition] } }
__declspec(naked) void  FakewaveOutGetVolume()				{ _asm { jmp [winmm.waveOutGetVolume] } }
__declspec(naked) void  FakewaveOutMessage()				{ _asm { jmp [winmm.waveOutMessage] } }
__declspec(naked) void  FakewaveOutOpen()					{ _asm { jmp [winmm.waveOutOpen] } }
__declspec(naked) void  FakewaveOutPause()					{ _asm { jmp [winmm.waveOutPause] } }
__declspec(naked) void  FakewaveOutPrepareHeader()			{ _asm { jmp [winmm.waveOutPrepareHeader] } }
__declspec(naked) void  FakewaveOutReset()					{ _asm { jmp [winmm.waveOutReset] } }
__declspec(naked) void  FakewaveOutRestart()				{ _asm { jmp [winmm.waveOutRestart] } }
__declspec(naked) void  FakewaveOutSetPitch()				{ _asm { jmp [winmm.waveOutSetPitch] } }
__declspec(naked) void  FakewaveOutSetPlaybackRate()		{ _asm { jmp [winmm.waveOutSetPlaybackRate] } }
__declspec(naked) void  FakewaveOutSetVolume()				{ _asm { jmp [winmm.waveOutSetVolume] } }
__declspec(naked) void  FakewaveOutUnprepareHeader()		{ _asm { jmp [winmm.waveOutUnprepareHeader] } }
__declspec(naked) void  FakewaveOutWrite()					{ _asm { jmp [winmm.waveOutWrite] } }
__declspec(naked) void  Fakewid32Message()					{ _asm { jmp [winmm.wid32Message] } }
__declspec(naked) void  FakewinmmDbgOut()					{ _asm { jmp [winmm.winmmDbgOut] } }
__declspec(naked) void  FakewinmmSetDebugLevel()			{ _asm { jmp [winmm.winmmSetDebugLevel] } }
__declspec(naked) void  Fakewod32Message()					{ _asm { jmp [winmm.wod32Message] } }

void LoadWinmm()
{
	// Load real dll
	winmm.dll = LoadDll("winmm.dll", dtype.winmm);
	// Load dll functions
	if (winmm.dll)
	{
		winmmFlag = true;
		winmm.CloseDriver					= GetProcAddress(winmm.dll, "CloseDriver");
		winmm.DefDriverProc					= GetProcAddress(winmm.dll, "DefDriverProc");
		winmm.DriverCallback				= GetProcAddress(winmm.dll, "DriverCallback");
		winmm.DrvGetModuleHandle			= GetProcAddress(winmm.dll, "DrvGetModuleHandle");
		winmm.GetDriverModuleHandle			= GetProcAddress(winmm.dll, "GetDriverModuleHandle");
		winmm.NotifyCallbackData			= GetProcAddress(winmm.dll, "NotifyCallbackData");
		winmm.OpenDriver					= GetProcAddress(winmm.dll, "OpenDriver");
		winmm.PlaySound						= GetProcAddress(winmm.dll, "PlaySound");
		winmm.PlaySoundA					= GetProcAddress(winmm.dll, "PlaySoundA");
		winmm.PlaySoundW					= GetProcAddress(winmm.dll, "PlaySoundW");
		winmm.SendDriverMessage				= GetProcAddress(winmm.dll, "SendDriverMessage");
		winmm.WOW32DriverCallback			= GetProcAddress(winmm.dll, "WOW32DriverCallback");
		winmm.WOW32ResolveMultiMediaHandle	= GetProcAddress(winmm.dll, "WOW32ResolveMultiMediaHandle");
		winmm.WOWAppExit					= GetProcAddress(winmm.dll, "WOWAppExit");
		winmm.aux32Message					= GetProcAddress(winmm.dll, "aux32Message");
		winmm.auxGetDevCapsA				= GetProcAddress(winmm.dll, "auxGetDevCapsA");
		winmm.auxGetDevCapsW				= GetProcAddress(winmm.dll, "auxGetDevCapsW");
		winmm.auxGetNumDevs					= GetProcAddress(winmm.dll, "auxGetNumDevs");
		winmm.auxGetVolume					= GetProcAddress(winmm.dll, "auxGetVolume");
		winmm.auxOutMessage					= GetProcAddress(winmm.dll, "auxOutMessage");
		winmm.auxSetVolume					= GetProcAddress(winmm.dll, "auxSetVolume");
		winmm.joy32Message					= GetProcAddress(winmm.dll, "joy32Message");
		winmm.joyConfigChanged				= GetProcAddress(winmm.dll, "joyConfigChanged");
		winmm.joyGetDevCapsA				= GetProcAddress(winmm.dll, "joyGetDevCapsA");
		winmm.joyGetDevCapsW				= GetProcAddress(winmm.dll, "joyGetDevCapsW");
		winmm.joyGetNumDevs					= GetProcAddress(winmm.dll, "joyGetNumDevs");
		winmm.joyGetPos						= GetProcAddress(winmm.dll, "joyGetPos");
		winmm.joyGetPosEx					= GetProcAddress(winmm.dll, "joyGetPosEx");
		winmm.joyGetThreshold				= GetProcAddress(winmm.dll, "joyGetThreshold");
		winmm.joyReleaseCapture				= GetProcAddress(winmm.dll, "joyReleaseCapture");
		winmm.joySetCapture					= GetProcAddress(winmm.dll, "joySetCapture");
		winmm.joySetThreshold				= GetProcAddress(winmm.dll, "joySetThreshold");
		winmm.mci32Message					= GetProcAddress(winmm.dll, "mci32Message");
		winmm.mciDriverNotify				= GetProcAddress(winmm.dll, "mciDriverNotify");
		winmm.mciDriverYield				= GetProcAddress(winmm.dll, "mciDriverYield");
		winmm.mciExecute					= GetProcAddress(winmm.dll, "mciExecute");
		winmm.mciFreeCommandResource		= GetProcAddress(winmm.dll, "mciFreeCommandResource");
		winmm.mciGetCreatorTask				= GetProcAddress(winmm.dll, "mciGetCreatorTask");
		winmm.mciGetDeviceIDA				= GetProcAddress(winmm.dll, "mciGetDeviceIDA");
		winmm.mciGetDeviceIDFromElementIDA	= GetProcAddress(winmm.dll, "mciGetDeviceIDFromElementIDA");
		winmm.mciGetDeviceIDFromElementIDW	= GetProcAddress(winmm.dll, "mciGetDeviceIDFromElementIDW");
		winmm.mciGetDeviceIDW				= GetProcAddress(winmm.dll, "mciGetDeviceIDW");
		winmm.mciGetDriverData				= GetProcAddress(winmm.dll, "mciGetDriverData");
		winmm.mciGetErrorStringA			= GetProcAddress(winmm.dll, "mciGetErrorStringA");
		winmm.mciGetErrorStringW			= GetProcAddress(winmm.dll, "mciGetErrorStringW");
		winmm.mciGetYieldProc				= GetProcAddress(winmm.dll, "mciGetYieldProc");
		winmm.mciLoadCommandResource		= GetProcAddress(winmm.dll, "mciLoadCommandResource");
		winmm.mciSendCommandA				= GetProcAddress(winmm.dll, "mciSendCommandA");
		winmm.mciSendCommandW				= GetProcAddress(winmm.dll, "mciSendCommandW");
		winmm.mciSendStringA				= GetProcAddress(winmm.dll, "mciSendStringA");
		winmm.mciSendStringW				= GetProcAddress(winmm.dll, "mciSendStringW");
		winmm.mciSetDriverData				= GetProcAddress(winmm.dll, "mciSetDriverData");
		winmm.mciSetYieldProc				= GetProcAddress(winmm.dll, "mciSetYieldProc");
		winmm.mid32Message					= GetProcAddress(winmm.dll, "mid32Message");
		winmm.midiConnect					= GetProcAddress(winmm.dll, "midiConnect");
		winmm.midiDisconnect				= GetProcAddress(winmm.dll, "midiDisconnect");
		winmm.midiInAddBuffer				= GetProcAddress(winmm.dll, "midiInAddBuffer");
		winmm.midiInClose					= GetProcAddress(winmm.dll, "midiInClose");
		winmm.midiInGetDevCapsA				= GetProcAddress(winmm.dll, "midiInGetDevCapsA");
		winmm.midiInGetDevCapsW				= GetProcAddress(winmm.dll, "midiInGetDevCapsW");
		winmm.midiInGetErrorTextA			= GetProcAddress(winmm.dll, "midiInGetErrorTextA");
		winmm.midiInGetErrorTextW			= GetProcAddress(winmm.dll, "midiInGetErrorTextW");
		winmm.midiInGetID					= GetProcAddress(winmm.dll, "midiInGetID");
		winmm.midiInGetNumDevs				= GetProcAddress(winmm.dll, "midiInGetNumDevs");
		winmm.midiInMessage					= GetProcAddress(winmm.dll, "midiInMessage");
		winmm.midiInOpen					= GetProcAddress(winmm.dll, "midiInOpen");
		winmm.midiInPrepareHeader			= GetProcAddress(winmm.dll, "midiInPrepareHeader");
		winmm.midiInReset					= GetProcAddress(winmm.dll, "midiInReset");
		winmm.midiInStart					= GetProcAddress(winmm.dll, "midiInStart");
		winmm.midiInStop					= GetProcAddress(winmm.dll, "midiInStop");
		winmm.midiInUnprepareHeader			= GetProcAddress(winmm.dll, "midiInUnprepareHeader");
		winmm.midiOutCacheDrumPatches		= GetProcAddress(winmm.dll, "midiOutCacheDrumPatches");
		winmm.midiOutCachePatches			= GetProcAddress(winmm.dll, "midiOutCachePatches");
		winmm.midiOutClose					= GetProcAddress(winmm.dll, "midiOutClose");
		winmm.midiOutGetDevCapsA			= GetProcAddress(winmm.dll, "midiOutGetDevCapsA");
		winmm.midiOutGetDevCapsW			= GetProcAddress(winmm.dll, "midiOutGetDevCapsW");
		winmm.midiOutGetErrorTextA			= GetProcAddress(winmm.dll, "midiOutGetErrorTextA");
		winmm.midiOutGetErrorTextW			= GetProcAddress(winmm.dll, "midiOutGetErrorTextW");
		winmm.midiOutGetID					= GetProcAddress(winmm.dll, "midiOutGetID");
		winmm.midiOutGetNumDevs				= GetProcAddress(winmm.dll, "midiOutGetNumDevs");
		winmm.midiOutGetVolume				= GetProcAddress(winmm.dll, "midiOutGetVolume");
		winmm.midiOutLongMsg				= GetProcAddress(winmm.dll, "midiOutLongMsg");
		winmm.midiOutMessage				= GetProcAddress(winmm.dll, "midiOutMessage");
		winmm.midiOutOpen					= GetProcAddress(winmm.dll, "midiOutOpen");
		winmm.midiOutPrepareHeader			= GetProcAddress(winmm.dll, "midiOutPrepareHeader");
		winmm.midiOutReset					= GetProcAddress(winmm.dll, "midiOutReset");
		winmm.midiOutSetVolume				= GetProcAddress(winmm.dll, "midiOutSetVolume");
		winmm.midiOutShortMsg				= GetProcAddress(winmm.dll, "midiOutShortMsg");
		winmm.midiOutUnprepareHeader		= GetProcAddress(winmm.dll, "midiOutUnprepareHeader");
		winmm.midiStreamClose				= GetProcAddress(winmm.dll, "midiStreamClose");
		winmm.midiStreamOpen				= GetProcAddress(winmm.dll, "midiStreamOpen");
		winmm.midiStreamOut					= GetProcAddress(winmm.dll, "midiStreamOut");
		winmm.midiStreamPause				= GetProcAddress(winmm.dll, "midiStreamPause");
		winmm.midiStreamPosition			= GetProcAddress(winmm.dll, "midiStreamPosition");
		winmm.midiStreamProperty			= GetProcAddress(winmm.dll, "midiStreamProperty");
		winmm.midiStreamRestart				= GetProcAddress(winmm.dll, "midiStreamRestart");
		winmm.midiStreamStop				= GetProcAddress(winmm.dll, "midiStreamStop");
		winmm.mixerClose					= GetProcAddress(winmm.dll, "mixerClose");
		winmm.mixerGetControlDetailsA		= GetProcAddress(winmm.dll, "mixerGetControlDetailsA");
		winmm.mixerGetControlDetailsW		= GetProcAddress(winmm.dll, "mixerGetControlDetailsW");
		winmm.mixerGetDevCapsA				= GetProcAddress(winmm.dll, "mixerGetDevCapsA");
		winmm.mixerGetDevCapsW				= GetProcAddress(winmm.dll, "mixerGetDevCapsW");
		winmm.mixerGetID					= GetProcAddress(winmm.dll, "mixerGetID");
		winmm.mixerGetLineControlsA			= GetProcAddress(winmm.dll, "mixerGetLineControlsA");
		winmm.mixerGetLineControlsW			= GetProcAddress(winmm.dll, "mixerGetLineControlsW");
		winmm.mixerGetLineInfoA				= GetProcAddress(winmm.dll, "mixerGetLineInfoA");
		winmm.mixerGetLineInfoW				= GetProcAddress(winmm.dll, "mixerGetLineInfoW");
		winmm.mixerGetNumDevs				= GetProcAddress(winmm.dll, "mixerGetNumDevs");
		winmm.mixerMessage					= GetProcAddress(winmm.dll, "mixerMessage");
		winmm.mixerOpen						= GetProcAddress(winmm.dll, "mixerOpen");
		winmm.mixerSetControlDetails		= GetProcAddress(winmm.dll, "mixerSetControlDetails");
		winmm.mmDrvInstall					= GetProcAddress(winmm.dll, "mmDrvInstall");
		winmm.mmGetCurrentTask				= GetProcAddress(winmm.dll, "mmGetCurrentTask");
		winmm.mmTaskBlock					= GetProcAddress(winmm.dll, "mmTaskBlock");
		winmm.mmTaskCreate					= GetProcAddress(winmm.dll, "mmTaskCreate");
		winmm.mmTaskSignal					= GetProcAddress(winmm.dll, "mmTaskSignal");
		winmm.mmTaskYield					= GetProcAddress(winmm.dll, "mmTaskYield");
		winmm.mmioAdvance					= GetProcAddress(winmm.dll, "mmioAdvance");
		winmm.mmioAscend					= GetProcAddress(winmm.dll, "mmioAscend");
		winmm.mmioClose						= GetProcAddress(winmm.dll, "mmioClose");
		winmm.mmioCreateChunk				= GetProcAddress(winmm.dll, "mmioCreateChunk");
		winmm.mmioDescend					= GetProcAddress(winmm.dll, "mmioDescend");
		winmm.mmioFlush						= GetProcAddress(winmm.dll, "mmioFlush");
		winmm.mmioGetInfo					= GetProcAddress(winmm.dll, "mmioGetInfo");
		winmm.mmioInstallIOProcA			= GetProcAddress(winmm.dll, "mmioInstallIOProcA");
		winmm.mmioInstallIOProcW			= GetProcAddress(winmm.dll, "mmioInstallIOProcW");
		winmm.mmioOpenA						= GetProcAddress(winmm.dll, "mmioOpenA");
		winmm.mmioOpenW						= GetProcAddress(winmm.dll, "mmioOpenW");
		winmm.mmioRead						= GetProcAddress(winmm.dll, "mmioRead");
		winmm.mmioRenameA					= GetProcAddress(winmm.dll, "mmioRenameA");
		winmm.mmioRenameW					= GetProcAddress(winmm.dll, "mmioRenameW");
		winmm.mmioSeek						= GetProcAddress(winmm.dll, "mmioSeek");
		winmm.mmioSendMessage				= GetProcAddress(winmm.dll, "mmioSendMessage");
		winmm.mmioSetBuffer					= GetProcAddress(winmm.dll, "mmioSetBuffer");
		winmm.mmioSetInfo					= GetProcAddress(winmm.dll, "mmioSetInfo");
		winmm.mmioStringToFOURCCA			= GetProcAddress(winmm.dll, "mmioStringToFOURCCA");
		winmm.mmioStringToFOURCCW			= GetProcAddress(winmm.dll, "mmioStringToFOURCCW");
		winmm.mmioWrite						= GetProcAddress(winmm.dll, "mmioWrite");
		winmm.mmsystemGetVersion			= GetProcAddress(winmm.dll, "mmsystemGetVersion");
		winmm.mod32Message					= GetProcAddress(winmm.dll, "mod32Message");
		winmm.mxd32Message					= GetProcAddress(winmm.dll, "mxd32Message");
		winmm.sndPlaySoundA					= GetProcAddress(winmm.dll, "sndPlaySoundA");
		winmm.sndPlaySoundW					= GetProcAddress(winmm.dll, "sndPlaySoundW");
		winmm.tid32Message					= GetProcAddress(winmm.dll, "tid32Message");
		winmm.timeBeginPeriod				= GetProcAddress(winmm.dll, "timeBeginPeriod");
		winmm.timeEndPeriod					= GetProcAddress(winmm.dll, "timeEndPeriod");
		winmm.timeGetDevCaps				= GetProcAddress(winmm.dll, "timeGetDevCaps");
		winmm.timeGetSystemTime				= GetProcAddress(winmm.dll, "timeGetSystemTime");
		winmm.timeGetTime					= GetProcAddress(winmm.dll, "timeGetTime");
		winmm.timeKillEvent					= GetProcAddress(winmm.dll, "timeKillEvent");
		winmm.timeSetEvent					= GetProcAddress(winmm.dll, "timeSetEvent");
		winmm.waveInAddBuffer				= GetProcAddress(winmm.dll, "waveInAddBuffer");
		winmm.waveInClose					= GetProcAddress(winmm.dll, "waveInClose");
		winmm.waveInGetDevCapsA				= GetProcAddress(winmm.dll, "waveInGetDevCapsA");
		winmm.waveInGetDevCapsW				= GetProcAddress(winmm.dll, "waveInGetDevCapsW");
		winmm.waveInGetErrorTextA			= GetProcAddress(winmm.dll, "waveInGetErrorTextA");
		winmm.waveInGetErrorTextW			= GetProcAddress(winmm.dll, "waveInGetErrorTextW");
		winmm.waveInGetID					= GetProcAddress(winmm.dll, "waveInGetID");
		winmm.waveInGetNumDevs				= GetProcAddress(winmm.dll, "waveInGetNumDevs");
		winmm.waveInGetPosition				= GetProcAddress(winmm.dll, "waveInGetPosition");
		winmm.waveInMessage					= GetProcAddress(winmm.dll, "waveInMessage");
		winmm.waveInOpen					= GetProcAddress(winmm.dll, "waveInOpen");
		winmm.waveInPrepareHeader			= GetProcAddress(winmm.dll, "waveInPrepareHeader");
		winmm.waveInReset					= GetProcAddress(winmm.dll, "waveInReset");
		winmm.waveInStart					= GetProcAddress(winmm.dll, "waveInStart");
		winmm.waveInStop					= GetProcAddress(winmm.dll, "waveInStop");
		winmm.waveInUnprepareHeader			= GetProcAddress(winmm.dll, "waveInUnprepareHeader");
		winmm.waveOutBreakLoop				= GetProcAddress(winmm.dll, "waveOutBreakLoop");
		winmm.waveOutClose					= GetProcAddress(winmm.dll, "waveOutClose");
		winmm.waveOutGetDevCapsA			= GetProcAddress(winmm.dll, "waveOutGetDevCapsA");
		winmm.waveOutGetDevCapsW			= GetProcAddress(winmm.dll, "waveOutGetDevCapsW");
		winmm.waveOutGetErrorTextA			= GetProcAddress(winmm.dll, "waveOutGetErrorTextA");
		winmm.waveOutGetErrorTextW			= GetProcAddress(winmm.dll, "waveOutGetErrorTextW");
		winmm.waveOutGetID					= GetProcAddress(winmm.dll, "waveOutGetID");
		winmm.waveOutGetNumDevs				= GetProcAddress(winmm.dll, "waveOutGetNumDevs");
		winmm.waveOutGetPitch				= GetProcAddress(winmm.dll, "waveOutGetPitch");
		winmm.waveOutGetPlaybackRate		= GetProcAddress(winmm.dll, "waveOutGetPlaybackRate");
		winmm.waveOutGetPosition			= GetProcAddress(winmm.dll, "waveOutGetPosition");
		winmm.waveOutGetVolume				= GetProcAddress(winmm.dll, "waveOutGetVolume");
		winmm.waveOutMessage				= GetProcAddress(winmm.dll, "waveOutMessage");
		winmm.waveOutOpen					= GetProcAddress(winmm.dll, "waveOutOpen");
		winmm.waveOutPause					= GetProcAddress(winmm.dll, "waveOutPause");
		winmm.waveOutPrepareHeader			= GetProcAddress(winmm.dll, "waveOutPrepareHeader");
		winmm.waveOutReset					= GetProcAddress(winmm.dll, "waveOutReset");
		winmm.waveOutRestart				= GetProcAddress(winmm.dll, "waveOutRestart");
		winmm.waveOutSetPitch				= GetProcAddress(winmm.dll, "waveOutSetPitch");
		winmm.waveOutSetPlaybackRate		= GetProcAddress(winmm.dll, "waveOutSetPlaybackRate");
		winmm.waveOutSetVolume				= GetProcAddress(winmm.dll, "waveOutSetVolume");
		winmm.waveOutUnprepareHeader		= GetProcAddress(winmm.dll, "waveOutUnprepareHeader");
		winmm.waveOutWrite					= GetProcAddress(winmm.dll, "waveOutWrite");
		winmm.wid32Message					= GetProcAddress(winmm.dll, "wid32Message");
		winmm.winmmDbgOut					= GetProcAddress(winmm.dll, "winmmDbgOut");
		winmm.winmmSetDebugLevel			= GetProcAddress(winmm.dll, "winmmSetDebugLevel");
		winmm.wod32Message					= GetProcAddress(winmm.dll, "wod32Message");
	}
}

void FreeWinmmLibrary()
{
	if (winmmFlag) FreeLibrary(winmm.dll);
}