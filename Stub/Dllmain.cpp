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
#include "..\MemoryModule\MemoryModule.h"
#include "..\Wrappers\wrapper.h"

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
		// Start wrapper
		proxy_dll = Wrapper::CreateWrapper(hModule);

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
			char * memblock;
			memblock = new char[size];
			myfile.read(memblock, size);

			// Load library into memory
			wrapper_dll = MemoryLoadLibrary(memblock, size);

			// Delete memory block from the loaded file
			delete[] memblock;
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
