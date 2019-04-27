#include <atomic>

//********** Begin Edit *************
#include "winmm.h"
//********** End Edit ***************
#include "CompatDirectDraw.h"
#include "CompatDirectDrawPalette.h"
#include "CompatDirectDrawSurface.h"
#include "CompatGdi.h"
#include "CompatPaletteConverter.h"
#include "CompatPrimarySurface.h"
#include "Config.h"
#include "DDrawScopedThreadLock.h"
#include "DDrawProcs.h"
#include "DDrawTypes.h"
#include "Hook.h"
#include "IReleaseNotifier.h"
#include "RealPrimarySurface.h"
#include "Time.h"

namespace Compat21
{
	namespace
	{
		void onRelease();
		void updateNow(long long qpcNow);

		IDirectDrawSurface7* g_frontBuffer = nullptr;
		IDirectDrawSurface7* g_backBuffer = nullptr;
		IReleaseNotifier g_releaseNotifier(onRelease);

		bool g_stopUpdateThread = false;
		HANDLE g_updateThread = nullptr;
		HANDLE g_updateEvent = nullptr;
		RECT g_updateRect = {};
		std::atomic<int> g_disableUpdateCount = 0;
		long long g_qpcMinUpdateInterval = 0;
		std::atomic<long long> g_qpcNextUpdate = 0;

		std::atomic<bool> g_isFullScreen(false);

		bool compatBlt(IDirectDrawSurface7* dest)
		{
			Compat::LogEnter("RealPrimarySurface::compatBlt", dest);

			if (g_disableUpdateCount > 0)
			{
				return false;
			}

			bool result = false;
			const auto& origVtable = CompatDirectDrawSurface<IDirectDrawSurface7>::s_origVtable;

			if (!RealPrimarySurface::isFullScreen())
			{
				IDirectDrawClipper* clipper = nullptr;
				if (FAILED(origVtable.GetClipper(g_frontBuffer, &clipper)))
				{
					return false;
				}
				clipper->lpVtbl->Release(clipper);
			}

			if (CompatPrimarySurface::pixelFormat.dwRGBBitCount <= 8)
			{
				origVtable.Blt(CompatPaletteConverter::getSurface(), &g_updateRect,
					CompatPrimarySurface::surface, &g_updateRect, DDBLT_WAIT, nullptr);

				HDC destDc = nullptr;
				origVtable.GetDC(dest, &destDc);
				result = TRUE == CALL_ORIG_FUNC(BitBlt)(destDc, g_updateRect.left, g_updateRect.top,
					g_updateRect.right - g_updateRect.left, g_updateRect.bottom - g_updateRect.top,
					CompatPaletteConverter::getDc(), g_updateRect.left, g_updateRect.top, SRCCOPY);
				origVtable.ReleaseDC(dest, destDc);

				if (dest == g_frontBuffer)
				{
					// Force the screen to be updated. It won't refresh from BitBlt alone.
					RECT r = { 0, 0, 1, 1 };
					CompatDirectDrawSurface<IDirectDrawSurface7>::s_origVtable.BltFast(
						g_frontBuffer, 0, 0, g_frontBuffer, &r, DDBLTFAST_WAIT);
				}
			}
			else
			{
				result = SUCCEEDED(origVtable.Blt(dest, &g_updateRect,
					CompatPrimarySurface::surface, &g_updateRect, DDBLT_WAIT, nullptr));
			}

			if (result)
			{
				SetRectEmpty(&g_updateRect);
			}

			Compat::LogLeave("RealPrimarySurface::compatBlt", dest);
			return result;
		}

		long long getNextUpdateQpc(long long qpcNow)
		{
			long long qpcNextUpdate = g_qpcNextUpdate;
			const long long missedIntervals = (qpcNow - qpcNextUpdate) / g_qpcMinUpdateInterval;
			return qpcNextUpdate + g_qpcMinUpdateInterval * (missedIntervals + 1);
		}

		bool isNextUpdateSignaledAndReady(long long qpcNow)
		{
			return Time::qpcToMs(qpcNow - g_qpcNextUpdate) >= 0 &&
				WAIT_OBJECT_0 == WaitForSingleObject(g_updateEvent, 0);
		}

		void onRelease()
		{
			Compat::LogEnter("RealPrimarySurface::onRelease");

			ResetEvent(g_updateEvent);
			timeEndPeriod(1);
			g_frontBuffer = nullptr;
			if (g_backBuffer)
			{
				CompatDirectDrawSurface<IDirectDrawSurface7>::s_origVtable.Release(g_backBuffer);
				g_backBuffer = nullptr;
			}
			g_isFullScreen = false;
			CompatPaletteConverter::release();

			ZeroMemory(&RealPrimarySurface::s_surfaceDesc, sizeof(RealPrimarySurface::s_surfaceDesc));

			Compat::LogLeave("RealPrimarySurface::onRelease");
		}

