#define WIN32_LEAN_AND_MEAN
#define CINTERFACE

#include <Windows.h>

#include <DDrawCompat/v0.3.2/Common/Hook.h>
#include <DDrawCompat/DDrawLog.h>
#include <DDrawCompat/v0.3.2/Gdi/Metrics.h>

typedef int(WINAPI* GetSystemMetricsForDpiProc)(int  nIndex, UINT dpi);

namespace
{
	GetSystemMetricsForDpiProc g_origGetSystemMetricsForDpi = nullptr;

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
			Compat32::hookFunction("user32", "GetSystemMetricsForDpi",
				reinterpret_cast<void*&>(g_origGetSystemMetricsForDpi), getSystemMetricsForDpi);
		}
	}
}
