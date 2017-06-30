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

#include "Settings\Settings.h"
#include "wrapper.h"
#include "Utils\Utils.h"

struct winmm_dll
{
	HMODULE dll = nullptr;
	FARPROC CloseDriver = jmpaddr;
	FARPROC DefDriverProc = jmpaddr;
	FARPROC DriverCallback = jmpaddr;
	FARPROC DrvGetModuleHandle = jmpaddr;
	FARPROC GetDriverModuleHandle = jmpaddr;
	FARPROC NotifyCallbackData = jmpaddr;
	FARPROC OpenDriver = jmpaddr;
	FARPROC PlaySound = jmpaddr;
	FARPROC PlaySoundA = jmpaddr;
	FARPROC PlaySoundW = jmpaddr;
	FARPROC SendDriverMessage = jmpaddr;
	FARPROC WOW32DriverCallback = jmpaddr;
	FARPROC WOW32ResolveMultiMediaHandle = jmpaddr;
	FARPROC WOWAppExit = jmpaddr;
	FARPROC aux32Message = jmpaddr;
	FARPROC auxGetDevCapsA = jmpaddr;
	FARPROC auxGetDevCapsW = jmpaddr;
	FARPROC auxGetNumDevs = jmpaddr;
	FARPROC auxGetVolume = jmpaddr;
	FARPROC auxOutMessage = jmpaddr;
	FARPROC auxSetVolume = jmpaddr;
	FARPROC joy32Message = jmpaddr;
	FARPROC joyConfigChanged = jmpaddr;
	FARPROC joyGetDevCapsA = jmpaddr;
	FARPROC joyGetDevCapsW = jmpaddr;
	FARPROC joyGetNumDevs = jmpaddr;
	FARPROC joyGetPos = jmpaddr;
	FARPROC joyGetPosEx = jmpaddr;
	FARPROC joyGetThreshold = jmpaddr;
	FARPROC joyReleaseCapture = jmpaddr;
	FARPROC joySetCapture = jmpaddr;
	FARPROC joySetThreshold = jmpaddr;
	FARPROC mci32Message = jmpaddr;
	FARPROC mciDriverNotify = jmpaddr;
	FARPROC mciDriverYield = jmpaddr;
	FARPROC mciExecute = jmpaddr;
	FARPROC mciFreeCommandResource = jmpaddr;
	FARPROC mciGetCreatorTask = jmpaddr;
	FARPROC mciGetDeviceIDA = jmpaddr;
	FARPROC mciGetDeviceIDFromElementIDA = jmpaddr;
	FARPROC mciGetDeviceIDFromElementIDW = jmpaddr;
	FARPROC mciGetDeviceIDW = jmpaddr;
	FARPROC mciGetDriverData = jmpaddr;
	FARPROC mciGetErrorStringA = jmpaddr;
	FARPROC mciGetErrorStringW = jmpaddr;
	FARPROC mciGetYieldProc = jmpaddr;
	FARPROC mciLoadCommandResource = jmpaddr;
	FARPROC mciSendCommandA = jmpaddr;
	FARPROC mciSendCommandW = jmpaddr;
	FARPROC mciSendStringA = jmpaddr;
	FARPROC mciSendStringW = jmpaddr;
	FARPROC mciSetDriverData = jmpaddr;
	FARPROC mciSetYieldProc = jmpaddr;
	FARPROC mid32Message = jmpaddr;
	FARPROC midiConnect = jmpaddr;
	FARPROC midiDisconnect = jmpaddr;
	FARPROC midiInAddBuffer = jmpaddr;
	FARPROC midiInClose = jmpaddr;
	FARPROC midiInGetDevCapsA = jmpaddr;
	FARPROC midiInGetDevCapsW = jmpaddr;
	FARPROC midiInGetErrorTextA = jmpaddr;
	FARPROC midiInGetErrorTextW = jmpaddr;
	FARPROC midiInGetID = jmpaddr;
	FARPROC midiInGetNumDevs = jmpaddr;
	FARPROC midiInMessage = jmpaddr;
	FARPROC midiInOpen = jmpaddr;
	FARPROC midiInPrepareHeader = jmpaddr;
	FARPROC midiInReset = jmpaddr;
	FARPROC midiInStart = jmpaddr;
	FARPROC midiInStop = jmpaddr;
	FARPROC midiInUnprepareHeader = jmpaddr;
	FARPROC midiOutCacheDrumPatches = jmpaddr;
	FARPROC midiOutCachePatches = jmpaddr;
	FARPROC midiOutClose = jmpaddr;
	FARPROC midiOutGetDevCapsA = jmpaddr;
	FARPROC midiOutGetDevCapsW = jmpaddr;
	FARPROC midiOutGetErrorTextA = jmpaddr;
	FARPROC midiOutGetErrorTextW = jmpaddr;
	FARPROC midiOutGetID = jmpaddr;
	FARPROC midiOutGetNumDevs = jmpaddr;
	FARPROC midiOutGetVolume = jmpaddr;
	FARPROC midiOutLongMsg = jmpaddr;
	FARPROC midiOutMessage = jmpaddr;
	FARPROC midiOutOpen = jmpaddr;
	FARPROC midiOutPrepareHeader = jmpaddr;
	FARPROC midiOutReset = jmpaddr;
	FARPROC midiOutSetVolume = jmpaddr;
	FARPROC midiOutShortMsg = jmpaddr;
	FARPROC midiOutUnprepareHeader = jmpaddr;
	FARPROC midiStreamClose = jmpaddr;
	FARPROC midiStreamOpen = jmpaddr;
	FARPROC midiStreamOut = jmpaddr;
	FARPROC midiStreamPause = jmpaddr;
	FARPROC midiStreamPosition = jmpaddr;
	FARPROC midiStreamProperty = jmpaddr;
	FARPROC midiStreamRestart = jmpaddr;
	FARPROC midiStreamStop = jmpaddr;
	FARPROC mixerClose = jmpaddr;
	FARPROC mixerGetControlDetailsA = jmpaddr;
	FARPROC mixerGetControlDetailsW = jmpaddr;
	FARPROC mixerGetDevCapsA = jmpaddr;
	FARPROC mixerGetDevCapsW = jmpaddr;
	FARPROC mixerGetID = jmpaddr;
	FARPROC mixerGetLineControlsA = jmpaddr;
	FARPROC mixerGetLineControlsW = jmpaddr;
	FARPROC mixerGetLineInfoA = jmpaddr;
	FARPROC mixerGetLineInfoW = jmpaddr;
	FARPROC mixerGetNumDevs = jmpaddr;
	FARPROC mixerMessage = jmpaddr;
	FARPROC mixerOpen = jmpaddr;
	FARPROC mixerSetControlDetails = jmpaddr;
	FARPROC mmDrvInstall = jmpaddr;
	FARPROC mmGetCurrentTask = jmpaddr;
	FARPROC mmTaskBlock = jmpaddr;
	FARPROC mmTaskCreate = jmpaddr;
	FARPROC mmTaskSignal = jmpaddr;
	FARPROC mmTaskYield = jmpaddr;
	FARPROC mmioAdvance = jmpaddr;
	FARPROC mmioAscend = jmpaddr;
	FARPROC mmioClose = jmpaddr;
	FARPROC mmioCreateChunk = jmpaddr;
	FARPROC mmioDescend = jmpaddr;
	FARPROC mmioFlush = jmpaddr;
	FARPROC mmioGetInfo = jmpaddr;
	FARPROC mmioInstallIOProcA = jmpaddr;
	FARPROC mmioInstallIOProcW = jmpaddr;
	FARPROC mmioOpenA = jmpaddr;
	FARPROC mmioOpenW = jmpaddr;
	FARPROC mmioRead = jmpaddr;
	FARPROC mmioRenameA = jmpaddr;
	FARPROC mmioRenameW = jmpaddr;
	FARPROC mmioSeek = jmpaddr;
	FARPROC mmioSendMessage = jmpaddr;
	FARPROC mmioSetBuffer = jmpaddr;
	FARPROC mmioSetInfo = jmpaddr;
	FARPROC mmioStringToFOURCCA = jmpaddr;
	FARPROC mmioStringToFOURCCW = jmpaddr;
	FARPROC mmioWrite = jmpaddr;
	FARPROC mmsystemGetVersion = jmpaddr;
	FARPROC mod32Message = jmpaddr;
	FARPROC mxd32Message = jmpaddr;
	FARPROC sndPlaySoundA = jmpaddr;
	FARPROC sndPlaySoundW = jmpaddr;
	FARPROC tid32Message = jmpaddr;
	FARPROC timeBeginPeriod = jmpaddr;
	FARPROC timeEndPeriod = jmpaddr;
	FARPROC timeGetDevCaps = jmpaddr;
	FARPROC timeGetSystemTime = jmpaddr;
	FARPROC timeGetTime = jmpaddr;
	FARPROC timeKillEvent = jmpaddr;
	FARPROC timeSetEvent = jmpaddr;
	FARPROC waveInAddBuffer = jmpaddr;
	FARPROC waveInClose = jmpaddr;
	FARPROC waveInGetDevCapsA = jmpaddr;
	FARPROC waveInGetDevCapsW = jmpaddr;
	FARPROC waveInGetErrorTextA = jmpaddr;
	FARPROC waveInGetErrorTextW = jmpaddr;
	FARPROC waveInGetID = jmpaddr;
	FARPROC waveInGetNumDevs = jmpaddr;
	FARPROC waveInGetPosition = jmpaddr;
	FARPROC waveInMessage = jmpaddr;
	FARPROC waveInOpen = jmpaddr;
	FARPROC waveInPrepareHeader = jmpaddr;
	FARPROC waveInReset = jmpaddr;
	FARPROC waveInStart = jmpaddr;
	FARPROC waveInStop = jmpaddr;
	FARPROC waveInUnprepareHeader = jmpaddr;
	FARPROC waveOutBreakLoop = jmpaddr;
	FARPROC waveOutClose = jmpaddr;
	FARPROC waveOutGetDevCapsA = jmpaddr;
	FARPROC waveOutGetDevCapsW = jmpaddr;
	FARPROC waveOutGetErrorTextA = jmpaddr;
	FARPROC waveOutGetErrorTextW = jmpaddr;
	FARPROC waveOutGetID = jmpaddr;
	FARPROC waveOutGetNumDevs = jmpaddr;
	FARPROC waveOutGetPitch = jmpaddr;
	FARPROC waveOutGetPlaybackRate = jmpaddr;
	FARPROC waveOutGetPosition = jmpaddr;
	FARPROC waveOutGetVolume = jmpaddr;
	FARPROC waveOutMessage = jmpaddr;
	FARPROC waveOutOpen = jmpaddr;
	FARPROC waveOutPause = jmpaddr;
	FARPROC waveOutPrepareHeader = jmpaddr;
	FARPROC waveOutReset = jmpaddr;
	FARPROC waveOutRestart = jmpaddr;
	FARPROC waveOutSetPitch = jmpaddr;
	FARPROC waveOutSetPlaybackRate = jmpaddr;
	FARPROC waveOutSetVolume = jmpaddr;
	FARPROC waveOutUnprepareHeader = jmpaddr;
	FARPROC waveOutWrite = jmpaddr;
	FARPROC wid32Message = jmpaddr;
	FARPROC winmmDbgOut = jmpaddr;
	FARPROC winmmSetDebugLevel = jmpaddr;
	FARPROC wod32Message = jmpaddr;
} winmm;

