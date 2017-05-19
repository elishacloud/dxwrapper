/**
 * Copyright (C) 2015 Patrick Mours. All rights reserved.
 * License: https://github.com/crosire/d3d8to9#license
 *
 * Updated 2017 by Elisha Riedlinger
 */

#include "d3dx9.hpp"
#include "d3d8to9.hpp"
#include "wrappers\wrapper.h"

typedef LPDIRECT3D9(WINAPI *PFN_Direct3DCreate9)(UINT SDKVersion);

PFN_D3DXAssembleShader D3DXAssembleShader = nullptr;
PFN_D3DXDisassembleShader D3DXDisassembleShader = nullptr;
PFN_D3DXLoadSurfaceFromSurface D3DXLoadSurfaceFromSurface = nullptr;

// Very simple logging for the purpose of debugging only.
//std::ofstream LOG;

extern "C" Direct3D8 *WINAPI _Direct3DCreate8(UINT SDKVersion)
{
	UNREFERENCED_PARAMETER(SDKVersion);

	/*LOG.open("d3d8.log", std::ios::trunc);

	if (!LOG.is_open())
	{
		MessageBoxA(nullptr, "Failed to open debug log file \"d3d8.log\"!", nullptr, MB_ICONWARNING);
	}*/

#ifdef _DEBUG
	Compat::Log() << "Redirecting '" << "Direct3DCreate8" << "(" << SDKVersion << ")' ...";
	Compat::Log() << "> Passing on to 'Direct3DCreate9':";
#endif

	// Declare Direct3DCreate9
	static PFN_Direct3DCreate9 Direct3DCreate9 = nullptr;

	// Load d3d9.dll
	if (!Direct3DCreate9)
	{
		// Load module
		static HMODULE d3d9Module = LoadDll("d3d9.dll", dtype.d3d9);

		// Check if module is loaded
		if (!d3d9Module)
		{
			Compat::Log() << "Failed to load d3d9.dll!";
			return nullptr;
		}

		// Get function ProcAddress
		Direct3DCreate9 = reinterpret_cast<PFN_Direct3DCreate9>(GetProcAddress(d3d9Module, "Direct3DCreate9"));
		if (!Direct3DCreate9)
		{
			Compat::Log() << "Failed to get 'Direct3DCreate9' ProcAddress of d3d9.dll!";
			return nullptr;
		}
	}

	// Create Direct3D9 interface
	IDirect3D9 *const d3d = Direct3DCreate9(D3D_SDK_VERSION);

	// Check for valid Direct3D9 interface
	if (d3d == nullptr)
	{
		return nullptr;
	}

	// Load D3DX
	if (!D3DXAssembleShader || !D3DXDisassembleShader || !D3DXLoadSurfaceFromSurface)
	{
		// Declare module vars
		static HMODULE module = NULL;

		// Load d3dx9_xx.dll module
		if (!module)
		{
			// Declare d3dx9_xx.dll name
			char d3dx9name[MAX_PATH];

			// Declare d3dx9_xx.dll version
			for (int x = 99; x > 9 && module == NULL; x--)
			{
				// Get dll name
				strcpy_s(d3dx9name, "d3dx9_");
				char buffer[11];
				_itoa_s(x, buffer, 10);
				strcat_s(d3dx9name, buffer);
				strcat_s(d3dx9name, ".dll");

				// Load dll
				module = LoadLibrary(d3dx9name);
			}

			// Check if module is loaded
			if (module != nullptr)
			{
				Compat::Log() << "Loaded " << d3dx9name;
				D3DXAssembleShader = reinterpret_cast<PFN_D3DXAssembleShader>(GetProcAddress(module, "D3DXAssembleShader"));
				D3DXDisassembleShader = reinterpret_cast<PFN_D3DXDisassembleShader>(GetProcAddress(module, "D3DXDisassembleShader"));
				D3DXLoadSurfaceFromSurface = reinterpret_cast<PFN_D3DXLoadSurfaceFromSurface>(GetProcAddress(module, "D3DXLoadSurfaceFromSurface"));
			}
			else
			{
				Compat::Log() << "Failed to load d3dx9_xx.dll! Some features will not work correctly.";
			}
		}
	}

	return new Direct3D8(d3d);
}