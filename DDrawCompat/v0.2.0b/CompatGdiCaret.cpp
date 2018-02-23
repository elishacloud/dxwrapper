#define CINTERFACE
#define WIN32_LEAN_AND_MEAN

#include <oleacc.h>
#include <Windows.h>
#include <detours.h>

#include "CompatGdi.h"
#include "CompatGdiCaret.h"
#include "CompatGdiDc.h"

namespace Compat20
{
	namespace
	{
		struct CaretData
		{
			HWND hwnd;
			long left;
			long top;
			long width;
			long height;
			bool isDrawn;
		};

		CaretData g_caret = {};

		void drawCaret()
		{
			if (CompatGdi::beginGdiRendering())
			{
				HDC dc = GetDC(g_caret.hwnd);
				HDC compatDc = CompatGdiDc::getDc(dc);
				PatBlt(compatDc, g_caret.left, g_caret.top, g_caret.width, g_caret.height, PATINVERT);
				CompatGdiDc::releaseDc(dc);
				ReleaseDC(g_caret.hwnd, dc);
				CompatGdi::endGdiRendering();
			}
		}

		LONG getCaretState(IAccessible* accessible)
		{
			VARIANT varChild = {};
			varChild.vt = VT_I4;
			varChild.lVal = CHILDID_SELF;
			VARIANT varState = {};
			accessible->lpVtbl->get_accState(accessible, varChild, &varState);
			return varState.lVal;
		}

		void CALLBACK caretDestroyEvent(
			HWINEVENTHOOK /*hWinEventHook*/,
			DWORD /*event*/,
			HWND hwnd,
			LONG idObject,
			LONG idChild,
			DWORD /*dwEventThread*/,
			DWORD /*dwmsEventTime*/)
		{
			CompatGdi::GdiScopedThreadLock gdiLock;
			if (OBJID_CARET != idObject || !g_caret.isDrawn || g_caret.hwnd != hwnd)
			{
				return;
			}

			IAccessible* accessible = nullptr;
			VARIANT varChild = {};
			AccessibleObjectFromEvent(hwnd, idObject, idChild, &accessible, &varChild);
			if (accessible)
			{
				if (STATE_SYSTEM_INVISIBLE == getCaretState(accessible))
				{
					drawCaret();
					g_caret.isDrawn = false;
				}
				accessible->lpVtbl->Release(accessible);
			}
		}

		BOOL WINAPI createCaret(HWND hWnd, HBITMAP hBitmap, int nWidth, int nHeight)
		{
			BOOL result = CALL_ORIG_GDI(CreateCaret)(hWnd, hBitmap, nWidth, nHeight);
			if (result)
			{
				CompatGdi::GdiScopedThreadLock gdiLock;
				if (g_caret.isDrawn)
				{
					drawCaret();
					g_caret.isDrawn = false;
				}
				g_caret.width = nWidth ? nWidth : GetSystemMetrics(SM_CXBORDER);
				g_caret.height = nHeight ? nHeight : GetSystemMetrics(SM_CYBORDER);
			}
			return result;
		}

		BOOL WINAPI showCaret(HWND hWnd)
		{
			if (!CALL_ORIG_GDI(ShowCaret)(hWnd))
			{
				return FALSE;
			}

			CompatGdi::GdiScopedThreadLock gdiLock;
			if (!g_caret.isDrawn)
			{
				IAccessible* accessible = nullptr;
				AccessibleObjectFromWindow(hWnd, static_cast<DWORD>(OBJID_CARET), IID_IAccessible,
					reinterpret_cast<void**>(&accessible));
				if (accessible)
				{
					if (0 == getCaretState(accessible))
					{
						POINT caretPos = {};
						GetCaretPos(&caretPos);
						g_caret.left = caretPos.x;
						g_caret.top = caretPos.y;
						g_caret.hwnd = hWnd;
						drawCaret();
						g_caret.isDrawn = true;
					}
					accessible->lpVtbl->Release(accessible);
				}
			}

			return TRUE;
		}

		BOOL WINAPI hideCaret(HWND hWnd)
		{
			BOOL result = CALL_ORIG_GDI(HideCaret)(hWnd);
			if (result)
			{
				CompatGdi::GdiScopedThreadLock gdiLock;
				if (g_caret.isDrawn)
				{
					drawCaret();
					g_caret.isDrawn = false;
				}
			}
			return result;
		}
	}

#define HOOK_GDI_FUNCTION(module, func, newFunc) \
	CompatGdi::hookGdiFunction<decltype(&func), &func>(#module, #func, &newFunc);

	namespace CompatGdiCaret
	{
		void installHooks()
		{
			DetourTransactionBegin();
			HOOK_GDI_FUNCTION(user32, CreateCaret, createCaret);
			HOOK_GDI_FUNCTION(user32, ShowCaret, showCaret);
			HOOK_GDI_FUNCTION(user32, HideCaret, hideCaret);
			DetourTransactionCommit();

			const DWORD threadId = GetCurrentThreadId();
			SetWinEventHook(EVENT_OBJECT_DESTROY, EVENT_OBJECT_DESTROY,
				nullptr, &caretDestroyEvent, 0, threadId, WINEVENT_OUTOFCONTEXT);
		}
	}
}
