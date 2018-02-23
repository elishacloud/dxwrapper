#include <atomic>

//********** Begin Edit *************
//#include "winmm.h"
//********** End Edit ***************
#include "Common/CompatPtr.h"
#include "Common/Hook.h"
#include "Common/Time.h"
#include "Config/Config.h"
#include "D3dDdi/KernelModeThunks.h"
#include "DDraw/DirectDraw.h"
#include "DDraw/DirectDrawSurface.h"
#include "DDraw/IReleaseNotifier.h"
#include "DDraw/RealPrimarySurface.h"
#include "DDraw/ScopedThreadLock.h"
#include "DDraw/Surfaces/PrimarySurface.h"
#include "DDraw/Types.h"
#include "Gdi/Gdi.h"
#include "Win32/DisplayMode.h"





namespace
{
	void onRelease();
	DWORD WINAPI updateThreadProc(LPVOID lpParameter);

	DWORD g_primaryThreadId = 0;
	CompatWeakPtr<IDirectDrawSurface7> g_frontBuffer;
	CompatWeakPtr<IDirectDrawSurface7> g_backBuffer;
	CompatWeakPtr<IDirectDrawSurface7> g_paletteConverter;
	CompatWeakPtr<IDirectDrawClipper> g_clipper;
	DDSURFACEDESC2 g_surfaceDesc = {};
	DDraw::IReleaseNotifier g_releaseNotifier(onRelease);

	bool g_stopUpdateThread = false;
	HANDLE g_updateThread = nullptr;
	HANDLE g_updateEvent = nullptr;
	std::atomic<int> g_disableUpdateCount = 0;
	bool g_isUpdateSuspended = false;
	long long g_qpcFlipModeTimeout = 0;
	long long g_qpcLastFlip = 0;
	long long g_qpcNextUpdate = 0;
	long long g_qpcUpdateInterval = 0;

	std::atomic<bool> g_isFullScreen(false);

	BOOL CALLBACK bltToWindow(HWND hwnd, LPARAM lParam)
	{
		g_clipper->SetHWnd(g_clipper, 0, hwnd);
		auto src = reinterpret_cast<IDirectDrawSurface7*>(lParam);
		g_frontBuffer->Blt(g_frontBuffer, nullptr, src, nullptr, DDBLT_WAIT, nullptr);
		return TRUE;
	}

	HRESULT bltToPrimaryChain(CompatRef<IDirectDrawSurface7> src)
	{
		if (g_isFullScreen)
		{
			return g_backBuffer->Blt(g_backBuffer, nullptr, &src, nullptr, DDBLT_WAIT, nullptr);
		}

		EnumThreadWindows(g_primaryThreadId, bltToWindow, reinterpret_cast<LPARAM>(&src));
		return DD_OK;
	}

	bool compatBlt()
	{
		Compat::LogEnter("RealPrimarySurface::compatBlt");

		bool result = false;

		auto primary(DDraw::PrimarySurface::getPrimary());
		if (DDraw::PrimarySurface::getDesc().ddpfPixelFormat.dwRGBBitCount <= 8)
		{
			HDC paletteConverterDc = nullptr;
			g_paletteConverter->GetDC(g_paletteConverter, &paletteConverterDc);
			HDC primaryDc = nullptr;
			primary->GetDC(primary, &primaryDc);

			if (paletteConverterDc && primaryDc)
			{
				result = TRUE == CALL_ORIG_FUNC(BitBlt)(paletteConverterDc,
					0, 0, g_surfaceDesc.dwWidth, g_surfaceDesc.dwHeight, primaryDc, 0, 0, SRCCOPY);
			}

			primary->ReleaseDC(primary, primaryDc);
			g_paletteConverter->ReleaseDC(g_paletteConverter, paletteConverterDc);

			if (result)
			{
				result = SUCCEEDED(bltToPrimaryChain(*g_paletteConverter));
			}
		}
		else
		{
			result = SUCCEEDED(bltToPrimaryChain(*primary));
		}

		Compat::LogLeave("RealPrimarySurface::compatBlt") << result;
		return result;
	}

