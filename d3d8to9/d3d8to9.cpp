/**
 * Copyright (C) 2015 Patrick Mours. All rights reserved.
 * License: https://github.com/crosire/d3d8to9#license
 *
 * Updated 2017 by Elisha Riedlinger
 */

#include "d3dx9.hpp"
#include "d3d8to9.hpp"

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

	IDirect3D9 *const d3d = Direct3DCreate9(D3D_SDK_VERSION);

	if (d3d == nullptr)
	{
		return nullptr;
	}

	// Load D3DX
	if (!D3DXAssembleShader || !D3DXDisassembleShader || !D3DXLoadSurfaceFromSurface)
	{
		// Declare vars
		HMODULE dllHandle = NULL;
		char d3dx9name[MAX_PATH];

		// Check for different versions of d3dx9_xx.dll
		for (int x = 99; x > 9 && dllHandle == NULL; x--)
		{
			// Get dll name
			strcpy_s(d3dx9name, "d3dx9_");
			char buffer[11];
			_itoa_s(x, buffer, 10);
			strcat_s(d3dx9name, buffer);
			strcat_s(d3dx9name, ".dll");

			// Load dll
			dllHandle = LoadLibrary(d3dx9name);
		}

		const HMODULE module = dllHandle;

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

	return new Direct3D8(d3d);
}