#pragma once

#include <ddraw.h>

namespace DDraw
{
	namespace DirectDrawPalette
	{
		void waitForNextUpdate();

		void hookVtable(const IDirectDrawPaletteVtbl& vtable);
	}
}
