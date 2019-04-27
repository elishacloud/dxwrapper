#include "CompatDirectDraw.h"
#include "CompatDirectDrawSurface.h"
#include "CompatPrimarySurface.h"
#include "IReleaseNotifier.h"
#include "RealPrimarySurface.h"

namespace Compat20
{
	namespace
	{
		void onRelease()
		{
			Compat::LogEnter("CompatPrimarySurface::onRelease");

			CompatPrimarySurface::surface = nullptr;
			CompatPrimarySurface::palette = nullptr;
			CompatPrimarySurface::width = 0;
			CompatPrimarySurface::height = 0;
			ZeroMemory(&CompatPrimarySurface::pixelFormat, sizeof(CompatPrimarySurface::pixelFormat));

			CompatDirectDrawSurface<IDirectDrawSurface>::resetPrimarySurfacePtr();
			CompatDirectDrawSurface<IDirectDrawSurface2>::resetPrimarySurfacePtr();
			CompatDirectDrawSurface<IDirectDrawSurface3>::resetPrimarySurfacePtr();
			CompatDirectDrawSurface<IDirectDrawSurface4>::resetPrimarySurfacePtr();
			CompatDirectDrawSurface<IDirectDrawSurface7>::resetPrimarySurfacePtr();

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
			return dm;
		}

		template DisplayMode getDisplayMode(IDirectDraw& dd);
		template DisplayMode getDisplayMode(IDirectDraw2& dd);
		template DisplayMode getDisplayMode(IDirectDraw4& dd);
		template DisplayMode getDisplayMode(IDirectDraw7& dd);

		DisplayMode displayMode = {};
		IDirectDrawSurface7* surface = nullptr;
		LPDIRECTDRAWPALETTE palette = nullptr;
		LONG width = 0;
		LONG height = 0;
		DDPIXELFORMAT pixelFormat = {};
		IReleaseNotifier releaseNotifier(onRelease);
	}
}
