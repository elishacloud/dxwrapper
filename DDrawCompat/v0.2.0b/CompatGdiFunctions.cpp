#include "CompatGdi.h"
#include "CompatGdiDc.h"
#include "CompatGdiFunctions.h"
#include "DDrawLog.h"

#include <detours.h>

namespace Compat20
{
	namespace
	{
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
			HDC compatDc = CompatGdiDc::getDc(dc);
			return compatDc ? compatDc : dc;
		}

		template <typename T>
		void releaseDc(T) {}

		void releaseDc(HDC dc)
		{
			CompatGdiDc::releaseDc(dc);
		}

		template <typename T, typename... Params>
		void releaseDc(T t, Params... params)
		{
			releaseDc(params...);
			releaseDc(t);
		}

		template <typename OrigFuncPtr, OrigFuncPtr origFunc, typename Result, typename... Params>
		Result WINAPI compatGdiFunc(Params... params)
		{
			if (!hasDisplayDcArg(params...) || !CompatGdi::beginGdiRendering())
			{
				return CompatGdi::getOrigFuncPtr<OrigFuncPtr, origFunc>()(params...);
			}

#ifdef _DEBUG
			Logging::LogEnter(CompatGdi::g_funcNames[origFunc], params...);
#endif

			Result result = CompatGdi::getOrigFuncPtr<OrigFuncPtr, origFunc>()(replaceDc(params)...);
			releaseDc(params...);
			CompatGdi::endGdiRendering();

#ifdef _DEBUG
			Logging::LogLeave(CompatGdi::g_funcNames[origFunc], params...) << result;
#endif

			return result;
		}

		template <typename OrigFuncPtr, OrigFuncPtr origFunc, typename Result, typename... Params>
		OrigFuncPtr getCompatGdiFuncPtr(FuncPtr<Result, Params...>)
		{
			return &compatGdiFunc<OrigFuncPtr, origFunc, Result, Params...>;
		}
	}

#define HOOK_GDI_FUNCTION(module, func) \
	CompatGdi::hookGdiFunction<decltype(&func), &func>( \
		#module, #func, getCompatGdiFuncPtr<decltype(&func), &func>(&func));

#define HOOK_GDI_TEXT_FUNCTION(module, func) \
	HOOK_GDI_FUNCTION(module, func##A); \
	HOOK_GDI_FUNCTION(module, func##W)

	namespace CompatGdiFunctions
	{
		void installHooks()
		{
			DetourTransactionBegin();

			// Bitmap functions
			HOOK_GDI_FUNCTION(msimg32, AlphaBlend);
			HOOK_GDI_FUNCTION(gdi32, BitBlt);
			HOOK_GDI_FUNCTION(gdi32, CreateCompatibleBitmap);
			HOOK_GDI_FUNCTION(gdi32, CreateDIBitmap);
			HOOK_GDI_FUNCTION(gdi32, CreateDIBSection);
			HOOK_GDI_FUNCTION(gdi32, CreateDiscardableBitmap);
			HOOK_GDI_FUNCTION(gdi32, ExtFloodFill);
			HOOK_GDI_FUNCTION(gdi32, GetDIBits);
			HOOK_GDI_FUNCTION(gdi32, GetPixel);
			HOOK_GDI_FUNCTION(msimg32, GradientFill);
			HOOK_GDI_FUNCTION(gdi32, MaskBlt);
			HOOK_GDI_FUNCTION(gdi32, PlgBlt);
			HOOK_GDI_FUNCTION(gdi32, SetDIBits);
			HOOK_GDI_FUNCTION(gdi32, SetDIBitsToDevice);
			HOOK_GDI_FUNCTION(gdi32, SetPixel);
			HOOK_GDI_FUNCTION(gdi32, SetPixelV);
			HOOK_GDI_FUNCTION(gdi32, StretchBlt);
			HOOK_GDI_FUNCTION(gdi32, StretchDIBits);
			HOOK_GDI_FUNCTION(msimg32, TransparentBlt);

			// Brush functions
			HOOK_GDI_FUNCTION(gdi32, PatBlt);

			// Device context functions
			HOOK_GDI_FUNCTION(gdi32, CreateCompatibleDC);
			HOOK_GDI_FUNCTION(gdi32, DrawEscape);

			// Filled shape functions
			HOOK_GDI_FUNCTION(gdi32, Chord);
			HOOK_GDI_FUNCTION(gdi32, Ellipse);
			HOOK_GDI_FUNCTION(user32, FillRect);
			HOOK_GDI_FUNCTION(user32, FrameRect);
			HOOK_GDI_FUNCTION(user32, InvertRect);
			HOOK_GDI_FUNCTION(gdi32, Pie);
			HOOK_GDI_FUNCTION(gdi32, Polygon);
			HOOK_GDI_FUNCTION(gdi32, PolyPolygon);
			HOOK_GDI_FUNCTION(gdi32, Rectangle);
			HOOK_GDI_FUNCTION(gdi32, RoundRect);

			// Font and text functions
			HOOK_GDI_TEXT_FUNCTION(user32, DrawText);
			HOOK_GDI_TEXT_FUNCTION(user32, DrawTextEx);
			HOOK_GDI_TEXT_FUNCTION(gdi32, ExtTextOut);
			HOOK_GDI_TEXT_FUNCTION(gdi32, PolyTextOut);
			HOOK_GDI_TEXT_FUNCTION(user32, TabbedTextOut);
			HOOK_GDI_TEXT_FUNCTION(gdi32, TextOut);

			// Line and curve functions
			HOOK_GDI_FUNCTION(gdi32, AngleArc);
			HOOK_GDI_FUNCTION(gdi32, Arc);
			HOOK_GDI_FUNCTION(gdi32, ArcTo);
			HOOK_GDI_FUNCTION(gdi32, LineTo);
			HOOK_GDI_FUNCTION(gdi32, PolyBezier);
			HOOK_GDI_FUNCTION(gdi32, PolyBezierTo);
			HOOK_GDI_FUNCTION(gdi32, PolyDraw);
			HOOK_GDI_FUNCTION(gdi32, Polyline);
			HOOK_GDI_FUNCTION(gdi32, PolylineTo);
			HOOK_GDI_FUNCTION(gdi32, PolyPolyline);

			// Painting and drawing functions
			HOOK_GDI_FUNCTION(user32, DrawCaption);
			HOOK_GDI_FUNCTION(user32, DrawEdge);
			HOOK_GDI_FUNCTION(user32, DrawFocusRect);
			HOOK_GDI_FUNCTION(user32, DrawFrameControl);
			HOOK_GDI_TEXT_FUNCTION(user32, DrawState);
			HOOK_GDI_TEXT_FUNCTION(user32, GrayString);
			HOOK_GDI_FUNCTION(user32, PaintDesktop);

			// Region functions
			HOOK_GDI_FUNCTION(gdi32, FillRgn);
			HOOK_GDI_FUNCTION(gdi32, FrameRgn);
			HOOK_GDI_FUNCTION(gdi32, InvertRgn);
			HOOK_GDI_FUNCTION(gdi32, PaintRgn);

			// Scroll bar functions
			HOOK_GDI_FUNCTION(user32, ScrollDC);

			DetourTransactionCommit();
		}
	}
}
