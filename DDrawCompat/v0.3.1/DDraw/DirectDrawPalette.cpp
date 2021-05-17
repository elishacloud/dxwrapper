#define WIN32_LEAN_AND_MEAN
#define CINTERFACE

#include <cstring>
#include <deque>

#include <DDrawCompat/v0.3.1/Common/CompatVtable.h>
#include <DDrawCompat/v0.3.1/Common/Time.h>
#include <DDrawCompat/v0.3.1/Config/Config.h>
#include <DDrawCompat/v0.3.1/DDraw/DirectDrawPalette.h>
#include <DDrawCompat/v0.3.1/DDraw/ScopedThreadLock.h>
#include <DDrawCompat/v0.3.1/DDraw/Surfaces/PrimarySurface.h>
#include <DDrawCompat/v0.3.1/DDraw/Visitors/DirectDrawPaletteVtblVisitor.h>

namespace
{
	HRESULT STDMETHODCALLTYPE SetEntries(
		IDirectDrawPalette* This, DWORD dwFlags, DWORD dwStartingEntry, DWORD dwCount, LPPALETTEENTRY lpEntries)
	{
		if (This == DDraw::PrimarySurface::s_palette)
		{
			DDraw::DirectDrawPalette::waitForNextUpdate();
		}

		HRESULT result = getOrigVtable(This).SetEntries(
			This, dwFlags, dwStartingEntry, dwCount, lpEntries);
		if (SUCCEEDED(result) && This == DDraw::PrimarySurface::s_palette)
		{
			DDraw::PrimarySurface::updatePalette();
		}
		return result;
	}

	constexpr void setCompatVtable(IDirectDrawPaletteVtbl& vtable)
	{
		vtable.SetEntries = &SetEntries;
	}
}

namespace DDraw
{
	namespace DirectDrawPalette
	{
		void waitForNextUpdate()
		{
			static std::deque<long long> updatesInLastMs;

			const long long qpcNow = Time::queryPerformanceCounter();
			const long long qpcLastMsBegin = qpcNow - Time::g_qpcFrequency / 1000;
			while (!updatesInLastMs.empty() && qpcLastMsBegin - updatesInLastMs.front() > 0)
			{
				updatesInLastMs.pop_front();
			}

			if (updatesInLastMs.size() >= Config30::maxPaletteUpdatesPerMs)
			{
				Sleep(1);
				updatesInLastMs.clear();
			}

			updatesInLastMs.push_back(Time::queryPerformanceCounter());
		}

		void hookVtable(const IDirectDrawPaletteVtbl& vtable)
		{
			CompatVtable<IDirectDrawPaletteVtbl>::hookVtable<ScopedThreadLock>(vtable);
		}
	}
}
