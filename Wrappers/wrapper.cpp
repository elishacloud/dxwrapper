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

HMODULE Wrapper::CreateWrapper(const char *ProxyDll, const char *WrapperMode)
{
	// Declare vars
	HMODULE dll = nullptr;

	// Check dll name and load correct wrapper
	{ using namespace bcrypt; if (_strcmpi(WrapperMode, Name) == 0) dll = Load(ProxyDll); }
	{ using namespace cryptsp; if (_strcmpi(WrapperMode, Name) == 0) dll = Load(ProxyDll); }
	{ using namespace d2d1; if (_strcmpi(WrapperMode, Name) == 0) dll = Load(ProxyDll); }
	{ using namespace d3d8; if (_strcmpi(WrapperMode, Name) == 0) dll = Load(ProxyDll); }
	{ using namespace d3d9; if (_strcmpi(WrapperMode, Name) == 0) dll = Load(ProxyDll); }
	{ using namespace d3d10; if (_strcmpi(WrapperMode, Name) == 0) dll = Load(ProxyDll); }
	{ using namespace d3d10core; if (_strcmpi(WrapperMode, Name) == 0) dll = Load(ProxyDll); }
	{ using namespace d3d11; if (_strcmpi(WrapperMode, Name) == 0) dll = Load(ProxyDll); }
	{ using namespace d3d12; if (_strcmpi(WrapperMode, Name) == 0) dll = Load(ProxyDll); }
	{ using namespace d3dim; if (_strcmpi(WrapperMode, Name) == 0) dll = Load(ProxyDll); }
	{ using namespace d3dim700; if (_strcmpi(WrapperMode, Name) == 0) dll = Load(ProxyDll); }
	{ using namespace dciman32; if (_strcmpi(WrapperMode, Name) == 0) dll = Load(ProxyDll); }
	{ using namespace ddraw; if (_strcmpi(WrapperMode, Name) == 0) dll = Load(ProxyDll); }
	{ using namespace dinput; if (_strcmpi(WrapperMode, Name) == 0) dll = Load(ProxyDll); }
	{ using namespace dinput8; if (_strcmpi(WrapperMode, Name) == 0) dll = Load(ProxyDll); }
	{ using namespace dplayx; if (_strcmpi(WrapperMode, Name) == 0) dll = Load(ProxyDll); }
	{ using namespace dsound; if (_strcmpi(WrapperMode, Name) == 0) dll = Load(ProxyDll); }
	{ using namespace dxgi; if (_strcmpi(WrapperMode, Name) == 0) dll = Load(ProxyDll); }
	{ using namespace msacm32; if (_strcmpi(WrapperMode, Name) == 0) dll = Load(ProxyDll); }
	{ using namespace msvfw32; if (_strcmpi(WrapperMode, Name) == 0) dll = Load(ProxyDll); }
	{ using namespace vorbisfile; if (_strcmpi(WrapperMode, Name) == 0) dll = Load(ProxyDll); }
	{ using namespace winspool; if (_strcmpi(WrapperMode, Name) == 0) dll = Load(ProxyDll); }
	{ using namespace xlive; if (_strcmpi(WrapperMode, Name) == 0) dll = Load(ProxyDll); }

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
		}
	}
	// Special for winmmbase.dll because it is sharing procs from winmm
	{ using namespace winmm; if (_strcmpi(WrapperMode, "winmmbase.dll") == 0) { Name = "winmmbase.dll";  dll = Load(ProxyDll); }}

	// Exit and return handle
	return dll;
}