	template <typename TDirectDraw>
	HRESULT createPaletteConverter(CompatRef<TDirectDraw> dd)
	{
		auto dm = DDraw::getDisplayMode(*CompatPtr<IDirectDraw7>::from(&dd));
		if (dm.ddpfPixelFormat.dwRGBBitCount > 8)
		{
			return DD_OK;
		}

		typename DDraw::Types<TDirectDraw>::TSurfaceDesc desc = {};
		desc.dwSize = sizeof(desc);
		desc.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT | DDSD_CAPS;
		desc.dwWidth = dm.dwWidth;
		desc.dwHeight = dm.dwHeight;
		desc.ddpfPixelFormat.dwSize = sizeof(desc.ddpfPixelFormat);
		desc.ddpfPixelFormat.dwFlags = DDPF_RGB;
		desc.ddpfPixelFormat.dwRGBBitCount = 32;
		desc.ddpfPixelFormat.dwRBitMask = 0x00FF0000;
		desc.ddpfPixelFormat.dwGBitMask = 0x0000FF00;
		desc.ddpfPixelFormat.dwBBitMask = 0x000000FF;
		desc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;

		CompatPtr<DDraw::Types<TDirectDraw>::TCreatedSurface> paletteConverter;
		HRESULT result = dd->CreateSurface(&dd, &desc, &paletteConverter.getRef(), nullptr);
		if (SUCCEEDED(result))
		{
			g_paletteConverter = Compat::queryInterface<IDirectDrawSurface7>(paletteConverter.get());
		}

		return result;
	}

	template <typename DirectDraw>
	HRESULT init(CompatRef<DirectDraw> dd, CompatPtr<IDirectDrawSurface7> surface)
	{
		DDSURFACEDESC2 desc = {};
		desc.dwSize = sizeof(desc);
		surface->GetSurfaceDesc(surface, &desc);

		const bool isFlippable = 0 != (desc.ddsCaps.dwCaps & DDSCAPS_FLIP);
		CompatPtr<IDirectDrawSurface7> backBuffer;
		if (isFlippable)
		{
			DDSCAPS2 backBufferCaps = {};
			backBufferCaps.dwCaps = DDSCAPS_BACKBUFFER;
			surface->GetAttachedSurface(surface, &backBufferCaps, &backBuffer.getRef());
		}
		else
		{
			CALL_ORIG_PROC(DirectDrawCreateClipper, 0, &g_clipper.getRef(), nullptr);
			surface->SetClipper(surface, g_clipper);
		}

		g_qpcFlipModeTimeout = Time::g_qpcFrequency / Config::minExpectedFlipsPerSec;
		g_qpcLastFlip = Time::queryPerformanceCounter() - g_qpcFlipModeTimeout;
		g_qpcNextUpdate = Time::queryPerformanceCounter();

		typename DDraw::Types<DirectDraw>::TSurfaceDesc dm = {};
		dm.dwSize = sizeof(dm);
		dd->GetDisplayMode(&dd, &dm);
		if (dm.dwRefreshRate <= 1 || dm.dwRefreshRate >= 1000)
		{
			dm.dwRefreshRate = 60;
		}
		g_qpcUpdateInterval = Time::g_qpcFrequency / dm.dwRefreshRate;

		if (!g_updateEvent)
		{
			g_updateEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
		}

		if (!g_updateThread)
		{
			g_updateThread = CreateThread(nullptr, 0, &updateThreadProc, nullptr, 0, nullptr);
			SetThreadPriority(g_updateThread, THREAD_PRIORITY_TIME_CRITICAL);
		}

		surface->SetPrivateData(surface, IID_IReleaseNotifier,
			&g_releaseNotifier, sizeof(&g_releaseNotifier), DDSPD_IUNKNOWNPOINTER);

		g_frontBuffer = surface.detach();
		g_backBuffer = backBuffer;
		g_surfaceDesc = desc;
		g_isFullScreen = isFlippable;
		g_primaryThreadId = GetCurrentThreadId();

		return DD_OK;
	}

