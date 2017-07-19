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
#include "Hook\inithook.h"
#include "Utils\Utils.h"
#include "wrapper.h"
#include "dsound.h"

dsound_dll dsound;

#define module dsound

VISIT_DSOUND_PROCS(CREATE_PROC_STUB)

void dsound_dll::Load()
{
	// Load real dll
	dll = Wrapper.LoadDll(dtype.dsound);

	// Load dll functions
	if (dll)
	{
		VISIT_DSOUND_PROCS(LOAD_ORIGINAL_PROC);

		// Enable DSoundCtrl functions
		if (Config.DSoundCtrl && Config.RealWrapperMode == dtype.dsound)
		{
			DirectSoundCreate = GetFunctionAddress(hModule_dll, "_DirectSoundCreate", jmpaddr);
			DirectSoundEnumerateA = GetFunctionAddress(hModule_dll, "_DirectSoundEnumerateA", jmpaddr);
			DirectSoundEnumerateW = GetFunctionAddress(hModule_dll, "_DirectSoundEnumerateW", jmpaddr);
			DllCanUnloadNow = GetFunctionAddress(hModule_dll, "_DllCanUnloadNow_DSoundCtrl", jmpaddr);
			DllGetClassObject = GetFunctionAddress(hModule_dll, "_DllGetClassObject_DSoundCtrl", jmpaddr);
			DirectSoundCaptureCreate = GetFunctionAddress(hModule_dll, "_DirectSoundCaptureCreate", jmpaddr);
			DirectSoundCaptureEnumerateA = GetFunctionAddress(hModule_dll, "_DirectSoundCaptureEnumerateA", jmpaddr);
			DirectSoundCaptureEnumerateW = GetFunctionAddress(hModule_dll, "_DirectSoundCaptureEnumerateW", jmpaddr);
			GetDeviceID = GetFunctionAddress(hModule_dll, "_GetDeviceID", jmpaddr);
			DirectSoundFullDuplexCreate = GetFunctionAddress(hModule_dll, "_DirectSoundFullDuplexCreate", jmpaddr);
			DirectSoundCreate8 = GetFunctionAddress(hModule_dll, "_DirectSoundCreate8", jmpaddr);
			DirectSoundCaptureCreate8 = GetFunctionAddress(hModule_dll, "_DirectSoundCaptureCreate8", jmpaddr);
		}

		// Hook APIs for DSoundCtrl functions
		else if (Config.DSoundCtrl)
		{
			LOG << "Hooking dll APIs...";
			DirectSoundCreate = (FARPROC)HookAPI(hModule_dll, dtypename[dtype.dsound], GetFunctionAddress(dll, "DirectSoundCreate"), "DirectSoundCreate", GetFunctionAddress(hModule_dll, "_DirectSoundCreate"));
			DirectSoundEnumerateA = (FARPROC)HookAPI(hModule_dll, dtypename[dtype.dsound], GetFunctionAddress(dll, "DirectSoundEnumerateA"), "DirectSoundEnumerateA", GetFunctionAddress(hModule_dll, "_DirectSoundEnumerateA"));
			DirectSoundEnumerateW = (FARPROC)HookAPI(hModule_dll, dtypename[dtype.dsound], GetFunctionAddress(dll, "DirectSoundEnumerateW"), "DirectSoundEnumerateW", GetFunctionAddress(hModule_dll, "_DirectSoundEnumerateW"));
			DirectSoundCaptureCreate = (FARPROC)HookAPI(hModule_dll, dtypename[dtype.dsound], GetFunctionAddress(dll, "DirectSoundCaptureCreate"), "DirectSoundCaptureCreate", GetFunctionAddress(hModule_dll, "_DirectSoundCaptureCreate"));
			DirectSoundCaptureEnumerateA = (FARPROC)HookAPI(hModule_dll, dtypename[dtype.dsound], GetFunctionAddress(dll, "DirectSoundCaptureEnumerateA"), "DirectSoundCaptureEnumerateA", GetFunctionAddress(hModule_dll, "_DirectSoundCaptureEnumerateA"));
			DirectSoundCaptureEnumerateW = (FARPROC)HookAPI(hModule_dll, dtypename[dtype.dsound], GetFunctionAddress(dll, "DirectSoundCaptureEnumerateW"), "DirectSoundCaptureEnumerateW", GetFunctionAddress(hModule_dll, "_DirectSoundCaptureEnumerateW"));
			GetDeviceID = (FARPROC)HookAPI(hModule_dll, dtypename[dtype.dsound], GetFunctionAddress(dll, "GetDeviceID"), "GetDeviceID", GetFunctionAddress(hModule_dll, "_GetDeviceID"));
			DirectSoundFullDuplexCreate = (FARPROC)HookAPI(hModule_dll, dtypename[dtype.dsound], GetFunctionAddress(dll, "DirectSoundFullDuplexCreate"), "DirectSoundFullDuplexCreate", GetFunctionAddress(hModule_dll, "_DirectSoundFullDuplexCreate"));
			DirectSoundCreate8 = (FARPROC)HookAPI(hModule_dll, dtypename[dtype.dsound], GetFunctionAddress(dll, "DirectSoundCreate8"), "DirectSoundCreate8", GetFunctionAddress(hModule_dll, "_DirectSoundCreate8"));
			DirectSoundCaptureCreate8 = (FARPROC)HookAPI(hModule_dll, dtypename[dtype.dsound], GetFunctionAddress(dll, "DirectSoundCaptureCreate8"), "DirectSoundCaptureCreate8", GetFunctionAddress(hModule_dll, "_DirectSoundCaptureCreate8"));
		}
	}
}
