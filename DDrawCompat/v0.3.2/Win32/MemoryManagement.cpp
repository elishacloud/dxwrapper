#define WIN32_LEAN_AND_MEAN
#define CINTERFACE

#include <Windows.h>

#include <DDrawCompat/v0.3.2/Common/Hook.h>
#include <DDrawCompat/DDrawLog.h>
#include <DDrawCompat/v0.3.2/Win32/MemoryManagement.h>

namespace
{
	void limitTo2Gb(SIZE_T& mem);

	void WINAPI globalMemoryStatus(LPMEMORYSTATUS lpBuffer)
	{
		LOG_FUNC("GlobalMemoryStatus", lpBuffer);
		CALL_ORIG_FUNC(GlobalMemoryStatus)(lpBuffer);
		limitTo2Gb(lpBuffer->dwTotalPhys);
		limitTo2Gb(lpBuffer->dwAvailPhys);
		limitTo2Gb(lpBuffer->dwTotalPageFile);
		limitTo2Gb(lpBuffer->dwAvailPageFile);
		limitTo2Gb(lpBuffer->dwTotalVirtual);
		limitTo2Gb(lpBuffer->dwAvailVirtual);
	}

	void limitTo2Gb(SIZE_T& mem)
	{
		if (mem > INT_MAX)
		{
			mem = INT_MAX;
		}
	}
}

namespace Win32
{
	namespace MemoryManagement
	{
		void installHooks()
		{
			HOOK_FUNCTION(kernel32, GlobalMemoryStatus, globalMemoryStatus);
		}
	}
}
