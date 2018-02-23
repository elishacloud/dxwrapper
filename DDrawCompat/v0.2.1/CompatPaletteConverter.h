#pragma once

#define CINTERFACE

#include <ddraw.h>

namespace Compat21
{
	namespace CompatPaletteConverter
	{
		bool create();
		HDC getDc();
		IDirectDrawSurface7* getSurface();
		void release();
		void setClipper(IDirectDrawClipper* clipper);
		void updatePalette(DWORD startingEntry, DWORD count);
	}
}