__declspec(naked) void  FakeCloseDriver() { _asm { jmp[winmm.CloseDriver] } }
__declspec(naked) void  FakeDefDriverProc() { _asm { jmp[winmm.DefDriverProc] } }
__declspec(naked) void  FakeDriverCallback() { _asm { jmp[winmm.DriverCallback] } }
__declspec(naked) void  FakeDrvGetModuleHandle() { _asm { jmp[winmm.DrvGetModuleHandle] } }
__declspec(naked) void  FakeGetDriverModuleHandle() { _asm { jmp[winmm.GetDriverModuleHandle] } }
__declspec(naked) void  FakeNotifyCallbackData() { _asm { jmp[winmm.NotifyCallbackData] } }
__declspec(naked) void  FakeOpenDriver() { _asm { jmp[winmm.OpenDriver] } }
__declspec(naked) void  FakePlaySound() { _asm { jmp[winmm.PlaySound] } }
__declspec(naked) void  FakePlaySoundA() { _asm { jmp[winmm.PlaySoundA] } }
__declspec(naked) void  FakePlaySoundW() { _asm { jmp[winmm.PlaySoundW] } }
__declspec(naked) void  FakeSendDriverMessage() { _asm { jmp[winmm.SendDriverMessage] } }
__declspec(naked) void  FakeWOW32DriverCallback() { _asm { jmp[winmm.WOW32DriverCallback] } }
__declspec(naked) void  FakeWOW32ResolveMultiMediaHandle() { _asm { jmp[winmm.WOW32ResolveMultiMediaHandle] } }
__declspec(naked) void  FakeWOWAppExit() { _asm { jmp[winmm.WOWAppExit] } }
__declspec(naked) void  Fakeaux32Message() { _asm { jmp[winmm.aux32Message] } }
__declspec(naked) void  FakeauxGetDevCapsA() { _asm { jmp[winmm.auxGetDevCapsA] } }
__declspec(naked) void  FakeauxGetDevCapsW() { _asm { jmp[winmm.auxGetDevCapsW] } }
__declspec(naked) void  FakeauxGetNumDevs() { _asm { jmp[winmm.auxGetNumDevs] } }
__declspec(naked) void  FakeauxGetVolume() { _asm { jmp[winmm.auxGetVolume] } }
__declspec(naked) void  FakeauxOutMessage() { _asm { jmp[winmm.auxOutMessage] } }
__declspec(naked) void  FakeauxSetVolume() { _asm { jmp[winmm.auxSetVolume] } }
__declspec(naked) void  Fakejoy32Message() { _asm { jmp[winmm.joy32Message] } }
__declspec(naked) void  FakejoyConfigChanged() { _asm { jmp[winmm.joyConfigChanged] } }
__declspec(naked) void  FakejoyGetDevCapsA() { _asm { jmp[winmm.joyGetDevCapsA] } }
__declspec(naked) void  FakejoyGetDevCapsW() { _asm { jmp[winmm.joyGetDevCapsW] } }
__declspec(naked) void  FakejoyGetNumDevs() { _asm { jmp[winmm.joyGetNumDevs] } }
__declspec(naked) void  FakejoyGetPos() { _asm { jmp[winmm.joyGetPos] } }
__declspec(naked) void  FakejoyGetPosEx() { _asm { jmp[winmm.joyGetPosEx] } }
__declspec(naked) void  FakejoyGetThreshold() { _asm { jmp[winmm.joyGetThreshold] } }
__declspec(naked) void  FakejoyReleaseCapture() { _asm { jmp[winmm.joyReleaseCapture] } }
__declspec(naked) void  FakejoySetCapture() { _asm { jmp[winmm.joySetCapture] } }
__declspec(naked) void  FakejoySetThreshold() { _asm { jmp[winmm.joySetThreshold] } }
__declspec(naked) void  Fakemci32Message() { _asm { jmp[winmm.mci32Message] } }
__declspec(naked) void  FakemciDriverNotify() { _asm { jmp[winmm.mciDriverNotify] } }
__declspec(naked) void  FakemciDriverYield() { _asm { jmp[winmm.mciDriverYield] } }
__declspec(naked) void  FakemciExecute() { _asm { jmp[winmm.mciExecute] } }
__declspec(naked) void  FakemciFreeCommandResource() { _asm { jmp[winmm.mciFreeCommandResource] } }
__declspec(naked) void  FakemciGetCreatorTask() { _asm { jmp[winmm.mciGetCreatorTask] } }
__declspec(naked) void  FakemciGetDeviceIDA() { _asm { jmp[winmm.mciGetDeviceIDA] } }
__declspec(naked) void  FakemciGetDeviceIDFromElementIDA() { _asm { jmp[winmm.mciGetDeviceIDFromElementIDA] } }
__declspec(naked) void  FakemciGetDeviceIDFromElementIDW() { _asm { jmp[winmm.mciGetDeviceIDFromElementIDW] } }
__declspec(naked) void  FakemciGetDeviceIDW() { _asm { jmp[winmm.mciGetDeviceIDW] } }
__declspec(naked) void  FakemciGetDriverData() { _asm { jmp[winmm.mciGetDriverData] } }
__declspec(naked) void  FakemciGetErrorStringA() { _asm { jmp[winmm.mciGetErrorStringA] } }
__declspec(naked) void  FakemciGetErrorStringW() { _asm { jmp[winmm.mciGetErrorStringW] } }
__declspec(naked) void  FakemciGetYieldProc() { _asm { jmp[winmm.mciGetYieldProc] } }
__declspec(naked) void  FakemciLoadCommandResource() { _asm { jmp[winmm.mciLoadCommandResource] } }
__declspec(naked) void  FakemciSendCommandA() { _asm { jmp[winmm.mciSendCommandA] } }
__declspec(naked) void  FakemciSendCommandW() { _asm { jmp[winmm.mciSendCommandW] } }
__declspec(naked) void  FakemciSendStringA() { _asm { jmp[winmm.mciSendStringA] } }
__declspec(naked) void  FakemciSendStringW() { _asm { jmp[winmm.mciSendStringW] } }
__declspec(naked) void  FakemciSetDriverData() { _asm { jmp[winmm.mciSetDriverData] } }
__declspec(naked) void  FakemciSetYieldProc() { _asm { jmp[winmm.mciSetYieldProc] } }
__declspec(naked) void  Fakemid32Message() { _asm { jmp[winmm.mid32Message] } }
__declspec(naked) void  FakemidiConnect() { _asm { jmp[winmm.midiConnect] } }
__declspec(naked) void  FakemidiDisconnect() { _asm { jmp[winmm.midiDisconnect] } }
__declspec(naked) void  FakemidiInAddBuffer() { _asm { jmp[winmm.midiInAddBuffer] } }
__declspec(naked) void  FakemidiInClose() { _asm { jmp[winmm.midiInClose] } }
__declspec(naked) void  FakemidiInGetDevCapsA() { _asm { jmp[winmm.midiInGetDevCapsA] } }
__declspec(naked) void  FakemidiInGetDevCapsW() { _asm { jmp[winmm.midiInGetDevCapsW] } }
__declspec(naked) void  FakemidiInGetErrorTextA() { _asm { jmp[winmm.midiInGetErrorTextA] } }
__declspec(naked) void  FakemidiInGetErrorTextW() { _asm { jmp[winmm.midiInGetErrorTextW] } }
__declspec(naked) void  FakemidiInGetID() { _asm { jmp[winmm.midiInGetID] } }
__declspec(naked) void  FakemidiInGetNumDevs() { _asm { jmp[winmm.midiInGetNumDevs] } }
__declspec(naked) void  FakemidiInMessage() { _asm { jmp[winmm.midiInMessage] } }
__declspec(naked) void  FakemidiInOpen() { _asm { jmp[winmm.midiInOpen] } }
__declspec(naked) void  FakemidiInPrepareHeader() { _asm { jmp[winmm.midiInPrepareHeader] } }
__declspec(naked) void  FakemidiInReset() { _asm { jmp[winmm.midiInReset] } }
__declspec(naked) void  FakemidiInStart() { _asm { jmp[winmm.midiInStart] } }
__declspec(naked) void  FakemidiInStop() { _asm { jmp[winmm.midiInStop] } }
__declspec(naked) void  FakemidiInUnprepareHeader() { _asm { jmp[winmm.midiInUnprepareHeader] } }
__declspec(naked) void  FakemidiOutCacheDrumPatches() { _asm { jmp[winmm.midiOutCacheDrumPatches] } }
__declspec(naked) void  FakemidiOutCachePatches() { _asm { jmp[winmm.midiOutCachePatches] } }
__declspec(naked) void  FakemidiOutClose() { _asm { jmp[winmm.midiOutClose] } }
__declspec(naked) void  FakemidiOutGetDevCapsA() { _asm { jmp[winmm.midiOutGetDevCapsA] } }
__declspec(naked) void  FakemidiOutGetDevCapsW() { _asm { jmp[winmm.midiOutGetDevCapsW] } }
__declspec(naked) void  FakemidiOutGetErrorTextA() { _asm { jmp[winmm.midiOutGetErrorTextA] } }
__declspec(naked) void  FakemidiOutGetErrorTextW() { _asm { jmp[winmm.midiOutGetErrorTextW] } }
__declspec(naked) void  FakemidiOutGetID() { _asm { jmp[winmm.midiOutGetID] } }
__declspec(naked) void  FakemidiOutGetNumDevs() { _asm { jmp[winmm.midiOutGetNumDevs] } }
__declspec(naked) void  FakemidiOutGetVolume() { _asm { jmp[winmm.midiOutGetVolume] } }
__declspec(naked) void  FakemidiOutLongMsg() { _asm { jmp[winmm.midiOutLongMsg] } }
__declspec(naked) void  FakemidiOutMessage() { _asm { jmp[winmm.midiOutMessage] } }
__declspec(naked) void  FakemidiOutOpen() { _asm { jmp[winmm.midiOutOpen] } }
__declspec(naked) void  FakemidiOutPrepareHeader() { _asm { jmp[winmm.midiOutPrepareHeader] } }
__declspec(naked) void  FakemidiOutReset() { _asm { jmp[winmm.midiOutReset] } }
__declspec(naked) void  FakemidiOutSetVolume() { _asm { jmp[winmm.midiOutSetVolume] } }
__declspec(naked) void  FakemidiOutShortMsg() { _asm { jmp[winmm.midiOutShortMsg] } }
__declspec(naked) void  FakemidiOutUnprepareHeader() { _asm { jmp[winmm.midiOutUnprepareHeader] } }
__declspec(naked) void  FakemidiStreamClose() { _asm { jmp[winmm.midiStreamClose] } }
__declspec(naked) void  FakemidiStreamOpen() { _asm { jmp[winmm.midiStreamOpen] } }
__declspec(naked) void  FakemidiStreamOut() { _asm { jmp[winmm.midiStreamOut] } }
__declspec(naked) void  FakemidiStreamPause() { _asm { jmp[winmm.midiStreamPause] } }
__declspec(naked) void  FakemidiStreamPosition() { _asm { jmp[winmm.midiStreamPosition] } }
__declspec(naked) void  FakemidiStreamProperty() { _asm { jmp[winmm.midiStreamProperty] } }
__declspec(naked) void  FakemidiStreamRestart() { _asm { jmp[winmm.midiStreamRestart] } }
__declspec(naked) void  FakemidiStreamStop() { _asm { jmp[winmm.midiStreamStop] } }
__declspec(naked) void  FakemixerClose() { _asm { jmp[winmm.mixerClose] } }
__declspec(naked) void  FakemixerGetControlDetailsA() { _asm { jmp[winmm.mixerGetControlDetailsA] } }
__declspec(naked) void  FakemixerGetControlDetailsW() { _asm { jmp[winmm.mixerGetControlDetailsW] } }
__declspec(naked) void  FakemixerGetDevCapsA() { _asm { jmp[winmm.mixerGetDevCapsA] } }
__declspec(naked) void  FakemixerGetDevCapsW() { _asm { jmp[winmm.mixerGetDevCapsW] } }
__declspec(naked) void  FakemixerGetID() { _asm { jmp[winmm.mixerGetID] } }
__declspec(naked) void  FakemixerGetLineControlsA() { _asm { jmp[winmm.mixerGetLineControlsA] } }
__declspec(naked) void  FakemixerGetLineControlsW() { _asm { jmp[winmm.mixerGetLineControlsW] } }
__declspec(naked) void  FakemixerGetLineInfoA() { _asm { jmp[winmm.mixerGetLineInfoA] } }
__declspec(naked) void  FakemixerGetLineInfoW() { _asm { jmp[winmm.mixerGetLineInfoW] } }
__declspec(naked) void  FakemixerGetNumDevs() { _asm { jmp[winmm.mixerGetNumDevs] } }
__declspec(naked) void  FakemixerMessage() { _asm { jmp[winmm.mixerMessage] } }
__declspec(naked) void  FakemixerOpen() { _asm { jmp[winmm.mixerOpen] } }
__declspec(naked) void  FakemixerSetControlDetails() { _asm { jmp[winmm.mixerSetControlDetails] } }
__declspec(naked) void  FakemmDrvInstall() { _asm { jmp[winmm.mmDrvInstall] } }
__declspec(naked) void  FakemmGetCurrentTask() { _asm { jmp[winmm.mmGetCurrentTask] } }
__declspec(naked) void  FakemmTaskBlock() { _asm { jmp[winmm.mmTaskBlock] } }
__declspec(naked) void  FakemmTaskCreate() { _asm { jmp[winmm.mmTaskCreate] } }
__declspec(naked) void  FakemmTaskSignal() { _asm { jmp[winmm.mmTaskSignal] } }
__declspec(naked) void  FakemmTaskYield() { _asm { jmp[winmm.mmTaskYield] } }
__declspec(naked) void  FakemmioAdvance() { _asm { jmp[winmm.mmioAdvance] } }
__declspec(naked) void  FakemmioAscend() { _asm { jmp[winmm.mmioAscend] } }
__declspec(naked) void  FakemmioClose() { _asm { jmp[winmm.mmioClose] } }
__declspec(naked) void  FakemmioCreateChunk() { _asm { jmp[winmm.mmioCreateChunk] } }
__declspec(naked) void  FakemmioDescend() { _asm { jmp[winmm.mmioDescend] } }
__declspec(naked) void  FakemmioFlush() { _asm { jmp[winmm.mmioFlush] } }
__declspec(naked) void  FakemmioGetInfo() { _asm { jmp[winmm.mmioGetInfo] } }
__declspec(naked) void  FakemmioInstallIOProcA() { _asm { jmp[winmm.mmioInstallIOProcA] } }
__declspec(naked) void  FakemmioInstallIOProcW() { _asm { jmp[winmm.mmioInstallIOProcW] } }
__declspec(naked) void  FakemmioOpenA() { _asm { jmp[winmm.mmioOpenA] } }
__declspec(naked) void  FakemmioOpenW() { _asm { jmp[winmm.mmioOpenW] } }
__declspec(naked) void  FakemmioRead() { _asm { jmp[winmm.mmioRead] } }
__declspec(naked) void  FakemmioRenameA() { _asm { jmp[winmm.mmioRenameA] } }
__declspec(naked) void  FakemmioRenameW() { _asm { jmp[winmm.mmioRenameW] } }
__declspec(naked) void  FakemmioSeek() { _asm { jmp[winmm.mmioSeek] } }
__declspec(naked) void  FakemmioSendMessage() { _asm { jmp[winmm.mmioSendMessage] } }
__declspec(naked) void  FakemmioSetBuffer() { _asm { jmp[winmm.mmioSetBuffer] } }
__declspec(naked) void  FakemmioSetInfo() { _asm { jmp[winmm.mmioSetInfo] } }
__declspec(naked) void  FakemmioStringToFOURCCA() { _asm { jmp[winmm.mmioStringToFOURCCA] } }
__declspec(naked) void  FakemmioStringToFOURCCW() { _asm { jmp[winmm.mmioStringToFOURCCW] } }
__declspec(naked) void  FakemmioWrite() { _asm { jmp[winmm.mmioWrite] } }
__declspec(naked) void  FakemmsystemGetVersion() { _asm { jmp[winmm.mmsystemGetVersion] } }
__declspec(naked) void  Fakemod32Message() { _asm { jmp[winmm.mod32Message] } }
__declspec(naked) void  Fakemxd32Message() { _asm { jmp[winmm.mxd32Message] } }
__declspec(naked) void  FakesndPlaySoundA() { _asm { jmp[winmm.sndPlaySoundA] } }
__declspec(naked) void  FakesndPlaySoundW() { _asm { jmp[winmm.sndPlaySoundW] } }
__declspec(naked) void  Faketid32Message() { _asm { jmp[winmm.tid32Message] } }
__declspec(naked) void  FaketimeBeginPeriod() { _asm { jmp[winmm.timeBeginPeriod] } }
__declspec(naked) void  FaketimeEndPeriod() { _asm { jmp[winmm.timeEndPeriod] } }
__declspec(naked) void  FaketimeGetDevCaps() { _asm { jmp[winmm.timeGetDevCaps] } }
__declspec(naked) void  FaketimeGetSystemTime() { _asm { jmp[winmm.timeGetSystemTime] } }
__declspec(naked) void  FaketimeGetTime() { _asm { jmp[winmm.timeGetTime] } }
__declspec(naked) void  FaketimeKillEvent() { _asm { jmp[winmm.timeKillEvent] } }
__declspec(naked) void  FaketimeSetEvent() { _asm { jmp[winmm.timeSetEvent] } }
__declspec(naked) void  FakewaveInAddBuffer() { _asm { jmp[winmm.waveInAddBuffer] } }
__declspec(naked) void  FakewaveInClose() { _asm { jmp[winmm.waveInClose] } }
__declspec(naked) void  FakewaveInGetDevCapsA() { _asm { jmp[winmm.waveInGetDevCapsA] } }
__declspec(naked) void  FakewaveInGetDevCapsW() { _asm { jmp[winmm.waveInGetDevCapsW] } }
__declspec(naked) void  FakewaveInGetErrorTextA() { _asm { jmp[winmm.waveInGetErrorTextA] } }
__declspec(naked) void  FakewaveInGetErrorTextW() { _asm { jmp[winmm.waveInGetErrorTextW] } }
__declspec(naked) void  FakewaveInGetID() { _asm { jmp[winmm.waveInGetID] } }
__declspec(naked) void  FakewaveInGetNumDevs() { _asm { jmp[winmm.waveInGetNumDevs] } }
__declspec(naked) void  FakewaveInGetPosition() { _asm { jmp[winmm.waveInGetPosition] } }
__declspec(naked) void  FakewaveInMessage() { _asm { jmp[winmm.waveInMessage] } }
__declspec(naked) void  FakewaveInOpen() { _asm { jmp[winmm.waveInOpen] } }
__declspec(naked) void  FakewaveInPrepareHeader() { _asm { jmp[winmm.waveInPrepareHeader] } }
__declspec(naked) void  FakewaveInReset() { _asm { jmp[winmm.waveInReset] } }
__declspec(naked) void  FakewaveInStart() { _asm { jmp[winmm.waveInStart] } }
__declspec(naked) void  FakewaveInStop() { _asm { jmp[winmm.waveInStop] } }
__declspec(naked) void  FakewaveInUnprepareHeader() { _asm { jmp[winmm.waveInUnprepareHeader] } }
__declspec(naked) void  FakewaveOutBreakLoop() { _asm { jmp[winmm.waveOutBreakLoop] } }
__declspec(naked) void  FakewaveOutClose() { _asm { jmp[winmm.waveOutClose] } }
__declspec(naked) void  FakewaveOutGetDevCapsA() { _asm { jmp[winmm.waveOutGetDevCapsA] } }
__declspec(naked) void  FakewaveOutGetDevCapsW() { _asm { jmp[winmm.waveOutGetDevCapsW] } }
__declspec(naked) void  FakewaveOutGetErrorTextA() { _asm { jmp[winmm.waveOutGetErrorTextA] } }
__declspec(naked) void  FakewaveOutGetErrorTextW() { _asm { jmp[winmm.waveOutGetErrorTextW] } }
__declspec(naked) void  FakewaveOutGetID() { _asm { jmp[winmm.waveOutGetID] } }
__declspec(naked) void  FakewaveOutGetNumDevs() { _asm { jmp[winmm.waveOutGetNumDevs] } }
__declspec(naked) void  FakewaveOutGetPitch() { _asm { jmp[winmm.waveOutGetPitch] } }
__declspec(naked) void  FakewaveOutGetPlaybackRate() { _asm { jmp[winmm.waveOutGetPlaybackRate] } }
__declspec(naked) void  FakewaveOutGetPosition() { _asm { jmp[winmm.waveOutGetPosition] } }
__declspec(naked) void  FakewaveOutGetVolume() { _asm { jmp[winmm.waveOutGetVolume] } }
__declspec(naked) void  FakewaveOutMessage() { _asm { jmp[winmm.waveOutMessage] } }
__declspec(naked) void  FakewaveOutOpen() { _asm { jmp[winmm.waveOutOpen] } }
__declspec(naked) void  FakewaveOutPause() { _asm { jmp[winmm.waveOutPause] } }
__declspec(naked) void  FakewaveOutPrepareHeader() { _asm { jmp[winmm.waveOutPrepareHeader] } }
__declspec(naked) void  FakewaveOutReset() { _asm { jmp[winmm.waveOutReset] } }
__declspec(naked) void  FakewaveOutRestart() { _asm { jmp[winmm.waveOutRestart] } }
__declspec(naked) void  FakewaveOutSetPitch() { _asm { jmp[winmm.waveOutSetPitch] } }
__declspec(naked) void  FakewaveOutSetPlaybackRate() { _asm { jmp[winmm.waveOutSetPlaybackRate] } }
__declspec(naked) void  FakewaveOutSetVolume() { _asm { jmp[winmm.waveOutSetVolume] } }
__declspec(naked) void  FakewaveOutUnprepareHeader() { _asm { jmp[winmm.waveOutUnprepareHeader] } }
__declspec(naked) void  FakewaveOutWrite() { _asm { jmp[winmm.waveOutWrite] } }
__declspec(naked) void  Fakewid32Message() { _asm { jmp[winmm.wid32Message] } }
__declspec(naked) void  FakewinmmDbgOut() { _asm { jmp[winmm.winmmDbgOut] } }
__declspec(naked) void  FakewinmmSetDebugLevel() { _asm { jmp[winmm.winmmSetDebugLevel] } }
__declspec(naked) void  Fakewod32Message() { _asm { jmp[winmm.wod32Message] } }

