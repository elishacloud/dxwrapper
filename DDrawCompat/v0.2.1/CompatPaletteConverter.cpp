#include <algorithm>
#include <cstring>

#include "CompatDirectDraw.h"
#include "CompatDirectDrawPalette.h"
#include "CompatDirectDrawSurface.h"
#include "CompatPaletteConverter.h"
#include "CompatPrimarySurface.h"
#include "DDrawRepository.h"
#include "DDrawTypes.h"
#include "Hook.h"
#include "RealPrimarySurface.h"
#include "ScopedCriticalSection.h"

namespace Compat21
{
	namespace
	{
		HDC g_dc = nullptr;
		HGDIOBJ g_oldBitmap = nullptr;
		IDirectDrawSurface7* g_surface = nullptr;

		void convertPaletteEntriesToRgbQuad(RGBQUAD* entries, DWORD count)
		{
			for (DWORD i = 0; i < count; ++i)
			{
				entries[i].rgbReserved = 0;
				std::swap(entries[i].rgbRed, entries[i].rgbBlue);
			}
		}

		HBITMAP createDibSection(void*& bits)
		{
			struct PalettizedBitmapInfo
			{
				BITMAPINFOHEADER header;
				PALETTEENTRY colors[256];
			};

			PalettizedBitmapInfo bmi = {};
			bmi.header.biSize = sizeof(bmi.header);
			bmi.header.biWidth = RealPrimarySurface::s_surfaceDesc.dwWidth;
			bmi.header.biHeight = -static_cast<LONG>(RealPrimarySurface::s_surfaceDesc.dwHeight);
			bmi.header.biPlanes = 1;
			bmi.header.biBitCount = 8;
			bmi.header.biCompression = BI_RGB;
			bmi.header.biClrUsed = 256;

			return CreateDIBSection(nullptr, reinterpret_cast<BITMAPINFO*>(&bmi),
				DIB_RGB_COLORS, &bits, nullptr, 0);
		}

		IDirectDrawSurface7* createSurface(void* bits)
		{
			IDirectDraw7* dd = DDrawRepository::getDirectDraw();
			if (!dd)
			{
				return nullptr;
			}

			DDSURFACEDESC2 desc = {};
			desc.dwSize = sizeof(desc);
			desc.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT | DDSD_CAPS |
				DDSD_PITCH | DDSD_LPSURFACE;
			desc.dwWidth = RealPrimarySurface::s_surfaceDesc.dwWidth;
			desc.dwHeight = RealPrimarySurface::s_surfaceDesc.dwHeight;
			desc.ddpfPixelFormat = CompatPrimarySurface::displayMode.pixelFormat;
			desc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
			desc.lPitch = (RealPrimarySurface::s_surfaceDesc.dwWidth + 3) & ~3;
			desc.lpSurface = bits;

			IDirectDrawSurface7* surface = nullptr;
			CompatDirectDraw<IDirectDraw7>::s_origVtable.CreateSurface(dd, &desc, &surface, nullptr);
			return surface;
		}
	}

	namespace CompatPaletteConverter
	{
		bool create()
		{
			if (CompatPrimarySurface::displayMode.pixelFormat.dwRGBBitCount > 8 &&
				RealPrimarySurface::s_surfaceDesc.ddpfPixelFormat.dwRGBBitCount > 8)
			{
				return true;
			}

			void* bits = nullptr;
			HBITMAP dib = createDibSection(bits);
			if (!dib)
			{
				Logging::Log() << "Failed to create the palette converter DIB section";
				return false;
			}

			IDirectDrawSurface7* surface = createSurface(bits);
			if (!surface)
			{
				Logging::Log() << "Failed to create the palette converter surface";
				DeleteObject(dib);
				return false;
			}

			HDC dc = CALL_ORIG_FUNC(CreateCompatibleDC)(nullptr);
			if (!dc)
			{
				Logging::Log() << "Failed to create the palette converter DC";
				CompatDirectDrawSurface<IDirectDrawSurface7>::s_origVtable.Release(surface);
				DeleteObject(dib);
				return false;
			}

			g_oldBitmap = SelectObject(dc, dib);
			g_dc = dc;
			g_surface = surface;
			return true;
		}

		HDC getDc()
		{
			return g_dc;
		}

		IDirectDrawSurface7* getSurface()
		{
			return g_surface;
		}

		void release()
		{
			if (!g_surface)
			{
				return;
			}

			CompatDirectDrawSurface<IDirectDrawSurface7>::s_origVtable.Release(g_surface);
			g_surface = nullptr;

			DeleteObject(SelectObject(g_dc, g_oldBitmap));
			DeleteDC(g_dc);
			g_dc = nullptr;
		}

		void setClipper(IDirectDrawClipper* clipper)
		{
			if (g_surface)
			{
				HRESULT result = CompatDirectDrawSurface<IDirectDrawSurface7>::s_origVtable.SetClipper(
					g_surface, clipper);
				if (FAILED(result))
				{
					LOG_ONCE("Failed to set a clipper on the palette converter surface: " << result);
				}
			}
		}

		void updatePalette(DWORD startingEntry, DWORD count)
		{
			if (g_dc && CompatPrimarySurface::palette)
			{
				RGBQUAD entries[256] = {};
				std::memcpy(entries, &CompatPrimarySurface::paletteEntries[startingEntry],
					count * sizeof(PALETTEENTRY));
				convertPaletteEntriesToRgbQuad(entries, count);
				SetDIBColorTable(g_dc, startingEntry, count, entries);
			}
		}
	};
}
