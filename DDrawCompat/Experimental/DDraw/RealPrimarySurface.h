#pragma once

#define CINTERFACE

#include <ddraw.h>

#include "Common/CompatWeakPtr.h"
#include "Common/CompatRef.h"

namespace DDraw
{
	class RealPrimarySurface
	{
	public:
		template <typename DirectDraw>
		static HRESULT create(CompatRef<DirectDraw> dd);

		static void disableUpdates();
		static void enableUpdates();
		static HRESULT flip(DWORD flags);
		static HRESULT getGammaRamp(DDGAMMARAMP* rampData);
		static CompatWeakPtr<IDirectDrawSurface7> getSurface();
		static bool isFullScreen();
		static bool isLost();
		static void release();
		static void removeUpdateThread();
		static HRESULT restore();
		static HRESULT setGammaRamp(DDGAMMARAMP* rampData);
		static void setPalette();
		static void update();
		static void updatePalette(DWORD startingEntry, DWORD count);
	};
}