void LoadWinmm()
{
	// Load real dll
	winmm.dll = LoadDll(dtype.winmm);

	// Load dll functions
	if (winmm.dll)
	{
		winmm.CloseDriver = GetFunctionAddress(winmm.dll, "CloseDriver", jmpaddr);
		winmm.DefDriverProc = GetFunctionAddress(winmm.dll, "DefDriverProc", jmpaddr);
		winmm.DriverCallback = GetFunctionAddress(winmm.dll, "DriverCallback", jmpaddr);
		winmm.DrvGetModuleHandle = GetFunctionAddress(winmm.dll, "DrvGetModuleHandle", jmpaddr);
		winmm.GetDriverModuleHandle = GetFunctionAddress(winmm.dll, "GetDriverModuleHandle", jmpaddr);
		winmm.NotifyCallbackData = GetFunctionAddress(winmm.dll, "NotifyCallbackData", jmpaddr);
		winmm.OpenDriver = GetFunctionAddress(winmm.dll, "OpenDriver", jmpaddr);
		winmm.PlaySound = GetFunctionAddress(winmm.dll, "PlaySound", jmpaddr);
		winmm.PlaySoundA = GetFunctionAddress(winmm.dll, "PlaySoundA", jmpaddr);
		winmm.PlaySoundW = GetFunctionAddress(winmm.dll, "PlaySoundW", jmpaddr);
		winmm.SendDriverMessage = GetFunctionAddress(winmm.dll, "SendDriverMessage", jmpaddr);
		winmm.WOW32DriverCallback = GetFunctionAddress(winmm.dll, "WOW32DriverCallback", jmpaddr);
		winmm.WOW32ResolveMultiMediaHandle = GetFunctionAddress(winmm.dll, "WOW32ResolveMultiMediaHandle", jmpaddr);
		winmm.WOWAppExit = GetFunctionAddress(winmm.dll, "WOWAppExit", jmpaddr);
		winmm.aux32Message = GetFunctionAddress(winmm.dll, "aux32Message", jmpaddr);
		winmm.auxGetDevCapsA = GetFunctionAddress(winmm.dll, "auxGetDevCapsA", jmpaddr);
		winmm.auxGetDevCapsW = GetFunctionAddress(winmm.dll, "auxGetDevCapsW", jmpaddr);
		winmm.auxGetNumDevs = GetFunctionAddress(winmm.dll, "auxGetNumDevs", jmpaddr);
		winmm.auxGetVolume = GetFunctionAddress(winmm.dll, "auxGetVolume", jmpaddr);
		winmm.auxOutMessage = GetFunctionAddress(winmm.dll, "auxOutMessage", jmpaddr);
		winmm.auxSetVolume = GetFunctionAddress(winmm.dll, "auxSetVolume", jmpaddr);
		winmm.joy32Message = GetFunctionAddress(winmm.dll, "joy32Message", jmpaddr);
		winmm.joyConfigChanged = GetFunctionAddress(winmm.dll, "joyConfigChanged", jmpaddr);
		winmm.joyGetDevCapsA = GetFunctionAddress(winmm.dll, "joyGetDevCapsA", jmpaddr);
		winmm.joyGetDevCapsW = GetFunctionAddress(winmm.dll, "joyGetDevCapsW", jmpaddr);
		winmm.joyGetNumDevs = GetFunctionAddress(winmm.dll, "joyGetNumDevs", jmpaddr);
		winmm.joyGetPos = GetFunctionAddress(winmm.dll, "joyGetPos", jmpaddr);
		winmm.joyGetPosEx = GetFunctionAddress(winmm.dll, "joyGetPosEx", jmpaddr);
		winmm.joyGetThreshold = GetFunctionAddress(winmm.dll, "joyGetThreshold", jmpaddr);
		winmm.joyReleaseCapture = GetFunctionAddress(winmm.dll, "joyReleaseCapture", jmpaddr);
		winmm.joySetCapture = GetFunctionAddress(winmm.dll, "joySetCapture", jmpaddr);
		winmm.joySetThreshold = GetFunctionAddress(winmm.dll, "joySetThreshold", jmpaddr);
		winmm.mci32Message = GetFunctionAddress(winmm.dll, "mci32Message", jmpaddr);
		winmm.mciDriverNotify = GetFunctionAddress(winmm.dll, "mciDriverNotify", jmpaddr);
		winmm.mciDriverYield = GetFunctionAddress(winmm.dll, "mciDriverYield", jmpaddr);
		winmm.mciExecute = GetFunctionAddress(winmm.dll, "mciExecute", jmpaddr);
		winmm.mciFreeCommandResource = GetFunctionAddress(winmm.dll, "mciFreeCommandResource", jmpaddr);
		winmm.mciGetCreatorTask = GetFunctionAddress(winmm.dll, "mciGetCreatorTask", jmpaddr);
		winmm.mciGetDeviceIDA = GetFunctionAddress(winmm.dll, "mciGetDeviceIDA", jmpaddr);
		winmm.mciGetDeviceIDFromElementIDA = GetFunctionAddress(winmm.dll, "mciGetDeviceIDFromElementIDA", jmpaddr);
		winmm.mciGetDeviceIDFromElementIDW = GetFunctionAddress(winmm.dll, "mciGetDeviceIDFromElementIDW", jmpaddr);
		winmm.mciGetDeviceIDW = GetFunctionAddress(winmm.dll, "mciGetDeviceIDW", jmpaddr);
		winmm.mciGetDriverData = GetFunctionAddress(winmm.dll, "mciGetDriverData", jmpaddr);
		winmm.mciGetErrorStringA = GetFunctionAddress(winmm.dll, "mciGetErrorStringA", jmpaddr);
		winmm.mciGetErrorStringW = GetFunctionAddress(winmm.dll, "mciGetErrorStringW", jmpaddr);
		winmm.mciGetYieldProc = GetFunctionAddress(winmm.dll, "mciGetYieldProc", jmpaddr);
		winmm.mciLoadCommandResource = GetFunctionAddress(winmm.dll, "mciLoadCommandResource", jmpaddr);
		winmm.mciSendCommandA = GetFunctionAddress(winmm.dll, "mciSendCommandA", jmpaddr);
		winmm.mciSendCommandW = GetFunctionAddress(winmm.dll, "mciSendCommandW", jmpaddr);
		winmm.mciSendStringA = GetFunctionAddress(winmm.dll, "mciSendStringA", jmpaddr);
		winmm.mciSendStringW = GetFunctionAddress(winmm.dll, "mciSendStringW", jmpaddr);
		winmm.mciSetDriverData = GetFunctionAddress(winmm.dll, "mciSetDriverData", jmpaddr);
		winmm.mciSetYieldProc = GetFunctionAddress(winmm.dll, "mciSetYieldProc", jmpaddr);
		winmm.mid32Message = GetFunctionAddress(winmm.dll, "mid32Message", jmpaddr);
		winmm.midiConnect = GetFunctionAddress(winmm.dll, "midiConnect", jmpaddr);
		winmm.midiDisconnect = GetFunctionAddress(winmm.dll, "midiDisconnect", jmpaddr);
		winmm.midiInAddBuffer = GetFunctionAddress(winmm.dll, "midiInAddBuffer", jmpaddr);
		winmm.midiInClose = GetFunctionAddress(winmm.dll, "midiInClose", jmpaddr);
		winmm.midiInGetDevCapsA = GetFunctionAddress(winmm.dll, "midiInGetDevCapsA", jmpaddr);
		winmm.midiInGetDevCapsW = GetFunctionAddress(winmm.dll, "midiInGetDevCapsW", jmpaddr);
		winmm.midiInGetErrorTextA = GetFunctionAddress(winmm.dll, "midiInGetErrorTextA", jmpaddr);
		winmm.midiInGetErrorTextW = GetFunctionAddress(winmm.dll, "midiInGetErrorTextW", jmpaddr);
		winmm.midiInGetID = GetFunctionAddress(winmm.dll, "midiInGetID", jmpaddr);
		winmm.midiInGetNumDevs = GetFunctionAddress(winmm.dll, "midiInGetNumDevs", jmpaddr);
		winmm.midiInMessage = GetFunctionAddress(winmm.dll, "midiInMessage", jmpaddr);
		winmm.midiInOpen = GetFunctionAddress(winmm.dll, "midiInOpen", jmpaddr);
		winmm.midiInPrepareHeader = GetFunctionAddress(winmm.dll, "midiInPrepareHeader", jmpaddr);
		winmm.midiInReset = GetFunctionAddress(winmm.dll, "midiInReset", jmpaddr);
		winmm.midiInStart = GetFunctionAddress(winmm.dll, "midiInStart", jmpaddr);
		winmm.midiInStop = GetFunctionAddress(winmm.dll, "midiInStop", jmpaddr);
		winmm.midiInUnprepareHeader = GetFunctionAddress(winmm.dll, "midiInUnprepareHeader", jmpaddr);
		winmm.midiOutCacheDrumPatches = GetFunctionAddress(winmm.dll, "midiOutCacheDrumPatches", jmpaddr);
		winmm.midiOutCachePatches = GetFunctionAddress(winmm.dll, "midiOutCachePatches", jmpaddr);
		winmm.midiOutClose = GetFunctionAddress(winmm.dll, "midiOutClose", jmpaddr);
		winmm.midiOutGetDevCapsA = GetFunctionAddress(winmm.dll, "midiOutGetDevCapsA", jmpaddr);
		winmm.midiOutGetDevCapsW = GetFunctionAddress(winmm.dll, "midiOutGetDevCapsW", jmpaddr);
		winmm.midiOutGetErrorTextA = GetFunctionAddress(winmm.dll, "midiOutGetErrorTextA", jmpaddr);
		winmm.midiOutGetErrorTextW = GetFunctionAddress(winmm.dll, "midiOutGetErrorTextW", jmpaddr);
		winmm.midiOutGetID = GetFunctionAddress(winmm.dll, "midiOutGetID", jmpaddr);
		winmm.midiOutGetNumDevs = GetFunctionAddress(winmm.dll, "midiOutGetNumDevs", jmpaddr);
		winmm.midiOutGetVolume = GetFunctionAddress(winmm.dll, "midiOutGetVolume", jmpaddr);
		winmm.midiOutLongMsg = GetFunctionAddress(winmm.dll, "midiOutLongMsg", jmpaddr);
		winmm.midiOutMessage = GetFunctionAddress(winmm.dll, "midiOutMessage", jmpaddr);
		winmm.midiOutOpen = GetFunctionAddress(winmm.dll, "midiOutOpen", jmpaddr);
		winmm.midiOutPrepareHeader = GetFunctionAddress(winmm.dll, "midiOutPrepareHeader", jmpaddr);
		winmm.midiOutReset = GetFunctionAddress(winmm.dll, "midiOutReset", jmpaddr);
		winmm.midiOutSetVolume = GetFunctionAddress(winmm.dll, "midiOutSetVolume", jmpaddr);
		winmm.midiOutShortMsg = GetFunctionAddress(winmm.dll, "midiOutShortMsg", jmpaddr);
		winmm.midiOutUnprepareHeader = GetFunctionAddress(winmm.dll, "midiOutUnprepareHeader", jmpaddr);
		winmm.midiStreamClose = GetFunctionAddress(winmm.dll, "midiStreamClose", jmpaddr);
		winmm.midiStreamOpen = GetFunctionAddress(winmm.dll, "midiStreamOpen", jmpaddr);
		winmm.midiStreamOut = GetFunctionAddress(winmm.dll, "midiStreamOut", jmpaddr);
		winmm.midiStreamPause = GetFunctionAddress(winmm.dll, "midiStreamPause", jmpaddr);
		winmm.midiStreamPosition = GetFunctionAddress(winmm.dll, "midiStreamPosition", jmpaddr);
		winmm.midiStreamProperty = GetFunctionAddress(winmm.dll, "midiStreamProperty", jmpaddr);
		winmm.midiStreamRestart = GetFunctionAddress(winmm.dll, "midiStreamRestart", jmpaddr);
		winmm.midiStreamStop = GetFunctionAddress(winmm.dll, "midiStreamStop", jmpaddr);
		winmm.mixerClose = GetFunctionAddress(winmm.dll, "mixerClose", jmpaddr);
		winmm.mixerGetControlDetailsA = GetFunctionAddress(winmm.dll, "mixerGetControlDetailsA", jmpaddr);
		winmm.mixerGetControlDetailsW = GetFunctionAddress(winmm.dll, "mixerGetControlDetailsW", jmpaddr);
		winmm.mixerGetDevCapsA = GetFunctionAddress(winmm.dll, "mixerGetDevCapsA", jmpaddr);
		winmm.mixerGetDevCapsW = GetFunctionAddress(winmm.dll, "mixerGetDevCapsW", jmpaddr);
		winmm.mixerGetID = GetFunctionAddress(winmm.dll, "mixerGetID", jmpaddr);
		winmm.mixerGetLineControlsA = GetFunctionAddress(winmm.dll, "mixerGetLineControlsA", jmpaddr);
		winmm.mixerGetLineControlsW = GetFunctionAddress(winmm.dll, "mixerGetLineControlsW", jmpaddr);
		winmm.mixerGetLineInfoA = GetFunctionAddress(winmm.dll, "mixerGetLineInfoA", jmpaddr);
		winmm.mixerGetLineInfoW = GetFunctionAddress(winmm.dll, "mixerGetLineInfoW", jmpaddr);
		winmm.mixerGetNumDevs = GetFunctionAddress(winmm.dll, "mixerGetNumDevs", jmpaddr);
		winmm.mixerMessage = GetFunctionAddress(winmm.dll, "mixerMessage", jmpaddr);
		winmm.mixerOpen = GetFunctionAddress(winmm.dll, "mixerOpen", jmpaddr);
		winmm.mixerSetControlDetails = GetFunctionAddress(winmm.dll, "mixerSetControlDetails", jmpaddr);
		winmm.mmDrvInstall = GetFunctionAddress(winmm.dll, "mmDrvInstall", jmpaddr);
		winmm.mmGetCurrentTask = GetFunctionAddress(winmm.dll, "mmGetCurrentTask", jmpaddr);
		winmm.mmTaskBlock = GetFunctionAddress(winmm.dll, "mmTaskBlock", jmpaddr);
		winmm.mmTaskCreate = GetFunctionAddress(winmm.dll, "mmTaskCreate", jmpaddr);
		winmm.mmTaskSignal = GetFunctionAddress(winmm.dll, "mmTaskSignal", jmpaddr);
		winmm.mmTaskYield = GetFunctionAddress(winmm.dll, "mmTaskYield", jmpaddr);
		winmm.mmioAdvance = GetFunctionAddress(winmm.dll, "mmioAdvance", jmpaddr);
		winmm.mmioAscend = GetFunctionAddress(winmm.dll, "mmioAscend", jmpaddr);
		winmm.mmioClose = GetFunctionAddress(winmm.dll, "mmioClose", jmpaddr);
		winmm.mmioCreateChunk = GetFunctionAddress(winmm.dll, "mmioCreateChunk", jmpaddr);
		winmm.mmioDescend = GetFunctionAddress(winmm.dll, "mmioDescend", jmpaddr);
		winmm.mmioFlush = GetFunctionAddress(winmm.dll, "mmioFlush", jmpaddr);
		winmm.mmioGetInfo = GetFunctionAddress(winmm.dll, "mmioGetInfo", jmpaddr);
		winmm.mmioInstallIOProcA = GetFunctionAddress(winmm.dll, "mmioInstallIOProcA", jmpaddr);
		winmm.mmioInstallIOProcW = GetFunctionAddress(winmm.dll, "mmioInstallIOProcW", jmpaddr);
		winmm.mmioOpenA = GetFunctionAddress(winmm.dll, "mmioOpenA", jmpaddr);
		winmm.mmioOpenW = GetFunctionAddress(winmm.dll, "mmioOpenW", jmpaddr);
		winmm.mmioRead = GetFunctionAddress(winmm.dll, "mmioRead", jmpaddr);
		winmm.mmioRenameA = GetFunctionAddress(winmm.dll, "mmioRenameA", jmpaddr);
		winmm.mmioRenameW = GetFunctionAddress(winmm.dll, "mmioRenameW", jmpaddr);
		winmm.mmioSeek = GetFunctionAddress(winmm.dll, "mmioSeek", jmpaddr);
		winmm.mmioSendMessage = GetFunctionAddress(winmm.dll, "mmioSendMessage", jmpaddr);
		winmm.mmioSetBuffer = GetFunctionAddress(winmm.dll, "mmioSetBuffer", jmpaddr);
		winmm.mmioSetInfo = GetFunctionAddress(winmm.dll, "mmioSetInfo", jmpaddr);
		winmm.mmioStringToFOURCCA = GetFunctionAddress(winmm.dll, "mmioStringToFOURCCA", jmpaddr);
		winmm.mmioStringToFOURCCW = GetFunctionAddress(winmm.dll, "mmioStringToFOURCCW", jmpaddr);
		winmm.mmioWrite = GetFunctionAddress(winmm.dll, "mmioWrite", jmpaddr);
		winmm.mmsystemGetVersion = GetFunctionAddress(winmm.dll, "mmsystemGetVersion", jmpaddr);
		winmm.mod32Message = GetFunctionAddress(winmm.dll, "mod32Message", jmpaddr);
		winmm.mxd32Message = GetFunctionAddress(winmm.dll, "mxd32Message", jmpaddr);
		winmm.sndPlaySoundA = GetFunctionAddress(winmm.dll, "sndPlaySoundA", jmpaddr);
		winmm.sndPlaySoundW = GetFunctionAddress(winmm.dll, "sndPlaySoundW", jmpaddr);
		winmm.tid32Message = GetFunctionAddress(winmm.dll, "tid32Message", jmpaddr);
		winmm.timeBeginPeriod = GetFunctionAddress(winmm.dll, "timeBeginPeriod", jmpaddr);
		winmm.timeEndPeriod = GetFunctionAddress(winmm.dll, "timeEndPeriod", jmpaddr);
		winmm.timeGetDevCaps = GetFunctionAddress(winmm.dll, "timeGetDevCaps", jmpaddr);
		winmm.timeGetSystemTime = GetFunctionAddress(winmm.dll, "timeGetSystemTime", jmpaddr);
		winmm.timeGetTime = GetFunctionAddress(winmm.dll, "timeGetTime", jmpaddr);
		winmm.timeKillEvent = GetFunctionAddress(winmm.dll, "timeKillEvent", jmpaddr);
		winmm.timeSetEvent = GetFunctionAddress(winmm.dll, "timeSetEvent", jmpaddr);
		winmm.waveInAddBuffer = GetFunctionAddress(winmm.dll, "waveInAddBuffer", jmpaddr);
		winmm.waveInClose = GetFunctionAddress(winmm.dll, "waveInClose", jmpaddr);
		winmm.waveInGetDevCapsA = GetFunctionAddress(winmm.dll, "waveInGetDevCapsA", jmpaddr);
		winmm.waveInGetDevCapsW = GetFunctionAddress(winmm.dll, "waveInGetDevCapsW", jmpaddr);
		winmm.waveInGetErrorTextA = GetFunctionAddress(winmm.dll, "waveInGetErrorTextA", jmpaddr);
		winmm.waveInGetErrorTextW = GetFunctionAddress(winmm.dll, "waveInGetErrorTextW", jmpaddr);
		winmm.waveInGetID = GetFunctionAddress(winmm.dll, "waveInGetID", jmpaddr);
		winmm.waveInGetNumDevs = GetFunctionAddress(winmm.dll, "waveInGetNumDevs", jmpaddr);
		winmm.waveInGetPosition = GetFunctionAddress(winmm.dll, "waveInGetPosition", jmpaddr);
		winmm.waveInMessage = GetFunctionAddress(winmm.dll, "waveInMessage", jmpaddr);
		winmm.waveInOpen = GetFunctionAddress(winmm.dll, "waveInOpen", jmpaddr);
		winmm.waveInPrepareHeader = GetFunctionAddress(winmm.dll, "waveInPrepareHeader", jmpaddr);
		winmm.waveInReset = GetFunctionAddress(winmm.dll, "waveInReset", jmpaddr);
		winmm.waveInStart = GetFunctionAddress(winmm.dll, "waveInStart", jmpaddr);
		winmm.waveInStop = GetFunctionAddress(winmm.dll, "waveInStop", jmpaddr);
		winmm.waveInUnprepareHeader = GetFunctionAddress(winmm.dll, "waveInUnprepareHeader", jmpaddr);
		winmm.waveOutBreakLoop = GetFunctionAddress(winmm.dll, "waveOutBreakLoop", jmpaddr);
		winmm.waveOutClose = GetFunctionAddress(winmm.dll, "waveOutClose", jmpaddr);
		winmm.waveOutGetDevCapsA = GetFunctionAddress(winmm.dll, "waveOutGetDevCapsA", jmpaddr);
		winmm.waveOutGetDevCapsW = GetFunctionAddress(winmm.dll, "waveOutGetDevCapsW", jmpaddr);
		winmm.waveOutGetErrorTextA = GetFunctionAddress(winmm.dll, "waveOutGetErrorTextA", jmpaddr);
		winmm.waveOutGetErrorTextW = GetFunctionAddress(winmm.dll, "waveOutGetErrorTextW", jmpaddr);
		winmm.waveOutGetID = GetFunctionAddress(winmm.dll, "waveOutGetID", jmpaddr);
		winmm.waveOutGetNumDevs = GetFunctionAddress(winmm.dll, "waveOutGetNumDevs", jmpaddr);
		winmm.waveOutGetPitch = GetFunctionAddress(winmm.dll, "waveOutGetPitch", jmpaddr);
		winmm.waveOutGetPlaybackRate = GetFunctionAddress(winmm.dll, "waveOutGetPlaybackRate", jmpaddr);
		winmm.waveOutGetPosition = GetFunctionAddress(winmm.dll, "waveOutGetPosition", jmpaddr);
		winmm.waveOutGetVolume = GetFunctionAddress(winmm.dll, "waveOutGetVolume", jmpaddr);
		winmm.waveOutMessage = GetFunctionAddress(winmm.dll, "waveOutMessage", jmpaddr);
		winmm.waveOutOpen = GetFunctionAddress(winmm.dll, "waveOutOpen", jmpaddr);
		winmm.waveOutPause = GetFunctionAddress(winmm.dll, "waveOutPause", jmpaddr);
		winmm.waveOutPrepareHeader = GetFunctionAddress(winmm.dll, "waveOutPrepareHeader", jmpaddr);
		winmm.waveOutReset = GetFunctionAddress(winmm.dll, "waveOutReset", jmpaddr);
		winmm.waveOutRestart = GetFunctionAddress(winmm.dll, "waveOutRestart", jmpaddr);
		winmm.waveOutSetPitch = GetFunctionAddress(winmm.dll, "waveOutSetPitch", jmpaddr);
		winmm.waveOutSetPlaybackRate = GetFunctionAddress(winmm.dll, "waveOutSetPlaybackRate", jmpaddr);
		winmm.waveOutSetVolume = GetFunctionAddress(winmm.dll, "waveOutSetVolume", jmpaddr);
		winmm.waveOutUnprepareHeader = GetFunctionAddress(winmm.dll, "waveOutUnprepareHeader", jmpaddr);
		winmm.waveOutWrite = GetFunctionAddress(winmm.dll, "waveOutWrite", jmpaddr);
		winmm.wid32Message = GetFunctionAddress(winmm.dll, "wid32Message", jmpaddr);
		winmm.winmmDbgOut = GetFunctionAddress(winmm.dll, "winmmDbgOut", jmpaddr);
		winmm.winmmSetDebugLevel = GetFunctionAddress(winmm.dll, "winmmSetDebugLevel", jmpaddr);
		winmm.wod32Message = GetFunctionAddress(winmm.dll, "wod32Message", jmpaddr);
	}
}