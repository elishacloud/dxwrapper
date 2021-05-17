#pragma once

#include <Windows.h>

#include <DDrawCompat/v0.3.1/Common/CompatPtr.h>

namespace Gdi
{
	class Region;

	namespace VirtualScreen
	{
		HDC createDc(bool useDefaultPalette);
		HBITMAP createDib(bool useDefaultPalette);
		HBITMAP createOffScreenDib(LONG width, LONG height, bool useDefaultPalette);
		CompatPtr<IDirectDrawSurface7> createSurface(const RECT& rect);
		void deleteDc(HDC dc);

		RECT getBounds();
		Region getRegion();
		DDSURFACEDESC2 getSurfaceDesc(const RECT& rect);

		void init();
		bool update();
		void updatePalette(PALETTEENTRY(&palette)[256]);
	}
}
