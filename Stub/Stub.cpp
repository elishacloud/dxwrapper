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
#include <fstream>
#include "..\Settings\ReadParse.h"
#include "..\MemoryModule\MemoryModule.h"
#include "..\Wrappers\wrapper.h"

std::string RealDllPath;			// Manually set Dll to wrap
std::string WrapperMode;			// Name of dxwrapper

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

	static HMEMORYMODULE wrapper_dll = nullptr;
	static HMODULE proxy_dll = nullptr;

	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
	{
		// Get config file path
		char configname[MAX_PATH];
		GetModuleFileNameA(hModule, configname, MAX_PATH);
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
		proxy_dll = Wrapper::CreateWrapper(hModule, (RealDllPath.size()) ? RealDllPath.c_str() : nullptr, (WrapperMode.size()) ? WrapperMode.c_str() : nullptr);

		// Open file and get size
		char path[MAX_PATH];
		std::ifstream myfile;

		// Get config file path
		GetModuleFileNameA(hModule, path, sizeof(path));
		strcpy_s(strrchr(path, '\\'), MAX_PATH - strlen(path), "\\dxwrapper.dll");

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
			wrapper_dll = MemoryLoadLibrary(&memblock[0], size);
		}

		// Close the file
		myfile.close();

		// Check if DxWrapper is loaded
		if (!wrapper_dll)
		{
			MessageBoxA(nullptr, "Could not find DxWrapper.dll functions will be disabled!", "DxWrapper Stub", MB_ICONWARNING | MB_TOPMOST | MB_SETFOREGROUND);
		}
	}
	break;
	case DLL_PROCESS_DETACH:

		// Unload wrapped dll
		if (proxy_dll)
		{
			FreeLibrary(proxy_dll);
		}

		// Unload dxwrapper dll from memory
		if (wrapper_dll)
		{
			MemoryFreeLibrary(wrapper_dll);
		}
		break;
	}

	return true;
}
