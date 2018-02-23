#pragma once

namespace Compat21
{
	typedef unsigned long DWORD;

	namespace Config
	{
		const int maxPaletteUpdatesPerMs = 10;
		const int maxPrimaryUpdateRate = 60;
		const int primaryUpdateDelayAfterFlip = 100;
		const DWORD preallocatedGdiDcCount = 4;
	}
}
