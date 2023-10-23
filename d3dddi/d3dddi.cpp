#include <d3d.h>
#include "d3dddiExternal.h"
#include "ddraw\ddrawExternal.h"
#include "Logging\Logging.h"

#if (_WIN32_WINNT < 0x0502) // Using Windows XP

bool OpenD3DDDI(HDC hDC)
{
	UNREFERENCED_PARAMETER(hDC);
	return false;
}
bool CloseD3DDDI() { return false; }
bool D3DDDIWaitForVsync() { return false; }
bool D3DDDIGetVideoMemory(DWORD& TotalMemory, DWORD& AvailableMemory)
{
	UNREFERENCED_PARAMETER(TotalMemory);
	UNREFERENCED_PARAMETER(AvailableMemory);
	return false;
}

#else // Not using Windows XP

#include <d3dkmthk.h>

typedef NTSTATUS(WINAPI* D3DKMTOpenAdapterFromHdcProc)(D3DKMT_OPENADAPTERFROMHDC* Arg1);
typedef NTSTATUS(WINAPI* D3DKMTCloseAdapterProc)(const D3DKMT_CLOSEADAPTER* Arg1);
typedef NTSTATUS(WINAPI* D3DKMTWaitForVerticalBlankEventProc)(const D3DKMT_WAITFORVERTICALBLANKEVENT* Arg1);
typedef NTSTATUS(WINAPI* D3DKMTQueryVideoMemoryInfoProc)(D3DKMT_QUERYVIDEOMEMORYINFO* Arg1);

#define STATUS_SUCCESS 0

namespace
{
	bool IsD3DDDILoaded = false;
	D3DKMT_OPENADAPTERFROMHDC openAdapter = {};

	HMODULE GetGDIHandle()
	{
		static HMODULE gdi_dll = nullptr;
		if (!gdi_dll)
		{
			gdi_dll = LoadLibrary("gdi32.dll");
		}
		return gdi_dll;
	}
}

bool OpenD3DDDI(HDC hDC)
{
	// Already opened
	if (IsD3DDDILoaded)
	{
		return true;
	}

	DEFINE_STATIC_PROC_ADDRESS(D3DKMTOpenAdapterFromHdcProc, D3DKMTOpenAdapterFromHdc, GetProcAddress(GetGDIHandle(), "D3DKMTOpenAdapterFromHdc"));

	if (!D3DKMTOpenAdapterFromHdc)
	{
		LOG_LIMIT(1, __FUNCTION__ << " Error: failed to get \"D3DKMTOpenAdapterFromHdc\" address from gdi.dll.");
		return false;
	}

	if (hDC)
	{
		openAdapter.hDc = hDC;
		if (D3DKMTOpenAdapterFromHdc(&openAdapter) == STATUS_SUCCESS)
		{
			IsD3DDDILoaded = true;
		}
	}
	return IsD3DDDILoaded;
}

bool CloseD3DDDI()
{
	DEFINE_STATIC_PROC_ADDRESS(D3DKMTCloseAdapterProc, D3DKMTCloseAdapter, GetProcAddress(GetGDIHandle(), "D3DKMTCloseAdapter"));

	if (!D3DKMTCloseAdapter)
	{
		LOG_LIMIT(1, __FUNCTION__ << " Error: failed to get \"D3DKMTCloseAdapter\" address from gdi.dll.");
		return false;
	}

	if (IsD3DDDILoaded)
	{
		D3DKMT_CLOSEADAPTER closeAdapter = {};
		closeAdapter.hAdapter = openAdapter.hAdapter;

		if (D3DKMTCloseAdapter(&closeAdapter) == STATUS_SUCCESS)
		{
			IsD3DDDILoaded = false;
			return true;
		}
	}
	return false;
}

bool D3DDDIWaitForVsync()
{
	DEFINE_STATIC_PROC_ADDRESS(D3DKMTWaitForVerticalBlankEventProc, D3DKMTWaitForVerticalBlankEvent, GetProcAddress(GetGDIHandle(), "D3DKMTWaitForVerticalBlankEvent"));

	if (!D3DKMTWaitForVerticalBlankEvent)
	{
		LOG_LIMIT(1, __FUNCTION__ << " Error: failed to get \"D3DKMTWaitForVerticalBlankEvent\" address from gdi.dll.");
		return false;
	}

	D3DKMT_WAITFORVERTICALBLANKEVENT VBlankEvent = {};
	VBlankEvent.hAdapter = openAdapter.hAdapter;

	return (IsD3DDDILoaded && D3DKMTWaitForVerticalBlankEvent(&VBlankEvent) == STATUS_SUCCESS);
}

bool D3DDDIGetVideoMemory(DWORD& TotalMemory, DWORD& AvailableMemory)
{
	DEFINE_STATIC_PROC_ADDRESS(D3DKMTQueryVideoMemoryInfoProc, D3DKMTQueryVideoMemoryInfo, GetProcAddress(GetGDIHandle(), "D3DKMTQueryVideoMemoryInfo"));

	if (!D3DKMTQueryVideoMemoryInfo)
	{
		LOG_LIMIT(1, __FUNCTION__ << " Error: failed to get \"D3DKMTQueryVideoMemoryInfo\" address from gdi.dll.");
		return false;
	}

	D3DKMT_QUERYVIDEOMEMORYINFO VideoMemoryInfo = {};
	VideoMemoryInfo.hAdapter = openAdapter.hAdapter;

	// Query the video memory info for the adapter
	if (D3DKMTQueryVideoMemoryInfo(&VideoMemoryInfo) == STATUS_SUCCESS)
	{
		// Return the video memory size
		TotalMemory = (DWORD)min(VideoMemoryInfo.Budget, 0xFFFFFFFF);
		AvailableMemory = (DWORD)min(VideoMemoryInfo.Budget - VideoMemoryInfo.CurrentUsage, 0xFFFFFFFF);
		return true;
	}
	return false;
}

#endif
