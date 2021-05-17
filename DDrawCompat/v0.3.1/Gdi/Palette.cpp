#define WIN32_LEAN_AND_MEAN
#define CINTERFACE

#include <map>

#include <DDrawCompat/v0.3.1/Common/Hook.h>
#include <DDrawCompat/DDrawLog.h>
#include <DDrawCompat/v0.3.1/Common/ScopedSrwLock.h>
#include <DDrawCompat/v0.3.1/Gdi/Gdi.h>
#include <DDrawCompat/v0.3.1/Gdi/Palette.h>
#include <DDrawCompat/v0.3.1/Gdi/VirtualScreen.h>
#include <DDrawCompat/v0.3.1/Win32/DisplayMode.h>

namespace
{
	struct PaletteInfo
	{
		bool isForeground;
		bool isRealized;

		PaletteInfo()
			: isForeground(false)
			, isRealized(false)
		{
		}
	};

	Compat31::SrwLock g_srwLock;
	PALETTEENTRY g_defaultPalette[256] = {};
	PALETTEENTRY g_hardwarePalette[256] = {};
	PALETTEENTRY g_systemPalette[256] = {};
	UINT g_systemPaletteUse = SYSPAL_STATIC;
	UINT g_systemPaletteFirstUnusedIndex = 10;
	UINT g_systemPaletteFirstNonReservedIndex = 10;
	UINT g_systemPaletteLastNonReservedIndex = 245;

	std::map<HPALETTE, PaletteInfo> g_paletteInfo;

	bool isSameColor(PALETTEENTRY entry1, PALETTEENTRY entry2)
	{
		return entry1.peRed == entry2.peRed &&
			entry1.peGreen == entry2.peGreen &&
			entry1.peBlue == entry2.peBlue;
	}

	bool exactMatch(PALETTEENTRY entry)
	{
		for (UINT i = 0; i < g_systemPaletteFirstUnusedIndex; ++i)
		{
			if (isSameColor(entry, g_systemPalette[i]))
			{
				return true;
			}
		}

		for (UINT i = g_systemPaletteLastNonReservedIndex + 1; i < 256; ++i)
		{
			if (isSameColor(entry, g_systemPalette[i]))
			{
				return true;
			}
		}

		return false;
	}

	void updateStaticSysPalEntries()
	{
		const UINT count = g_systemPaletteFirstNonReservedIndex;
		if (0 == count)
		{
			return;
		}

		std::memcpy(g_systemPalette, g_defaultPalette, count * sizeof(g_systemPalette[0]));
		std::memcpy(&g_systemPalette[256 - count], &g_defaultPalette[256 - count], count * sizeof(g_systemPalette[0]));
		std::memcpy(g_hardwarePalette, g_systemPalette, sizeof(g_hardwarePalette));
		Gdi::VirtualScreen::updatePalette(g_systemPalette);
	}

	BOOL WINAPI deleteObject(HGDIOBJ ho)
	{
		BOOL result = CALL_ORIG_FUNC(DeleteObject)(ho);
		if (result && OBJ_PAL == GetObjectType(ho))
		{
			Compat31::ScopedSrwLockExclusive lock(g_srwLock);
			g_paletteInfo.erase(static_cast<HPALETTE>(ho));
		}
		return result;
	}

	UINT WINAPI getSystemPaletteEntries(HDC hdc, UINT iStartIndex, UINT nEntries, LPPALETTEENTRY lppe)
	{
		LOG_FUNC("GetSystemPaletteEntries", hdc, iStartIndex, nEntries, lppe);
		if (!Gdi::isDisplayDc(hdc))
		{
			return LOG_RESULT(0);
		}

		if (!lppe)
		{
			return LOG_RESULT(256);
		}

		if (iStartIndex >= 256)
		{
			return LOG_RESULT(0);
		}

		if (nEntries > 256 - iStartIndex)
		{
			nEntries = 256 - iStartIndex;
		}

		Compat31::ScopedSrwLockShared lock(g_srwLock);
		std::memcpy(lppe, &g_systemPalette[iStartIndex], nEntries * sizeof(PALETTEENTRY));

		return LOG_RESULT(nEntries);
	}

