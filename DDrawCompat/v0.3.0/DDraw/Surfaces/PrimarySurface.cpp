#define WIN32_LEAN_AND_MEAN
#define CINTERFACE

#include <Common/CompatPtr.h>
#include <Common/CompatRef.h>
#include <Config/Config.h>
#include <D3dDdi/Device.h>
#include <D3dDdi/KernelModeThunks.h>
#include <DDraw/DirectDraw.h>
#include <DDraw/DirectDrawSurface.h>
#include <DDraw/RealPrimarySurface.h>
#include <DDraw/Surfaces/PrimarySurface.h>
#include <DDraw/Surfaces/PrimarySurfaceImpl.h>
#include <Gdi/Palette.h>
#include <Gdi/VirtualScreen.h>

namespace
{
	CompatWeakPtr<IDirectDrawSurface7> g_primarySurface;
	HANDLE g_gdiResourceHandle = nullptr;
	HANDLE g_frontResource = nullptr;
	DWORD g_origCaps = 0;
	HWND g_deviceWindow = nullptr;
	HPALETTE g_palette = nullptr;
}

namespace DDraw
{
	PrimarySurface::~PrimarySurface()
	{
		LOG_FUNC("PrimarySurface::~PrimarySurface");

		g_gdiResourceHandle = nullptr;
		g_frontResource = nullptr;
		g_primarySurface = nullptr;
		g_origCaps = 0;
		g_deviceWindow = nullptr;
		if (g_palette)
		{
			DeleteObject(g_palette);
			g_palette = nullptr;
		}
		s_palette = nullptr;

		DDraw::RealPrimarySurface::release();
	}

	template <typename TDirectDraw, typename TSurface, typename TSurfaceDesc>
	HRESULT PrimarySurface::create(CompatRef<TDirectDraw> dd, TSurfaceDesc desc, TSurface*& surface)
	{
		HRESULT result = RealPrimarySurface::create(dd);
		if (FAILED(result))
		{
			return result;
		}

		const DWORD origCaps = desc.ddsCaps.dwCaps;

		const auto& dm = DDraw::DirectDraw::getDisplayMode(*CompatPtr<IDirectDraw7>::from(&dd));
		desc.dwFlags |= DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;
		desc.dwWidth = dm.dwWidth;
		desc.dwHeight = dm.dwHeight;
		desc.ddsCaps.dwCaps &= ~(DDSCAPS_PRIMARYSURFACE | DDSCAPS_SYSTEMMEMORY |
			DDSCAPS_VIDEOMEMORY | DDSCAPS_LOCALVIDMEM | DDSCAPS_NONLOCALVIDMEM);
		desc.ddsCaps.dwCaps |= DDSCAPS_OFFSCREENPLAIN;
		desc.ddpfPixelFormat = dm.ddpfPixelFormat;

		auto privateData(std::make_unique<PrimarySurface>(origCaps));
		auto data = privateData.get();
		result = Surface::create(dd, desc, surface, std::move(privateData));
		if (FAILED(result))
		{
			Compat30::Log() << "ERROR: Failed to create the compat primary surface: " << Compat30::hex(result);
			RealPrimarySurface::release();
			return result;
		}

		g_origCaps = origCaps;
		g_deviceWindow = DDraw::DirectDraw::getDeviceWindow(dd.get());

		if (desc.ddpfPixelFormat.dwRGBBitCount <= 8)
		{
			LOGPALETTE lp = {};
			lp.palVersion = 0x300;
			lp.palNumEntries = 1;
			g_palette = CreatePalette(&lp);
			ResizePalette(g_palette, 256);
		}

		data->restore();
		return DD_OK;
	}

	template HRESULT PrimarySurface::create(
		CompatRef<IDirectDraw> dd, DDSURFACEDESC desc, IDirectDrawSurface*& surface);
	template HRESULT PrimarySurface::create(
		CompatRef<IDirectDraw2> dd, DDSURFACEDESC desc, IDirectDrawSurface*& surface);
	template HRESULT PrimarySurface::create(
		CompatRef<IDirectDraw4> dd, DDSURFACEDESC2 desc, IDirectDrawSurface4*& surface);
	template HRESULT PrimarySurface::create(
		CompatRef<IDirectDraw7> dd, DDSURFACEDESC2 desc, IDirectDrawSurface7*& surface);

	void PrimarySurface::createImpl()
	{
		m_impl.reset(new PrimarySurfaceImpl<IDirectDrawSurface>(this));
		m_impl2.reset(new PrimarySurfaceImpl<IDirectDrawSurface2>(this));
		m_impl3.reset(new PrimarySurfaceImpl<IDirectDrawSurface3>(this));
		m_impl4.reset(new PrimarySurfaceImpl<IDirectDrawSurface4>(this));
		m_impl7.reset(new PrimarySurfaceImpl<IDirectDrawSurface7>(this));
	}

