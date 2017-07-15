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

#define module dinput

#define VISIT_PROCS(visit) \
	visit(DirectInputCreateA) \
	visit(DirectInputCreateEx) \
	visit(DirectInputCreateW) \
	visit(DllRegisterServer) \
	visit(DllUnregisterServer) \
	//visit(DllCanUnloadNow) \		 // <---  Shared with dsound.dll
	//visit(DllGetClassObject)		 // <---  Shared with dsound.dll


struct dinput_dll
{
	HMODULE dll = nullptr;
	VISIT_PROCS(ADD_FARPROC_MEMBER);
} dinput;

VISIT_PROCS(CREATE_PROC_STUB)

void LoadDinput()
{
	// Load real dll
	dinput.dll = LoadDll(dtype.dinput);

	// Load dll functions
	if (dinput.dll)
	{
		VISIT_PROCS(LOAD_ORIGINAL_PROC);
		Set_DllCanUnloadNow(GetFunctionAddress(dinput.dll, "DllCanUnloadNow", jmpaddr));
		Set_DllGetClassObject(GetFunctionAddress(dinput.dll, "DllGetClassObject", jmpaddr));
	}
}