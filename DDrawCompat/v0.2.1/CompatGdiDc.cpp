#include <algorithm>
#include <unordered_map>

#include "CompatGdi.h"
#include "CompatGdiDc.h"
#include "CompatGdiDcCache.h"
#include "DDrawCompat\DDrawLog.h"
#include "Hook.h"
#include "ScopedCriticalSection.h"

namespace Compat21
{
	namespace
	{
		using CompatGdiDcCache::CachedDc;

		struct CompatDc : CachedDc
		{
			CompatDc(const CachedDc& cachedDc = {}) : CachedDc(cachedDc) {}
			DWORD refCount;
			HDC origDc;
			int savedState;
		};

		typedef std::unordered_map<HDC, CompatDc> CompatDcMap;
		CompatDcMap g_origDcToCompatDc;

		struct ExcludeClipRectsData
		{
			HDC compatDc;
			HWND rootWnd;
		};

		void copyDcAttributes(CompatDc& compatDc, HDC origDc, POINT& origin)
		{
			SelectObject(compatDc.dc, GetCurrentObject(origDc, OBJ_FONT));
			SelectObject(compatDc.dc, GetCurrentObject(origDc, OBJ_BRUSH));
			SelectObject(compatDc.dc, GetCurrentObject(origDc, OBJ_PEN));

			if (GM_ADVANCED == GetGraphicsMode(origDc))
			{
				SetGraphicsMode(compatDc.dc, GM_ADVANCED);
				XFORM transform = {};
				GetWorldTransform(origDc, &transform);
				SetWorldTransform(compatDc.dc, &transform);
			}

			SetMapMode(compatDc.dc, GetMapMode(origDc));

			POINT viewportOrg = {};
			GetViewportOrgEx(origDc, &viewportOrg);
			SetViewportOrgEx(compatDc.dc, viewportOrg.x + origin.x, viewportOrg.y + origin.y, nullptr);
			SIZE viewportExt = {};
			GetViewportExtEx(origDc, &viewportExt);
			SetViewportExtEx(compatDc.dc, viewportExt.cx, viewportExt.cy, nullptr);

			POINT windowOrg = {};
			GetWindowOrgEx(origDc, &windowOrg);
			SetWindowOrgEx(compatDc.dc, windowOrg.x, windowOrg.y, nullptr);
			SIZE windowExt = {};
			GetWindowExtEx(origDc, &windowExt);
			SetWindowExtEx(compatDc.dc, windowExt.cx, windowExt.cy, nullptr);

			SetArcDirection(compatDc.dc, GetArcDirection(origDc));
			SetBkColor(compatDc.dc, GetBkColor(origDc));
			SetBkMode(compatDc.dc, GetBkMode(origDc));
			SetDCBrushColor(compatDc.dc, GetDCBrushColor(origDc));
			SetDCPenColor(compatDc.dc, GetDCPenColor(origDc));
			SetLayout(compatDc.dc, GetLayout(origDc));
			SetPolyFillMode(compatDc.dc, GetPolyFillMode(origDc));
			SetROP2(compatDc.dc, GetROP2(origDc));
			SetStretchBltMode(compatDc.dc, GetStretchBltMode(origDc));
			SetTextAlign(compatDc.dc, GetTextAlign(origDc));
			SetTextCharacterExtra(compatDc.dc, GetTextCharacterExtra(origDc));
			SetTextColor(compatDc.dc, GetTextColor(origDc));

			POINT brushOrg = {};
			GetBrushOrgEx(origDc, &brushOrg);
			SetBrushOrgEx(compatDc.dc, brushOrg.x, brushOrg.y, nullptr);

			POINT currentPos = {};
			GetCurrentPositionEx(origDc, &currentPos);
			MoveToEx(compatDc.dc, currentPos.x, currentPos.y, nullptr);
		}

		BOOL CALLBACK excludeClipRectForOverlappingWindow(HWND hwnd, LPARAM lParam)
		{
			auto excludeClipRectsData = reinterpret_cast<ExcludeClipRectsData*>(lParam);
			if (hwnd == excludeClipRectsData->rootWnd)
			{
				return FALSE;
			}

			if (!IsWindowVisible(hwnd))
			{
				return TRUE;
			}

			RECT windowRect = {};
			GetWindowRect(hwnd, &windowRect);

			HRGN windowRgn = CreateRectRgnIndirect(&windowRect);
			ExtSelectClipRgn(excludeClipRectsData->compatDc, windowRgn, RGN_DIFF);
			DeleteObject(windowRgn);

			return TRUE;
		}

