#include <map>
#include <vector>

#include "Common/CompatRef.h"
#include "DDraw/DirectDrawClipper.h"
#include "Gdi/Gdi.h"

namespace
{
	struct ClipperData
	{
		HWND hwnd;
		std::vector<unsigned char> oldClipList;
	};

	std::map<IDirectDrawClipper*, ClipperData> g_clipperData;

	void updateWindowClipList(CompatRef<IDirectDrawClipper> clipper, ClipperData& data);

	void onWindowPosChange(HWND /*hwnd*/, const RECT& oldWindowRect, const RECT& newWindowRect)
	{
		for (auto& clipperData : g_clipperData)
		{
			if (!IsRectEmpty(&oldWindowRect) || !IsRectEmpty(&newWindowRect))
			{
				updateWindowClipList(*clipperData.first, clipperData.second);
			}
		}
	}

	void updateWindowClipList(CompatRef<IDirectDrawClipper> clipper, ClipperData& data)
	{
		HDC dc = GetDC(data.hwnd);
		HRGN rgn = CreateRectRgn(0, 0, 0, 0);

		GetRandomRgn(dc, rgn, SYSRGN);
		DWORD rgnSize = GetRegionData(rgn, 0, nullptr);
		std::vector<unsigned char> rgnData(rgnSize);
		GetRegionData(rgn, rgnSize, reinterpret_cast<RGNDATA*>(rgnData.data()));

		clipper->SetHWnd(&clipper, 0, nullptr);
		if (FAILED(clipper->SetClipList(&clipper, reinterpret_cast<RGNDATA*>(rgnData.data()), 0)))
		{
			clipper->SetHWnd(&clipper, 0, data.hwnd);
		}

		DeleteObject(rgn);
		ReleaseDC(data.hwnd, dc);
	}

	HRESULT STDMETHODCALLTYPE GetHWnd(IDirectDrawClipper* This, HWND* lphWnd)
	{
		if (lphWnd)
		{
			auto it = g_clipperData.find(This);
			if (it != g_clipperData.end())
			{
				*lphWnd = it->second.hwnd;
				return DD_OK;
			}
		}
		return DDraw::DirectDrawClipper::s_origVtable.GetHWnd(This, lphWnd);
	}
	
	ULONG STDMETHODCALLTYPE Release(IDirectDrawClipper* This)
	{
		ULONG result = DDraw::DirectDrawClipper::s_origVtable.Release(This);
		if (0 == result)
		{
			g_clipperData.erase(This);
		}
		return result;
	}

	HRESULT STDMETHODCALLTYPE SetClipList(IDirectDrawClipper* This, LPRGNDATA lpClipList, DWORD dwFlags)
	{
		if (g_clipperData.find(This) != g_clipperData.end())
		{
			return DDERR_CLIPPERISUSINGHWND;
		}
		return DDraw::DirectDrawClipper::s_origVtable.SetClipList(This, lpClipList, dwFlags);
	}

	HRESULT STDMETHODCALLTYPE SetHWnd(IDirectDrawClipper* This, DWORD dwFlags, HWND hWnd)
	{
		const auto& origVtable = DDraw::DirectDrawClipper::s_origVtable;

		HRESULT result = origVtable.SetHWnd(This, dwFlags, hWnd);
		if (SUCCEEDED(result))
		{
			auto it = g_clipperData.find(This);
			if (hWnd)
			{
				if (it == g_clipperData.end())
				{
					it = g_clipperData.insert({ This, ClipperData() }).first;
					it->second.hwnd = hWnd;

					DWORD size = 0;
					origVtable.GetClipList(This, nullptr, nullptr, &size);
					it->second.oldClipList.resize(size);
					origVtable.GetClipList(This, nullptr,
						reinterpret_cast<RGNDATA*>(it->second.oldClipList.data()), &size);
				}
				updateWindowClipList(*This, it->second);
				Gdi::watchWindowPosChanges(&onWindowPosChange);
			}
			else if (it != g_clipperData.end())
			{
				origVtable.SetClipList(This, it->second.oldClipList.empty() ? nullptr :
					reinterpret_cast<RGNDATA*>(it->second.oldClipList.data()), 0);
				g_clipperData.erase(it);
			}
		}
		return result;
	}
}

namespace DDraw
{
	void DirectDrawClipper::setCompatVtable(IDirectDrawClipperVtbl& vtable)
	{
		vtable.GetHWnd = &GetHWnd;
		vtable.Release = &Release;
		vtable.SetClipList = &SetClipList;
		vtable.SetHWnd = &SetHWnd;
	}
}
