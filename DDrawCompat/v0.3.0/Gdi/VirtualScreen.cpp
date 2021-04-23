#define WIN32_LEAN_AND_MEAN
#define CINTERFACE

#include <map>

#include <DDrawCompat/v0.3.0/Config/Config.h>
#include <DDrawCompat/v0.3.0/Common/ScopedCriticalSection.h>
#include <DDrawCompat/v0.3.0/D3dDdi/Device.h>
#include <DDrawCompat/v0.3.0/D3dDdi/ScopedCriticalSection.h>
#include <DDrawCompat/v0.3.0/DDraw/DirectDraw.h>
#include <DDrawCompat/v0.3.0/DDraw/RealPrimarySurface.h>
#include <DDrawCompat/v0.3.0/DDraw/ScopedThreadLock.h>
#include <DDrawCompat/v0.3.0/DDraw/Surfaces/PrimarySurface.h>
#include <DDrawCompat/v0.3.0/Gdi/Gdi.h>
#include <DDrawCompat/v0.3.0/Gdi/Region.h>
#include <DDrawCompat/v0.3.0/Gdi/VirtualScreen.h>
#include <DDrawCompat/v0.3.0/Gdi/Window.h>
#include <DDrawCompat/v0.3.0/Win32/DisplayMode.h>

namespace
{
	struct VirtualScreenDc
	{
		bool useDefaultPalette;
	};

	Compat30::CriticalSection g_cs;
	Gdi::Region g_region;
	RECT g_bounds = {};
	DWORD g_bpp = 0;
	LONG g_width = 0;
	LONG g_height = 0;
	DWORD g_pitch = 0;
	HANDLE g_surfaceFileMapping = nullptr;
	void* g_surfaceView = nullptr;

	HGDIOBJ g_stockBitmap = nullptr;
	RGBQUAD g_defaultPalette[256] = {};
	RGBQUAD g_systemPalette[256] = {};
	std::map<HDC, VirtualScreenDc> g_dcs;

	BOOL CALLBACK addMonitorRectToRegion(
		HMONITOR /*hMonitor*/, HDC /*hdcMonitor*/, LPRECT lprcMonitor, LPARAM dwData)
	{
		Gdi::Region& virtualScreenRegion = *reinterpret_cast<Gdi::Region*>(dwData);
		Gdi::Region monitorRegion(*lprcMonitor);
		virtualScreenRegion |= monitorRegion;
		return TRUE;
	}

	RGBQUAD convertToRgbQuad(PALETTEENTRY entry)
	{
		RGBQUAD quad = {};
		quad.rgbRed = entry.peRed;
		quad.rgbGreen = entry.peGreen;
		quad.rgbBlue = entry.peBlue;
		return quad;
	}

	HBITMAP createDibSection(LONG width, LONG height, HANDLE section, bool useDefaultPalette)
	{
		struct BITMAPINFO256 : public BITMAPINFO
		{
			RGBQUAD bmiRemainingColors[255];
		};

		BITMAPINFO256 bmi = {};
		bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
		bmi.bmiHeader.biWidth = width;
		bmi.bmiHeader.biHeight = height;
		bmi.bmiHeader.biPlanes = 1;
		bmi.bmiHeader.biBitCount = static_cast<WORD>(g_bpp);
		bmi.bmiHeader.biCompression = 8 == g_bpp ? BI_RGB : BI_BITFIELDS;

		if (8 == g_bpp)
		{
			if (useDefaultPalette)
			{
				memcpy(bmi.bmiColors, g_defaultPalette, sizeof(g_defaultPalette));
			}
			else
			{
				memcpy(bmi.bmiColors, g_systemPalette, sizeof(g_systemPalette));
			}
		}
		else
		{
			const auto pf = DDraw::DirectDraw::getRgbPixelFormat(g_bpp);
			reinterpret_cast<DWORD&>(bmi.bmiColors[0]) = pf.dwRBitMask;
			reinterpret_cast<DWORD&>(bmi.bmiColors[1]) = pf.dwGBitMask;
			reinterpret_cast<DWORD&>(bmi.bmiColors[2]) = pf.dwBBitMask;
		}

		void* bits = nullptr;
		return CreateDIBSection(nullptr, &bmi, DIB_RGB_COLORS, &bits, section, 8);
	}
}

namespace Gdi
{
	namespace VirtualScreen
	{
		HDC createDc(bool useDefaultPalette)
		{
			Compat30::ScopedCriticalSection lock(g_cs);
			std::unique_ptr<void, decltype(&DeleteObject)> dib(createDib(useDefaultPalette), DeleteObject);
			if (!dib)
			{
				return nullptr;
			}

			std::unique_ptr<HDC__, decltype(&DeleteDC)> dc(CreateCompatibleDC(nullptr), DeleteDC);
			if (!dc)
			{
				return nullptr;
			}

			HGDIOBJ stockBitmap = SelectObject(dc.get(), dib.get());
			if (!stockBitmap)
			{
				return nullptr;
			}

			dib.release();

			g_stockBitmap = stockBitmap;
			g_dcs[dc.get()] = { useDefaultPalette };
			return dc.release();
		}

		HBITMAP createDib(bool useDefaultPalette)
		{
			Compat30::ScopedCriticalSection lock(g_cs);
			if (!g_surfaceFileMapping)
			{
				return nullptr;
			}
			return createDibSection(g_width, -g_height, g_surfaceFileMapping, useDefaultPalette);
		}

		HBITMAP createOffScreenDib(LONG width, LONG height, bool useDefaultPalette)
		{
			Compat30::ScopedCriticalSection lock(g_cs);
			return createDibSection(width, height, nullptr, useDefaultPalette);
		}

