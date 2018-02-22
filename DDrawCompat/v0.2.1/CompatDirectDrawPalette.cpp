#include <cstring>
#include <deque>

#include "CompatDirectDrawPalette.h"
#include "CompatPrimarySurface.h"
#include "Config.h"
#include "RealPrimarySurface.h"
#include "Time.h"

void CompatDirectDrawPalette::setCompatVtable(IDirectDrawPaletteVtbl& vtable)
{
	vtable.SetEntries = &SetEntries;
}

HRESULT STDMETHODCALLTYPE CompatDirectDrawPalette::SetEntries(
	IDirectDrawPalette* This,
	DWORD dwFlags,
	DWORD dwStartingEntry,
	DWORD dwCount,
	LPPALETTEENTRY lpEntries)
{
	if (This == CompatPrimarySurface::palette)
	{
		waitForNextUpdate();
		if (lpEntries && dwStartingEntry + dwCount <= 256 &&
			0 == std::memcmp(&CompatPrimarySurface::paletteEntries[dwStartingEntry],
				lpEntries, dwCount * sizeof(PALETTEENTRY)))
		{
			return DD_OK;
		}
	}

	HRESULT result = s_origVtable.SetEntries(This, dwFlags, dwStartingEntry, dwCount, lpEntries);
	if (This == CompatPrimarySurface::palette && SUCCEEDED(result))
	{
		std::memcpy(&CompatPrimarySurface::paletteEntries[dwStartingEntry], lpEntries,
			dwCount * sizeof(PALETTEENTRY));
		RealPrimarySurface::updatePalette(dwStartingEntry, dwCount);
	}
	return result;
}

void CompatDirectDrawPalette::waitForNextUpdate()
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
