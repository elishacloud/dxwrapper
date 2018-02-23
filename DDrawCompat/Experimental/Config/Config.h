#pragma once

typedef unsigned long DWORD;

namespace Config
{
	const int maxPaletteUpdatesPerMs = 5;
	const int minExpectedFlipsPerSec = 5;
	const DWORD preallocatedGdiDcCount = 4;
	const DWORD primarySurfaceExtraRows = 2;
}
