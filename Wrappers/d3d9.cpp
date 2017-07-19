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
#include "Utils\Utils.h"
#include "wrapper.h"
#include "d3d8.h"
#include "d3d9.h"

d3d9_dll d3d9;

#define module d3d9

VISIT_D3D9_PROCS(CREATE_PROC_STUB)

void d3d9_dll::Load()
{
	// Load real dll
	dll = Wrapper.LoadDll(dtype.d3d9);

	// Load dll functions
	if (dll)
	{
		VISIT_D3D9_PROCS(LOAD_ORIGINAL_PROC);
		d3d8.Set_DebugSetMute(GetFunctionAddress(dll, "DebugSetMute", jmpaddr));		// <---  Shared with d3d8.dll
	}
}
