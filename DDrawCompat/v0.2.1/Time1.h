#pragma once

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>

namespace Time
{
	extern long long g_qpcFrequency;

	void init();

	inline long long msToQpc(int ms)
	{
		return static_cast<long long>(ms) * g_qpcFrequency / 1000;
	}

	inline int qpcToMs(long long qpc)
	{
		return static_cast<int>(qpc * 1000 / g_qpcFrequency);
	}

	inline long long queryPerformanceCounter()
	{
		LARGE_INTEGER qpc = {};
		QueryPerformanceCounter(&qpc);
		return qpc.QuadPart;
	}
}
