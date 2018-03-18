#include <cstring>
#include <vector>

#include "Common/CompatPtr.h"
#include "DDrawCompat\DDrawLog.h"
#include "Config/Config.h"
#include "DDraw/Repository.h"
#include "DDraw/Surfaces/PrimarySurface.h"
#include "Dll/Procs.h"
#include "Gdi/DcCache.h"

namespace
{
	using Gdi::DcCache::CachedDc;
	
	std::vector<CachedDc> g_cache;
	DWORD g_cacheSize = 0;
	DWORD g_cacheId = 0;
	DWORD g_maxUsedCacheSize = 0;
	DWORD g_ddLockThreadId = 0;

	CompatWeakPtr<IDirectDrawPalette> g_palette;
	PALETTEENTRY g_paletteEntries[256] = {};
	void* g_surfaceMemory = nullptr;
	LONG g_pitch = 0;

	CompatPtr<IDirectDrawSurface7> createGdiSurface();

	CachedDc createCachedDc()
	{
		CachedDc cachedDc = {};

		CompatPtr<IDirectDrawSurface7> surface(createGdiSurface());
		if (!surface)
		{
			return cachedDc;
		}

		HDC dc = nullptr;
		HRESULT result = surface->GetDC(surface, &dc);
		if (FAILED(result))
		{
			LOG_ONCE("Failed to create a GDI DC: " << result);
			return cachedDc;
		}

		// Release DD critical section acquired by IDirectDrawSurface7::GetDC to avoid deadlocks
		Dll::g_origProcs.ReleaseDDThreadLock();

		cachedDc.surface = surface.detach();
		cachedDc.dc = dc;
		cachedDc.cacheId = g_cacheId;
		return cachedDc;
	}

	CompatPtr<IDirectDrawSurface7> createGdiSurface()
	{
		DDSURFACEDESC2 desc = DDraw::PrimarySurface::getDesc();
		desc.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT | DDSD_CAPS | DDSD_PITCH | DDSD_LPSURFACE;
		desc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
		desc.lPitch = g_pitch;
		desc.lpSurface = g_surfaceMemory;

		auto dd(DDraw::Repository::getDirectDraw());
		CompatPtr<IDirectDrawSurface7> surface;
		HRESULT result = dd->CreateSurface(dd, &desc, &surface.getRef(), nullptr);
		if (FAILED(result))
		{
			LOG_ONCE("Failed to create a GDI surface: " << result);
			return nullptr;
		}

		if (desc.ddpfPixelFormat.dwRGBBitCount <= 8)
		{
			surface->SetPalette(surface, g_palette);
		}

		return surface;
	}

	void extendCache()
	{
		if (g_cacheSize >= Config::preallocatedGdiDcCount)
		{
			LOG_ONCE("Warning: Preallocated GDI DC count is insufficient. This may lead to graphical issues.");
		}

		if (GetCurrentThreadId() != g_ddLockThreadId)
		{
			return;
		}

		for (DWORD i = 0; i < Config::preallocatedGdiDcCount; ++i)
		{
			CachedDc cachedDc = createCachedDc();
			if (!cachedDc.dc)
			{
				return;
			}
			g_cache.push_back(cachedDc);
			++g_cacheSize;
		}
	}

	void releaseCachedDc(CachedDc cachedDc)
	{
		// Reacquire DD critical section that was temporarily released after IDirectDrawSurface7::GetDC
		Dll::g_origProcs.AcquireDDThreadLock();

		HRESULT result = cachedDc.surface->ReleaseDC(cachedDc.surface, cachedDc.dc);
		if (FAILED(result))
		{
			LOG_ONCE("Failed to release a cached DC: " << result);
		}

		cachedDc.surface.release();
	}
}

namespace Gdi
{
	namespace DcCache
	{
		void clear()
		{
			for (auto& cachedDc : g_cache)
			{
				releaseCachedDc(cachedDc);
			}
			g_cache.clear();
			g_cacheSize = 0;
			++g_cacheId;
		}

		CachedDc getDc()
		{
			CachedDc cachedDc = {};
			if (!g_surfaceMemory)
			{
				return cachedDc;
			}

			if (g_cache.empty())
			{
				extendCache();
			}

			if (!g_cache.empty())
			{
				cachedDc = g_cache.back();
				g_cache.pop_back();

				const DWORD usedCacheSize = g_cacheSize - g_cache.size();
				if (usedCacheSize > g_maxUsedCacheSize)
				{
					g_maxUsedCacheSize = usedCacheSize;
					Compat::Log() << "GDI used DC cache size: " << g_maxUsedCacheSize;
				}
			}

			return cachedDc;
		}

		bool init()
		{
			auto dd(DDraw::Repository::getDirectDraw());
			dd->CreatePalette(dd,
				DDPCAPS_8BIT | DDPCAPS_ALLOW256, g_paletteEntries, &g_palette.getRef(), nullptr);
			return nullptr != g_palette;
		}

		void releaseDc(const CachedDc& cachedDc)
		{
			if (cachedDc.cacheId == g_cacheId)
			{
				g_cache.push_back(cachedDc);
			}
			else
			{
				releaseCachedDc(cachedDc);
			}
		}

		void setDdLockThreadId(DWORD ddLockThreadId)
		{
			g_ddLockThreadId = ddLockThreadId;
		}

		void setSurfaceMemory(void* surfaceMemory, LONG pitch)
		{
			if (g_surfaceMemory == surfaceMemory && g_pitch == pitch)
			{
				return;
			}

			g_surfaceMemory = surfaceMemory;
			g_pitch = pitch;
			clear();
		}

		void updatePalette(DWORD startingEntry, DWORD count)
		{
			PALETTEENTRY entries[256] = {};
			std::memcpy(&entries[startingEntry],
				&DDraw::PrimarySurface::s_paletteEntries[startingEntry],
				count * sizeof(PALETTEENTRY));

			for (DWORD i = startingEntry; i < startingEntry + count; ++i)
			{
				if (entries[i].peFlags & PC_RESERVED)
				{
					entries[i] = DDraw::PrimarySurface::s_paletteEntries[0];
					entries[i].peFlags = DDraw::PrimarySurface::s_paletteEntries[i].peFlags;
				}
			}

			if (0 != std::memcmp(&g_paletteEntries[startingEntry], &entries[startingEntry],
				count * sizeof(PALETTEENTRY)))
			{
				std::memcpy(&g_paletteEntries[startingEntry], &entries[startingEntry],
					count * sizeof(PALETTEENTRY));
				g_palette->SetEntries(g_palette, 0, startingEntry, count, g_paletteEntries);
				clear();
			}
		}
	}
}