		void excludeClipRectsForOverlappingWindows(HWND hwnd, bool isMenuWindow, HDC compatDc)
		{
			ExcludeClipRectsData excludeClipRectsData = { compatDc, GetAncestor(hwnd, GA_ROOT) };
			if (!isMenuWindow)
			{
				EnumWindows(&excludeClipRectForOverlappingWindow,
					reinterpret_cast<LPARAM>(&excludeClipRectsData));
			}

			HWND menuWindow = FindWindow(reinterpret_cast<LPCSTR>(0x8000), nullptr);
			while (menuWindow && menuWindow != hwnd)
			{
				excludeClipRectForOverlappingWindow(
					menuWindow, reinterpret_cast<LPARAM>(&excludeClipRectsData));
				menuWindow = FindWindowEx(nullptr, menuWindow, reinterpret_cast<LPCSTR>(0x8000), nullptr);
			}
		}

		void setClippingRegion(HDC compatDc, HDC origDc, HWND hwnd, bool isMenuWindow, const POINT& origin)
		{
			HRGN clipRgn = CreateRectRgn(0, 0, 0, 0);
			if (1 == GetClipRgn(origDc, clipRgn))
			{
				OffsetRgn(clipRgn, origin.x, origin.y);
				SelectClipRgn(compatDc, clipRgn);
			}

			if (hwnd)
			{
				if (isMenuWindow || 1 != GetRandomRgn(origDc, clipRgn, SYSRGN))
				{
					RECT rect = {};
					GetWindowRect(hwnd, &rect);
					SetRectRgn(clipRgn, rect.left, rect.top, rect.right, rect.bottom);
				}

				excludeClipRectsForOverlappingWindows(hwnd, isMenuWindow, compatDc);
				ExtSelectClipRgn(compatDc, clipRgn, RGN_AND);
			}

			DeleteObject(clipRgn);
			SetMetaRgn(compatDc);
		}
	}

	namespace CompatGdiDc
	{
		HDC getDc(HDC origDc, bool isMenuPaintDc)
		{
			if (!origDc || OBJ_DC != GetObjectType(origDc) || DT_RASDISPLAY != GetDeviceCaps(origDc, TECHNOLOGY))
			{
				return nullptr;
			}

			Compat::ScopedCriticalSection gdiLock(CompatGdi::g_gdiCriticalSection);

			auto it = g_origDcToCompatDc.find(origDc);
			if (it != g_origDcToCompatDc.end())
			{
				++it->second.refCount;
				return it->second.dc;
			}

			const HWND hwnd = WindowFromDC(origDc);
			const bool isMenuWindow = hwnd && 0x8000 == GetClassLongPtr(hwnd, GCW_ATOM);
			if (isMenuWindow && !isMenuPaintDc)
			{
				return nullptr;
			}

			CompatDc compatDc(CompatGdiDcCache::getDc());
			if (!compatDc.dc)
			{
				return nullptr;
			}

			POINT origin = {};
			GetDCOrgEx(origDc, &origin);

			compatDc.savedState = SaveDC(compatDc.dc);
			copyDcAttributes(compatDc, origDc, origin);
			setClippingRegion(compatDc.dc, origDc, hwnd, isMenuWindow, origin);

			compatDc.refCount = 1;
			compatDc.origDc = origDc;
			g_origDcToCompatDc.insert(CompatDcMap::value_type(origDc, compatDc));

			return compatDc.dc;
		}

		HDC getOrigDc(HDC dc)
		{
			const auto it = std::find_if(g_origDcToCompatDc.begin(), g_origDcToCompatDc.end(),
				[dc](const CompatDcMap::value_type& compatDc) { return compatDc.second.dc == dc; });
			return it != g_origDcToCompatDc.end() ? it->first : dc;
		}

		void releaseDc(HDC origDc)
		{
			Compat::ScopedCriticalSection gdiLock(CompatGdi::g_gdiCriticalSection);

			auto it = g_origDcToCompatDc.find(origDc);
			if (it == g_origDcToCompatDc.end())
			{
				return;
			}

			CompatDc& compatDc = it->second;
			--compatDc.refCount;
			if (0 == compatDc.refCount)
			{
				RestoreDC(compatDc.dc, compatDc.savedState);
				CompatGdiDcCache::releaseDc(compatDc);
				g_origDcToCompatDc.erase(origDc);
			}
		}
	}
}
