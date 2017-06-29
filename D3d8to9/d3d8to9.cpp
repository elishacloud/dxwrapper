/**
 * Copyright (C) 2015 Patrick Mours. All rights reserved.
 * License: https://github.com/crosire/d3d8to9#license
 *
 * Updated 2017 by Elisha Riedlinger
 */

//********** Begin Edit *************
#include "d3d8to9.hpp"
#include "wrappers\wrapper.h"
typedef LPDIRECT3D9(WINAPI *PFN_Direct3DCreate9)(UINT SDKVersion);
//********** End Edit ***************

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

	// Load module
	static HMODULE d3d9Module = LoadDll(dtype.d3d9);
	if (!d3d9Module)
	{
		Compat::Log() << "Failed to load d3d9.dll!";
		return nullptr;
	}

	// Declare Direct3DCreate9
	static PFN_Direct3DCreate9 Direct3DCreate9 = reinterpret_cast<PFN_Direct3DCreate9>(GetProcAddress(d3d9Module, "Direct3DCreate9"));
	if (!Direct3DCreate9)
	{
		Compat::Log() << "Failed to get 'Direct3DCreate9' ProcAddress of d3d9.dll!";
		return nullptr;
	}
	//********** End Edit ***************

	IDirect3D9 *const d3d = Direct3DCreate9(D3D_SDK_VERSION);

	if (d3d == nullptr)
	{
		return nullptr;
	}

	return new Direct3D8(d3d);
}
