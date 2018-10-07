/**
* Copyright (C) 2018 Elisha Riedlinger
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
#include <windows.h>
#include <fstream>
#include "..\Settings\ReadParse.h"
#include "..\Wrappers\wrapper.h"
#include "Logging\Logging.h"
#include "..\Hooking\Hook.h"

std::ofstream Logging::Log::LOG("stub.log");

std::string RealDllPath;			// Manually set Dll to wrap
std::string WrapperMode;			// Name of dxwrapper

const char *HookedDllName = "quartz.dll";

#define VISIT_PROCS_HOOKED_DLL(visit) \
	visit(AMGetErrorTextA) \
	visit(AMGetErrorTextW) \
	visit(AmpFactorToDB) \
	visit(DBToAmpFactor) \
	visit(DllCanUnloadNow) \
	visit(DllGetClassObject) \
	visit(DllRegisterServer) \
	visit(DllUnregisterServer) \
	visit(GetProxyDllInfo)

// Set booloean value from string (file)
bool IsValueEnabled(char* name)
{
	return (atoi(name) > 0 ||
		_strcmpi("on", name) == 0 ||
		_strcmpi("yes", name) == 0 ||
		_strcmpi("true", name) == 0 ||
		_strcmpi("enabled", name) == 0);
}

// Set config from string (file)
void __stdcall ParseCallback(char* name, char* value)
{
	if (!_strcmpi(name, "RealDllPath"))
	{
		RealDllPath.assign(value);
		return;
	}

	if (!_strcmpi(name, "WrapperMode"))
	{
		WrapperMode.assign(value);
		return;
	}
}

// Dll main function
bool APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved)
{
	UNREFERENCED_PARAMETER(lpReserved);

	static HMODULE wrapper_dll = nullptr;
	static HMODULE proxy_dll = nullptr;

	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
	{
		// Get config file path
		char configname[MAX_PATH];
		GetModuleFileNameA(hModule, configname, MAX_PATH);
		std::string WrapperName;
		WrapperName.assign(strrchr(configname, '\\') + 1);
		strcpy_s(strrchr(configname, '.'), MAX_PATH - strlen(configname), ".ini");

		// Read config file
		char* szCfg = Settings::Read(configname);

		// Parce config file
		if (szCfg)
		{
			Settings::Parse(szCfg, ParseCallback);
			free(szCfg);
		}

		// Start wrapper
		proxy_dll = Wrapper::CreateWrapper((RealDllPath.size()) ? RealDllPath.c_str() : nullptr, (WrapperMode.size()) ? WrapperMode.c_str() : nullptr, WrapperName.c_str());

		// Load system dll
		Logging::Log() << "Loading '" << HookedDllName << "' from System32...";
		char path[MAX_PATH];
		GetSystemDirectoryA(path, MAX_PATH);
		strcat_s(path, MAX_PATH, "\\");
		strcat_s(path, MAX_PATH, HookedDllName);
		HMODULE sys_dll = LoadLibraryA(path);
		if (!sys_dll)
		{
			Logging::Log() << "Error: Failed to load dll from System32...";
			return true;
		}

		// Load local dll
		Logging::Log() << "Loading local '" << HookedDllName << "'...";
		GetModuleFileNameA(hModule, path, MAX_PATH);
		strcpy_s(strrchr(path, '\\'), MAX_PATH - strlen(path), "\\");
		strcat_s(path, MAX_PATH, HookedDllName);
		Logging::Log() << "Loading dll from: " << path << "";
		HMODULE loc_dll = LoadLibraryA(path);
		if (!loc_dll)
		{
			Logging::Log() << "Error: Failed to load dll locally...";
			return true;
		}

		// Hooking system dll
		FARPROC procAddr = nullptr;

#define HOOK_PROC(procName) \
		procAddr = Hook::GetProcAddress(loc_dll, #procName); \
		if (procAddr) \
		{ \
			Hook::HotPatch(Hook::GetProcAddress(sys_dll, #procName), #procName, procAddr, true); \
		}

		VISIT_PROCS_HOOKED_DLL(HOOK_PROC);
	}
	break;
	case DLL_PROCESS_DETACH:

#ifdef DEBUG
		// Unload wrapped dll
		if (proxy_dll)
		{
			FreeLibrary(proxy_dll);
		}

		// Unload dxwrapper dll from memory
		if (m_wrapper_dll && LoadFromMemory)
		{
			MemoryFreeLibrary(m_wrapper_dll);
		}
		else if (wrapper_dll && !LoadFromMemory)
		{
			FreeLibrary(wrapper_dll);
		}
#endif // DEBUG

		break;
	}

	return true;
}