	HRESULT PrimarySurface::flipToGdiSurface()
	{
		CompatPtr<IDirectDrawSurface7> gdiSurface;
		if (!g_primarySurface || !(gdiSurface = getGdiSurface()))
		{
			return DDERR_NOTFOUND;
		}
		return g_primarySurface.get()->lpVtbl->Flip(g_primarySurface, gdiSurface, DDFLIP_WAIT);
	}

	CompatPtr<IDirectDrawSurface7> PrimarySurface::getGdiSurface()
	{
		if (!g_primarySurface)
		{
			return nullptr;
		}

		DDSCAPS2 caps = {};
		caps.dwCaps = DDSCAPS_FLIP;
		CompatWeakPtr<IDirectDrawSurface7> surface(g_primarySurface);

		do
		{
			if (isGdiSurface(surface.get()))
			{
				return CompatPtr<IDirectDrawSurface7>::from(surface.get());
			}

			if (FAILED(surface->GetAttachedSurface(surface, &caps, &surface.getRef())))
			{
				return nullptr;
			}
			surface->Release(surface);
		} while (surface != g_primarySurface);

		return nullptr;
	}

	CompatPtr<IDirectDrawSurface7> PrimarySurface::getBackBuffer()
	{
		DDSCAPS2 caps = {};
		caps.dwCaps = DDSCAPS_BACKBUFFER;
		CompatPtr<IDirectDrawSurface7> backBuffer;
		g_primarySurface->GetAttachedSurface(g_primarySurface, &caps, &backBuffer.getRef());
		return backBuffer;
	}

	CompatPtr<IDirectDrawSurface7> PrimarySurface::getLastSurface()
	{
		DDSCAPS2 caps = {};
		caps.dwCaps = DDSCAPS_FLIP;
		auto surface(CompatPtr<IDirectDrawSurface7>::from(g_primarySurface.get()));
		CompatPtr<IDirectDrawSurface7> nextSurface;

		while (SUCCEEDED(surface->GetAttachedSurface(surface, &caps, &nextSurface.getRef())) &&
			nextSurface != g_primarySurface)
		{
			surface.swap(nextSurface);
			nextSurface.release();
		}

		return surface;
	}

	CompatWeakPtr<IDirectDrawSurface7> PrimarySurface::getPrimary()
	{
		return g_primarySurface;
	}

	HANDLE PrimarySurface::getFrontResource()
	{
		return g_frontResource;
	}

	DWORD PrimarySurface::getOrigCaps()
	{
		return g_origCaps;
	}

	template <typename TSurface>
	static bool PrimarySurface::isGdiSurface(TSurface* surface)
	{
		return surface && DirectDrawSurface::getRuntimeResourceHandle(*surface) == g_gdiResourceHandle;
	}

	template bool PrimarySurface::isGdiSurface(IDirectDrawSurface*);
	template bool PrimarySurface::isGdiSurface(IDirectDrawSurface2*);
	template bool PrimarySurface::isGdiSurface(IDirectDrawSurface3*);
	template bool PrimarySurface::isGdiSurface(IDirectDrawSurface4*);
	template bool PrimarySurface::isGdiSurface(IDirectDrawSurface7*);

	void PrimarySurface::restore()
	{
		LOG_FUNC("PrimarySurface::restore");

		Gdi::VirtualScreen::update();
		g_primarySurface = m_surface;
		g_gdiResourceHandle = DirectDrawSurface::getRuntimeResourceHandle(*g_primarySurface);

		DDSURFACEDESC2 desc = {};
		desc.dwSize = sizeof(desc);
		m_surface->GetSurfaceDesc(m_surface, &desc);
		if (desc.ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY)
		{
			DDSURFACEDESC2 gdiDesc = Gdi::VirtualScreen::getSurfaceDesc(D3dDdi::KernelModeThunks::getMonitorRect());
			desc.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_PITCH | DDSD_LPSURFACE;
			desc.lPitch = gdiDesc.lPitch;
			desc.lpSurface = gdiDesc.lpSurface;
			m_surface->SetSurfaceDesc(m_surface, &desc, 0);
		}

		updateFrontResource();
		D3dDdi::Device::setGdiResourceHandle(g_frontResource);

		Surface::restore();
	}

	void PrimarySurface::updateFrontResource()
	{
		g_frontResource = DirectDrawSurface::getDriverResourceHandle(*g_primarySurface);
	}

	void PrimarySurface::updatePalette()
	{
		if (!s_palette)
		{
			return;
		}

		PALETTEENTRY entries[256] = {};
		PrimarySurface::s_palette->GetEntries(s_palette, 0, 0, 256, entries);

		if (RealPrimarySurface::isFullScreen())
		{
			Gdi::Palette::setHardwarePalette(entries);
		}
		else
		{
			SetPaletteEntries(g_palette, 0, 256, entries);
			HDC dc = GetDC(g_deviceWindow);
			HPALETTE oldPal = SelectPalette(dc, g_palette, FALSE);
			RealizePalette(dc);
			SelectPalette(dc, oldPal, FALSE);
			ReleaseDC(g_deviceWindow, dc);
		}

		RealPrimarySurface::update();
	}

	CompatWeakPtr<IDirectDrawPalette> PrimarySurface::s_palette;
}
