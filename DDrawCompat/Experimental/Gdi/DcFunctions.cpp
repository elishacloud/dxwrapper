#include <unordered_map>

#include "Common/Hook.h"
#include "DDrawCompat\DDrawLog.h"
#include "Gdi/Dc.h"
#include "Gdi/DcFunctions.h"
#include "Gdi/Gdi.h"
#include "Win32/DisplayMode.h"

namespace
{
	struct ExcludeRectContext
	{
		HRGN rgn;
		HWND rootWnd;
	};

	std::unordered_map<void*, const char*> g_funcNames;

	template <typename OrigFuncPtr, OrigFuncPtr origFunc, typename... Params>
	DWORD getDdLockFlags(Params... params);

	BOOL WINAPI GdiDrawStream(HDC, DWORD, DWORD) { return FALSE; }
	BOOL WINAPI PolyPatBlt(HDC, DWORD, DWORD, DWORD, DWORD) { return FALSE; }

	template <typename Result, typename... Params>
	using FuncPtr = Result(WINAPI *)(Params...);

	bool hasDisplayDcArg(HDC dc)
	{
		return dc && OBJ_DC == GetObjectType(dc) && DT_RASDISPLAY == GetDeviceCaps(dc, TECHNOLOGY);
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

	template <typename T>
	T replaceDc(T t)
	{
		return t;
	}

	HDC replaceDc(HDC dc)
	{
		HDC compatDc = Gdi::Dc::getDc(dc);
		return compatDc ? compatDc : dc;
	}

	template <typename T>
	void releaseDc(T) {}

	void releaseDc(HDC dc)
	{
		Gdi::Dc::releaseDc(dc);
	}

	template <typename T, typename... Params>
	void releaseDc(T t, Params... params)
	{
		releaseDc(params...);
		releaseDc(t);
	}

	template <typename OrigFuncPtr, OrigFuncPtr origFunc, typename Result, typename... Params>
	Result WINAPI compatGdiDcFunc(Params... params)
	{
#ifdef _DEBUG
		Compat::LogEnter(g_funcNames[origFunc], params...);
#endif

		if (!hasDisplayDcArg(params...) ||
			!Gdi::beginGdiRendering(getDdLockFlags<OrigFuncPtr, origFunc>(params...)))
		{
			Result result = Compat::getOrigFuncPtr<OrigFuncPtr, origFunc>()(params...);

#ifdef _DEBUG
			if (!hasDisplayDcArg(params...))
			{
				Compat::Log() << "Skipping redirection since there is no display DC argument";
			}
			else if (!Gdi::isEmulationEnabled())
			{
				Compat::Log() << "Skipping redirection since GDI emulation is disabled";
			}
			else
			{
				Compat::Log() << "Skipping redirection since the primary surface could not be locked";
			}
			Compat::LogLeave(g_funcNames[origFunc], params...) << result;
#endif

			return result;
		}

		Result result = Compat::getOrigFuncPtr<OrigFuncPtr, origFunc>()(replaceDc(params)...);
		releaseDc(params...);
		Gdi::endGdiRendering();

#ifdef _DEBUG
		Compat::LogLeave(g_funcNames[origFunc], params...) << result;
#endif

		return result;
	}

	void enumTopLevelThreadWindows(WNDENUMPROC enumFunc, LPARAM lParam)
	{
		const DWORD currentThreadId = GetCurrentThreadId();
		const char* MENU_ATOM = reinterpret_cast<LPCSTR>(0x8000);
		HWND menuWindow = FindWindow(MENU_ATOM, nullptr);
		BOOL cont = TRUE;
		while (menuWindow && cont)
		{
			if (currentThreadId == GetWindowThreadProcessId(menuWindow, nullptr))
			{
				cont = enumFunc(menuWindow, lParam);
			}
			if (cont)
			{
				menuWindow = FindWindowEx(nullptr, menuWindow, MENU_ATOM, nullptr);
			}
		}

		if (cont)
		{
			EnumThreadWindows(currentThreadId, enumFunc, lParam);
		}
	}

	BOOL CALLBACK excludeRectForOverlappingWindow(HWND hwnd, LPARAM lParam)
	{
		auto excludeRectContext = reinterpret_cast<ExcludeRectContext*>(lParam);
		if (hwnd == excludeRectContext->rootWnd)
		{
			return FALSE;
		}

		if (!IsWindowVisible(hwnd) || (GetWindowLongPtr(hwnd, GWL_EXSTYLE) & WS_EX_TRANSPARENT))
		{
			return TRUE;
		}

		RECT windowRect = {};
		GetWindowRect(hwnd, &windowRect);

		HRGN windowRgn = CreateRectRgnIndirect(&windowRect);
		CombineRgn(excludeRectContext->rgn, excludeRectContext->rgn, windowRgn, RGN_DIFF);
		DeleteObject(windowRgn);

		return TRUE;
	}

	template <typename OrigFuncPtr, OrigFuncPtr origFunc, typename Result, typename... Params>
	OrigFuncPtr getCompatGdiDcFuncPtr(FuncPtr<Result, Params...>)
	{
		return &compatGdiDcFunc<OrigFuncPtr, origFunc, Result, Params...>;
	}

	DWORD getDdLockFlagsBlt(HDC hdcDest, HDC hdcSrc)
	{
		return hasDisplayDcArg(hdcSrc) && !hasDisplayDcArg(hdcDest) ? DDLOCK_READONLY : 0;
	}

	template <typename OrigFuncPtr, OrigFuncPtr origFunc, typename... Params>
	DWORD getDdLockFlags(Params...)
	{
		return 0;
	}

	template <>
	DWORD getDdLockFlags<decltype(&AlphaBlend), &AlphaBlend>(
		HDC hdcDest, int, int, int, int, HDC hdcSrc, int, int, int, int, BLENDFUNCTION)
	{
		return getDdLockFlagsBlt(hdcDest, hdcSrc);
	}

	template <>
	DWORD getDdLockFlags<decltype(&BitBlt), &BitBlt>(
		HDC hdcDest, int, int, int, int, HDC hdcSrc, int, int, DWORD)
	{
		return getDdLockFlagsBlt(hdcDest, hdcSrc);
	}

	template <>
	DWORD getDdLockFlags<decltype(&GdiAlphaBlend), &GdiAlphaBlend>(
		HDC hdcDest, int, int, int, int, HDC hdcSrc, int, int, int, int, BLENDFUNCTION)
	{
		return getDdLockFlagsBlt(hdcDest, hdcSrc);
	}

	template <>
	DWORD getDdLockFlags<decltype(&GdiTransparentBlt), &GdiTransparentBlt >(
		HDC hdcDest, int, int, int, int, HDC hdcSrc, int, int, int, int, UINT)
	{
		return getDdLockFlagsBlt(hdcDest, hdcSrc);
	}

	template <>
	DWORD getDdLockFlags<decltype(&GetDIBits), &GetDIBits>(
		HDC, HBITMAP, UINT, UINT, LPVOID, LPBITMAPINFO, UINT)
	{
		return DDLOCK_READONLY;
	}

	template <>
	DWORD getDdLockFlags<decltype(&GetPixel), &GetPixel>(HDC, int, int)
	{
		return DDLOCK_READONLY;
	}

	template <>
	DWORD getDdLockFlags<decltype(&MaskBlt), &MaskBlt>(
		HDC hdcDest, int, int, int, int, HDC hdcSrc, int, int, HBITMAP, int, int, DWORD)
	{
		return getDdLockFlagsBlt(hdcDest, hdcSrc);
	}

	template <>
	DWORD getDdLockFlags<decltype(&PlgBlt), &PlgBlt>(
		HDC hdcDest, const POINT*, HDC hdcSrc, int, int, int, int, HBITMAP, int, int)
	{
		return getDdLockFlagsBlt(hdcDest, hdcSrc);
	}

	template <>
	DWORD getDdLockFlags<decltype(&StretchBlt), &StretchBlt>(
		HDC hdcDest, int, int, int, int, HDC hdcSrc, int, int, int, int, DWORD)
	{
		return getDdLockFlagsBlt(hdcDest, hdcSrc);
	}

	template <>
	DWORD getDdLockFlags<decltype(&TransparentBlt), &TransparentBlt>(
		HDC hdcDest, int, int, int, int, HDC hdcSrc, int, int, int, int, UINT)
	{
		return getDdLockFlagsBlt(hdcDest, hdcSrc);
	}

	template <typename OrigFuncPtr, OrigFuncPtr origFunc>
	void hookGdiDcFunction(const char* moduleName, const char* funcName)
	{
#ifdef _DEBUG
		g_funcNames[origFunc] = funcName;
#endif

		Compat::hookFunction<OrigFuncPtr, origFunc>(
			moduleName, funcName, getCompatGdiDcFuncPtr<OrigFuncPtr, origFunc>(origFunc));
	}

	int WINAPI getRandomRgn(HDC hdc, HRGN hrgn, INT iNum)
	{
		int result = CALL_ORIG_FUNC(GetRandomRgn)(hdc, hrgn, iNum);
		if (1 != result)
		{
			return result;
		}

		HWND hwnd = WindowFromDC(hdc);
		if (!hwnd)
		{
			return 1;
		}

		if ((GetWindowLongPtr(hwnd, GWL_EXSTYLE) & WS_EX_LAYERED) &&
			!GetLayeredWindowAttributes(hwnd, nullptr, nullptr, nullptr))
		{
			RECT rect = {};
			GetWindowRect(hwnd, &rect);
			SetRectRgn(hrgn, rect.left, rect.top, rect.right, rect.bottom);
		}

		ExcludeRectContext excludeRectContext = { hrgn, GetAncestor(hwnd, GA_ROOT) };
		enumTopLevelThreadWindows(excludeRectForOverlappingWindow,
			reinterpret_cast<LPARAM>(&excludeRectContext));

		return 1;
	}

	HWND WINAPI windowFromDc(HDC dc)
	{
		return CALL_ORIG_FUNC(WindowFromDC)(Gdi::Dc::getOrigDc(dc));
	}
}

#define HOOK_GDI_DC_FUNCTION(module, func) \
	hookGdiDcFunction<decltype(&func), &func>(#module, #func)

#define HOOK_GDI_TEXT_DC_FUNCTION(module, func) \
	HOOK_GDI_DC_FUNCTION(module, func##A); \
	HOOK_GDI_DC_FUNCTION(module, func##W)

namespace Gdi
{
	namespace DcFunctions
	{
		void installHooks()
		{
			// Bitmap functions
			HOOK_GDI_DC_FUNCTION(msimg32, AlphaBlend);
			HOOK_GDI_DC_FUNCTION(gdi32, BitBlt);
			HOOK_FUNCTION(gdi32, CreateCompatibleBitmap, Win32::DisplayMode::createCompatibleBitmap);
			HOOK_FUNCTION(gdi32, CreateDIBitmap, Win32::DisplayMode::createDIBitmap);
			HOOK_FUNCTION(gdi32, CreateDiscardableBitmap, Win32::DisplayMode::createDiscardableBitmap);
			HOOK_GDI_DC_FUNCTION(gdi32, ExtFloodFill);
			HOOK_GDI_DC_FUNCTION(gdi32, GdiAlphaBlend);
			HOOK_GDI_DC_FUNCTION(gdi32, GdiGradientFill);
			HOOK_GDI_DC_FUNCTION(gdi32, GdiTransparentBlt);
			HOOK_GDI_DC_FUNCTION(gdi32, GetDIBits);
			HOOK_GDI_DC_FUNCTION(gdi32, GetPixel);
			HOOK_GDI_DC_FUNCTION(msimg32, GradientFill);
			HOOK_GDI_DC_FUNCTION(gdi32, MaskBlt);
			HOOK_GDI_DC_FUNCTION(gdi32, PlgBlt);
			HOOK_GDI_DC_FUNCTION(gdi32, SetDIBits);
			HOOK_GDI_DC_FUNCTION(gdi32, SetDIBitsToDevice);
			HOOK_GDI_DC_FUNCTION(gdi32, SetPixel);
			HOOK_GDI_DC_FUNCTION(gdi32, SetPixelV);
			HOOK_GDI_DC_FUNCTION(gdi32, StretchBlt);
			HOOK_GDI_DC_FUNCTION(gdi32, StretchDIBits);
			HOOK_GDI_DC_FUNCTION(msimg32, TransparentBlt);

			// Brush functions
			HOOK_GDI_DC_FUNCTION(gdi32, PatBlt);

			// Clipping functions
			HOOK_FUNCTION(gdi32, GetRandomRgn, getRandomRgn);

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
			HOOK_GDI_DC_FUNCTION(user32, DrawCaption);
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
