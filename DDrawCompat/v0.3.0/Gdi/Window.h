#pragma once

#include <ddraw.h>

#include <DDrawCompat/v0.3.0/Common/CompatRef.h>
#include <DDrawCompat/v0.3.0/Gdi/Region.h>

namespace Gdi
{
	namespace Window
	{
		void onStyleChanged(HWND hwnd, WPARAM wParam);
		void onSyncPaint(HWND hwnd);
		void present(CompatRef<IDirectDrawSurface7> dst, CompatRef<IDirectDrawSurface7> src,
			CompatRef<IDirectDrawClipper> clipper);
		void present(Gdi::Region excludeRegion);
		void presentLayered(CompatRef<IDirectDrawSurface7> dst, POINT offset);
		void updateAll();
		void updateLayeredWindowInfo(HWND hwnd, COLORREF colorKey, BYTE alpha);
	}
}
