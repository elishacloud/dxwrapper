/**
 * Copyright (C) 2015 Patrick Mours. All rights reserved.
 * License: https://github.com/crosire/d3d8to9#license
 *
 * Updated 2017 by Elisha Riedlinger
 */

#include "d3dx9.hpp"
#include "d3d8to9.hpp"
//********** Begin Edit *************
#include "wrappers\wrapper.h"
typedef LPDIRECT3D9(WINAPI *PFN_Direct3DCreate9)(UINT SDKVersion);
//********** End Edit ***************

PFN_D3DXAssembleShader D3DXAssembleShader = nullptr;
PFN_D3DXDisassembleShader D3DXDisassembleShader = nullptr;
PFN_D3DXLoadSurfaceFromSurface D3DXLoadSurfaceFromSurface = nullptr;

#ifndef D3D8TO9NOLOG
 // Very simple logging for the purpose of debugging only.
std::ofstream LOG;
#endif

extern "C" Direct3D8 *WINAPI Direct3DCreate8(UINT SDKVersion)
{
#ifndef D3D8TO9NOLOG
	LOG.open("d3d8.log", std::ios::trunc);

	if (!LOG.is_open())
	{
		MessageBox(nullptr, TEXT("Failed to open debug log file \"d3d8.log\"!"), nullptr, MB_ICONWARNING);
	}

	LOG << "Redirecting '" << "Direct3DCreate8" << "(" << SDKVersion << ")' ..." << std::endl;
	LOG << "> Passing on to 'Direct3DCreate9':" << std::endl;
#endif
	//********** Begin Edit *************
	Compat::Log() << "Enabling D3d8to9 function (" << SDKVersion << ")";

	// Declare Direct3DCreate9
	static PFN_Direct3DCreate9 Direct3DCreate9 = nullptr;

	// Load d3d9.dll
	if (!Direct3DCreate9)
	{
		// Load module
		static HMODULE d3d9Module = LoadDll(dtype.d3d9);

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
	//********** End Edit ***************

	IDirect3D9 *const d3d = Direct3DCreate9(D3D_SDK_VERSION);

	if (d3d == nullptr)
	{
		return nullptr;
	}

	// Load D3DX
	if (!D3DXAssembleShader || !D3DXDisassembleShader || !D3DXLoadSurfaceFromSurface)
	{
		//********** Begin Edit *************
		//const HMODULE module = LoadLibrary(TEXT("d3dx9_43.dll"));
		
		// Declare module vars
		static HMODULE module = NULL;
		// Declare d3dx9_xx.dll name
		static char d3dx9name[MAX_PATH];
		if (!module)
		{
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
		}
		//********** End Edit ***************

		if (module != nullptr)
		{
			//********** Begin Edit *************
			Compat::Log() << "Loaded " << d3dx9name;
			//********** End Edit ***************
			D3DXAssembleShader = reinterpret_cast<PFN_D3DXAssembleShader>(GetProcAddress(module, "D3DXAssembleShader"));
			D3DXDisassembleShader = reinterpret_cast<PFN_D3DXDisassembleShader>(GetProcAddress(module, "D3DXDisassembleShader"));
			D3DXLoadSurfaceFromSurface = reinterpret_cast<PFN_D3DXLoadSurfaceFromSurface>(GetProcAddress(module, "D3DXLoadSurfaceFromSurface"));
		}
		else
		{
#ifndef D3D8TO9NOLOG
			LOG << "Failed to load d3dx9_43.dll! Some features will not work correctly." << std::endl;
#endif
			//********** Begin Edit *************
			Compat::Log() << "Failed to load d3dx9_xx.dll! Some features will not work correctly.";
			/*if (MessageBox(nullptr, TEXT(
				"Failed to load d3dx9_43.dll! Some features will not work correctly.\n\n"
				"It's required to install the \"Microsoft DirectX End-User Runtime\" in order to use d3d8to9.\n\n"
				"Please click \"OK\" to open the official download page or \"CANCEL\" to continue anyway."), nullptr, MB_ICONWARNING | MB_TOPMOST | MB_SETFOREGROUND | MB_OKCANCEL | MB_DEFBUTTON1) == IDOK)
			{
				ShellExecute(nullptr, TEXT("open"), TEXT("https://www.microsoft.com/download/details.aspx?id=35"), nullptr, nullptr, SW_SHOW);

				return nullptr;
			}*/
			//********** End Edit ***************
		}
	}

	return new Direct3D8(d3d);
}
