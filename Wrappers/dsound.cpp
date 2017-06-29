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

struct dsound_dll
{
	HMODULE dll = nullptr;
	FARPROC DirectSoundCreate = jmpaddr;
	FARPROC DirectSoundEnumerateA = jmpaddr;
	FARPROC DirectSoundEnumerateW = jmpaddr;
	FARPROC DllCanUnloadNow = jmpaddr;
	FARPROC DllGetClassObject = jmpaddr;
	FARPROC DirectSoundCaptureCreate = jmpaddr;
	FARPROC DirectSoundCaptureEnumerateA = jmpaddr;
	FARPROC DirectSoundCaptureEnumerateW = jmpaddr;
	FARPROC GetDeviceID = jmpaddr;
	FARPROC DirectSoundFullDuplexCreate = jmpaddr;
	FARPROC DirectSoundCreate8 = jmpaddr;
	FARPROC DirectSoundCaptureCreate8 = jmpaddr;
} dsound;

__declspec(naked) void  FakeDirectSoundCreate()				{ _asm { jmp [dsound.DirectSoundCreate] } }
__declspec(naked) void  FakeDirectSoundEnumerateA()			{ _asm { jmp [dsound.DirectSoundEnumerateA] } }
__declspec(naked) void  FakeDirectSoundEnumerateW()			{ _asm { jmp [dsound.DirectSoundEnumerateW] } }
__declspec(naked) void  FakeDllCanUnloadNow()				{ _asm { jmp [dsound.DllCanUnloadNow] } }
__declspec(naked) void  FakeDllGetClassObject()				{ _asm { jmp [dsound.DllGetClassObject] } }
__declspec(naked) void  FakeDirectSoundCaptureCreate()		{ _asm { jmp [dsound.DirectSoundCaptureCreate] } }
__declspec(naked) void  FakeDirectSoundCaptureEnumerateA()	{ _asm { jmp [dsound.DirectSoundCaptureEnumerateA] } }
__declspec(naked) void  FakeDirectSoundCaptureEnumerateW()	{ _asm { jmp [dsound.DirectSoundCaptureEnumerateW] } }
__declspec(naked) void  FakeGetDeviceID()					{ _asm { jmp [dsound.GetDeviceID] } }
__declspec(naked) void  FakeDirectSoundFullDuplexCreate()	{ _asm { jmp [dsound.DirectSoundFullDuplexCreate] } }
__declspec(naked) void  FakeDirectSoundCreate8()			{ _asm { jmp [dsound.DirectSoundCreate8] } }
__declspec(naked) void  FakeDirectSoundCaptureCreate8()		{ _asm { jmp [dsound.DirectSoundCaptureCreate8] } }

void LoadDsound()
{
	// Load real dll
	dsound.dll = LoadDll(dtype.dsound);

	// Load dll functions
	if (dsound.dll)
	{
		dsound.DirectSoundCreate = GetFunctionAddress(dsound.dll, "DirectSoundCreate", jmpaddr);
		dsound.DirectSoundEnumerateA = GetFunctionAddress(dsound.dll, "DirectSoundEnumerateA", jmpaddr);
		dsound.DirectSoundEnumerateW = GetFunctionAddress(dsound.dll, "DirectSoundEnumerateW", jmpaddr);
		dsound.DllCanUnloadNow = GetFunctionAddress(dsound.dll, "DllCanUnloadNow", jmpaddr);
		dsound.DllGetClassObject = GetFunctionAddress(dsound.dll, "DllGetClassObject", jmpaddr);
		dsound.DirectSoundCaptureCreate = GetFunctionAddress(dsound.dll, "DirectSoundCaptureCreate", jmpaddr);
		dsound.DirectSoundCaptureEnumerateA = GetFunctionAddress(dsound.dll, "DirectSoundCaptureEnumerateA", jmpaddr);
		dsound.DirectSoundCaptureEnumerateW = GetFunctionAddress(dsound.dll, "DirectSoundCaptureEnumerateW", jmpaddr);
		dsound.GetDeviceID = GetFunctionAddress(dsound.dll, "GetDeviceID", jmpaddr);
		dsound.DirectSoundFullDuplexCreate = GetFunctionAddress(dsound.dll, "DirectSoundFullDuplexCreate", jmpaddr);
		dsound.DirectSoundCreate8 = GetFunctionAddress(dsound.dll, "DirectSoundCreate8", jmpaddr);
		dsound.DirectSoundCaptureCreate8 = GetFunctionAddress(dsound.dll, "DirectSoundCaptureCreate8", jmpaddr);

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
			Compat::Log() << "Hooking dsound.dll APIs...";
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