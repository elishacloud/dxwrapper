#define WIN32_LEAN_AND_MEAN
#define CINTERFACE

#include <map>
#include <vector>

#include <DDrawCompat/v0.3.1/Common/CompatRef.h>
#include <DDrawCompat/v0.3.1/Common/CompatVtable.h>
#include <DDrawCompat/v0.3.1/D3dDdi/KernelModeThunks.h>
#include <DDrawCompat/v0.3.1/DDraw/DirectDrawClipper.h>
#include <DDrawCompat/v0.3.1/DDraw/ScopedThreadLock.h>
#include <DDrawCompat/v0.3.1/DDraw/Surfaces/Surface.h>
#include <DDrawCompat/v0.3.1/DDraw/Visitors/DirectDrawClipperVtblVisitor.h>
#include <DDrawCompat/v0.3.1/Gdi/Gdi.h>
#include <DDrawCompat/v0.3.1/Gdi/Region.h>

namespace
{
	struct ClipperData
	{
		HWND hwnd;
		DWORD refCount;
		std::vector<unsigned char> origClipList;
	};

	std::map<IDirectDrawClipper*, ClipperData> g_clipperData;
	std::map<DDraw::Surface*, std::map<IDirectDrawClipper*, ClipperData>::iterator> g_surfaceToClipperData;
	bool g_isInvalidated = false;

	void updateWindowClipList(CompatRef<IDirectDrawClipper> clipper, ClipperData& data);

	void onWindowPosChange()
	{
		g_isInvalidated = true;
	}

	void restoreOrigClipList(IDirectDrawClipper* clipper, ClipperData& clipperData)
	{
		getOrigVtable(clipper).SetClipList(clipper,
			clipperData.origClipList.empty() ? nullptr : reinterpret_cast<RGNDATA*>(clipperData.origClipList.data()), 0);
		clipperData.origClipList.clear();
	}

	void updateWindowClipList(CompatRef<IDirectDrawClipper> clipper, ClipperData& data)
	{
		HDC dc = GetDCEx(data.hwnd, nullptr, DCX_CACHE | DCX_USESTYLE);
		Gdi::Region rgn;
		GetRandomRgn(dc, rgn, SYSRGN);
		CALL_ORIG_FUNC(ReleaseDC)(data.hwnd, dc);

		RECT primaryRect = D3dDdi::KernelModeThunks::getMonitorRect();
		if (0 != primaryRect.left || 0 != primaryRect.top)
		{
			rgn.offset(-primaryRect.left, -primaryRect.top);
		}

		DWORD rgnSize = GetRegionData(rgn, 0, nullptr);
		std::vector<unsigned char> rgnData(rgnSize);
		GetRegionData(rgn, rgnSize, reinterpret_cast<RGNDATA*>(rgnData.data()));

		clipper->SetHWnd(&clipper, 0, nullptr);
		clipper->SetClipList(&clipper, rgnData.empty() ? nullptr : reinterpret_cast<RGNDATA*>(rgnData.data()), 0);
	}

	HRESULT STDMETHODCALLTYPE GetHWnd(IDirectDrawClipper* This, HWND* lphWnd)
	{
		if (lphWnd)
		{
			auto it = g_clipperData.find(This);
			if (it != g_clipperData.end() && it->second.hwnd)
			{
				*lphWnd = it->second.hwnd;
				return DD_OK;
			}
		}
		return getOrigVtable(This).GetHWnd(This, lphWnd);
	}

	HRESULT STDMETHODCALLTYPE SetClipList(IDirectDrawClipper* This, LPRGNDATA lpClipList, DWORD dwFlags)
	{
		auto it = g_clipperData.find(This);
		if (it != g_clipperData.end() && it->second.hwnd)
		{
			return DDERR_CLIPPERISUSINGHWND;
		}
		return getOrigVtable(This).SetClipList(This, lpClipList, dwFlags);
	}

