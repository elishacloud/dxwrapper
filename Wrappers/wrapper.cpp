/**
* Copyright (C) 2025 Elisha Riedlinger
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
#include "Logging\Logging.h"

namespace dsound
{
	extern volatile FARPROC GetDeviceID_var;
}
namespace ddraw
{
	extern volatile FARPROC DirectDrawCreate_var;
	extern "C" void __stdcall DirectDrawCreate();
}

#define VISIT_PROCS_BLANK(visit)

#define CREATE_PROC_STUB(procName, unused) \
	volatile FARPROC procName ## _var = nullptr; \
	extern "C" __declspec(naked) void __stdcall procName() \
	{ \
		__asm mov edi, edi \
		__asm jmp procName ## _var \
	} \
	volatile FARPROC procName ## _funct = (FARPROC)*procName;

#define CREATE_PROC_STUB_SHARED(procName, procName_shared, unused) \
	volatile FARPROC procName ## _var = nullptr; \
	extern "C" __declspec(naked) void __stdcall procName_shared() \
	{ \
		__asm mov edi, edi \
		__asm jmp procName ## _var \
	} \
	volatile FARPROC procName ## _funct = (FARPROC)*procName_shared;

#define	CREATE_PROC_STUB_ORDINALS(procName, num, prodAddr) \
	CREATE_PROC_STUB(procName, prodAddr)

#define	LOAD_ORIGINAL_PROC(procName, prodAddr) \
	{ \
		FARPROC pAddr = GetProcAddress(dll, #procName); \
		if (pAddr) \
		{ \
			procName ## _var = pAddr; \
		} \
		else \
		{ \
			procName ## _var = prodAddr; \
		} \
	}

#define	LOAD_ORIGINAL_PROC_SHARED(procName, unused, prodAddr) \
	LOAD_ORIGINAL_PROC(procName, prodAddr)

#define	STORE_ORIGINAL_PROC(procName, unused) \
	tmpMap.Proc = procName ## _funct; \
	tmpMap.val = &(procName ## _var); \
	jmpArray.push_back(tmpMap);

#define	STORE_ORIGINAL_PROC_SHARED(procName, unused, unused_2) \
	STORE_ORIGINAL_PROC(procName, unused)

#define	LOAD_PROC_ORDINALS(procName, num) \
	{ \
		FARPROC prodAddr = GetProcAddress(dll, reinterpret_cast<LPCSTR>(num)); \
		if (prodAddr) \
		{ \
			procName ## _var = prodAddr; \
		} \
	}
#define	LOAD_PROC_STUB_ORDINALS(procName, num, prodAddr) \
	LOAD_PROC_ORDINALS(procName, num)

#define PROC_CLASS(className, Extension, VISIT_PROCS, VISIT_PROCS_SHARED, VISIT_PROCS_ORDINALS) \
	namespace className \
	{ \
		using namespace Wrapper; \
		char *Name = #className ## "." ## #Extension; \
		VISIT_PROCS(CREATE_PROC_STUB); \
		VISIT_PROCS_SHARED(CREATE_PROC_STUB_SHARED); \
		VISIT_PROCS_ORDINALS(CREATE_PROC_STUB_ORDINALS); \
		HMODULE Load(const char *ProxyDll, const char *MyDllName) \
		{ \
			char path[MAX_PATH]; \
			const char *DllName = (MyDllName) ? MyDllName : Name; \
			HMODULE dll = nullptr; \
			if (ProxyDll && _stricmp(ProxyDll, DllName) != 0) \
			{ \
				Logging::Log() << "Loading '" << ProxyDll << "' as real dll..."; \
				dll = LoadLibraryA(ProxyDll); \
				if (!dll) \
				{ \
					Logging::Log() << "Error: Failed to load '" << ProxyDll << "'!"; \
				} \
			} \
			if (!dll && _stricmp(Name, DllName) != 0) \
			{ \
				Logging::Log() << "Loading '" << Name << "'..."; \
				dll = LoadLibraryA(Name); \
			} \
			if (!dll) \
			{ \
				Logging::Log() << "Loading '" << Name << "' from System32..."; \
				GetSystemDirectoryA(path, MAX_PATH); \
				strcat_s(path, MAX_PATH, "\\"); \
				strcat_s(path, MAX_PATH, Name); \
				dll = LoadLibraryA(path); \
			} \
			if (dll) \
			{ \
				FARPROC DirectDrawCreate_tmp = ddraw::DirectDrawCreate_var; \
				VISIT_PROCS(LOAD_ORIGINAL_PROC); \
				VISIT_PROCS_SHARED(LOAD_ORIGINAL_PROC_SHARED); \
				VISIT_PROCS_ORDINALS(LOAD_PROC_STUB_ORDINALS); \
				/* Some games hard code ordinal to 9 for DirectDrawCreate */ \
				if (DirectDrawCreate_tmp != ddraw::DirectDrawCreate_var) \
				{ \
					dsound::GetDeviceID_var = (FARPROC)ddraw::DirectDrawCreate; \
				} \
			} \
			else \
			{ \
				Logging::Log() << "Error: Failed to load wrapper for '" << Name << "'!"; \
			} \
			return dll; \
		} \
		void AddToArray() \
		{ \
			wrapper_map tmpMap; \
			VISIT_PROCS(STORE_ORIGINAL_PROC); \
			VISIT_PROCS_SHARED(STORE_ORIGINAL_PROC_SHARED); \
			VISIT_PROCS_ORDINALS(STORE_ORIGINAL_PROC_SHARED); \
		} \
	}

