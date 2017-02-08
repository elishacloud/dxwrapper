#pragma once

#define CINTERFACE

#include <ddraw.h>

class RealPrimarySurface
{
public:
	template <typename DirectDraw>
	static HRESULT create(DirectDraw& dd);

	static HRESULT flip(DWORD flags);
	static IDirectDrawSurface7* getSurface();
	static bool isFullScreen();
	static bool isLost();
	static void release();
	static HRESULT restore();
	static void setClipper(LPDIRECTDRAWCLIPPER clipper);
	static void setPalette(LPDIRECTDRAWPALETTE palette);
	static void update();
	static void updatePalette();

	static DDSURFACEDESC2 s_surfaceDesc;
};
