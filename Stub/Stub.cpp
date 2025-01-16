/**
* Copyright (C) 2024 Elisha Riedlinger
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
#include "stub.h"
#include "..\Settings\ReadParse.h"
#if (_WIN32_WINNT >= 0x0502)
#include "..\External\MemoryModule\MemoryModule.h"
#endif
#include "..\Wrappers\wrapper.h"
#include "..\External\Hooking\Hook.h"

bool StubOnly = false;				// Don't load dxwrapper
bool LoadFromMemory = false;		// Use MemoryModule to load dxwrapper
std::string RealDllPath;			// Manually set Dll to wrap
std::string WrapperMode;			// Name of dxwrapper

std::string WrapperName;
HMODULE wrapper_dll = nullptr;
HMODULE proxy_dll = nullptr;

DXWAPPERSETTINGS DxSettings = {};

void WINAPI DxWrapperSettings(DXWAPPERSETTINGS *)
{
	return;
}

// Set booloean value from string (file)
bool IsValueEnabled(char* name)
{
	return (atoi(name) > 0 ||
		_stricmp("on", name) == 0 ||
		_stricmp("yes", name) == 0 ||
		_stricmp("true", name) == 0 ||
		_stricmp("enabled", name) == 0);
}

// Set config from string (file)
void __stdcall ParseCallback(char* name, char* value)
{
	if (!_stricmp(name, "StubOnly"))
	{
		StubOnly = IsValueEnabled(value);
		return;
	}

	if (!_stricmp(name, "LoadFromMemory"))
	{
		LoadFromMemory = IsValueEnabled(value);
		return;
	}

	if (!_stricmp(name, "RealDllPath"))
	{
		RealDllPath.assign(value);
		return;
	}

	if (!_stricmp(name, "WrapperMode"))
	{
		WrapperMode.assign(value);
		return;
	}
}

inline void GetConfig(HMODULE hModule)
{
	// Get config file path
	char configname[MAX_PATH];
	GetModuleFileNameA(hModule, configname, MAX_PATH);
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
}

inline void LoadDxWrapper(HMODULE hModule)
{
	// Open file and get size
	char path[MAX_PATH];
	std::ifstream myfile;

	// Get config file path
	GetModuleFileNameA(hModule, path, sizeof(path));
	strcpy_s(strrchr(path, '\\'), MAX_PATH - strlen(path), "\\dxwrapper.dll");

	// Check if the dll is already loaded
	wrapper_dll = GetModuleHandle(path);
	if (wrapper_dll)
	{
		// Already loaded
	}
#if (_WIN32_WINNT >= 0x0502)
	// Use MemoryModule to load dxwrapper
	else if (LoadFromMemory)
	{
		// Get config file name for log
		myfile.open(path, std::ios::binary | std::ios::in | std::ios::ate);
		DWORD size = (DWORD)myfile.tellg();

		// If size is greater than 0
		if (size && myfile.is_open())
		{
			// Read file
			myfile.seekg(0, std::ios::beg);
			std::string memblock(size, '\0');
			myfile.read(&memblock[0], size);

			// Load library into memory
			wrapper_dll = (HMODULE)MemoryLoadLibrary(&memblock[0], size);
		}

		// Close the file
		myfile.close();
	}
#endif
	// Load dxwrapper normally
	else
	{
		wrapper_dll = LoadLibrary(path);
	}

	// Check if DxWrapper is loaded
	if (wrapper_dll)
	{
		DxWrapperSettingsProc DxWrapperSettings = (DxWrapperSettingsProc)Hook::GetProcAddress(wrapper_dll, "DxWrapperSettings");
		if (DxWrapperSettings)
		{
			DxWrapperSettings(&DxSettings);
		}
	}
	else
	{
		MessageBoxA(nullptr, "Could not find DxWrapper.dll functions will be disabled!", "DxWrapper Stub", MB_ICONWARNING | MB_TOPMOST | MB_SETFOREGROUND);
	}
}

void LoadRealDLL()
{
	// Get wrapper mode
	const char *RealWrapperMode = Wrapper::GetWrapperName((WrapperMode.size()) ? WrapperMode.c_str() : WrapperName.c_str());

	// Load custom wrapper
	if (DxSettings.Dd7to9 && _stricmp(RealWrapperMode, "ddraw.dll") == 0)
	{
		DdrawWrapper::Start((RealDllPath.size()) ? RealDllPath.c_str() : nullptr);
	}
	else if (DxSettings.D3d8to9 && _stricmp(RealWrapperMode, "d3d8.dll") == 0)
	{
		D3d8Wrapper::Start((RealDllPath.size()) ? RealDllPath.c_str() : nullptr);
	}
	else if (DxSettings.Dinputto8 && _stricmp(RealWrapperMode, "dinput.dll") == 0)
	{
		DinputWrapper::Start((RealDllPath.size()) ? RealDllPath.c_str() : nullptr);
	}
	// Start normal wrapper
	else
	{
		proxy_dll = Wrapper::CreateWrapper((RealDllPath.size()) ? RealDllPath.c_str() : nullptr, (WrapperMode.size()) ? WrapperMode.c_str() : nullptr, WrapperName.c_str());
	}
}

// Dll main function
bool APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved)
{
	UNREFERENCED_PARAMETER(lpReserved);

	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
	{
		// Get config settings
		GetConfig(hModule);

		// Load the real dll
		LoadRealDLL();

		// Load DxWrapper
		if (!StubOnly)
		{
			LoadDxWrapper(hModule);
		}
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