	bool isUpdateScheduled()
	{
		return WAIT_OBJECT_0 == WaitForSingleObject(g_updateEvent, 0);
	}

	int msUntilNextUpdate()
	{
		DDraw::ScopedThreadLock lock;
		const auto qpcNow = Time::queryPerformanceCounter();
		const int result = max(0, Time::qpcToMs(g_qpcNextUpdate - qpcNow));
		if (0 == result && g_isFullScreen && qpcNow - g_qpcLastFlip >= g_qpcFlipModeTimeout)
		{
			return D3dDdi::KernelModeThunks::isPresentReady() ? 0 : 2;
		}
		return result;
	}

	void onRelease()
	{
		Compat::LogEnter("RealPrimarySurface::onRelease");

		ResetEvent(g_updateEvent);
		g_frontBuffer = nullptr;
		g_backBuffer = nullptr;
		g_clipper.release();
		g_isFullScreen = false;
		g_paletteConverter.release();

		ZeroMemory(&g_surfaceDesc, sizeof(g_surfaceDesc));

		Compat::LogLeave("RealPrimarySurface::onRelease");
	}

	void updateNow()
	{
		ResetEvent(g_updateEvent);

		if (compatBlt() && g_isFullScreen)
		{
			D3dDdi::KernelModeThunks::overrideFlipInterval(
				Time::queryPerformanceCounter() - g_qpcLastFlip >= g_qpcFlipModeTimeout
				? D3DDDI_FLIPINTERVAL_ONE
				: D3DDDI_FLIPINTERVAL_IMMEDIATE);
			g_frontBuffer->Flip(g_frontBuffer, nullptr, DDFLIP_WAIT);
			D3dDdi::KernelModeThunks::overrideFlipInterval(D3DDDI_FLIPINTERVAL_NOOVERRIDE);
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

			const int waitTime = msUntilNextUpdate();
			if (waitTime > 0)
			{
				Sleep(waitTime);
				continue;
			}

			DDraw::ScopedThreadLock lock;
			if (isUpdateScheduled() && msUntilNextUpdate() <= 0)
			{
				updateNow();
			}
		}
	}
}

namespace DDraw
{
	template <typename DirectDraw>
	HRESULT RealPrimarySurface::create(CompatRef<DirectDraw> dd)
	{
		HRESULT result = createPaletteConverter(dd);
		if (FAILED(result))
		{
			Compat::Log() << "Failed to create the palette converter surface: " << Compat::hex(result);
			return result;
		}

		typename Types<DirectDraw>::TSurfaceDesc desc = {};
		desc.dwSize = sizeof(desc);
		desc.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
		desc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_COMPLEX | DDSCAPS_FLIP;
		desc.dwBackBufferCount = 1;

		CompatPtr<typename Types<DirectDraw>::TCreatedSurface> surface;
		result = dd->CreateSurface(&dd, &desc, &surface.getRef(), nullptr);

		if (DDERR_NOEXCLUSIVEMODE == result)
		{
			desc.dwFlags = DDSD_CAPS;
			desc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
			desc.dwBackBufferCount = 0;
			result = dd->CreateSurface(&dd, &desc, &surface.getRef(), nullptr);
		}

		if (FAILED(result))
		{
			Compat::Log() << "Failed to create the real primary surface: " << Compat::hex(result);
			g_paletteConverter.release();
			return result;
		}

		return init(dd, surface);
	}

	template HRESULT RealPrimarySurface::create(CompatRef<IDirectDraw>);
	template HRESULT RealPrimarySurface::create(CompatRef<IDirectDraw2>);
	template HRESULT RealPrimarySurface::create(CompatRef<IDirectDraw4>);
	template HRESULT RealPrimarySurface::create(CompatRef<IDirectDraw7>);

