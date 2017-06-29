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
#include "dsound.h"
#include "Utils\Utils.h"
#include "Hook\inithook.h"

struct ddraw_dll
{
	HMODULE dll = nullptr;
	FARPROC AcquireDDThreadLock = jmpaddr;
	FARPROC CheckFullscreen = jmpaddr;
	FARPROC CompleteCreateSysmemSurface = jmpaddr;
	FARPROC D3DParseUnknownCommand = jmpaddr;
	FARPROC DDGetAttachedSurfaceLcl = jmpaddr;
	FARPROC DDInternalLock = jmpaddr;
	FARPROC DDInternalUnlock = jmpaddr;
	FARPROC DSoundHelp = jmpaddr;
	FARPROC DirectDrawCreate = jmpaddr;
	FARPROC DirectDrawCreateClipper = jmpaddr;
	FARPROC DirectDrawCreateEx = jmpaddr;
	FARPROC DirectDrawEnumerateA = jmpaddr;
	FARPROC DirectDrawEnumerateExA = jmpaddr;
	FARPROC DirectDrawEnumerateExW = jmpaddr;
	FARPROC DirectDrawEnumerateW = jmpaddr;
	FARPROC DllCanUnloadNow = jmpaddr;
	FARPROC DllGetClassObject = jmpaddr;
	FARPROC GetDDSurfaceLocal = jmpaddr;
	FARPROC GetOLEThunkData = jmpaddr;
	FARPROC GetSurfaceFromDC = jmpaddr;
	FARPROC RegisterSpecialCase = jmpaddr;
	FARPROC ReleaseDDThreadLock = jmpaddr;
	FARPROC SetAppCompatData = jmpaddr;
} ddraw;

__declspec(naked) void FakeAcquireDDThreadLock()			{ _asm { jmp [ddraw.AcquireDDThreadLock] } }
__declspec(naked) void FakeCheckFullscreen()				{ _asm { jmp [ddraw.CheckFullscreen] } }
__declspec(naked) void FakeCompleteCreateSysmemSurface()	{ _asm { jmp [ddraw.CompleteCreateSysmemSurface] } }
__declspec(naked) void FakeD3DParseUnknownCommand()			{ _asm { jmp [ddraw.D3DParseUnknownCommand] } }
__declspec(naked) void FakeDDGetAttachedSurfaceLcl()		{ _asm { jmp [ddraw.DDGetAttachedSurfaceLcl] } }
__declspec(naked) void FakeDDInternalLock()					{ _asm { jmp [ddraw.DDInternalLock] } }
__declspec(naked) void FakeDDInternalUnlock()				{ _asm { jmp [ddraw.DDInternalUnlock] } }
__declspec(naked) void FakeDSoundHelp()						{ _asm { jmp [ddraw.DSoundHelp] } }
__declspec(naked) void FakeDirectDrawCreate()				{ _asm { jmp [ddraw.DirectDrawCreate] } }
__declspec(naked) void FakeDirectDrawCreateClipper()		{ _asm { jmp [ddraw.DirectDrawCreateClipper] } }
__declspec(naked) void FakeDirectDrawCreateEx()				{ _asm { jmp [ddraw.DirectDrawCreateEx] } }
__declspec(naked) void FakeDirectDrawEnumerateA()			{ _asm { jmp [ddraw.DirectDrawEnumerateA] } }
__declspec(naked) void FakeDirectDrawEnumerateExA()			{ _asm { jmp [ddraw.DirectDrawEnumerateExA] } }
__declspec(naked) void FakeDirectDrawEnumerateExW()			{ _asm { jmp [ddraw.DirectDrawEnumerateExW] } }
__declspec(naked) void FakeDirectDrawEnumerateW()			{ _asm { jmp [ddraw.DirectDrawEnumerateW] } }
//__declspec(naked) void FakeDllCanUnloadNow()				{ _asm { jmp [ddraw.DllCanUnloadNow] } }		 // <---  Shared with dsound.dll
//__declspec(naked) void FakeDllGetClassObject()			{ _asm { jmp [ddraw.DllGetClassObject] } }		 // <---  Shared with dsound.dll
__declspec(naked) void FakeGetDDSurfaceLocal()				{ _asm { jmp [ddraw.GetDDSurfaceLocal] } }
__declspec(naked) void FakeGetOLEThunkData()				{ _asm { jmp [ddraw.GetOLEThunkData] } }
__declspec(naked) void FakeGetSurfaceFromDC()				{ _asm { jmp [ddraw.GetSurfaceFromDC] } }
__declspec(naked) void FakeRegisterSpecialCase()			{ _asm { jmp [ddraw.RegisterSpecialCase] } }
__declspec(naked) void FakeReleaseDDThreadLock()			{ _asm { jmp [ddraw.ReleaseDDThreadLock] } }
__declspec(naked) void FakeSetAppCompatData()				{ _asm { jmp [ddraw.SetAppCompatData] } }

