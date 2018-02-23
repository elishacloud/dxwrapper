#include <algorithm>
#include <unordered_map>

#include "CompatGdiDc.h"
#include "CompatGdiDcCache.h"
#include "DDrawLog.h"

namespace Compat20
{
	namespace
	{
		using CompatGdiDcCache::CachedDc;

		struct CompatDc : CachedDc
		{
			CompatDc(const CachedDc& cachedDc = {}) : CachedDc(cachedDc) {}
			DWORD refCount;
			HDC origDc;
			HGDIOBJ origFont;
			HGDIOBJ origBrush;
			HGDIOBJ origPen;
		};

		typedef std::unordered_map<HDC, CompatDc> CompatDcMap;
		CompatDcMap g_origDcToCompatDc;

		struct ExcludeClipRectsData
		{
			HDC compatDc;
			POINT origin;
			HWND rootWnd;
		};

		void copyDcAttributes(CompatDc& compatDc, HDC origDc, POINT& origin)
		{
			compatDc.origFont = SelectObject(compatDc.dc, GetCurrentObject(origDc, OBJ_FONT));
			compatDc.origBrush = SelectObject(compatDc.dc, GetCurrentObject(origDc, OBJ_BRUSH));
			compatDc.origPen = SelectObject(compatDc.dc, GetCurrentObject(origDc, OBJ_PEN));

			SetArcDirection(compatDc.dc, GetArcDirection(origDc));
			SetBkColor(compatDc.dc, GetBkColor(origDc));
			SetBkMode(compatDc.dc, GetBkMode(origDc));
			SetDCBrushColor(compatDc.dc, GetDCBrushColor(origDc));
			SetDCPenColor(compatDc.dc, GetDCPenColor(origDc));
			SetPolyFillMode(compatDc.dc, GetPolyFillMode(origDc));
			SetROP2(compatDc.dc, GetROP2(origDc));
			SetStretchBltMode(compatDc.dc, GetStretchBltMode(origDc));
			SetTextAlign(compatDc.dc, GetTextAlign(origDc));
			SetTextCharacterExtra(compatDc.dc, GetTextCharacterExtra(origDc));
			SetTextColor(compatDc.dc, GetTextColor(origDc));
			SetWindowOrgEx(compatDc.dc, -origin.x, -origin.y, nullptr);

			POINT brushOrg = {};
			GetBrushOrgEx(origDc, &brushOrg);
			SetBrushOrgEx(compatDc.dc, brushOrg.x, brushOrg.y, nullptr);

			POINT currentPos = {};
			MoveToEx(origDc, 0, 0, &currentPos);
			MoveToEx(origDc, currentPos.x, currentPos.y, nullptr);
			MoveToEx(compatDc.dc, currentPos.x, currentPos.y, nullptr);
		}

		BOOL CALLBACK excludeClipRectsForOverlappingWindows(HWND hwnd, LPARAM lParam)
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

			RECT rect = {};
			GetWindowRect(hwnd, &rect);
			OffsetRect(&rect, -excludeClipRectsData->origin.x, -excludeClipRectsData->origin.y);
			ExcludeClipRect(excludeClipRectsData->compatDc, rect.left, rect.top, rect.right, rect.bottom);
			return TRUE;
		}

		void setClippingRegion(HDC compatDc, HDC origDc, POINT& origin)
		{
			HRGN clipRgn = CreateRectRgn(0, 0, 0, 0);
			const bool isEmptyClipRgn = 1 != GetRandomRgn(origDc, clipRgn, SYSRGN);
			SelectClipRgn(compatDc, isEmptyClipRgn ? nullptr : clipRgn);
			DeleteObject(clipRgn);

			HRGN origClipRgn = CreateRectRgn(0, 0, 0, 0);
			if (1 == GetClipRgn(origDc, origClipRgn))
			{
				OffsetRgn(origClipRgn, origin.x, origin.y);
				ExtSelectClipRgn(compatDc, origClipRgn, RGN_AND);
			}
			DeleteObject(origClipRgn);

			if (!isEmptyClipRgn)
			{
				HWND hwnd = WindowFromDC(origDc);
				if (hwnd)
				{
					ExcludeClipRectsData excludeClipRectsData = { compatDc, origin, GetAncestor(hwnd, GA_ROOT) };
					EnumThreadWindows(GetCurrentThreadId(), &excludeClipRectsForOverlappingWindows,
						reinterpret_cast<LPARAM>(&excludeClipRectsData));
					RECT windowRect = {};
					GetWindowRect(hwnd, &windowRect);
				}
			}
		}
	}

	namespace CompatGdiDc
	{
		HDC getDc(HDC origDc)
		{
			if (!origDc || OBJ_DC != GetObjectType(origDc) || DT_RASDISPLAY != GetDeviceCaps(origDc, TECHNOLOGY) ||
				g_origDcToCompatDc.end() != std::find_if(g_origDcToCompatDc.begin(), g_origDcToCompatDc.end(),
					[=](const CompatDcMap::value_type& compatDc) { return compatDc.second.dc == origDc; }))
			{
				return nullptr;
			}

			auto it = g_origDcToCompatDc.find(origDc);
			if (it != g_origDcToCompatDc.end())
			{
				++it->second.refCount;
				return it->second.dc;
			}

			CompatDc compatDc(CompatGdiDcCache::getDc());
			if (!compatDc.dc)
			{
				return nullptr;
			}

			POINT origin = {};
			GetDCOrgEx(origDc, &origin);

			copyDcAttributes(compatDc, origDc, origin);
			setClippingRegion(compatDc.dc, origDc, origin);

			compatDc.refCount = 1;
			compatDc.origDc = origDc;
			g_origDcToCompatDc.insert(CompatDcMap::value_type(origDc, compatDc));

			return compatDc.dc;
		}

		void releaseDc(HDC origDc)
		{
			auto it = g_origDcToCompatDc.find(origDc);
			if (it == g_origDcToCompatDc.end())
			{
				return;
			}

			CompatDc& compatDc = it->second;
			--compatDc.refCount;
			if (0 == compatDc.refCount)
			{
				SelectObject(compatDc.dc, compatDc.origFont);
				SelectObject(compatDc.dc, compatDc.origBrush);
				SelectObject(compatDc.dc, compatDc.origPen);
				CompatGdiDcCache::releaseDc(compatDc);
				g_origDcToCompatDc.erase(origDc);
			}
		}
	}
}