	void RealPrimarySurface::disableUpdates()
	{
		if (0 == g_disableUpdateCount++ && isUpdateScheduled())
		{
			ResetEvent(g_updateEvent);
			g_isUpdateSuspended = true;
		}
	}

	void RealPrimarySurface::enableUpdates()
	{
		if (0 == --g_disableUpdateCount && g_isUpdateSuspended)
		{
			SetEvent(g_updateEvent);
			g_isUpdateSuspended = false;
		}
	}

	HRESULT RealPrimarySurface::flip(DWORD flags)
	{
		if (!g_isFullScreen)
		{
			return DDERR_NOTFLIPPABLE;
		}

		ResetEvent(g_updateEvent);
		g_isUpdateSuspended = false;

		g_qpcLastFlip = Time::queryPerformanceCounter();
		compatBlt();
		HRESULT result = g_frontBuffer->Flip(g_frontBuffer, nullptr, flags);
		g_qpcNextUpdate = Time::queryPerformanceCounter();
		return result;
	}

	HRESULT RealPrimarySurface::getGammaRamp(DDGAMMARAMP* rampData)
	{
		auto gammaControl(CompatPtr<IDirectDrawGammaControl>::from(g_frontBuffer.get()));
		if (!gammaControl)
		{
			return DDERR_INVALIDPARAMS;
		}

		return gammaControl->GetGammaRamp(gammaControl, 0, rampData);
	}

	CompatWeakPtr<IDirectDrawSurface7> RealPrimarySurface::getSurface()
	{
		return g_frontBuffer;
	}

	bool RealPrimarySurface::isFullScreen()
	{
		return g_isFullScreen;
	}

	bool RealPrimarySurface::isLost()
	{
		return g_frontBuffer && DDERR_SURFACELOST == g_frontBuffer->IsLost(g_frontBuffer);
	}

	void RealPrimarySurface::release()
	{
		g_frontBuffer.release();
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
		return g_frontBuffer->Restore(g_frontBuffer);
	}

	HRESULT RealPrimarySurface::setGammaRamp(DDGAMMARAMP* rampData)
	{
		auto gammaControl(CompatPtr<IDirectDrawGammaControl>::from(g_frontBuffer.get()));
		if (!gammaControl)
		{
			return DDERR_INVALIDPARAMS;
		}

		return gammaControl->SetGammaRamp(gammaControl, 0, rampData);
	}

	void RealPrimarySurface::setPalette()
	{
		if (g_surfaceDesc.ddpfPixelFormat.dwRGBBitCount <= 8)
		{
			g_frontBuffer->SetPalette(g_frontBuffer, PrimarySurface::s_palette);
		}

		updatePalette(0, 256);
	}

	void RealPrimarySurface::update()
	{
		if (g_isUpdateSuspended)
		{
			return;
		}

		if (!isUpdateScheduled())
		{
			const auto qpcNow = Time::queryPerformanceCounter();
			const long long missedIntervals = (qpcNow - g_qpcNextUpdate) / g_qpcUpdateInterval;
			g_qpcNextUpdate += g_qpcUpdateInterval * (missedIntervals + 1);
			if (Time::qpcToMs(g_qpcNextUpdate - qpcNow) < 2)
			{
				g_qpcNextUpdate += g_qpcUpdateInterval;
			}

			if (g_disableUpdateCount <= 0)
			{
				SetEvent(g_updateEvent);
			}
			else
			{
				g_isUpdateSuspended = true;
			}
		}
		else if (msUntilNextUpdate() <= 0)
		{
			updateNow();
		}
	}

	void RealPrimarySurface::updatePalette(DWORD startingEntry, DWORD count)
	{
		Gdi::updatePalette(startingEntry, count);
		if (PrimarySurface::s_palette)
		{
			update();
		}
	}
}
