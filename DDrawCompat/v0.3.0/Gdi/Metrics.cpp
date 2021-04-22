#define WIN32_LEAN_AND_MEAN
#define CINTERFACE

#include <Windows.h>

#include <Common/Hook.h>
#include <../DDrawLog.h>
#include <Gdi/Metrics.h>

namespace
{
	decltype(&GetSystemMetricsForDpi) g_origGetSystemMetricsForDpi = nullptr;

	int WINAPI getSystemMetrics(int nIndex)
	{
		LOG_FUNC("GetSystemMetrics", nIndex);
		if (SM_CXSIZE == nIndex)
		{
			nIndex = SM_CYSIZE;
		}
		return LOG_RESULT(CALL_ORIG_FUNC(GetSystemMetrics)(nIndex));
	}

	int WINAPI getSystemMetricsForDpi(int nIndex, UINT dpi)
	{
		LOG_FUNC("GetSystemMetricsForDpi", nIndex, dpi);
		if (SM_CXSIZE == nIndex)
		{
			nIndex = SM_CYSIZE;
		}
		return LOG_RESULT(g_origGetSystemMetricsForDpi(nIndex, dpi));
	}
}

namespace Gdi
{
	namespace Metrics
	{
		void installHooks()
		{
			HOOK_FUNCTION(user32, GetSystemMetrics, getSystemMetrics);
			Compat30::hookFunction("user32", "GetSystemMetricsForDpi",
				reinterpret_cast<void*&>(g_origGetSystemMetricsForDpi), getSystemMetricsForDpi);
		}
	}
}
