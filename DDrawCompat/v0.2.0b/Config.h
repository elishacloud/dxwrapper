#pragma once

namespace Compat20
{
	typedef unsigned long DWORD;

	namespace Config
	{
		const DWORD minRefreshInterval = 1000 / 60;
		const DWORD minRefreshIntervalAfterFlip = 1000 / 10;
	}
}
