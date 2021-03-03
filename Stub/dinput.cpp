/**
* Copyright (C) 2021 Elisha Riedlinger
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
	VISIT_PROCS_DINPUT(visit) \
	VISIT_PROCS_DINPUT_SHARED(visit)

#define DLL_NAME "\\dinput.dll"

#define CREATE_WRAPPED_PROC(procName, unused) \
	FARPROC m_p ## procName = nullptr;

#define INITIALIZE_WRAPPED_PROC(procName, unused) \
	m_p ## procName = (FARPROC)GetProcAddress(dll, #procName); \
	dinput::procName ## _var = m_p ## procName;

#define REDIRECT_WRAPPED_PROC(procName, unused) \
	dinput::procName ## _var = (FARPROC)*di_ ## procName;

#define CREATE_PROC_STUB(procName, unused) \
	extern "C" __declspec(naked) void __stdcall di_ ## procName() \
	{ \
		__asm mov edi, edi \
		__asm pushf \
		__asm cmp IsLoaded, 0 \
		__asm jne NEAR AsmExit \
		__asm push eax \
		__asm push ebx \
		__asm push ecx \
		__asm push edx \
		__asm push esi \
		__asm push edi \
		__asm call InitDll \
		__asm pop edi \
		__asm pop esi \
		__asm pop edx \
		__asm pop ecx \
		__asm pop ebx \
		__asm pop eax \
		__asm AsmExit: \
		__asm popf \
		__asm jmp m_p ## procName \
	}

namespace DinputWrapper
{
	bool IsLoaded = false;

	char dllname[MAX_PATH];

	VISIT_ALL_PROCS(CREATE_WRAPPED_PROC);

	void InitDll()
	{
		// Load dll
		HMODULE dll = LoadLibraryA(dllname);

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
