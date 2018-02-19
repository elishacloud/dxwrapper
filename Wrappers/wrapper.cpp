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

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <vector>
#include <algorithm>
#include <fstream>

#define ADD_FARPROC_MEMBER(procName, unused) \
	FARPROC procName ## _var = jmpaddr;

#define CREATE_PROC_STUB(procName, unused) \
	extern "C" __declspec(naked) void __stdcall procName() \
	{ \
		__asm mov edi, edi \
		__asm jmp procName ## _var \
	}

#define	LOAD_ORIGINAL_PROC(procName, prodAddr) \
	procName ## _var = GetProcAddress(dll, #procName); \
	if (procName ## _var == nullptr) \
	{ \
		procName ## _var =  prodAddr; \
	}

#define	STORE_ORIGINAL_PROC(procName, prodAddr) \
	tmpMap.Proc = (FARPROC)*(procName); \
	tmpMap.val = &(procName ## _var); \
	jmpArray.push_back(tmpMap);

#define PROC_CLASS(className, Extension, VISIT_PROCS) \
	namespace className \
	{ \
		using namespace Wrapper; \
		char *Name = #className ## "." ## #Extension; \
		VISIT_PROCS(ADD_FARPROC_MEMBER); \
		VISIT_PROCS(CREATE_PROC_STUB); \
		HMODULE Load(const char *strName) \
		{ \
			char path[MAX_PATH]; \
			HMODULE dll = nullptr; \
			if (strName && _strcmpi(strName, Name) != 0) \
			{ \
				dll = LoadLibraryA(strName); \
			} \
			if (!dll) \
			{ \
				GetSystemDirectoryA(path, MAX_PATH); \
				strcat_s(path, MAX_PATH, "\\"); \
				strcat_s(path, MAX_PATH, Name); \
				dll = LoadLibraryA(path); \
			} \
			if (dll) \
			{ \
				VISIT_PROCS(LOAD_ORIGINAL_PROC); \
				ShardProcs::Load(dll); \
			} \
			return dll; \
		} \
		void AddToArray() \
		{ \
			wrapper_map tmpMap; \
			VISIT_PROCS(STORE_ORIGINAL_PROC); \
		} \
	}

namespace Wrapper
{
	struct wrapper_map
	{
		FARPROC Proc;
		FARPROC *val;
	};

	// Forward function decalration
	HRESULT __stdcall _jmpaddr();
	HRESULT __stdcall _jmpaddrvoid();

	// Varable decalration
	constexpr FARPROC jmpaddr = (FARPROC)*_jmpaddr;
	constexpr FARPROC jmpaddrvoid = (FARPROC)*_jmpaddrvoid;
	std::vector<wrapper_map> jmpArray;
}

#include "wrapper.h"

__declspec(naked) HRESULT __stdcall Wrapper::_jmpaddrvoid()
{
	__asm
	{
		retn
	}
}

__declspec(naked) HRESULT __stdcall Wrapper::_jmpaddr()
{
	__asm
	{
		mov eax, 0x80004001L	// return E_NOTIMPL
		retn 16
	}
}

bool Wrapper::ValidProcAddress(FARPROC ProcAddress)
{
	for (wrapper_map i : jmpArray)
	{
		if (i.Proc == ProcAddress)
		{
			if (*(i.val) == jmpaddr || *(i.val) == jmpaddrvoid || *(i.val) == nullptr)
			{
				return false;
			}
		}
	}
	return (ProcAddress != nullptr &&
		ProcAddress != jmpaddr &&
		ProcAddress != jmpaddrvoid);
}

void Wrapper::ShimProc(FARPROC &var, FARPROC in, FARPROC &out)
{
	if (ValidProcAddress(var))
	{
		out = var;
		var = in;
	}
}

#define ADD_PROC_TO_ARRAY(dllName) \
	dllName::AddToArray();

#define CHECK_FOR_WRAPPER(dllName) \
	{ using namespace dllName; if (_strcmpi(WrapperMode, Name) == 0) { dll = Load(ProxyDll); return dll; }}

HMODULE Wrapper::CreateWrapper(const char *ProxyDll, const char *WrapperMode)
{
	// Declare vars
	HMODULE dll = nullptr;

	// Add all procs to array
	VISIT_DLLS(ADD_PROC_TO_ARRAY);
	ShardProcs::AddToArray();

	// Check dll name and load correct wrapper
	VISIT_DLLS(CHECK_FOR_WRAPPER);

	// Special for winmm.dll because sometimes it is changed to win32 or winnm or some other name
	if (strlen(WrapperMode) > 8)
	{
		using namespace winmm;
		if (tolower(WrapperMode[0]) == Name[0] &&
			tolower(WrapperMode[1]) == Name[1] &&
			tolower(WrapperMode[2]) == Name[2] &&
			tolower(WrapperMode[5]) == Name[5] &&
			tolower(WrapperMode[6]) == Name[6] &&
			tolower(WrapperMode[7]) == Name[7] &&
			tolower(WrapperMode[8]) == Name[8])
		{
			Name = "winmm.dll";
			dll = Load(ProxyDll);
			return dll;
		}
	}
	// Special for winmmbase.dll because it is sharing procs from winmm
	{ using namespace winmm; if (_strcmpi(WrapperMode, "winmmbase.dll") == 0) { Name = "winmmbase.dll";  dll = Load(ProxyDll); return dll; }}

	// Exit and return handle
	return dll;
}
