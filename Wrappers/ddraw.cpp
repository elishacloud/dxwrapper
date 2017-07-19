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
#include "ddraw.h"

ddraw_dll ddraw;

#define module ddraw

VISIT_DRAW_PROCS(CREATE_PROC_STUB)

void ddraw_dll::Load()
{
	// Load real dll
	dll = Wrapper.LoadDll(dtype.ddraw);

	// Load dll functions
	if (dll)
	{
		VISIT_DRAW_PROCS(LOAD_ORIGINAL_PROC);
		dsound.Set_DllCanUnloadNow(GetFunctionAddress(dll, "DllCanUnloadNow", jmpaddr));			 // <---  Shared with dsound.dll
		dsound.Set_DllGetClassObject(GetFunctionAddress(dll, "DllGetClassObject", jmpaddr));		 // <---  Shared with dsound.dll

		// Enable DDrawCompat
		if (Config.DDrawCompat && Config.RealWrapperMode == dtype.ddraw)
		{
			DirectDrawCreate = GetFunctionAddress(hModule_dll, "_DirectDrawCreate", jmpaddr);
			DirectDrawCreateEx = GetFunctionAddress(hModule_dll, "_DirectDrawCreateEx", jmpaddr);
		}

		// Hook ddraw APIs for DDrawCompat
		else if (Config.DDrawCompat)
		{
			LOG << "Hooking ddraw.dll APIs...";
			DirectDrawCreate = (FARPROC)HookAPI(hModule_dll, dtypename[dtype.ddraw], GetFunctionAddress(dll, "DirectDrawCreate"), "DirectDrawCreate", GetFunctionAddress(hModule_dll, "_DirectDrawCreate"));
			DirectDrawCreateEx = (FARPROC)HookAPI(hModule_dll, dtypename[dtype.ddraw], GetFunctionAddress(dll, "DirectDrawCreateEx"), "DirectDrawCreateEx", GetFunctionAddress(hModule_dll, "_DirectDrawCreateEx"));
		}
	}
}
