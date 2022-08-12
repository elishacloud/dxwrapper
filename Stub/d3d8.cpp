/**
* Copyright (C) 2022 Elisha Riedlinger
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

#include <windows.h>
#include "..\Wrappers\wrapper.h"

#undef VISIT_ALL_PROCS
#define VISIT_ALL_PROCS(visit) \
	VISIT_PROCS_D3D8(visit)

#define DLL_NAME "\\d3d8.dll"

#define CREATE_WRAPPED_PROC(procName, unused) \
	FARPROC m_p ## procName = nullptr;

#define INITIALIZE_WRAPPED_PROC(procName, unused) \
	m_p ## procName = (FARPROC)GetProcAddress(dll, #procName); \
	d3d8::procName ## _var = m_p ## procName;

#define REDIRECT_WRAPPED_PROC(procName, unused) \
	d3d8::procName ## _var = (FARPROC)*d8_ ## procName;

#define CREATE_PROC_STUB(procName, unused) \
	extern "C" __declspec(naked) void __stdcall d8_ ## procName() \
	{ \
		__asm mov edi, edi \
		__asm pushfd \
		__asm cmp IsLoaded, 0 \
		__asm jne NEAR AsmExit \
		__asm pushad \
		__asm call InitDll \
		__asm popad \
		__asm AsmExit: \
		__asm popfd \
		__asm jmp m_p ## procName \
	}

namespace D3d8Wrapper
{
	bool IsLoaded = false;

	char dllname[MAX_PATH];

	VISIT_ALL_PROCS(CREATE_WRAPPED_PROC);

	void InitDll()
	{
		// Load dll
		HMODULE dll = LoadLibraryA(dllname);

		// Check dll
		if (!dll)
		{
			return;
		}

		// Get function addresses
		VISIT_ALL_PROCS(INITIALIZE_WRAPPED_PROC);

		// Mark ddraw as loaded
		IsLoaded = true;
	}

	VISIT_ALL_PROCS(CREATE_PROC_STUB);

	void Start(const char *name)
	{
		if (name)
		{
			strcpy_s(dllname, MAX_PATH, name);
		}
		else
		{
			GetSystemDirectoryA(dllname, MAX_PATH);
			strcat_s(dllname, DLL_NAME);
		}

		VISIT_ALL_PROCS(REDIRECT_WRAPPED_PROC);
	}
}