	HRESULT STDMETHODCALLTYPE SetHWnd(IDirectDrawClipper* This, DWORD dwFlags, HWND hWnd)
	{
		auto it = g_clipperData.find(This);
		if (it == g_clipperData.end())
		{
			return getOrigVtable(This).SetHWnd(This, dwFlags, hWnd);
		}

		std::vector<unsigned char> origClipList;
		if (hWnd && !it->second.hwnd)
		{
			DWORD size = 0;
			getOrigVtable(This).GetClipList(This, nullptr, nullptr, &size);
			origClipList.resize(size);
			getOrigVtable(This).GetClipList(This, nullptr, reinterpret_cast<RGNDATA*>(origClipList.data()), &size);
		}

		HRESULT result = getOrigVtable(This).SetHWnd(This, dwFlags, hWnd);
		if (SUCCEEDED(result))
		{
			if (hWnd)
			{
				if (!it->second.hwnd)
				{
					it->second.origClipList = origClipList;
				}
				it->second.hwnd = hWnd;
				updateWindowClipList(*This, it->second);
				Gdi::watchWindowPosChanges(&onWindowPosChange);
			}
			else if (it->second.hwnd)
			{
				restoreOrigClipList(it->first, it->second);
				it->second.hwnd = nullptr;
			}
		}
		return result;
	}

	constexpr void setCompatVtable(IDirectDrawClipperVtbl& vtable)
	{
		vtable.GetHWnd = &GetHWnd;
		vtable.SetClipList = &SetClipList;
		vtable.SetHWnd = &SetHWnd;
	}
}

namespace DDraw
{
	namespace DirectDrawClipper
	{
		HRGN getClipRgn(CompatRef<IDirectDrawClipper> clipper)
		{
			std::vector<unsigned char> rgnData;
			DWORD size = 0;
			clipper->GetClipList(&clipper, nullptr, nullptr, &size);
			rgnData.resize(size);
			clipper->GetClipList(&clipper, nullptr, reinterpret_cast<RGNDATA*>(rgnData.data()), &size);
			return ExtCreateRegion(nullptr, size, reinterpret_cast<RGNDATA*>(rgnData.data()));
		}

		void setClipper(Surface& surface, IDirectDrawClipper* clipper)
		{
			auto it = g_surfaceToClipperData.find(&surface);
			if (it != g_surfaceToClipperData.end())
			{
				auto& [prevClipper, prevClipperData] = *it->second;
				if (prevClipper == clipper)
				{
					return;
				}

				--prevClipperData.refCount;
				if (0 == prevClipperData.refCount)
				{
					if (prevClipperData.hwnd)
					{
						restoreOrigClipList(prevClipper, prevClipperData);
						getOrigVtable(prevClipper).SetHWnd(prevClipper, 0, prevClipperData.hwnd);
					}
					g_clipperData.erase(it->second);
				}
				g_surfaceToClipperData.erase(it);
			}

			if (clipper)
			{
				auto [clipperDataIter, inserted] = g_clipperData.insert({ clipper, ClipperData{} });
				if (inserted)
				{
					HWND hwnd = nullptr;
					getOrigVtable(clipper).GetHWnd(clipper, &hwnd);
					if (hwnd)
					{
						SetHWnd(clipper, 0, hwnd);
					}
				}
				++clipperDataIter->second.refCount;
				g_surfaceToClipperData[&surface] = clipperDataIter;
			}
		}

		HRESULT setClipRgn(CompatRef<IDirectDrawClipper> clipper, HRGN rgn)
		{
			std::vector<unsigned char> rgnData;
			rgnData.resize(GetRegionData(rgn, 0, nullptr));
			GetRegionData(rgn, rgnData.size(), reinterpret_cast<RGNDATA*>(rgnData.data()));
			return clipper->SetClipList(&clipper, reinterpret_cast<RGNDATA*>(rgnData.data()), 0);
		}

		void update()
		{
			if (g_isInvalidated)
			{
				g_isInvalidated = false;
				for (auto& clipperData : g_clipperData)
				{
					updateWindowClipList(*clipperData.first, clipperData.second);
				}
			}
		}

		void hookVtable(const IDirectDrawClipperVtbl& vtable)
		{
			CompatVtable<IDirectDrawClipperVtbl>::hookVtable<ScopedThreadLock>(vtable);
		}
	}
}