		CompatPtr<IDirectDrawSurface7> createSurface(const RECT& rect)
		{
			DDraw::ScopedThreadLock ddLock;
			D3dDdi::ScopedCriticalSection driverLock;
			Compat30::ScopedCriticalSection lock(g_cs);

			auto desc = getSurfaceDesc(rect);
			if (!desc.lpSurface)
			{
				return nullptr;
			}

			auto primary(DDraw::PrimarySurface::getPrimary());
			CompatPtr<IUnknown> ddUnk;
			primary->GetDDInterface(primary, reinterpret_cast<void**>(&ddUnk.getRef()));
			CompatPtr<IDirectDraw7> dd(ddUnk);

			CompatPtr<IDirectDrawSurface7> surface;
			dd.get()->lpVtbl->CreateSurface(dd, &desc, &surface.getRef(), nullptr);
			return surface;
		}

		void deleteDc(HDC dc)
		{
			if (!dc)
			{
				return;
			}

			Compat30::ScopedCriticalSection lock(g_cs);
			DeleteObject(SelectObject(dc, g_stockBitmap));
			DeleteDC(dc);
			g_dcs.erase(dc);
		}

		RECT getBounds()
		{
			Compat30::ScopedCriticalSection lock(g_cs);
			return g_bounds;
		}

		Region getRegion()
		{
			Compat30::ScopedCriticalSection lock(g_cs);
			return g_region;
		}

		DDSURFACEDESC2 getSurfaceDesc(const RECT& rect)
		{
			Compat30::ScopedCriticalSection lock(g_cs);
			if (rect.left < g_bounds.left || rect.top < g_bounds.top ||
				rect.right > g_bounds.right || rect.bottom > g_bounds.bottom)
			{
				return {};
			}

			DDSURFACEDESC2 desc = {};
			desc.dwSize = sizeof(desc);
			desc.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT | DDSD_CAPS | DDSD_PITCH | DDSD_LPSURFACE;
			desc.dwWidth = rect.right - rect.left;
			desc.dwHeight = rect.bottom - rect.top;
			desc.ddpfPixelFormat = DDraw::DirectDraw::getRgbPixelFormat(g_bpp);
			desc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
			desc.lPitch = g_pitch;
			desc.lpSurface = static_cast<unsigned char*>(g_surfaceView) + 8 +
				(rect.top - g_bounds.top) * g_pitch +
				(rect.left - g_bounds.left) * g_bpp / 8;
			return desc;
		}

		void init()
		{
			PALETTEENTRY entries[20] = {};
			HPALETTE defaultPalette = reinterpret_cast<HPALETTE>(GetStockObject(DEFAULT_PALETTE));
			GetPaletteEntries(defaultPalette, 0, 20, entries);

			for (int i = 0; i < 10; ++i)
			{
				g_defaultPalette[i] = convertToRgbQuad(entries[i]);
				g_defaultPalette[246 + i] = convertToRgbQuad(entries[10 + i]);
			}

			update();
		}

		bool update()
		{
			LOG_FUNC("VirtualScreen::update");
			static auto prevDisplaySettingsUniqueness = Win32::DisplayMode::queryDisplaySettingsUniqueness() - 1;

			{
				Compat30::ScopedCriticalSection lock(g_cs);
				if (Win32::DisplayMode::queryDisplaySettingsUniqueness() == prevDisplaySettingsUniqueness &&
					Win32::DisplayMode::getBpp() == g_bpp)
				{
					return LOG_RESULT(false);
				}
			}

			{
				D3dDdi::ScopedCriticalSection driverLock;
				Compat30::ScopedCriticalSection lock(g_cs);

				prevDisplaySettingsUniqueness = Win32::DisplayMode::queryDisplaySettingsUniqueness();
				D3dDdi::Device::setGdiResourceHandle(nullptr);

				g_region = Region();
				EnumDisplayMonitors(nullptr, nullptr, addMonitorRectToRegion, reinterpret_cast<LPARAM>(&g_region));
				GetRgnBox(g_region, &g_bounds);

				g_bpp = Win32::DisplayMode::getBpp();
				g_width = g_bounds.right - g_bounds.left;
				g_height = g_bounds.bottom - g_bounds.top;
				g_pitch = (g_width * g_bpp / 8 + 3) & ~3;

				if (g_surfaceFileMapping)
				{
					for (auto& dc : g_dcs)
					{
						DeleteObject(SelectObject(dc.first, g_stockBitmap));
					}
					UnmapViewOfFile(g_surfaceView);
					CloseHandle(g_surfaceFileMapping);
				}

				g_surfaceFileMapping = CreateFileMapping(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0,
					g_pitch * g_height + 8, nullptr);
				g_surfaceView = MapViewOfFile(g_surfaceFileMapping, FILE_MAP_WRITE, 0, 0, 0);

				for (auto& dc : g_dcs)
				{
					SelectObject(dc.first, createDib(dc.second.useDefaultPalette));
				}
			}

			Gdi::Window::updateAll();
			Gdi::redraw(nullptr);
			return LOG_RESULT(true);
		}

		void updatePalette(PALETTEENTRY(&palette)[256])
		{
			Compat30::ScopedCriticalSection lock(g_cs);

			RGBQUAD systemPalette[256] = {};
			for (int i = 0; i < 256; ++i)
			{
				systemPalette[i] = convertToRgbQuad(palette[i]);
			}

			if (0 != memcmp(g_systemPalette, systemPalette, sizeof(systemPalette)))
			{
				memcpy(g_systemPalette, systemPalette, sizeof(systemPalette));
				for (auto& dc : g_dcs)
				{
					if (!dc.second.useDefaultPalette)
					{
						SetDIBColorTable(dc.first, 0, 256, systemPalette);
					}
				}
			}

			DDraw::RealPrimarySurface::scheduleUpdate();
		}
	}
}