namespace Wrapper
{
	struct wrapper_map
	{
		volatile FARPROC Proc;
		volatile FARPROC *val;
	};

	// Forward function declaration
	HRESULT __stdcall _jmpaddr();
	HRESULT __stdcall _jmpaddrvoid();

	// Variable declaration
	const FARPROC jmpaddr = (FARPROC)*_jmpaddr;
	const FARPROC jmpaddrvoid = (FARPROC)*_jmpaddrvoid;
	std::vector<wrapper_map> jmpArray;
	HMODULE GetWrapperType(const char *ProxyDll, const char *WrapperMode, const char *MyDllName);
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
	for (auto& i : jmpArray)
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

void Wrapper::ShimProc(volatile FARPROC &var, FARPROC in, volatile FARPROC &out)
{
	if (ValidProcAddress(var) && ValidProcAddress(in) && var != in)
	{
		out = var;
		var = in;
	}
}

const char *Wrapper::GetWrapperName(const char *WrapperMode)
{
	if (!WrapperMode)
	{
		return nullptr;
	}

	// Check dll name and load correct wrapper
#define CHECK_WRAPPER(dllName) \
	{ using namespace dllName; if (_stricmp(WrapperMode, Name) == 0) { return Name; }}

	VISIT_DLLS(CHECK_WRAPPER);

	// Special handling for winmm.dll because sometimes it is changed to win32 or winnm or some other name
	if (strlen(WrapperMode) > 8)
	{
		using namespace winmm;
		if (WrapperMode &&
			tolower(WrapperMode[0]) == Name[0] &&
			tolower(WrapperMode[1]) == Name[1] &&
			tolower(WrapperMode[2]) == Name[2] &&
			tolower(WrapperMode[5]) == Name[5] &&
			tolower(WrapperMode[6]) == Name[6] &&
			tolower(WrapperMode[7]) == Name[7] &&
			tolower(WrapperMode[8]) == Name[8])
		{
			return "winmm.dll";
		}
	}

	// Special handling for winmmbase.dll because it is sharing procs from winmm
	{ using namespace winmm; if (_stricmp(WrapperMode, "winmmbase.dll") == 0) { return "winmmbase.dll"; }}

	return nullptr;
}

bool Wrapper::CheckWrapperName(const char *WrapperMode)
{
	return (WrapperMode && GetWrapperName(WrapperMode));
}

HMODULE Wrapper::CreateWrapper(const char *ProxyDll, const char *WrapperMode, const char *MyDllName)
{
	HMODULE dll = GetWrapperType(ProxyDll, WrapperMode, MyDllName);

	if (dll)
	{
		ShardProcs::Load(dll);
	}

	return dll;
}

HMODULE Wrapper::GetWrapperType(const char *ProxyDll, const char *WrapperMode, const char *MyDllName)
{
	// Add all procs to array
#define ADD_PROC_TO_ARRAY(dllName) \
	dllName::AddToArray();

	VISIT_DLLS(ADD_PROC_TO_ARRAY);
	ShardProcs::AddToArray();

	// Get wrapper name
	const char *WrapperName = GetWrapperName((CheckWrapperName(WrapperMode)) ? WrapperMode : MyDllName);

	if (!WrapperName)
	{
		Logging::Log() << "Warning. Wrapper mode not found!";
		return nullptr;
	}

	Logging::Log() << "Wrapping '" << WrapperName << "'...";

	// Check dll name and load correct wrapper
#define CHECK_FOR_WRAPPER(dllName) \
	{ using namespace dllName; if (_stricmp(WrapperName, Name) == 0) { return Load(ProxyDll, MyDllName); }}

	VISIT_DLLS(CHECK_FOR_WRAPPER);

	// Special handling for winmmbase.dll because it is sharing procs from winmm
	{ using namespace winmm; if (_stricmp(WrapperName, "winmmbase.dll") == 0) { Name = "winmmbase.dll"; return Load(ProxyDll, MyDllName); }}

	return nullptr;
}