		void updateNow(long long qpcNow)
		{
			ResetEvent(g_updateEvent);

			if (compatBlt(g_frontBuffer))
			{
				long long qpcNextUpdate = getNextUpdateQpc(qpcNow);
				if (Time::qpcToMs(qpcNow - qpcNextUpdate) >= 0)
				{
					qpcNextUpdate += g_qpcMinUpdateInterval;
				}
				g_qpcNextUpdate = qpcNextUpdate;
			}
		}

		DWORD WINAPI updateThreadProc(LPVOID /*lpParameter*/)
		{
			while (true)
			{
				WaitForSingleObject(g_updateEvent, INFINITE);

				if (g_stopUpdateThread)
				{
					return 0;
				}

				const long long qpcTargetNextUpdate = g_qpcNextUpdate;
				const int msUntilNextUpdate =
					Time::qpcToMs(qpcTargetNextUpdate - Time::queryPerformanceCounter());
				if (msUntilNextUpdate > 0)
				{
					Sleep(msUntilNextUpdate);
				}

				Compat::DDrawScopedThreadLock lock;
				const long long qpcNow = Time::queryPerformanceCounter();
				const bool isTargetUpdateStillNeeded = qpcTargetNextUpdate == g_qpcNextUpdate;
				if (g_frontBuffer && (isTargetUpdateStillNeeded || isNextUpdateSignaledAndReady(qpcNow)))
				{
					updateNow(qpcNow);
				}
			}
		}
	}

	DDSURFACEDESC2 RealPrimarySurface::s_surfaceDesc = {};

	template <typename DirectDraw>
	HRESULT RealPrimarySurface::create(DirectDraw& dd)
	{
		typename Types<DirectDraw>::TSurfaceDesc desc = {};
		desc.dwSize = sizeof(desc);
		desc.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
		desc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_COMPLEX | DDSCAPS_FLIP;
		desc.dwBackBufferCount = 1;

		typename Types<DirectDraw>::TCreatedSurface* surface = nullptr;
		HRESULT result = CompatDirectDraw<DirectDraw>::s_origVtable.CreateSurface(
			&dd, &desc, &surface, nullptr);

		bool isFlippable = true;
		if (DDERR_NOEXCLUSIVEMODE == result)
		{
			desc.dwFlags = DDSD_CAPS;
			desc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
			desc.dwBackBufferCount = 0;
			isFlippable = false;
			result = CompatDirectDraw<DirectDraw>::s_origVtable.CreateSurface(
				&dd, &desc, &surface, nullptr);
		}

		if (FAILED(result))
		{
			Compat::Log() << "Failed to create the real primary surface";
			return result;
		}

		surface->lpVtbl->QueryInterface(
			surface, IID_IDirectDrawSurface7, reinterpret_cast<LPVOID*>(&g_frontBuffer));
		surface->lpVtbl->Release(surface);

		s_surfaceDesc.dwSize = sizeof(s_surfaceDesc);
		g_frontBuffer->lpVtbl->GetSurfaceDesc(g_frontBuffer, &s_surfaceDesc);

		if (!CompatPaletteConverter::create())
		{
			g_frontBuffer->lpVtbl->Release(g_frontBuffer);
			g_frontBuffer = nullptr;
			return DDERR_GENERIC;
		}

		if (isFlippable)
		{
			DDSCAPS2 backBufferCaps = {};
			backBufferCaps.dwCaps = DDSCAPS_BACKBUFFER;
			g_frontBuffer->lpVtbl->GetAttachedSurface(g_frontBuffer, &backBufferCaps, &g_backBuffer);
		}

		g_qpcMinUpdateInterval = Time::g_qpcFrequency / Config::maxPrimaryUpdateRate;
		g_qpcNextUpdate = Time::queryPerformanceCounter();

		if (!g_updateEvent)
		{
			g_updateEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
		}

		if (!g_updateThread)
		{
			g_updateThread = CreateThread(nullptr, 0, &updateThreadProc, nullptr, 0, nullptr);
			SetThreadPriority(g_updateThread, THREAD_PRIORITY_ABOVE_NORMAL);
		}

		g_frontBuffer->lpVtbl->SetPrivateData(g_frontBuffer,
			IID_IReleaseNotifier, &g_releaseNotifier, sizeof(&g_releaseNotifier), DDSPD_IUNKNOWNPOINTER);

		g_isFullScreen = isFlippable;
		timeBeginPeriod(1);

		return DD_OK;
	}

