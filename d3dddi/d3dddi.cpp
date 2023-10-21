#if (_WIN32_WINNT < 0x0502) // Using Windows XP

#include "d3dddiExternal.h"

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

#include <d3d.h>
#include <d3dkmthk.h>
#include "d3dddiExternal.h"
#include "Logging\Logging.h"

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
		static HMODULE gdi_dll = LoadLibrary("gdi32.dll");
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

	static D3DKMTOpenAdapterFromHdcProc m_pD3DKMTOpenAdapterFromHdc = (D3DKMTOpenAdapterFromHdcProc)GetProcAddress(GetGDIHandle(), "D3DKMTOpenAdapterFromHdc");

	if (!m_pD3DKMTOpenAdapterFromHdc)
	{
		LOG_LIMIT(1, __FUNCTION__ << " Error: failed to get \"D3DKMTOpenAdapterFromHdc\" address from gdi.dll.");
		return false;
	}

	if (hDC)
	{
		openAdapter.hDc = hDC;
		if (m_pD3DKMTOpenAdapterFromHdc(&openAdapter) == STATUS_SUCCESS)
		{
			IsD3DDDILoaded = true;
		}
	}
	return IsD3DDDILoaded;
}

bool CloseD3DDDI()
{
	static D3DKMTCloseAdapterProc m_pD3DKMTCloseAdapter = (D3DKMTCloseAdapterProc)GetProcAddress(GetGDIHandle(), "D3DKMTCloseAdapter");

	if (!m_pD3DKMTCloseAdapter)
	{
		LOG_LIMIT(1, __FUNCTION__ << " Error: failed to get \"D3DKMTCloseAdapter\" address from gdi.dll.");
		return false;
	}

	if (IsD3DDDILoaded)
	{
		D3DKMT_CLOSEADAPTER closeAdapter = {};
		closeAdapter.hAdapter = openAdapter.hAdapter;

		if (m_pD3DKMTCloseAdapter(&closeAdapter) == STATUS_SUCCESS)
		{
			IsD3DDDILoaded = false;
			return true;
		}
	}
	return false;
}

bool D3DDDIWaitForVsync()
{
	static D3DKMTWaitForVerticalBlankEventProc m_pD3DKMTWaitForVerticalBlankEvent = (D3DKMTWaitForVerticalBlankEventProc)GetProcAddress(GetGDIHandle(), "D3DKMTWaitForVerticalBlankEvent");

	if (!m_pD3DKMTWaitForVerticalBlankEvent)
	{
		LOG_LIMIT(1, __FUNCTION__ << " Error: failed to get \"D3DKMTWaitForVerticalBlankEvent\" address from gdi.dll.");
		return false;
	}

	D3DKMT_WAITFORVERTICALBLANKEVENT VBlankEvent = {};
	VBlankEvent.hAdapter = openAdapter.hAdapter;

	return (IsD3DDDILoaded && m_pD3DKMTWaitForVerticalBlankEvent(&VBlankEvent) == STATUS_SUCCESS);
}

bool D3DDDIGetVideoMemory(DWORD& TotalMemory, DWORD& AvailableMemory)
{
	static D3DKMTQueryVideoMemoryInfoProc m_pD3DKMTQueryVideoMemoryInfo = (D3DKMTQueryVideoMemoryInfoProc)GetProcAddress(GetGDIHandle(), "D3DKMTQueryVideoMemoryInfo");

	if (!m_pD3DKMTQueryVideoMemoryInfo)
	{
		LOG_LIMIT(1, __FUNCTION__ << " Error: failed to get \"D3DKMTQueryVideoMemoryInfo\" address from gdi.dll.");
		return false;
	}

	D3DKMT_QUERYVIDEOMEMORYINFO VideoMemoryInfo = {};
	VideoMemoryInfo.hAdapter = openAdapter.hAdapter;

	// Query the video memory info for the adapter
	if (m_pD3DKMTQueryVideoMemoryInfo(&VideoMemoryInfo) == STATUS_SUCCESS)
	{
		// Return the video memory size
		TotalMemory = (DWORD)min(VideoMemoryInfo.Budget, 0xFFFFFFFF);
		AvailableMemory = (DWORD)min(VideoMemoryInfo.Budget - VideoMemoryInfo.CurrentUsage, 0xFFFFFFFF);
		return true;
	}
	return false;
}

#endif
