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

#include "dgame.h"

bool ddrawFlag = false;

struct ddraw_dll
{
	HMODULE dll;
	FARPROC AcquireDDThreadLock;
	FARPROC CheckFullscreen;
	FARPROC CompleteCreateSysmemSurface;
	FARPROC D3DParseUnknownCommand;
	FARPROC DDGetAttachedSurfaceLcl;
	FARPROC DDInternalLock;
	FARPROC DDInternalUnlock;
	FARPROC DSoundHelp;
	FARPROC DirectDrawCreate;
	FARPROC DirectDrawCreateClipper;
	FARPROC DirectDrawCreateEx;
	FARPROC DirectDrawEnumerateA;
	FARPROC DirectDrawEnumerateExA;
	FARPROC DirectDrawEnumerateExW;
	FARPROC DirectDrawEnumerateW;
	FARPROC DllCanUnloadNow;
	FARPROC DllGetClassObject;
	FARPROC GetDDSurfaceLocal;
	FARPROC GetOLEThunkData;
	FARPROC GetSurfaceFromDC;
	FARPROC RegisterSpecialCase;
	FARPROC ReleaseDDThreadLock;
	FARPROC SetAppCompatData;
	FARPROC DirectInputCreateA;
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
__declspec(naked) void FakeDllCanUnloadNow()				{ _asm { jmp [ddraw.DllCanUnloadNow] } }
__declspec(naked) void FakeDllGetClassObject()				{ _asm { jmp [ddraw.DllGetClassObject] } }
__declspec(naked) void FakeGetDDSurfaceLocal()				{ _asm { jmp [ddraw.GetDDSurfaceLocal] } }
__declspec(naked) void FakeGetOLEThunkData()				{ _asm { jmp [ddraw.GetOLEThunkData] } }
__declspec(naked) void FakeGetSurfaceFromDC()				{ _asm { jmp [ddraw.GetSurfaceFromDC] } }
__declspec(naked) void FakeRegisterSpecialCase()			{ _asm { jmp [ddraw.RegisterSpecialCase] } }
__declspec(naked) void FakeReleaseDDThreadLock()			{ _asm { jmp [ddraw.ReleaseDDThreadLock] } }
__declspec(naked) void FakeSetAppCompatData()				{ _asm { jmp [ddraw.SetAppCompatData] } }
__declspec(naked) void FakeDirectInputCreateA()				{ _asm { jmp [ddraw.DirectInputCreateA] } }

void LoadDdraw()
{
	// Enable DdrawCompat
	if (Config.DdrawCompat)
	{
		ddraw.dll = nullptr;
		ddraw.AcquireDDThreadLock = GetProcAddress(hModule_dll, "_AcquireDDThreadLock");
		ddraw.CheckFullscreen = GetProcAddress(hModule_dll, "_CheckFullscreen");
		ddraw.CompleteCreateSysmemSurface = GetProcAddress(hModule_dll, "_CompleteCreateSysmemSurface");
		ddraw.D3DParseUnknownCommand = GetProcAddress(hModule_dll, "_D3DParseUnknownCommand");
		ddraw.DDGetAttachedSurfaceLcl = GetProcAddress(hModule_dll, "_DDGetAttachedSurfaceLcl");
		ddraw.DDInternalLock = GetProcAddress(hModule_dll, "_DDInternalLock");
		ddraw.DDInternalUnlock = GetProcAddress(hModule_dll, "_DDInternalUnlock");
		ddraw.DSoundHelp = GetProcAddress(hModule_dll, "_DSoundHelp");
		ddraw.DirectDrawCreate = GetProcAddress(hModule_dll, "_DirectDrawCreate");
		ddraw.DirectDrawCreateClipper = GetProcAddress(hModule_dll, "_DirectDrawCreateClipper");
		ddraw.DirectDrawCreateEx = GetProcAddress(hModule_dll, "_DirectDrawCreateEx");
		ddraw.DirectDrawEnumerateA = GetProcAddress(hModule_dll, "_DirectDrawEnumerateA");
		ddraw.DirectDrawEnumerateExA = GetProcAddress(hModule_dll, "_DirectDrawEnumerateExA");
		ddraw.DirectDrawEnumerateExW = GetProcAddress(hModule_dll, "_DirectDrawEnumerateExW");
		ddraw.DirectDrawEnumerateW = GetProcAddress(hModule_dll, "_DirectDrawEnumerateW");
		ddraw.DllCanUnloadNow = GetProcAddress(hModule_dll, "_DllCanUnloadNow_ddraw");
		ddraw.DllGetClassObject = GetProcAddress(hModule_dll, "_DllGetClassObject_ddraw");
		ddraw.GetDDSurfaceLocal = GetProcAddress(hModule_dll, "_GetDDSurfaceLocal");
		ddraw.GetOLEThunkData = GetProcAddress(hModule_dll, "_GetOLEThunkData");
		ddraw.GetSurfaceFromDC = GetProcAddress(hModule_dll, "_GetSurfaceFromDC");
		ddraw.RegisterSpecialCase = GetProcAddress(hModule_dll, "_RegisterSpecialCase");
		ddraw.ReleaseDDThreadLock = GetProcAddress(hModule_dll, "_ReleaseDDThreadLock");
		ddraw.SetAppCompatData = GetProcAddress(hModule_dll, "_SetAppCompatData");
		ddraw.DirectInputCreateA = GetProcAddress(hModule_dll, "_DirectInputCreateA");
	}
	else
	{
		// Load real dll
		ddraw.dll = LoadDll("ddraw.dll", dtype.ddraw);
		// Load dll functions
		if (ddraw.dll)
		{
			ddrawFlag = true;
			ddraw.AcquireDDThreadLock = GetProcAddress(ddraw.dll, "AcquireDDThreadLock");
			ddraw.CheckFullscreen = GetProcAddress(ddraw.dll, "CheckFullscreen");
			ddraw.CompleteCreateSysmemSurface = GetProcAddress(ddraw.dll, "CompleteCreateSysmemSurface");
			ddraw.D3DParseUnknownCommand = GetProcAddress(ddraw.dll, "D3DParseUnknownCommand");
			ddraw.DDGetAttachedSurfaceLcl = GetProcAddress(ddraw.dll, "DDGetAttachedSurfaceLcl");
			ddraw.DDInternalLock = GetProcAddress(ddraw.dll, "DDInternalLock");
			ddraw.DDInternalUnlock = GetProcAddress(ddraw.dll, "DDInternalUnlock");
			ddraw.DSoundHelp = GetProcAddress(ddraw.dll, "DSoundHelp");
			ddraw.DirectDrawCreate = GetProcAddress(ddraw.dll, "DirectDrawCreate");
			ddraw.DirectDrawCreateClipper = GetProcAddress(ddraw.dll, "DirectDrawCreateClipper");
			ddraw.DirectDrawCreateEx = GetProcAddress(ddraw.dll, "DirectDrawCreateEx");
			ddraw.DirectDrawEnumerateA = GetProcAddress(ddraw.dll, "DirectDrawEnumerateA");
			ddraw.DirectDrawEnumerateExA = GetProcAddress(ddraw.dll, "DirectDrawEnumerateExA");
			ddraw.DirectDrawEnumerateExW = GetProcAddress(ddraw.dll, "DirectDrawEnumerateExW");
			ddraw.DirectDrawEnumerateW = GetProcAddress(ddraw.dll, "DirectDrawEnumerateW");
			ddraw.DllCanUnloadNow = GetProcAddress(ddraw.dll, "DllCanUnloadNow");
			ddraw.DllGetClassObject = GetProcAddress(ddraw.dll, "DllGetClassObject");
			ddraw.GetDDSurfaceLocal = GetProcAddress(ddraw.dll, "GetDDSurfaceLocal");
			ddraw.GetOLEThunkData = GetProcAddress(ddraw.dll, "GetOLEThunkData");
			ddraw.GetSurfaceFromDC = GetProcAddress(ddraw.dll, "GetSurfaceFromDC");
			ddraw.RegisterSpecialCase = GetProcAddress(ddraw.dll, "RegisterSpecialCase");
			ddraw.ReleaseDDThreadLock = GetProcAddress(ddraw.dll, "ReleaseDDThreadLock");
			ddraw.SetAppCompatData = GetProcAddress(ddraw.dll, "SetAppCompatData");
			ddraw.DirectInputCreateA = GetProcAddress(ddraw.dll, "DirectInputCreateA");
		}
	}
	// SetAppCompatData see: http://www.blitzbasic.com/Community/post.php?topic=99477&post=1202996
	if (ddraw.SetAppCompatData)
	{
		typedef HRESULT(__stdcall *SetAppCompatDataFunc)(DWORD, DWORD);
		SetAppCompatDataFunc SetAppCompatData = (SetAppCompatDataFunc)ddraw.SetAppCompatData;
		for (int x = 1; x <= 12; x++)
		{
			if (Config.DXPrimaryEmulation[x])
			{
				Compat::Log() << "SetAppCompatData: " << x;
				// For LockColorkey, this one uses the second parameter
				if (x == AppCompatDataType.LockColorkey)
				{
					(SetAppCompatData)(x, Config.LockColorkey);
				}
				// For all the other items
				else
				{
					(SetAppCompatData)(x, 0);
				}
			}
		}
	}
}

void SetSharedDdraw(HMODULE dll)
{
		ddraw.DllCanUnloadNow				= GetProcAddress(dll, "DllCanUnloadNow");
		ddraw.DllGetClassObject				= GetProcAddress(dll, "DllGetClassObject");
}

void FreeDdrawLibrary()
{
	if (ddrawFlag) FreeLibrary(ddraw.dll);
}