	UINT WINAPI getSystemPaletteUse(HDC hdc)
	{
		LOG_FUNC("GetSystemPaletteUse", hdc);
		if (!Gdi::isDisplayDc(hdc))
		{
			return LOG_RESULT(SYSPAL_ERROR);
		}
		Compat31::ScopedSrwLockShared lock(g_srwLock);
		return LOG_RESULT(g_systemPaletteUse);
	}

	UINT WINAPI realizePalette(HDC hdc)
	{
		LOG_FUNC("RealizePalette", hdc);
		if (Gdi::isDisplayDc(hdc))
		{
			HPALETTE palette = reinterpret_cast<HPALETTE>(GetCurrentObject(hdc, OBJ_PAL));
			if (!palette || GetStockObject(DEFAULT_PALETTE) == palette)
			{
				return LOG_RESULT(0);
			}

			PALETTEENTRY entries[256] = {};
			const UINT count = GetPaletteEntries(palette, 0, 256, entries);

			Compat31::ScopedSrwLockExclusive lock(g_srwLock);
			auto& paletteInfo = g_paletteInfo[palette];
			if (paletteInfo.isRealized)
			{
				return LOG_RESULT(0);
			}

			if (paletteInfo.isForeground)
			{
				g_systemPaletteFirstUnusedIndex = g_systemPaletteFirstNonReservedIndex;
				for (auto& pi : g_paletteInfo)
				{
					pi.second.isRealized = false;
				}
			}

			for (UINT i = 0; i < count && g_systemPaletteFirstUnusedIndex <= g_systemPaletteLastNonReservedIndex; ++i)
			{
				if ((entries[i].peFlags & PC_EXPLICIT) ||
					0 == (entries[i].peFlags & (PC_NOCOLLAPSE | PC_RESERVED)) && exactMatch(entries[i]))
				{
					continue;
				}

				g_systemPalette[g_systemPaletteFirstUnusedIndex] = entries[i];
				g_systemPalette[g_systemPaletteFirstUnusedIndex].peFlags = 0;
				++g_systemPaletteFirstUnusedIndex;
			}

			paletteInfo.isRealized = true;
			std::memcpy(g_hardwarePalette, g_systemPalette, sizeof(g_hardwarePalette));
			Gdi::VirtualScreen::updatePalette(g_systemPalette);
			return LOG_RESULT(count);
		}
		return LOG_RESULT(CALL_ORIG_FUNC(RealizePalette)(hdc));
	}

	BOOL WINAPI resizePalette(HPALETTE hpal, UINT n)
	{
		LOG_FUNC("ResizePalette", hpal, n);
		BOOL result = CALL_ORIG_FUNC(ResizePalette)(hpal, n);
		if (result)
		{
			Compat31::ScopedSrwLockExclusive lock(g_srwLock);
			g_paletteInfo[hpal].isRealized = false;
		}
		return LOG_RESULT(result);
	}

	HPALETTE WINAPI selectPalette(HDC hdc, HPALETTE hpal, BOOL bForceBackground)
	{
		LOG_FUNC("SelectPalette", hdc, hpal, bForceBackground);
		HPALETTE result = CALL_ORIG_FUNC(SelectPalette)(hdc, hpal, bForceBackground);
		if (result && !bForceBackground)
		{
			HWND dcWindow = CALL_ORIG_FUNC(WindowFromDC)(hdc);
			if (dcWindow && !(GetWindowLong(dcWindow, GWL_EXSTYLE) & WS_EX_TOOLWINDOW))
			{
				HWND activeWindow = GetActiveWindow();
				if (activeWindow == dcWindow || IsChild(activeWindow, dcWindow))
				{
					Compat31::ScopedSrwLockExclusive lock(g_srwLock);
					g_paletteInfo[hpal].isForeground = true;
					g_paletteInfo[hpal].isRealized = false;
				}
			}
		}
		return LOG_RESULT(result);
	}

	UINT WINAPI setPaletteEntries(HPALETTE hpal, UINT iStart, UINT cEntries, const PALETTEENTRY* pPalEntries)
	{
		LOG_FUNC("SetPaletteEntries", hpal, iStart, cEntries, pPalEntries);
		UINT result = CALL_ORIG_FUNC(SetPaletteEntries)(hpal, iStart, cEntries, pPalEntries);
		if (result)
		{
			Compat31::ScopedSrwLockExclusive lock(g_srwLock);
			g_paletteInfo[hpal].isRealized = false;
		}
		return LOG_RESULT(result);
	}

