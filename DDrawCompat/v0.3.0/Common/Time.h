#pragma once

#include <Windows.h>

namespace Time
{
	extern long long g_qpcFrequency;

	void init();

	inline long long msToQpc(long long ms)
	{
		return static_cast<long long>(ms) * g_qpcFrequency / 1000;
	}

	inline long long qpcToMs(long long qpc)
	{
		return qpc * 1000 / g_qpcFrequency;
	}

	inline long long queryPerformanceCounter()
	{
		LARGE_INTEGER qpc = {};
		QueryPerformanceCounter(&qpc);
		return qpc.QuadPart;
	}

	inline ULONG64 queryThreadCycleTime()
	{
		ULONG64 cycleTime = 0;
		QueryThreadCycleTime(GetCurrentThread(), &cycleTime);
		return cycleTime;
	}
}
