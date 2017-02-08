#include <algorithm>
#include <vector>

#include "CompatDirectDraw.h"
#include "CompatDirectDrawSurface.h"
#include "CompatPrimarySurface.h"
#include "IReleaseNotifier.h"
#include "RealPrimarySurface.h"

namespace
{
	std::vector<void*> g_primarySurfacePtrs;

	void addPrimary(IDirectDrawSurface7* surface, const IID& iid)
	{
		IUnknown* intf = nullptr;
		CompatDirectDrawSurface<IDirectDrawSurface7>::s_origVtable.QueryInterface(
			surface, iid, reinterpret_cast<void**>(&intf));
		g_primarySurfacePtrs.push_back(intf);
		intf->lpVtbl->Release(intf);
	}

	void onRelease()
	{
		Compat::LogEnter("CompatPrimarySurface::onRelease");

		g_primarySurfacePtrs.clear();
		CompatPrimarySurface::surface = nullptr;
		CompatPrimarySurface::palette = nullptr;
		CompatPrimarySurface::width = 0;
		CompatPrimarySurface::height = 0;
		ZeroMemory(&CompatPrimarySurface::paletteEntries, sizeof(CompatPrimarySurface::paletteEntries));
		ZeroMemory(&CompatPrimarySurface::pixelFormat, sizeof(CompatPrimarySurface::pixelFormat));

		RealPrimarySurface::release();

		Compat::LogLeave("CompatPrimarySurface::onRelease");
	}
}

namespace CompatPrimarySurface
{
	template <typename TDirectDraw>
	DisplayMode getDisplayMode(TDirectDraw& dd)
	{
		DisplayMode dm = {};
		typename CompatDirectDraw<TDirectDraw>::TSurfaceDesc desc = {};
		desc.dwSize = sizeof(desc);
		CompatDirectDraw<TDirectDraw>::s_origVtable.GetDisplayMode(&dd, &desc);
		dm.width = desc.dwWidth;
		dm.height = desc.dwHeight;
		dm.pixelFormat = desc.ddpfPixelFormat;
		dm.refreshRate = desc.dwRefreshRate;
		return dm;
	}

	template DisplayMode getDisplayMode(IDirectDraw& dd);
	template DisplayMode getDisplayMode(IDirectDraw2& dd);
	template DisplayMode getDisplayMode(IDirectDraw4& dd);
	template DisplayMode getDisplayMode(IDirectDraw7& dd);

	bool isPrimary(void* surfacePtr)
	{
		return g_primarySurfacePtrs.end() !=
			std::find(g_primarySurfacePtrs.begin(), g_primarySurfacePtrs.end(), surfacePtr);
	}

	void setPrimary(IDirectDrawSurface7* surfacePtr)
	{
		surface = surfacePtr;

		g_primarySurfacePtrs.clear();
		g_primarySurfacePtrs.push_back(surfacePtr);
		addPrimary(surfacePtr, IID_IDirectDrawSurface4);
		addPrimary(surfacePtr, IID_IDirectDrawSurface3);
		addPrimary(surfacePtr, IID_IDirectDrawSurface2);
		addPrimary(surfacePtr, IID_IDirectDrawSurface);

		IReleaseNotifier* releaseNotifierPtr = &releaseNotifier;
		CompatDirectDrawSurface<IDirectDrawSurface7>::s_origVtable.SetPrivateData(
			surfacePtr, IID_IReleaseNotifier, releaseNotifierPtr, sizeof(releaseNotifierPtr),
			DDSPD_IUNKNOWNPOINTER);
	}

	DisplayMode displayMode = {};
	bool isDisplayModeChanged = false;
	IDirectDrawSurface7* surface = nullptr;
	LPDIRECTDRAWPALETTE palette = nullptr;
	PALETTEENTRY paletteEntries[256] = {};
	LONG width = 0;
	LONG height = 0;
	DDPIXELFORMAT pixelFormat = {};
	IReleaseNotifier releaseNotifier(onRelease);
}
