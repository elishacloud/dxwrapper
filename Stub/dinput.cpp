/**
* Copyright (C) 2026 Elisha Riedlinger
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

#define DLL_NAME L"\\dinput.dll"

#define CREATE_WRAPPED_PROC(procName, unused) \
	volatile FARPROC m_p ## procName = nullptr;

#define INITIALIZE_STUB_WRAPPED_PROC(procName, unused) \
	m_p ## procName = GetProcAddress(dll, #procName); \
	dinput::procName ## _var = m_p ## procName;

#define REDIRECT_WRAPPED_PROC(procName, unused) \
	dinput::procName ## _var = (FARPROC)*di_ ## procName;

#define CREATE_PROC_STUB(procName, unused) \
	extern "C" __declspec(naked) void __stdcall di_ ## procName() \
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

namespace DinputWrapper
{
	bool IsLoaded = false;

	wchar_t dllname[MAX_PATH];

	VISIT_ALL_PROCS(CREATE_WRAPPED_PROC);

	void InitDll()
	{
		// Load dll
		HMODULE dll = LoadLibraryW(dllname);

		// Check dll
		if (!dll)
		{
			return;
		}

		// Get function addresses
		VISIT_ALL_PROCS(INITIALIZE_STUB_WRAPPED_PROC);

		// Mark ddraw as loaded
		IsLoaded = true;
	}

	VISIT_ALL_PROCS(CREATE_PROC_STUB);

	void Start(const wchar_t* name)
	{
		if (name)
		{
			wcscpy_s(dllname, MAX_PATH, name);
		}
		else
		{
			GetSystemDirectoryW(dllname, MAX_PATH);
			wcscat_s(dllname, MAX_PATH, DLL_NAME);
		}

		VISIT_ALL_PROCS(REDIRECT_WRAPPED_PROC);
	}
}