void LoadDdraw()
{
	// Load real dll
	ddraw.dll = LoadDll(dtype.ddraw);

	// Load dll functions
	if (ddraw.dll)
	{
		ddraw.AcquireDDThreadLock = GetFunctionAddress(ddraw.dll, "AcquireDDThreadLock", jmpaddr);
		ddraw.CheckFullscreen = GetFunctionAddress(ddraw.dll, "CheckFullscreen", jmpaddr);
		ddraw.CompleteCreateSysmemSurface = GetFunctionAddress(ddraw.dll, "CompleteCreateSysmemSurface", jmpaddr);
		ddraw.D3DParseUnknownCommand = GetFunctionAddress(ddraw.dll, "D3DParseUnknownCommand", jmpaddr);
		ddraw.DDGetAttachedSurfaceLcl = GetFunctionAddress(ddraw.dll, "DDGetAttachedSurfaceLcl", jmpaddr);
		ddraw.DDInternalLock = GetFunctionAddress(ddraw.dll, "DDInternalLock", jmpaddr);
		ddraw.DDInternalUnlock = GetFunctionAddress(ddraw.dll, "DDInternalUnlock", jmpaddr);
		ddraw.DSoundHelp = GetFunctionAddress(ddraw.dll, "DSoundHelp", jmpaddr);
		ddraw.DirectDrawCreate = GetFunctionAddress(ddraw.dll, "DirectDrawCreate", jmpaddr);
		ddraw.DirectDrawCreateClipper = GetFunctionAddress(ddraw.dll, "DirectDrawCreateClipper", jmpaddr);
		ddraw.DirectDrawCreateEx = GetFunctionAddress(ddraw.dll, "DirectDrawCreateEx", jmpaddr);
		ddraw.DirectDrawEnumerateA = GetFunctionAddress(ddraw.dll, "DirectDrawEnumerateA", jmpaddr);
		ddraw.DirectDrawEnumerateExA = GetFunctionAddress(ddraw.dll, "DirectDrawEnumerateExA", jmpaddr);
		ddraw.DirectDrawEnumerateExW = GetFunctionAddress(ddraw.dll, "DirectDrawEnumerateExW", jmpaddr);
		ddraw.DirectDrawEnumerateW = GetFunctionAddress(ddraw.dll, "DirectDrawEnumerateW", jmpaddr);
		Set_DllCanUnloadNow(GetFunctionAddress(ddraw.dll, "DllCanUnloadNow", jmpaddr));
		Set_DllGetClassObject(GetFunctionAddress(ddraw.dll, "DllGetClassObject", jmpaddr));
		ddraw.GetDDSurfaceLocal = GetFunctionAddress(ddraw.dll, "GetDDSurfaceLocal", jmpaddr);
		ddraw.GetOLEThunkData = GetFunctionAddress(ddraw.dll, "GetOLEThunkData", jmpaddr);
		ddraw.GetSurfaceFromDC = GetFunctionAddress(ddraw.dll, "GetSurfaceFromDC", jmpaddr);
		ddraw.RegisterSpecialCase = GetFunctionAddress(ddraw.dll, "RegisterSpecialCase", jmpaddr);
		ddraw.ReleaseDDThreadLock = GetFunctionAddress(ddraw.dll, "ReleaseDDThreadLock", jmpaddr);
		ddraw.SetAppCompatData = GetFunctionAddress(ddraw.dll, "SetAppCompatData", jmpaddr);

		// Enable DDrawCompat
		if (Config.DDrawCompat && Config.RealWrapperMode == dtype.ddraw)
		{
			ddraw.DirectDrawCreate = GetFunctionAddress(hModule_dll, "_DirectDrawCreate", jmpaddr);
			ddraw.DirectDrawCreateEx = GetFunctionAddress(hModule_dll, "_DirectDrawCreateEx", jmpaddr);
		}

		// Hook ddraw APIs for DDrawCompat
		else if (Config.DDrawCompat)
		{
			Compat::Log() << "Hooking ddraw.dll APIs...";
			ddraw.DirectDrawCreate = (FARPROC)HookAPI(hModule_dll, dtypename[dtype.ddraw], GetFunctionAddress(ddraw.dll, "DirectDrawCreate"), "DirectDrawCreate", GetFunctionAddress(hModule_dll, "_DirectDrawCreate"));
			ddraw.DirectDrawCreateEx = (FARPROC)HookAPI(hModule_dll, dtypename[dtype.ddraw], GetFunctionAddress(ddraw.dll, "DirectDrawCreateEx"), "DirectDrawCreateEx", GetFunctionAddress(hModule_dll, "_DirectDrawCreateEx"));
		}
	}
}