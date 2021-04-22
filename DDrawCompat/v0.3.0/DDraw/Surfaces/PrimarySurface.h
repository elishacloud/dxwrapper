#pragma once

#include <ddraw.h>

#include <Common/CompatPtr.h>
#include <Common/CompatRef.h>
#include <DDraw/Surfaces/Surface.h>

namespace DDraw
{
	class PrimarySurface : public Surface
	{
	public:
		PrimarySurface(DWORD origCaps) : Surface(origCaps) {}
		virtual ~PrimarySurface();

		template <typename TDirectDraw, typename TSurface, typename TSurfaceDesc>
		static HRESULT create(CompatRef<TDirectDraw> dd, TSurfaceDesc desc, TSurface*& surface);

		static HRESULT flipToGdiSurface();
		static CompatPtr<IDirectDrawSurface7> getGdiSurface();
		static CompatPtr<IDirectDrawSurface7> getBackBuffer();
		static CompatPtr<IDirectDrawSurface7> getLastSurface();
		static CompatWeakPtr<IDirectDrawSurface7> getPrimary();
		static HANDLE getFrontResource();
		static DWORD getOrigCaps();
		static void updatePalette();

		template <typename TSurface>
		static bool isGdiSurface(TSurface* surface);

		static void updateFrontResource();

		virtual void restore();

		static CompatWeakPtr<IDirectDrawPalette> s_palette;

	private:
		virtual void createImpl() override;
	};
}
