#define WIN32_LEAN_AND_MEAN
#define CINTERFACE

#include <Windows.h>

#include <DDrawCompat/v0.3.0/Common/Hook.h>
#include <DDrawCompat/v0.3.0/Common/Time.h>
#include <DDrawCompat/v0.3.0/D3dDdi/ScopedCriticalSection.h>
#include <DDrawCompat/v0.3.0/Dll/Dll.h>
#include <DDrawCompat/v0.3.0/Gdi/Caret.h>
#include <DDrawCompat/v0.3.0/Gdi/Gdi.h>

namespace
{
	struct CaretData
	{
		HWND hwnd;
		long left;
		long top;
		long width;
		long height;
		bool isVisible;
		bool isDrawn;
	};

	CaretData g_caret = {};
	long long g_qpcLastBlink = 0;

	void updateCaret(DWORD threadId);

	void CALLBACK caretEvent(HWINEVENTHOOK, DWORD, HWND hwnd, LONG idObject, LONG, DWORD, DWORD)
	{
		if (OBJID_CARET == idObject)
		{
			updateCaret(GetWindowThreadProcessId(hwnd, nullptr));
		}
	}

	void drawCaret()
	{
		HDC dc = GetDCEx(g_caret.hwnd, nullptr, DCX_CACHE | DCX_USESTYLE);
		PatBlt(dc, g_caret.left, g_caret.top, g_caret.width, g_caret.height, PATINVERT);
		CALL_ORIG_FUNC(ReleaseDC)(g_caret.hwnd, dc);
	}

	CaretData getCaretData(DWORD threadId)
	{
		GUITHREADINFO gti = {};
		gti.cbSize = sizeof(gti);
		GetGUIThreadInfo(threadId, &gti);

		CaretData caretData = {};
		caretData.hwnd = gti.hwndCaret;
		caretData.left = gti.rcCaret.left;
		caretData.top = gti.rcCaret.top;
		caretData.width = gti.rcCaret.right - gti.rcCaret.left;
		caretData.height = gti.rcCaret.bottom - gti.rcCaret.top;
		caretData.isVisible = gti.flags & GUI_CARETBLINKING;
		return caretData;
	}

	void updateCaret(DWORD threadId)
	{
		D3dDdi::ScopedCriticalSection lock;
		if (g_caret.isDrawn)
		{
			drawCaret();
		}

		g_caret = getCaretData(threadId);

		if (g_caret.isVisible)
		{
			g_caret.isDrawn = true;
			drawCaret();
			g_qpcLastBlink = Time::queryPerformanceCounter();
		}
	}
}

namespace Gdi
{
	namespace Caret
	{
		void blink()
		{
			D3dDdi::ScopedCriticalSection lock;
			if (!g_caret.isVisible)
			{
				return;
			}

			UINT caretBlinkTime = GetCaretBlinkTime();
			if (INFINITE == caretBlinkTime)
			{
				return;
			}

			const long long qpcNow = Time::queryPerformanceCounter();
			if (Time::qpcToMs(qpcNow - g_qpcLastBlink) >= caretBlinkTime)
			{
				g_qpcLastBlink = qpcNow;

				GUITHREADINFO gti = {};
				gti.cbSize = sizeof(gti);
				GetGUIThreadInfo(GetWindowThreadProcessId(g_caret.hwnd, nullptr), &gti);
				if (!(gti.flags & (GUI_INMENUMODE | GUI_POPUPMENUMODE | GUI_SYSTEMMENUMODE)))
				{
					g_caret.isDrawn = !g_caret.isDrawn;
					drawCaret();
				}
			}
		}

		void installHooks()
		{
			SetWinEventHook(EVENT_OBJECT_SHOW, EVENT_OBJECT_HIDE,
				Dll::g_currentModule, &caretEvent, GetCurrentProcessId(), 0, WINEVENT_INCONTEXT);
			SetWinEventHook(EVENT_OBJECT_LOCATIONCHANGE, EVENT_OBJECT_LOCATIONCHANGE,
				Dll::g_currentModule, &caretEvent, GetCurrentProcessId(), 0, WINEVENT_INCONTEXT);
		}
	}
}
