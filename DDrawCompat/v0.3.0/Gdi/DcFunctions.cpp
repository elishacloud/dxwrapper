#define WIN32_LEAN_AND_MEAN
#define CINTERFACE

#include <DDrawCompat/v0.3.0/Common/Hook.h>
#include <DDrawCompat/DDrawLog.h>
#include <DDrawCompat/v0.3.0/Gdi/CompatDc.h>
#include <DDrawCompat/v0.3.0/Gdi/Dc.h>
#include <DDrawCompat/v0.3.0/Gdi/DcFunctions.h>
#include <DDrawCompat/v0.3.0/Gdi/Font.h>
#include <DDrawCompat/v0.3.0/Gdi/Gdi.h>
#include <DDrawCompat/v0.3.0/Gdi/Region.h>
#include <DDrawCompat/v0.3.0/Gdi/VirtualScreen.h>
#include <DDrawCompat/v0.3.0/Win32/DisplayMode.h>

namespace
{
	template <auto func>
	const char* g_funcName = nullptr;

	thread_local UINT g_disableDibRedirection = 0;

#define CREATE_DC_FUNC_ATTRIBUTE(attribute) \
	template <auto origFunc> \
	bool attribute() \
	{ \
		return false; \
	}

#define SET_DC_FUNC_ATTRIBUTE(attribute, func) \
	template <> \
	bool attribute<&func>() \
	{ \
		return true; \
	}

#define SET_TEXT_DC_FUNC_ATTRIBUTE(attribute, func) \
	SET_DC_FUNC_ATTRIBUTE(attribute, func##A) \
	SET_DC_FUNC_ATTRIBUTE(attribute, func##W)

	CREATE_DC_FUNC_ATTRIBUTE(isPositionUpdated);
	SET_DC_FUNC_ATTRIBUTE(isPositionUpdated, AngleArc);
	SET_DC_FUNC_ATTRIBUTE(isPositionUpdated, ArcTo);
	SET_DC_FUNC_ATTRIBUTE(isPositionUpdated, LineTo);
	SET_DC_FUNC_ATTRIBUTE(isPositionUpdated, PolyBezierTo);
	SET_DC_FUNC_ATTRIBUTE(isPositionUpdated, PolyDraw);
	SET_DC_FUNC_ATTRIBUTE(isPositionUpdated, PolylineTo);
	SET_TEXT_DC_FUNC_ATTRIBUTE(isPositionUpdated, ExtTextOut);
	SET_TEXT_DC_FUNC_ATTRIBUTE(isPositionUpdated, PolyTextOut);
	SET_TEXT_DC_FUNC_ATTRIBUTE(isPositionUpdated, TabbedTextOut);
	SET_TEXT_DC_FUNC_ATTRIBUTE(isPositionUpdated, TextOut);

	CREATE_DC_FUNC_ATTRIBUTE(isReadOnly);
	SET_DC_FUNC_ATTRIBUTE(isReadOnly, GetDIBits);
	SET_DC_FUNC_ATTRIBUTE(isReadOnly, GetPixel);

	BOOL WINAPI GdiDrawStream(HDC, DWORD, DWORD) { return FALSE; }
	BOOL WINAPI PolyPatBlt(HDC, DWORD, DWORD, DWORD, DWORD) { return FALSE; }

	bool hasDisplayDcArg(HDC dc)
	{
		return Gdi::isDisplayDc(dc);
	}

	template <typename T>
	bool hasDisplayDcArg(T)
	{
		return false;
	}

	template <typename T, typename... Params>
	bool hasDisplayDcArg(T t, Params... params)
	{
		return hasDisplayDcArg(t) || hasDisplayDcArg(params...);
	}

	bool lpToScreen(HWND hwnd, HDC dc, POINT& p)
	{
		LPtoDP(dc, &p, 1);
		RECT wr = {};
		GetWindowRect(hwnd, &wr);
		p.x += wr.left;
		p.y += wr.top;
		return true;
	}

	template <typename T>
	T replaceDc(T t)
	{
		return t;
	}

	Gdi::CompatDc replaceDc(HDC dc)
	{
		return Gdi::CompatDc(dc);
	}

	template <auto origFunc, typename Result, typename... Params>
	Result WINAPI compatGdiDcFunc(HDC hdc, Params... params)
	{
		LOG_FUNC(g_funcName<origFunc>, hdc, params...);

		if (hasDisplayDcArg(hdc, params...))
		{
			Gdi::CompatDc compatDc(hdc, isReadOnly<origFunc>());
			Result result = Compat30::g_origFuncPtr<origFunc>(compatDc, replaceDc(params)...);
			if (isPositionUpdated<origFunc>() && result)
			{
				POINT currentPos = {};
				GetCurrentPositionEx(compatDc, &currentPos);
				MoveToEx(hdc, currentPos.x, currentPos.y, nullptr);
			}
			return LOG_RESULT(result);
		}

		return LOG_RESULT(Compat30::g_origFuncPtr<origFunc>(hdc, params...));
	}

	template <>
	BOOL WINAPI compatGdiDcFunc<&ExtTextOutW>(
		HDC hdc, int x, int y, UINT options, const RECT* lprect, LPCWSTR lpString, UINT c, const INT* lpDx)
	{
		LOG_FUNC("ExtTextOutW", hdc, x, y, options, lprect, lpString, c, lpDx);

		if (hasDisplayDcArg(hdc))
		{
			HWND hwnd = CALL_ORIG_FUNC(WindowFromDC)(hdc);
			ATOM atom = static_cast<ATOM>(GetClassLong(hwnd, GCW_ATOM));
			POINT p = { x, y };
			if (Gdi::MENU_ATOM == atom)
			{
				RedrawWindow(hwnd, nullptr, nullptr, RDW_INVALIDATE | RDW_ERASE);
			}
			else if (GetCurrentThreadId() == GetWindowThreadProcessId(hwnd, nullptr) &&
				lpToScreen(hwnd, hdc, p) &&
				HTMENU == SendMessage(hwnd, WM_NCHITTEST, 0, (p.y << 16) | (p.x & 0xFFFF)))
			{
				WINDOWINFO wi = {};
				GetWindowInfo(hwnd, &wi);
				Gdi::Region ncRegion(wi.rcWindow);
				ncRegion -= wi.rcClient;
				ncRegion.offset(-wi.rcClient.left, -wi.rcClient.top);
				RedrawWindow(hwnd, nullptr, ncRegion, RDW_INVALIDATE | RDW_FRAME);
			}
			else
			{
				Gdi::CompatDc compatDc(hdc);
				BOOL result = CALL_ORIG_FUNC(ExtTextOutW)(compatDc, x, y, options, lprect, lpString, c, lpDx);
				if (result)
				{
					POINT currentPos = {};
					GetCurrentPositionEx(compatDc, &currentPos);
					MoveToEx(hdc, currentPos.x, currentPos.y, nullptr);
				}
				return LOG_RESULT(result);
			}
		}
		else
		{
			return LOG_RESULT(CALL_ORIG_FUNC(ExtTextOutW)(hdc, x, y, options, lprect, lpString, c, lpDx));
		}

		return LOG_RESULT(TRUE);
	}

	template <auto origFunc, typename Result, typename... Params>
	Result WINAPI compatGdiTextDcFunc(HDC dc, Params... params)
	{
		Gdi::Font::Mapper fontMapper(dc);
		return compatGdiDcFunc<origFunc, Result>(dc, params...);
	}

	HBITMAP WINAPI createCompatibleBitmap(HDC hdc, int cx, int cy)
	{
		LOG_FUNC("CreateCompatibleBitmap", hdc, cx, cy);
		if (!g_disableDibRedirection && Gdi::isDisplayDc(hdc))
		{
			const bool useDefaultPalette = false;
			return LOG_RESULT(Gdi::VirtualScreen::createOffScreenDib(cx, cy, useDefaultPalette));
		}
		return LOG_RESULT(CALL_ORIG_FUNC(CreateCompatibleBitmap)(hdc, cx, cy));
	}

	HBITMAP WINAPI createDIBitmap(HDC hdc, const BITMAPINFOHEADER* lpbmih, DWORD fdwInit,
		const void* lpbInit, const BITMAPINFO* lpbmi, UINT fuUsage)
	{
		LOG_FUNC("CreateDIBitmap", hdc, lpbmih, fdwInit, lpbInit, lpbmi, fuUsage);
		const DWORD CBM_CREATDIB = 2;
		if (!g_disableDibRedirection && !(fdwInit & CBM_CREATDIB) && lpbmih && Gdi::isDisplayDc(hdc))
		{
			const bool useDefaultPalette = false;
			HBITMAP bitmap = Gdi::VirtualScreen::createOffScreenDib(
				lpbmi->bmiHeader.biWidth, lpbmi->bmiHeader.biHeight, useDefaultPalette);
			if (bitmap && lpbInit && lpbmi)
			{
				SetDIBits(hdc, bitmap, 0, std::abs(lpbmi->bmiHeader.biHeight), lpbInit, lpbmi, fuUsage);
			}
			return LOG_RESULT(bitmap);
		}
		return LOG_RESULT(CALL_ORIG_FUNC(CreateDIBitmap)(hdc, lpbmih, fdwInit, lpbInit, lpbmi, fuUsage));
	}

	HBITMAP WINAPI createDiscardableBitmap(HDC hdc, int nWidth, int nHeight)
	{
		LOG_FUNC("CreateDiscardableBitmap", hdc, nWidth, nHeight);
		if (!g_disableDibRedirection && Gdi::isDisplayDc(hdc))
		{
			const bool useDefaultPalette = false;
			return LOG_RESULT(Gdi::VirtualScreen::createOffScreenDib(nWidth, nHeight, useDefaultPalette));
		}
		return LOG_RESULT(CALL_ORIG_FUNC(createDiscardableBitmap)(hdc, nWidth, nHeight));
	}

	BOOL WINAPI drawCaption(HWND hwnd, HDC hdc, const RECT* lprect, UINT flags)
	{
		LOG_FUNC("DrawCaption", hwnd, hdc, lprect, flags);
		if (Gdi::isDisplayDc(hdc))
		{
			return LOG_RESULT(CALL_ORIG_FUNC(DrawCaption)(hwnd, Gdi::CompatDc(hdc), lprect, flags));
		}
		return LOG_RESULT(CALL_ORIG_FUNC(DrawCaption)(hwnd, hdc, lprect, flags));
	}

	template <auto origFunc>
	void hookGdiDcFunction(const char* moduleName, const char* funcName)
	{
#ifdef DEBUGLOGS
		g_funcName<origFunc> = funcName;
#endif

		Compat30::hookFunction<origFunc>(moduleName, funcName, &compatGdiDcFunc<origFunc>);
	}

	template <auto origFunc>
	void hookGdiTextDcFunction(const char* moduleName, const char* funcName)
	{
#ifdef DEBUGLOGS
		g_funcName<origFunc> = funcName;
#endif

		Compat30::hookFunction<origFunc>(moduleName, funcName, &compatGdiTextDcFunc<origFunc>);
	}

	HWND WINAPI windowFromDc(HDC dc)
	{
		return CALL_ORIG_FUNC(WindowFromDC)(Gdi::Dc::getOrigDc(dc));
	}
}

#define HOOK_GDI_DC_FUNCTION(module, func) \
	hookGdiDcFunction<&func>(#module, #func)

#define HOOK_GDI_TEXT_DC_FUNCTION(module, func) \
	hookGdiTextDcFunction<&func##A>(#module, #func"A"); \
	hookGdiTextDcFunction<&func##W>(#module, #func"W")

namespace Gdi
{
	namespace DcFunctions
	{
		void disableDibRedirection(bool disable)
		{
			g_disableDibRedirection += disable ? 1 : -1;
		}

		void installHooks()
		{
			// Bitmap functions
			HOOK_GDI_DC_FUNCTION(msimg32, AlphaBlend);
			HOOK_GDI_DC_FUNCTION(gdi32, BitBlt);
			HOOK_FUNCTION(gdi32, CreateCompatibleBitmap, createCompatibleBitmap);
			HOOK_FUNCTION(gdi32, CreateDIBitmap, createDIBitmap);
			HOOK_FUNCTION(gdi32, CreateDiscardableBitmap, createDiscardableBitmap);
			HOOK_GDI_DC_FUNCTION(gdi32, ExtFloodFill);
			HOOK_GDI_DC_FUNCTION(gdi32, GdiAlphaBlend);
			HOOK_GDI_DC_FUNCTION(gdi32, GdiGradientFill);
			HOOK_GDI_DC_FUNCTION(gdi32, GdiTransparentBlt);
			HOOK_GDI_DC_FUNCTION(gdi32, GetPixel);
			HOOK_GDI_DC_FUNCTION(msimg32, GradientFill);
			HOOK_GDI_DC_FUNCTION(gdi32, MaskBlt);
			HOOK_GDI_DC_FUNCTION(gdi32, PlgBlt);
			HOOK_GDI_DC_FUNCTION(gdi32, SetDIBitsToDevice);
			HOOK_GDI_DC_FUNCTION(gdi32, SetPixel);
			HOOK_GDI_DC_FUNCTION(gdi32, SetPixelV);
			HOOK_GDI_DC_FUNCTION(gdi32, StretchBlt);
			HOOK_GDI_DC_FUNCTION(gdi32, StretchDIBits);
			HOOK_GDI_DC_FUNCTION(msimg32, TransparentBlt);

			// Brush functions
			HOOK_GDI_DC_FUNCTION(gdi32, PatBlt);

			// Device context functions
			HOOK_GDI_DC_FUNCTION(gdi32, DrawEscape);
			HOOK_FUNCTION(user32, WindowFromDC, windowFromDc);

			// Filled shape functions
			HOOK_GDI_DC_FUNCTION(gdi32, Chord);
			HOOK_GDI_DC_FUNCTION(gdi32, Ellipse);
			HOOK_GDI_DC_FUNCTION(user32, FillRect);
			HOOK_GDI_DC_FUNCTION(user32, FrameRect);
			HOOK_GDI_DC_FUNCTION(user32, InvertRect);
			HOOK_GDI_DC_FUNCTION(gdi32, Pie);
			HOOK_GDI_DC_FUNCTION(gdi32, Polygon);
			HOOK_GDI_DC_FUNCTION(gdi32, PolyPolygon);
			HOOK_GDI_DC_FUNCTION(gdi32, Rectangle);
			HOOK_GDI_DC_FUNCTION(gdi32, RoundRect);

			// Font and text functions
			HOOK_GDI_TEXT_DC_FUNCTION(user32, DrawText);
			HOOK_GDI_TEXT_DC_FUNCTION(user32, DrawTextEx);
			HOOK_GDI_TEXT_DC_FUNCTION(gdi32, ExtTextOut);
			HOOK_GDI_TEXT_DC_FUNCTION(gdi32, PolyTextOut);
			HOOK_GDI_TEXT_DC_FUNCTION(user32, TabbedTextOut);
			HOOK_GDI_TEXT_DC_FUNCTION(gdi32, TextOut);

			// Icon functions
			HOOK_GDI_DC_FUNCTION(user32, DrawIcon);
			HOOK_GDI_DC_FUNCTION(user32, DrawIconEx);

			// Line and curve functions
			HOOK_GDI_DC_FUNCTION(gdi32, AngleArc);
			HOOK_GDI_DC_FUNCTION(gdi32, Arc);
			HOOK_GDI_DC_FUNCTION(gdi32, ArcTo);
			HOOK_GDI_DC_FUNCTION(gdi32, LineTo);
			HOOK_GDI_DC_FUNCTION(gdi32, PolyBezier);
			HOOK_GDI_DC_FUNCTION(gdi32, PolyBezierTo);
			HOOK_GDI_DC_FUNCTION(gdi32, PolyDraw);
			HOOK_GDI_DC_FUNCTION(gdi32, Polyline);
			HOOK_GDI_DC_FUNCTION(gdi32, PolylineTo);
			HOOK_GDI_DC_FUNCTION(gdi32, PolyPolyline);

			// Painting and drawing functions
			HOOK_FUNCTION(user32, DrawCaption, drawCaption);
			HOOK_GDI_DC_FUNCTION(user32, DrawEdge);
			HOOK_GDI_DC_FUNCTION(user32, DrawFocusRect);
			HOOK_GDI_DC_FUNCTION(user32, DrawFrameControl);
			HOOK_GDI_TEXT_DC_FUNCTION(user32, DrawState);
			HOOK_GDI_TEXT_DC_FUNCTION(user32, GrayString);
			HOOK_GDI_DC_FUNCTION(user32, PaintDesktop);

			// Region functions
			HOOK_GDI_DC_FUNCTION(gdi32, FillRgn);
			HOOK_GDI_DC_FUNCTION(gdi32, FrameRgn);
			HOOK_GDI_DC_FUNCTION(gdi32, InvertRgn);
			HOOK_GDI_DC_FUNCTION(gdi32, PaintRgn);

			// Scroll bar functions
			HOOK_GDI_DC_FUNCTION(user32, ScrollDC);

			// Undocumented functions
			HOOK_GDI_DC_FUNCTION(gdi32, GdiDrawStream);
			HOOK_GDI_DC_FUNCTION(gdi32, PolyPatBlt);
		}
	}
}
