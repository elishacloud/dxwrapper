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
*
* ASI plugin loader taken from source code found in Ultimate ASI Loader
* https://github.com/ThirteenAG/Ultimate-ASI-Loader
*/

// Default
#include "Settings\Settings.h"
#include "Dllmain\Dllmain.h"
#include "Hooking\Hook.h"
#include "Logging\Logging.h"
#include "wrapper.h"
// Libraries
#include "d3dx9.h"
#include "dwmapi.h"
#include "uxtheme.h"

using namespace Wrapper;

// Wrappers
#include "bcrypt.h"
#include "cryptsp.h"
#include "D3d8to9\d3d8.h"
#include "d3d8.h"
#include "d3d9.h"
#include "dsound.h"
#include "ddraw.h"
#include "dinput.h"
#include "dplayx.h"
#include "dxgi.h"
#include "winmm.h"
#include "winspool.h"
#include "dciman32.h"

namespace Wrapper
{
	// Declare varables
	std::vector<HMODULE> custom_dll;					// Used for custom dll's and asi plugins
	HMODULE dllhandle[dtypeArraySize] = { nullptr };	// Used for wrapper dll's

	// Function declarations
	void LoadPlugins();
	void FindFiles(WIN32_FIND_DATA*);
	void LoadCustomDll();
	HRESULT WINAPI ReturnProc(...);
	const FARPROC _jmpaddr = (FARPROC)*ReturnProc;
}

// Wrapper classes
VISIT_WRAPPERS(ADD_NAMESPACE_CLASS)

// Proc functions
VISIT_WRAPPERS(CREATE_ALL_PROC_STUB)

// Default function
HRESULT WINAPI Wrapper::ReturnProc(...)
{
	// Do nothing
	return E_NOTIMPL;
}

// Get pointer for funtion name use custom return value
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

// Load real dll file that is being wrapped
HMODULE Wrapper::LoadDll(DWORD dlltype)
{
	// Check for valid dlltype
	if (dlltype == 0 || dlltype >= dtypeArraySize)
	{
		return nullptr;
	}

	// Check if dll is already loaded
	if (dllhandle[dlltype])
	{
		return dllhandle[dlltype];
	}

	// Load dll from ini, if DllPath is not '0'
	if (!Config.szDllPath.empty() && Config.WrapperMode == dlltype)
	{
		Logging::Log() << "Loading " << Config.szDllPath << " library";
		dllhandle[dlltype] = LoadLibrary(Config.szDllPath.c_str());
		if (!dllhandle[dlltype])
		{
			Logging::Log() << "Cannot load " << Config.szDllPath << " library";
		}
	}

	// Load current dll
	if (!dllhandle[dlltype] && Config.WrapperMode != dlltype)
	{
		Logging::Log() << "Loading " << dtypename[dlltype] << " library";
		dllhandle[dlltype] = LoadLibrary(dtypename[dlltype]);
		if (!dllhandle[dlltype])
		{
			Logging::Log() << "Cannot load " << dtypename[dlltype] << " library";
		}
	}

	// Load default system dll
	if (!dllhandle[dlltype])
	{
		char path[MAX_PATH];
		GetSystemDirectory(path, MAX_PATH);
		strcat_s(path, MAX_PATH, "\\");
		strcat_s(path, MAX_PATH, dtypename[dlltype]);
		Logging::Log() << "Loading " << path << " library";
		dllhandle[dlltype] = LoadLibrary(path);
	}

	// Cannot load dll
	if (!dllhandle[dlltype])
	{
		Logging::Log() << "Cannot load " << dtypename[dlltype] << " library";
		if (Config.WrapperMode != dtype.Auto)
		{
			ExitProcess(0);
		}
	}

	// Return dll handle
	return dllhandle[dlltype];
}

