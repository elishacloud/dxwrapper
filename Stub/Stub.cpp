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
#include <windows.h>
#include <cctype>
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
bool IsDdraw = false;
bool IsD3d8 = false;
bool IsDinput = false;

std::string StubName;
HMODULE wrapper_dll = nullptr;
HMODULE proxy_dll = nullptr;

DXWAPPERSETTINGS DxSettings = {};

// Set booloean value from string (file)
static bool IsValueEnabled(char* name)
{
	return (atoi(name) > 0 ||
		_stricmp("on", name) == 0 ||
		_stricmp("yes", name) == 0 ||
		_stricmp("true", name) == 0 ||
		_stricmp("enabled", name) == 0);
}

// Set config from string (file)
static void __stdcall ParseCallback(char* name, char* value)
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
		if (strlen(value) > 0 && _stricmp(value, "auto") != 0)
		{
			WrapperMode.assign(value);
		}
		return;
	}
}

static void GetConfig(HMODULE hModule)
{
	// Get config file path and stub name
	char configname[MAX_PATH] = {};
	GetModuleFileNameA(hModule, configname, MAX_PATH);
	StubName.assign(strrchr(configname, '\\') + 1);
	for (auto it = StubName.begin(); it != StubName.end(); ++it)
	{
		*it = static_cast<char>(std::tolower(static_cast<unsigned char>(*it)));
	}
	strcpy_s(strrchr(configname, '.'), MAX_PATH - strlen(configname), ".ini");

	// Read config file
	char* szCfg = Settings::ReadFileContent(configname);

	// Parce config file
	if (szCfg)
	{
		Settings::Parse(szCfg, ParseCallback);
		free(szCfg);
	}

	// Get wrapper mode after parcing config file
	const char* namePtr = Wrapper::GetWrapperName(!WrapperMode.empty() ? WrapperMode.c_str() : StubName.c_str());
	if (namePtr)
	{
		IsDdraw = _stricmp(namePtr, "ddraw.dll") == 0;
		IsD3d8 = _stricmp(namePtr, "d3d8.dll") == 0;
		IsDinput = _stricmp(namePtr, "dinput.dll") == 0;
	}
}

static void LoadDxWrapper(HMODULE hModule)
{
	// Path for dxwrapper file
	char path[MAX_PATH] = {};

	// Get config file path
	GetModuleFileNameA(hModule, path, sizeof(path));
	strcpy_s(strrchr(path, '\\'), MAX_PATH - strlen(path), "\\dxwrapper.dll");

	// Check if the dll is already loaded
	bool AlreadyLoaded = false;
	wrapper_dll = GetModuleHandle("dxwrapper.dll");
	if (!wrapper_dll)
	{
		wrapper_dll = GetModuleHandle("dxwrapper.asi");
	}
	if (wrapper_dll)
	{
		// Already loaded
		AlreadyLoaded = true;
	}
#if (_WIN32_WINNT >= 0x0502)
	// Use MemoryModule to load dxwrapper
	else if (LoadFromMemory)
	{
		std::ifstream myfile;

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
		DxWrapperLoggingProc DxWrapperLogging = (DxWrapperLoggingProc)Hook::GetProcAddress(wrapper_dll, "DxWrapperLogging");
		if (DxWrapperLogging)
		{
			if (AlreadyLoaded)
			{
				DxWrapperLogging(std::string("Secondary stub detected: " + StubName).c_str());
			}
			else
			{
				DxWrapperLogging(std::string("DxWrapper loaded by stub: " + StubName).c_str());
			}
		}
	}
	else
	{
		MessageBoxA(nullptr, "Could not find DxWrapper.dll functions will be disabled!", "DxWrapper Stub", MB_ICONWARNING | MB_TOPMOST | MB_SETFOREGROUND);
	}
}

static void LoadRealDLL()
{
	// Load custom wrapper
	if (DxSettings.Dd7to9 && IsDdraw)
	{
		DdrawWrapper::Start(!RealDllPath.empty() ? RealDllPath.c_str() : nullptr);
	}
	else if (DxSettings.D3d8to9 && IsD3d8)
	{
		D3d8Wrapper::Start(!RealDllPath.empty() ? RealDllPath.c_str() : nullptr);
	}
	else if (DxSettings.Dinputto8 && IsDinput)
	{
		DinputWrapper::Start(!RealDllPath.empty() ? RealDllPath.c_str() : nullptr);
	}
	// Start normal wrapper
	else
	{
		proxy_dll = Wrapper::CreateWrapper(!RealDllPath.empty() ? RealDllPath.c_str() : nullptr, !WrapperMode.empty() ? WrapperMode.c_str() : nullptr, StubName.c_str());
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

		// Load dlls before loading dxwrapper
		bool PreLoadRealDll = !IsDdraw && !IsD3d8 && !IsDinput;
		if (PreLoadRealDll)
		{
			LoadRealDLL();
		}

		// Load DxWrapper
		if (!StubOnly)
		{
			LoadDxWrapper(hModule);
		}

		// Load specific dlls after loading dxwrapper
		if (!PreLoadRealDll)
		{
			LoadRealDLL();
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
