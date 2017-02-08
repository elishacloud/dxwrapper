#pragma once

#define CINTERFACE

#include <ddraw.h>

class IReleaseNotifier;

namespace CompatPrimarySurface
{
	struct DisplayMode
	{
		LONG width;
		LONG height;
		DDPIXELFORMAT pixelFormat;
		DWORD refreshRate;
	};

	template <typename TDirectDraw>
	DisplayMode getDisplayMode(TDirectDraw& dd);

	bool isPrimary(void* surfacePtr);
	void setPrimary(IDirectDrawSurface7* surfacePtr);

	extern DisplayMode displayMode;
	extern bool isDisplayModeChanged;
	extern IDirectDrawSurface7* surface;
	extern LPDIRECTDRAWPALETTE palette;
	extern PALETTEENTRY paletteEntries[256];
	extern LONG width;
	extern LONG height;
	extern DDPIXELFORMAT pixelFormat;
	extern IReleaseNotifier releaseNotifier;
}