// Load custom dll files
void Wrapper::LoadCustomDll()
{
	for (size_t x = 0; x < Config.szCustomDllPath.size(); ++x)
	{
		// Check if path is empty
		if (!Config.szCustomDllPath[x].empty())
		{
			Logging::Log() << "Loading custom " << Config.szCustomDllPath[x] << " library";
			// Load dll from ini
			auto h = LoadLibrary(Config.szCustomDllPath[x].c_str());

			// Cannot load dll
			if (!h)
			{
				Logging::Log() << "Cannot load custom " << Config.szCustomDllPath[x] << " library";
			}
			else
			{
				custom_dll.push_back(h);
			}
		}
	}
}

// Find asi plugins to load
void Wrapper::FindFiles(WIN32_FIND_DATA* fd)
{
	char dir[MAX_PATH] = { 0 };
	GetCurrentDirectory(MAX_PATH, dir);

	HANDLE asiFile = FindFirstFile("*.asi", fd);
	if (asiFile != INVALID_HANDLE_VALUE)
	{
		do {
			if (!(fd->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				auto pos = strlen(fd->cFileName);

				if (fd->cFileName[pos - 4] == '.' &&
					(fd->cFileName[pos - 3] == 'a' || fd->cFileName[pos - 3] == 'A') &&
					(fd->cFileName[pos - 2] == 's' || fd->cFileName[pos - 2] == 'S') &&
					(fd->cFileName[pos - 1] == 'i' || fd->cFileName[pos - 1] == 'I'))
				{
					char path[MAX_PATH] = { 0 };
					sprintf_s(path, "%s\\%s", dir, fd->cFileName);

					Logging::Log() << "Loading Plugin: " << path;
					auto h = LoadLibrary(path);
					SetCurrentDirectory(dir); //in case asi switched it

					if (!h)
					{
						Logging::LogFormat("Unable to load %s. Error: %d", fd->cFileName, GetLastError());
					}
					else
					{
						custom_dll.push_back(h);
					}
				}
			}
		} while (FindNextFile(asiFile, fd));
		FindClose(asiFile);
	}
}

// Load asi plugins
void Wrapper::LoadPlugins()
{
	Logging::Log() << "Loading ASI Plugins";

	char oldDir[MAX_PATH]; // store the current directory
	GetCurrentDirectory(MAX_PATH, oldDir);

	char selfPath[MAX_PATH];
	GetModuleFileName(hModule_dll, selfPath, MAX_PATH);
	*strrchr(selfPath, '\\') = '\0';
	SetCurrentDirectory(selfPath);

	WIN32_FIND_DATA fd;
	if (!Config.LoadFromScriptsOnly)
		FindFiles(&fd);

	SetCurrentDirectory(selfPath);

	if (SetCurrentDirectory("scripts\\"))
		FindFiles(&fd);

	SetCurrentDirectory(selfPath);

	if (SetCurrentDirectory("plugins\\"))
		FindFiles(&fd);

	SetCurrentDirectory(oldDir); // Reset the current directory
}

// Load wrapper dll files
void Wrapper::DllAttach()
{
	// Load wrappers
	VISIT_WRAPPERS(LOAD_WRAPPER);

	// Load custom dlls
	if (Config.szCustomDllPath.size() != 0)
	{
		LoadCustomDll();
	}

	// Load ASI plugins
	if (Config.LoadPlugins)
	{
		LoadPlugins();
	}
}

// Unload all dll files loaded by the wrapper
void Wrapper::DllDetach()
{
	// Unload custom libraries
	while (Config.szCustomDllPath.size() != 0)
	{
		// Unload dll
		FreeLibrary(custom_dll.back());
		custom_dll.pop_back();
	}
	custom_dll.clear();

	// Unload wrapper libraries
	for (int x = 1; x < dtypeArraySize; ++x)
	{
		// If dll was loaded
		if (dllhandle[x])
		{
			// Unload dll
			FreeLibrary(dllhandle[x]);
		}
	}

	// Unload dynmaic libraries
	UnLoadd3dx9();
	UnLoaddwmapi();
	UnLoadUxtheme();
}
