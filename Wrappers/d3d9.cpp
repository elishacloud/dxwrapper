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
#include "wrapper.h"
#include "Utils\Utils.h"

#define module d3d9

#define VISIT_PROCS(visit) \
	visit(D3DPERF_BeginEvent) \
	visit(D3DPERF_EndEvent) \
	visit(D3DPERF_GetStatus) \
	visit(D3DPERF_QueryRepeatFrame) \
	visit(D3DPERF_SetMarker) \
	visit(D3DPERF_SetOptions) \
	visit(D3DPERF_SetRegion) \
	visit(DebugSetLevel) \
	visit(DebugSetMute) \
	visit(Direct3DCreate9) \
	visit(Direct3DCreate9Ex) \
	visit(Direct3DShaderValidatorCreate9) \
	visit(PSGPError) \
	visit(PSGPSampleTexture)

struct d3d9_dll
{
	HMODULE dll = nullptr;
	VISIT_PROCS(ADD_FARPROC_MEMBER);
} d3d9;

VISIT_PROCS(CREATE_PROC_STUB)

void LoadD3d9()
{
	// Load real dll
	d3d9.dll = LoadDll(dtype.d3d9);

	// Load dll functions
	if (d3d9.dll)
	{
		VISIT_PROCS(LOAD_ORIGINAL_PROC);
	}
}

void Set_DebugSetMute(FARPROC ProcAddress)
{
	d3d9.DebugSetMute = ProcAddress;
}