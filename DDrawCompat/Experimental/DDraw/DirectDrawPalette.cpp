#include <cstring>
#include <deque>

#include "Common/Time.h"
#include "Config/Config.h"
#include "DDraw/DirectDrawPalette.h"
#include "DDraw/RealPrimarySurface.h"
#include "DDraw/Surfaces/PrimarySurface.h"

namespace DDraw
{
	void DirectDrawPalette::setCompatVtable(IDirectDrawPaletteVtbl& vtable)
	{
		vtable.SetEntries = &SetEntries;
	}

	HRESULT STDMETHODCALLTYPE DirectDrawPalette::SetEntries(
		IDirectDrawPalette* This,
		DWORD dwFlags,
		DWORD dwStartingEntry,
		DWORD dwCount,
		LPPALETTEENTRY lpEntries)
	{
		if (This == PrimarySurface::s_palette)
		{
			waitForNextUpdate();
			if (lpEntries && dwStartingEntry + dwCount <= 256 &&
				0 == std::memcmp(&PrimarySurface::s_paletteEntries[dwStartingEntry],
					lpEntries, dwCount * sizeof(PALETTEENTRY)))
			{
				return DD_OK;
			}
		}

		HRESULT result = s_origVtable.SetEntries(This, dwFlags, dwStartingEntry, dwCount, lpEntries);
		if (This == PrimarySurface::s_palette && SUCCEEDED(result))
		{
			std::memcpy(&PrimarySurface::s_paletteEntries[dwStartingEntry], lpEntries,
				dwCount * sizeof(PALETTEENTRY));
			RealPrimarySurface::updatePalette(dwStartingEntry, dwCount);
		}
		return result;
	}

	void DirectDrawPalette::waitForNextUpdate()
	{
		static std::deque<long long> updatesInLastMs;

		const long long qpcNow = Time::queryPerformanceCounter();
		const long long qpcLastMsBegin = qpcNow - Time::g_qpcFrequency / 1000;
		while (!updatesInLastMs.empty() && qpcLastMsBegin - updatesInLastMs.front() > 0)
		{
			updatesInLastMs.pop_front();
		}

		if (updatesInLastMs.size() >= Config::maxPaletteUpdatesPerMs)
		{
			Sleep(1);
			updatesInLastMs.clear();
		}

		updatesInLastMs.push_back(Time::queryPerformanceCounter());
	}
}
