#pragma once

#include <Common/Time.h>

namespace Time
{
	long long g_qpcFrequency = 0;

	void init()
	{
		LARGE_INTEGER qpc;
		QueryPerformanceFrequency(&qpc);
		g_qpcFrequency = qpc.QuadPart;
	}
}
