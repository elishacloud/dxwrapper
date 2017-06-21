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
	FARPROC DirectSoundCreate;
	FARPROC DirectSoundEnumerateA;
	FARPROC DirectSoundEnumerateW;
	FARPROC DllCanUnloadNow;
	FARPROC DllGetClassObject;
	FARPROC DirectSoundCaptureCreate;
	FARPROC DirectSoundCaptureEnumerateA;
	FARPROC DirectSoundCaptureEnumerateW;
	FARPROC GetDeviceID;
	FARPROC DirectSoundFullDuplexCreate;
	FARPROC DirectSoundCreate8;
	FARPROC DirectSoundCaptureCreate8;
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
	// Enable DSoundCtrl functions
	if (Config.DSoundCtrl && Config.RealWrapperMode == dtype.dsound)
	{
		dsound.DirectSoundCreate = GetProcAddress(hModule_dll, "_DirectSoundCreate");
		dsound.DirectSoundEnumerateA = GetProcAddress(hModule_dll, "_DirectSoundEnumerateA");
		dsound.DirectSoundEnumerateW = GetProcAddress(hModule_dll, "_DirectSoundEnumerateW");
		dsound.DllCanUnloadNow = GetProcAddress(hModule_dll, "_DllCanUnloadNow_DSoundCtrl");
		dsound.DllGetClassObject = GetProcAddress(hModule_dll, "_DllGetClassObject_DSoundCtrl");
		dsound.DirectSoundCaptureCreate = GetProcAddress(hModule_dll, "_DirectSoundCaptureCreate");
		dsound.DirectSoundCaptureEnumerateA = GetProcAddress(hModule_dll, "_DirectSoundCaptureEnumerateA");
		dsound.DirectSoundCaptureEnumerateW = GetProcAddress(hModule_dll, "_DirectSoundCaptureEnumerateW");
		dsound.GetDeviceID = GetProcAddress(hModule_dll, "_GetDeviceID");
		dsound.DirectSoundFullDuplexCreate = GetProcAddress(hModule_dll, "_DirectSoundFullDuplexCreate");
		dsound.DirectSoundCreate8 = GetProcAddress(hModule_dll, "_DirectSoundCreate8");
		dsound.DirectSoundCaptureCreate8 = GetProcAddress(hModule_dll, "_DirectSoundCaptureCreate8");
	}
	else
	{
		// Load real dll
		dsound.dll = LoadDll(dtype.dsound);

		// Overload GetProcAddress function to GetFunctionAddress
		typedef FARPROC(*GetFunctionAddress_type)(HMODULE, LPCSTR);
		GetFunctionAddress_type GetProcAddress = reinterpret_cast<GetFunctionAddress_type>(GetFunctionAddress);

		// Load dll functions
		if (dsound.dll)
		{
			dsound.DirectSoundCreate = GetProcAddress(dsound.dll, "DirectSoundCreate");
			dsound.DirectSoundEnumerateA = GetProcAddress(dsound.dll, "DirectSoundEnumerateA");
			dsound.DirectSoundEnumerateW = GetProcAddress(dsound.dll, "DirectSoundEnumerateW");
			dsound.DllCanUnloadNow = GetProcAddress(dsound.dll, "DllCanUnloadNow");
			dsound.DllGetClassObject = GetProcAddress(dsound.dll, "DllGetClassObject");
			dsound.DirectSoundCaptureCreate = GetProcAddress(dsound.dll, "DirectSoundCaptureCreate");
			dsound.DirectSoundCaptureEnumerateA = GetProcAddress(dsound.dll, "DirectSoundCaptureEnumerateA");
			dsound.DirectSoundCaptureEnumerateW = GetProcAddress(dsound.dll, "DirectSoundCaptureEnumerateW");
			dsound.GetDeviceID = GetProcAddress(dsound.dll, "GetDeviceID");
			dsound.DirectSoundFullDuplexCreate = GetProcAddress(dsound.dll, "DirectSoundFullDuplexCreate");
			dsound.DirectSoundCreate8 = GetProcAddress(dsound.dll, "DirectSoundCreate8");
			dsound.DirectSoundCaptureCreate8 = GetProcAddress(dsound.dll, "DirectSoundCaptureCreate8");

			// Hook APIs for DSoundCtrl functions
			if (Config.DSoundCtrl)
			{
				Compat::Log() << "Hooking dsound.dll APIs...";
				dsound.DirectSoundCreate = (FARPROC)HookAPI(hModule_dll, dtypename[dtype.dsound], dsound.DirectSoundCreate, "DirectSoundCreate", GetProcAddress(hModule_dll, "_DirectSoundCreate"));
				dsound.DirectSoundEnumerateA = (FARPROC)HookAPI(hModule_dll, dtypename[dtype.dsound], dsound.DirectSoundEnumerateA, "DirectSoundEnumerateA", GetProcAddress(hModule_dll, "_DirectSoundEnumerateA"));
				dsound.DirectSoundEnumerateW = (FARPROC)HookAPI(hModule_dll, dtypename[dtype.dsound], dsound.DirectSoundEnumerateW, "DirectSoundEnumerateW", GetProcAddress(hModule_dll, "_DirectSoundEnumerateW"));
				dsound.DirectSoundCaptureCreate = (FARPROC)HookAPI(hModule_dll, dtypename[dtype.dsound], dsound.DirectSoundCaptureCreate, "DirectSoundCaptureCreate", GetProcAddress(hModule_dll, "_DirectSoundCaptureCreate"));
				dsound.DirectSoundCaptureEnumerateA = (FARPROC)HookAPI(hModule_dll, dtypename[dtype.dsound], dsound.DirectSoundCaptureEnumerateA, "DirectSoundCaptureEnumerateA", GetProcAddress(hModule_dll, "_DirectSoundCaptureEnumerateA"));
				dsound.DirectSoundCaptureEnumerateW = (FARPROC)HookAPI(hModule_dll, dtypename[dtype.dsound], dsound.DirectSoundCaptureEnumerateW, "DirectSoundCaptureEnumerateW", GetProcAddress(hModule_dll, "_DirectSoundCaptureEnumerateW"));
				dsound.GetDeviceID = (FARPROC)HookAPI(hModule_dll, dtypename[dtype.dsound], dsound.GetDeviceID, "GetDeviceID", GetProcAddress(hModule_dll, "_GetDeviceID"));
				dsound.DirectSoundFullDuplexCreate = (FARPROC)HookAPI(hModule_dll, dtypename[dtype.dsound], dsound.DirectSoundFullDuplexCreate, "DirectSoundFullDuplexCreate", GetProcAddress(hModule_dll, "_DirectSoundFullDuplexCreate"));
				dsound.DirectSoundCreate8 = (FARPROC)HookAPI(hModule_dll, dtypename[dtype.dsound], dsound.DirectSoundCreate8, "DirectSoundCreate8", GetProcAddress(hModule_dll, "_DirectSoundCreate8"));
				dsound.DirectSoundCaptureCreate8 = (FARPROC)HookAPI(hModule_dll, dtypename[dtype.dsound], dsound.DirectSoundCaptureCreate8, "DirectSoundCaptureCreate8", GetProcAddress(hModule_dll, "_DirectSoundCaptureCreate8"));
			}
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