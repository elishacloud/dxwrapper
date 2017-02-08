#include <atomic>

#include "CompatDirectDrawPalette.h"
#include "CompatDirectDrawSurface.h"
#include "CompatGdi.h"
#include "CompatGdiCaret.h"
#include "CompatGdiDcCache.h"
#include "CompatGdiDcFunctions.h"
#include "CompatGdiPaintHandlers.h"
#include "CompatGdiScrollFunctions.h"
#include "CompatGdiWinProc.h"
#include "CompatPaletteConverter.h"
#include "CompatPrimarySurface.h"
#include "DDrawProcs.h"
#include "RealPrimarySurface.h"
#include "ScopedCriticalSection.h"

namespace
{
	std::atomic<int> g_disableEmulationCount = 0;
	DWORD g_renderingRefCount = 0;
	DWORD g_ddLockThreadRenderingRefCount = 0;
	DWORD g_ddLockThreadId = 0;
	HANDLE g_ddUnlockBeginEvent = nullptr;
	HANDLE g_ddUnlockEndEvent = nullptr;
	bool g_isDelayedUnlockPending = false;

	BOOL CALLBACK invalidateWindow(HWND hwnd, LPARAM lParam)
	{
		if (!IsWindowVisible(hwnd))
		{
			return TRUE;
		}

		DWORD processId = 0;
		GetWindowThreadProcessId(hwnd, &processId);
		if (processId != GetCurrentProcessId())
		{
			return TRUE;
		}

		if (lParam)
		{
			POINT origin = {};
			ClientToScreen(hwnd, &origin);
			RECT rect = *reinterpret_cast<const RECT*>(lParam);
			OffsetRect(&rect, -origin.x, -origin.y);
			RedrawWindow(hwnd, &rect, nullptr, RDW_ERASE | RDW_FRAME | RDW_INVALIDATE | RDW_ALLCHILDREN);
		}
		else
		{
			RedrawWindow(hwnd, nullptr, nullptr, RDW_ERASE | RDW_FRAME | RDW_INVALIDATE | RDW_ALLCHILDREN);
		}

		return TRUE;
	}

	bool lockPrimarySurface()
	{
		DDSURFACEDESC2 desc = {};
		desc.dwSize = sizeof(desc);
		if (FAILED(CompatDirectDrawSurface<IDirectDrawSurface7>::s_origVtable.Lock(
			CompatPrimarySurface::surface, nullptr, &desc, DDLOCK_WAIT, nullptr)))
		{
			return false;
		}

		g_ddLockThreadId = GetCurrentThreadId();
		CompatGdiDcCache::setDdLockThreadId(g_ddLockThreadId);
		CompatGdiDcCache::setSurfaceMemory(desc.lpSurface, desc.lPitch);
		return true;
	}

	void unlockPrimarySurface()
	{
		GdiFlush();
		CompatDirectDrawSurface<IDirectDrawSurface7>::s_origVtable.Unlock(
			CompatPrimarySurface::surface, nullptr);
		RealPrimarySurface::invalidate(nullptr);
		RealPrimarySurface::update();

		Compat::origProcs.ReleaseDDThreadLock();
	}
}

namespace CompatGdi
{
	CRITICAL_SECTION g_gdiCriticalSection;

	bool beginGdiRendering()
	{
		if (!isEmulationEnabled())
		{
			return false;
		}

		Compat::ScopedCriticalSection gdiLock(g_gdiCriticalSection);

		if (0 == g_renderingRefCount)
		{
			LeaveCriticalSection(&g_gdiCriticalSection);
			Compat::origProcs.AcquireDDThreadLock();
			EnterCriticalSection(&g_gdiCriticalSection);
			if (!lockPrimarySurface())
			{
				Compat::origProcs.ReleaseDDThreadLock();
				return false;
			}
		}

		if (GetCurrentThreadId() == g_ddLockThreadId)
		{
			++g_ddLockThreadRenderingRefCount;
		}

		++g_renderingRefCount;
		return true;
	}