	template HRESULT RealPrimarySurface::create(IDirectDraw&);
	template HRESULT RealPrimarySurface::create(IDirectDraw2&);
	template HRESULT RealPrimarySurface::create(IDirectDraw4&);
	template HRESULT RealPrimarySurface::create(IDirectDraw7&);

	void RealPrimarySurface::disableUpdates()
	{
		++g_disableUpdateCount;
		ResetEvent(g_updateEvent);
	}

	void RealPrimarySurface::enableUpdates()
	{
		if (0 == --g_disableUpdateCount)
		{
			update();
		}
	}

	HRESULT RealPrimarySurface::flip(DWORD flags)
	{
		if (!g_backBuffer)
		{
			return DDERR_NOTFLIPPABLE;
		}

		ResetEvent(g_updateEvent);

		invalidate(nullptr);
		compatBlt(g_backBuffer);
		if (flags & DDFLIP_DONOTWAIT)
		{
			flags ^= DDFLIP_DONOTWAIT;
		}

		HRESULT result = g_frontBuffer->lpVtbl->Flip(g_frontBuffer, nullptr, flags | DDFLIP_WAIT);
		if (SUCCEEDED(result))
		{
			g_qpcNextUpdate = getNextUpdateQpc(
				Time::queryPerformanceCounter() + Time::msToQpc(Config::primaryUpdateDelayAfterFlip));
			SetRectEmpty(&g_updateRect);
		}
		return result;
	}

	IDirectDrawSurface7* RealPrimarySurface::getSurface()
	{
		return g_frontBuffer;
	}

	void RealPrimarySurface::invalidate(const RECT* rect)
	{
		if (rect)
		{
			UnionRect(&g_updateRect, &g_updateRect, rect);
		}
		else
		{
			SetRect(&g_updateRect, 0, 0,
				CompatPrimarySurface::displayMode.width, CompatPrimarySurface::displayMode.height);
		}
	}

	bool RealPrimarySurface::isFullScreen()
	{
		return g_isFullScreen;
	}

	bool RealPrimarySurface::isLost()
	{
		return g_frontBuffer &&
			DDERR_SURFACELOST == CompatDirectDrawSurface<IDirectDrawSurface7>::s_origVtable.IsLost(g_frontBuffer);
	}

	void RealPrimarySurface::release()
	{
		if (g_frontBuffer)
		{
			g_frontBuffer->lpVtbl->Release(g_frontBuffer);
		}
	}

	void RealPrimarySurface::removeUpdateThread()
	{
		if (!g_updateThread)
		{
			return;
		}

		g_stopUpdateThread = true;
		SetEvent(g_updateEvent);
		if (WAIT_OBJECT_0 != WaitForSingleObject(g_updateThread, 1000))
		{
			TerminateThread(g_updateThread, 0);
			Compat::Log() << "The update thread was terminated forcefully";
		}
		ResetEvent(g_updateEvent);
		g_stopUpdateThread = false;
		g_updateThread = nullptr;
	}

	HRESULT RealPrimarySurface::restore()
	{
		return g_frontBuffer->lpVtbl->Restore(g_frontBuffer);
	}

	void RealPrimarySurface::setClipper(LPDIRECTDRAWCLIPPER clipper)
	{
		CompatPaletteConverter::setClipper(clipper);

		HRESULT result = CompatDirectDrawSurface<IDirectDrawSurface7>::s_origVtable.SetClipper(
			g_frontBuffer, clipper);
		if (FAILED(result))
		{
			LOG_ONCE("Failed to set clipper on the real primary surface: " << result);
		}
	}

	void RealPrimarySurface::setPalette()
	{
		if (s_surfaceDesc.ddpfPixelFormat.dwRGBBitCount <= 8)
		{
			CompatDirectDrawSurface<IDirectDrawSurface7>::s_origVtable.SetPalette(
				g_frontBuffer, CompatPrimarySurface::palette);
		}

		updatePalette(0, 256);
	}

	void RealPrimarySurface::update()
	{
		if (!IsRectEmpty(&g_updateRect) && 0 == g_disableUpdateCount)
		{
			const long long qpcNow = Time::queryPerformanceCounter();
			if (Time::qpcToMs(qpcNow - g_qpcNextUpdate) >= 0)
			{
				updateNow(qpcNow);
			}
			else
			{
				SetEvent(g_updateEvent);
			}
		}
	}

	void RealPrimarySurface::updatePalette(DWORD startingEntry, DWORD count)
	{
		CompatPaletteConverter::updatePalette(startingEntry, count);
		CompatGdi::updatePalette(startingEntry, count);
		invalidate(nullptr);
		update();
	}
}
