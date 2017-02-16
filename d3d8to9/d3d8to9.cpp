/**
 * Copyright (C) 2015 Patrick Mours. All rights reserved.
 * License: https://github.com/crosire/d3d8to9#license
 */

#include "d3d8to9.hpp"

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

	if (Config.Debug)
	{
		Compat::Log() << "Redirecting '" << "Direct3DCreate8" << "(" << SDKVersion << ")' ...";
		Compat::Log() << "> Passing on to 'Direct3DCreate9':";
	}

	IDirect3D9 *const d3d = Direct3DCreate9(D3D_SDK_VERSION);

	if (d3d == nullptr)
	{
		return nullptr;
	}

	return new Direct3D8(d3d);
}
