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
#include "Dllmain\Dllmain.h"
#include "wrapper.h"
#include "Utils\Utils.h"
#include "Hook\inithook.h"

#define module dsound

#define VISIT_PROCS(visit) \
	visit(DirectSoundCreate) \
	visit(DirectSoundEnumerateA) \
	visit(DirectSoundEnumerateW) \
	visit(DllCanUnloadNow) \
	visit(DllGetClassObject) \
	visit(DirectSoundCaptureCreate) \
	visit(DirectSoundCaptureEnumerateA) \
	visit(DirectSoundCaptureEnumerateW) \
	visit(GetDeviceID) \
	visit(DirectSoundFullDuplexCreate) \
	visit(DirectSoundCreate8) \
	visit(DirectSoundCaptureCreate8) \

struct dsound_dll
{
	HMODULE dll = nullptr;
	VISIT_PROCS(ADD_FARPROC_MEMBER);
} dsound;

VISIT_PROCS(CREATE_PROC_STUB)

void LoadDsound()
{
	// Load real dll
	dsound.dll = LoadDll(dtype.dsound);

	// Load dll functions
	if (dsound.dll)
	{
		VISIT_PROCS(LOAD_ORIGINAL_PROC);

		// Enable DSoundCtrl functions
		if (Config.DSoundCtrl && Config.RealWrapperMode == dtype.dsound)
		{
			dsound.DirectSoundCreate = GetFunctionAddress(hModule_dll, "_DirectSoundCreate", jmpaddr);
			dsound.DirectSoundEnumerateA = GetFunctionAddress(hModule_dll, "_DirectSoundEnumerateA", jmpaddr);
			dsound.DirectSoundEnumerateW = GetFunctionAddress(hModule_dll, "_DirectSoundEnumerateW", jmpaddr);
			dsound.DllCanUnloadNow = GetFunctionAddress(hModule_dll, "_DllCanUnloadNow_DSoundCtrl", jmpaddr);
			dsound.DllGetClassObject = GetFunctionAddress(hModule_dll, "_DllGetClassObject_DSoundCtrl", jmpaddr);
			dsound.DirectSoundCaptureCreate = GetFunctionAddress(hModule_dll, "_DirectSoundCaptureCreate", jmpaddr);
			dsound.DirectSoundCaptureEnumerateA = GetFunctionAddress(hModule_dll, "_DirectSoundCaptureEnumerateA", jmpaddr);
			dsound.DirectSoundCaptureEnumerateW = GetFunctionAddress(hModule_dll, "_DirectSoundCaptureEnumerateW", jmpaddr);
			dsound.GetDeviceID = GetFunctionAddress(hModule_dll, "_GetDeviceID", jmpaddr);
			dsound.DirectSoundFullDuplexCreate = GetFunctionAddress(hModule_dll, "_DirectSoundFullDuplexCreate", jmpaddr);
			dsound.DirectSoundCreate8 = GetFunctionAddress(hModule_dll, "_DirectSoundCreate8", jmpaddr);
			dsound.DirectSoundCaptureCreate8 = GetFunctionAddress(hModule_dll, "_DirectSoundCaptureCreate8", jmpaddr);
		}

		// Hook APIs for DSoundCtrl functions
		else if (Config.DSoundCtrl)
		{
			LOG << "Hooking dsound.dll APIs...";
			dsound.DirectSoundCreate = (FARPROC)HookAPI(hModule_dll, dtypename[dtype.dsound], GetFunctionAddress(dsound.dll, "DirectSoundCreate"), "DirectSoundCreate", GetFunctionAddress(hModule_dll, "_DirectSoundCreate"));
			dsound.DirectSoundEnumerateA = (FARPROC)HookAPI(hModule_dll, dtypename[dtype.dsound], GetFunctionAddress(dsound.dll, "DirectSoundEnumerateA"), "DirectSoundEnumerateA", GetFunctionAddress(hModule_dll, "_DirectSoundEnumerateA"));
			dsound.DirectSoundEnumerateW = (FARPROC)HookAPI(hModule_dll, dtypename[dtype.dsound], GetFunctionAddress(dsound.dll, "DirectSoundEnumerateW"), "DirectSoundEnumerateW", GetFunctionAddress(hModule_dll, "_DirectSoundEnumerateW"));
			dsound.DirectSoundCaptureCreate = (FARPROC)HookAPI(hModule_dll, dtypename[dtype.dsound], GetFunctionAddress(dsound.dll, "DirectSoundCaptureCreate"), "DirectSoundCaptureCreate", GetFunctionAddress(hModule_dll, "_DirectSoundCaptureCreate"));
			dsound.DirectSoundCaptureEnumerateA = (FARPROC)HookAPI(hModule_dll, dtypename[dtype.dsound], GetFunctionAddress(dsound.dll, "DirectSoundCaptureEnumerateA"), "DirectSoundCaptureEnumerateA", GetFunctionAddress(hModule_dll, "_DirectSoundCaptureEnumerateA"));
			dsound.DirectSoundCaptureEnumerateW = (FARPROC)HookAPI(hModule_dll, dtypename[dtype.dsound], GetFunctionAddress(dsound.dll, "DirectSoundCaptureEnumerateW"), "DirectSoundCaptureEnumerateW", GetFunctionAddress(hModule_dll, "_DirectSoundCaptureEnumerateW"));
			dsound.GetDeviceID = (FARPROC)HookAPI(hModule_dll, dtypename[dtype.dsound], GetFunctionAddress(dsound.dll, "GetDeviceID"), "GetDeviceID", GetFunctionAddress(hModule_dll, "_GetDeviceID"));
			dsound.DirectSoundFullDuplexCreate = (FARPROC)HookAPI(hModule_dll, dtypename[dtype.dsound], GetFunctionAddress(dsound.dll, "DirectSoundFullDuplexCreate"), "DirectSoundFullDuplexCreate", GetFunctionAddress(hModule_dll, "_DirectSoundFullDuplexCreate"));
			dsound.DirectSoundCreate8 = (FARPROC)HookAPI(hModule_dll, dtypename[dtype.dsound], GetFunctionAddress(dsound.dll, "DirectSoundCreate8"), "DirectSoundCreate8", GetFunctionAddress(hModule_dll, "_DirectSoundCreate8"));
			dsound.DirectSoundCaptureCreate8 = (FARPROC)HookAPI(hModule_dll, dtypename[dtype.dsound], GetFunctionAddress(dsound.dll, "DirectSoundCaptureCreate8"), "DirectSoundCaptureCreate8", GetFunctionAddress(hModule_dll, "_DirectSoundCaptureCreate8"));
		}
	}
}

void Set_DllCanUnloadNow(FARPROC ProcAddress)
{
	dsound.DllCanUnloadNow = ProcAddress;
}

void Set_DllGetClassObject(FARPROC ProcAddress)
{
	dsound.DllGetClassObject = ProcAddress;
}