	UINT WINAPI setSystemPaletteUse(HDC hdc, UINT uUsage)
	{
		LOG_FUNC("SetSystemPaletteUse", hdc, uUsage);
		if (!Gdi::isDisplayDc(hdc))
		{
			return LOG_RESULT(SYSPAL_ERROR);
		}

		Compat31::ScopedSrwLockExclusive lock(g_srwLock);
		if (uUsage == g_systemPaletteUse)
		{
			return LOG_RESULT(g_systemPaletteUse);
		}

		const UINT prevUsage = g_systemPaletteUse;
		switch (uUsage)
		{
		case SYSPAL_STATIC:
			g_systemPaletteFirstNonReservedIndex = 10;
			g_systemPaletteLastNonReservedIndex = 245;
			break;

		case SYSPAL_NOSTATIC:
			g_systemPaletteFirstNonReservedIndex = 1;
			g_systemPaletteLastNonReservedIndex = 254;
			break;

		case SYSPAL_NOSTATIC256:
			g_systemPaletteFirstNonReservedIndex = 0;
			g_systemPaletteLastNonReservedIndex = 255;
			break;

		default:
			return LOG_RESULT(SYSPAL_ERROR);
		}

		g_systemPaletteUse = uUsage;
		updateStaticSysPalEntries();
		return LOG_RESULT(prevUsage);
	}

	BOOL WINAPI unrealizeObject(HGDIOBJ h)
	{
		LOG_FUNC("UnrealizeObject", h);
		BOOL result = CALL_ORIG_FUNC(UnrealizeObject)(h);
		if (result && OBJ_PAL == GetObjectType(h))
		{
			Compat31::ScopedSrwLockExclusive lock(g_srwLock);
			g_paletteInfo[static_cast<HPALETTE>(h)].isRealized = false;
		}
		return LOG_RESULT(result);
	}
}

namespace Gdi
{
	namespace Palette
	{
		PALETTEENTRY* getDefaultPalette()
		{
			return g_defaultPalette;
		}

		std::vector<PALETTEENTRY> getHardwarePalette()
		{
			Compat31::ScopedSrwLockShared lock(g_srwLock);
			return std::vector<PALETTEENTRY>(g_hardwarePalette, g_hardwarePalette + 256);
		}

		std::vector<PALETTEENTRY> getSystemPalette()
		{
			Compat31::ScopedSrwLockShared lock(g_srwLock);
			return std::vector<PALETTEENTRY>(g_systemPalette, g_systemPalette + 256);
		}

		void installHooks()
		{
			HPALETTE defaultPalette = reinterpret_cast<HPALETTE>(GetStockObject(DEFAULT_PALETTE));
			GetPaletteEntries(defaultPalette, 0, 10, g_defaultPalette);
			GetPaletteEntries(defaultPalette, 10, 10, &g_defaultPalette[246]);

			updateStaticSysPalEntries();

			HOOK_FUNCTION(gdi32, DeleteObject, deleteObject);
			HOOK_FUNCTION(gdi32, GetSystemPaletteEntries, getSystemPaletteEntries);
			HOOK_FUNCTION(gdi32, GetSystemPaletteUse, getSystemPaletteUse);
			HOOK_FUNCTION(gdi32, RealizePalette, realizePalette);
			HOOK_FUNCTION(gdi32, ResizePalette, resizePalette);
			HOOK_FUNCTION(gdi32, SelectPalette, selectPalette);
			HOOK_FUNCTION(gdi32, SetPaletteEntries, setPaletteEntries);
			HOOK_FUNCTION(gdi32, SetSystemPaletteUse, setSystemPaletteUse);
			HOOK_FUNCTION(gdi32, UnrealizeObject, unrealizeObject);
		}

		void setHardwarePalette(PALETTEENTRY* entries)
		{
			Compat31::ScopedSrwLockExclusive lock(g_srwLock);
			std::memcpy(g_hardwarePalette, entries, sizeof(g_hardwarePalette));
		}
	}
}
