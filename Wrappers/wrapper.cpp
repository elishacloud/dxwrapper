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
#include <fstream>
#include "wrapper.h"

#define ADD_FARPROC_MEMBER(procName) \
	FARPROC procName ## _var = jmpaddr;

#define	LOAD_ORIGINAL_PROC(procName) \
	procName ## _var = GetProcAddress(dll, #procName, jmpaddr);

#define CREATE_PROC_STUB(procName) \
	extern "C" __declspec(naked) void __stdcall procName() \
	{ \
		__asm mov edi, edi \
		__asm jmp procName ## _var \
	}

#define PROC_CLASS(className, Extension) \
	namespace className \
	{ \
		using namespace Wrapper; \
		char *Name = #className ## "." ## #Extension; \
		VISIT_PROCS(ADD_FARPROC_MEMBER); \
		HMODULE Load(char *strName) \
		{ \
			char path[MAX_PATH]; \
			GetSystemDirectoryA(path, MAX_PATH); \
			strcat_s(path, MAX_PATH, "\\"); \
			if (strName) \
			{ \
				strcat_s(path, MAX_PATH, strName); \
			} \
			else \
			{ \
				strcat_s(path, MAX_PATH, Name); \
			} \
			HMODULE dll = LoadLibraryA(path); \
			if (dll) \
			{ \
				VISIT_PROCS(LOAD_ORIGINAL_PROC); \
				ShardProcs::Load(dll); \
			} \
			return dll; \
		} \
		HMODULE Load() \
		{ \
			return Load(nullptr); \
		} \
		VISIT_PROCS(CREATE_PROC_STUB) \
	}

namespace Wrapper
{
	FARPROC GetProcAddress(HMODULE hModule, LPCSTR FunctionName, FARPROC SetReturnValue);
	__declspec() HRESULT __stdcall _jmpaddr();
	constexpr FARPROC jmpaddr = (FARPROC)*_jmpaddr;
}

// Shared procs
#include "shared.h"

// Wrappers
#include "bcrypt.h"
#include "cryptsp.h"
#include "d2d1.h"
#include "d3d8.h"
#include "d3d9.h"
#include "d3d10.h"
#include "d3d10core.h"
#include "d3d11.h"
#include "d3d12.h"
#include "d3dim.h"
#include "d3dim700.h"
#include "dciman32.h"
#include "ddraw.h"
#include "dinput.h"
#include "dinput8.h"
#include "dplayx.h"
#include "dsound.h"
#include "dxgi.h"
#include "msacm32.h"
#include "msvfw32.h"
#include "vorbisfile.h"
#include "winmm.h"
#include "winspool.h"
#include "xlive.h"

__declspec(naked) HRESULT __stdcall Wrapper::_jmpaddr()
{
	__asm
	{
		mov eax, 0x80004001L	// return E_NOTIMPL
		retn 16
	}
}

FARPROC Wrapper::GetProcAddress(HMODULE hModule, LPCSTR FunctionName, FARPROC SetReturnValue)
{
	if (!FunctionName || !hModule)
	{
		return SetReturnValue;
	}

	FARPROC ProcAddress = GetProcAddress(hModule, FunctionName);

	if (!ProcAddress)
	{
		ProcAddress = SetReturnValue;
	}

	return ProcAddress;
}

HMODULE Wrapper::CreateWrapper(HMODULE hModule)
{
	// Declare vars
	HMODULE dll = nullptr;

	// Get module full path and name
	char path[MAX_PATH];
	GetModuleFileNameA(hModule, path, sizeof(path));

	// Search backwards for last backslash in filepath 
	char* pdest = strrchr(path, '\\');

	// If backslash not found in filepath
	if (pdest)
	{
		// Extract filename from file path
		std::string input(pdest+1);
		strcpy_s(path, MAX_PATH, input.c_str());
	}

	// Check dll name and load correct wrapper
	{ using namespace bcrypt; if (_strcmpi(path, Name) == 0) dll = Load(); }
	{ using namespace cryptsp; if (_strcmpi(path, Name) == 0) dll = Load(); }
	{ using namespace d2d1; if (_strcmpi(path, Name) == 0) dll = Load(); }
	{ using namespace d3d8; if (_strcmpi(path, Name) == 0) dll = Load(); }
	{ using namespace d3d9; if (_strcmpi(path, Name) == 0) dll = Load(); }
	{ using namespace d3d10; if (_strcmpi(path, Name) == 0) dll = Load(); }
	{ using namespace d3d10core; if (_strcmpi(path, Name) == 0) dll = Load(); }
	{ using namespace d3d11; if (_strcmpi(path, Name) == 0) dll = Load(); }
	{ using namespace d3d12; if (_strcmpi(path, Name) == 0) dll = Load(); }
	{ using namespace d3dim; if (_strcmpi(path, Name) == 0) dll = Load(); }
	{ using namespace d3dim700; if (_strcmpi(path, Name) == 0) dll = Load(); }
	{ using namespace dciman32; if (_strcmpi(path, Name) == 0) dll = Load(); }
	{ using namespace ddraw; if (_strcmpi(path, Name) == 0) dll = Load(); }
	{ using namespace dinput; if (_strcmpi(path, Name) == 0) dll = Load(); }
	{ using namespace dinput8; if (_strcmpi(path, Name) == 0) dll = Load(); }
	{ using namespace dplayx; if (_strcmpi(path, Name) == 0) dll = Load(); }
	{ using namespace dsound; if (_strcmpi(path, Name) == 0) dll = Load(); }
	{ using namespace dxgi; if (_strcmpi(path, Name) == 0) dll = Load(); }
	{ using namespace msacm32; if (_strcmpi(path, Name) == 0) dll = Load(); }
	{ using namespace msvfw32; if (_strcmpi(path, Name) == 0) dll = Load(); }
	{ using namespace vorbisfile; if (_strcmpi(path, Name) == 0) dll = Load(); }
	{ using namespace winmm; if (_strcmpi(path, "winmmbase.dll") == 0) dll = Load("winmmbase.dll"); }
	{ using namespace winspool; if (_strcmpi(path, Name) == 0) dll = Load(); }
	{ using namespace xlive; if (_strcmpi(path, Name) == 0) dll = Load(); }

	// Special for winmm.dll because sometimes it is changed to win32 or winnm or some other name
	if (strlen(path) > 8)
	{
		path[3] = 'm';
		path[4] = 'm';
	}
	{ using namespace winmm; if (_strcmpi(path, Name) == 0) dll = Load(); }

	// Exit and return handle
	return dll;
}