	void endGdiRendering()
	{
		Compat::ScopedCriticalSection gdiLock(g_gdiCriticalSection);

		if (GetCurrentThreadId() == g_ddLockThreadId)
		{
			if (1 == g_renderingRefCount)
			{
				unlockPrimarySurface();
				g_ddLockThreadRenderingRefCount = 0;
				g_renderingRefCount = 0;
			}
			else if (1 == g_ddLockThreadRenderingRefCount)
			{
				g_isDelayedUnlockPending = true;
				gdiLock.unlock();
				WaitForSingleObject(g_ddUnlockBeginEvent, INFINITE);
				unlockPrimarySurface();
				g_ddLockThreadRenderingRefCount = 0;
				g_renderingRefCount = 0;
				SetEvent(g_ddUnlockEndEvent);
			}
			else
			{
				--g_ddLockThreadRenderingRefCount;
				--g_renderingRefCount;
			}
		}
		else
		{
			--g_renderingRefCount;
			if (1 == g_renderingRefCount && g_isDelayedUnlockPending)
			{
				SetEvent(g_ddUnlockBeginEvent);
				WaitForSingleObject(g_ddUnlockEndEvent, INFINITE);
				g_isDelayedUnlockPending = false;
			}
		}
	}

	void disableEmulation()
	{
		++g_disableEmulationCount;
	}

	void enableEmulation()
	{
		--g_disableEmulationCount;
	}

	void hookWndProc(LPCSTR className, WNDPROC &oldWndProc, WNDPROC newWndProc)
	{
		HWND hwnd = CreateWindow(className, nullptr, 0, 0, 0, 0, 0, nullptr, nullptr, nullptr, 0);
		oldWndProc = reinterpret_cast<WNDPROC>(
			SetClassLongPtr(hwnd, GCLP_WNDPROC, reinterpret_cast<LONG>(newWndProc)));
		DestroyWindow(hwnd);
	}

	void installHooks()
	{
		InitializeCriticalSection(&g_gdiCriticalSection);
		if (CompatGdiDcCache::init())
		{
			g_ddUnlockBeginEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
			g_ddUnlockEndEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
			if (!g_ddUnlockBeginEvent || !g_ddUnlockEndEvent)
			{
				Compat::Log() << "Failed to create the unlock events for GDI";
				return;
			}

			CompatGdiDcFunctions::installHooks();
			CompatGdiPaintHandlers::installHooks();
			CompatGdiScrollFunctions::installHooks();
			CompatGdiWinProc::installHooks();
			CompatGdiCaret::installHooks();
		}
	}

	void invalidate(const RECT* rect)
	{
		if (isEmulationEnabled())
		{
			EnumWindows(&invalidateWindow, reinterpret_cast<LPARAM>(rect));
		}
	}

	bool isEmulationEnabled()
	{
		return g_disableEmulationCount <= 0 && RealPrimarySurface::isFullScreen();
	}

	void unhookWndProc(LPCSTR className, WNDPROC oldWndProc)
	{
		HWND hwnd = CreateWindow(className, nullptr, 0, 0, 0, 0, 0, nullptr, nullptr, nullptr, 0);
		SetClassLongPtr(hwnd, GCLP_WNDPROC, reinterpret_cast<LONG>(oldWndProc));
		DestroyWindow(hwnd);
	}

	void uninstallHooks()
	{
		CompatGdiCaret::uninstallHooks();
		CompatGdiWinProc::uninstallHooks();
		CompatGdiPaintHandlers::uninstallHooks();
	}

	void updatePalette(DWORD startingEntry, DWORD count)
	{
		if (isEmulationEnabled() && CompatPrimarySurface::palette)
		{
			CompatGdiDcCache::updatePalette(startingEntry, count);
		}
	}
}
