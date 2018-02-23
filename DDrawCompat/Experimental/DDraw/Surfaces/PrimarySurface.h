#pragma once

#include <vector>

#include "Common/CompatPtr.h"
#include "Common/CompatRef.h"
#include "DDraw/Surfaces/Surface.h"

namespace DDraw
{
	class PrimarySurface : public Surface
	{
	public:
		virtual ~PrimarySurface();

		template <typename TDirectDraw, typename TSurface, typename TSurfaceDesc>
		static HRESULT create(CompatRef<TDirectDraw> dd, TSurfaceDesc desc, TSurface*& surface);

		static HRESULT flipToGdiSurface();
		static const DDSURFACEDESC2& getDesc();
		static CompatPtr<IDirectDrawSurface7> getGdiSurface();
		static CompatWeakPtr<IDirectDrawSurface7> getPrimary();
		static DWORD getOrigCaps();

		void updateGdiSurfacePtr(IDirectDrawSurface* flipTargetOverride);

		static CompatWeakPtr<IDirectDrawPalette> s_palette;
		static PALETTEENTRY s_paletteEntries[256];

	private:
		PrimarySurface(Surface* surface);

		virtual void createImpl() override;

		static void resizeBuffers(CompatRef<IDirectDrawSurface7> surface);

		std::unique_ptr<Surface> m_surface;

		static std::vector<std::vector<unsigned char>> s_surfaceBuffers;
	};
}
