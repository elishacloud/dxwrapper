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


#define module ddraw

#define VISIT_PROCS(visit) \
	visit(AcquireDDThreadLock) \
	visit(CheckFullscreen) \
	visit(CompleteCreateSysmemSurface) \
	visit(D3DParseUnknownCommand) \
	visit(DDGetAttachedSurfaceLcl) \
	visit(DDInternalLock) \
	visit(DDInternalUnlock) \
	visit(DSoundHelp) \
	visit(DirectDrawCreate) \
	visit(DirectDrawCreateClipper) \
	visit(DirectDrawCreateEx) \
	visit(DirectDrawEnumerateA) \
	visit(DirectDrawEnumerateExA) \
	visit(DirectDrawEnumerateExW) \
	visit(DirectDrawEnumerateW) \
	visit(GetDDSurfaceLocal) \
	visit(GetOLEThunkData) \
	visit(GetSurfaceFromDC) \
	visit(RegisterSpecialCase) \
	visit(ReleaseDDThreadLock) \
	visit(SetAppCompatData) \
	//visit(DllCanUnloadNow) \		 // <---  Shared with dsound.dll
	//visit(DllGetClassObject)		 // <---  Shared with dsound.dll

struct ddraw_dll
{
	HMODULE dll = nullptr;
	VISIT_PROCS(ADD_FARPROC_MEMBER);
} ddraw;

VISIT_PROCS(CREATE_PROC_STUB)

void LoadDdraw()
{
	// Load real dll
	ddraw.dll = LoadDll(dtype.ddraw);

	// Load dll functions
	if (ddraw.dll)
	{
		VISIT_PROCS(LOAD_ORIGINAL_PROC);
		Set_DllCanUnloadNow(GetFunctionAddress(ddraw.dll, "DllCanUnloadNow", jmpaddr));
		Set_DllGetClassObject(GetFunctionAddress(ddraw.dll, "DllGetClassObject", jmpaddr));

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