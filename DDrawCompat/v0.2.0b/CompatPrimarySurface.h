#pragma once

#define CINTERFACE

#include <ddraw.h>

namespace Compat20
{
	class IReleaseNotifier;

	namespace CompatPrimarySurface
	{
		struct DisplayMode
		{
			LONG width;
			LONG height;
			DDPIXELFORMAT pixelFormat;
		};

		template <typename TDirectDraw>
		DisplayMode getDisplayMode(TDirectDraw& dd);

		extern DisplayMode displayMode;
		extern IDirectDrawSurface7* surface;
		extern LPDIRECTDRAWPALETTE palette;
		extern LONG width;
		extern LONG height;
		extern DDPIXELFORMAT pixelFormat;
		extern IReleaseNotifier releaseNotifier;
	}
}
