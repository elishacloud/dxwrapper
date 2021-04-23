#define WIN32_LEAN_AND_MEAN
#define CINTERFACE

#include <set>

#include <DDrawCompat/v0.3.0/Common/CompatPtr.h>
#include <DDrawCompat/v0.3.0/DDraw/DirectDrawClipper.h>
#include <DDrawCompat/v0.3.0/DDraw/DirectDrawSurface.h>
#include <DDrawCompat/v0.3.0/DDraw/RealPrimarySurface.h>
#include <DDrawCompat/v0.3.0/DDraw/Surfaces/PrimarySurface.h>
#include <DDrawCompat/v0.3.0/DDraw/Surfaces/Surface.h>
#include <DDrawCompat/v0.3.0/DDraw/Surfaces/SurfaceImpl.h>
#include <DDrawCompat/v0.3.0/Dll/Dll.h>

namespace
{
	void* getSurface7VtablePtr(IUnknown* surface)
	{
		static void* vtable = CompatPtr<IDirectDrawSurface7>::from(surface).get()->lpVtbl;
		return vtable;
	}
}

namespace DDraw
{
	template <typename TSurface>
	SurfaceImpl<TSurface>::SurfaceImpl(Surface* data)
		: m_data(data)
	{
	}

	template <typename TSurface>
	SurfaceImpl<TSurface>::~SurfaceImpl()
	{
	}

	template <typename TSurface>
	HRESULT SurfaceImpl<TSurface>::Blt(
		TSurface* This, LPRECT lpDestRect, TSurface* lpDDSrcSurface, LPRECT lpSrcRect,
		DWORD dwFlags, LPDDBLTFX lpDDBltFx)
	{
		if (!waitForFlip(This, dwFlags, DDBLT_WAIT, DDBLT_DONOTWAIT))
		{
			return DDERR_WASSTILLDRAWING;
		}
		DirectDrawClipper::update();
		return getOrigVtable(This).Blt(This, lpDestRect, lpDDSrcSurface, lpSrcRect, dwFlags, lpDDBltFx);
	}

	template <typename TSurface>
	HRESULT SurfaceImpl<TSurface>::BltFast(
		TSurface* This, DWORD dwX, DWORD dwY, TSurface* lpDDSrcSurface, LPRECT lpSrcRect, DWORD dwTrans)
	{
		if (!waitForFlip(This, dwTrans, DDBLTFAST_WAIT, DDBLTFAST_DONOTWAIT))
		{
			return DDERR_WASSTILLDRAWING;
		}
		return getOrigVtable(This).BltFast(This, dwX, dwY, lpDDSrcSurface, lpSrcRect, dwTrans);
	}

	template <typename TSurface>
	HRESULT SurfaceImpl<TSurface>::Flip(TSurface* This, TSurface* lpDDSurfaceTargetOverride, DWORD dwFlags)
	{
		return getOrigVtable(This).Flip(This, lpDDSurfaceTargetOverride, dwFlags);
	}

	template <typename TSurface>
	HRESULT SurfaceImpl<TSurface>::GetBltStatus(TSurface* This, DWORD dwFlags)
	{
		HRESULT result = getOrigVtable(This).GetBltStatus(This, dwFlags);
		if (SUCCEEDED(result) && (dwFlags & DDGBS_CANBLT))
		{
			const bool wait = false;
			if (!RealPrimarySurface::waitForFlip(m_data, wait))
			{
				return DDERR_WASSTILLDRAWING;
			}
		}
		return result;
	}

	template <typename TSurface>
	HRESULT SurfaceImpl<TSurface>::GetCaps(TSurface* This, TDdsCaps* lpDDSCaps)
	{
		HRESULT result = getOrigVtable(This).GetCaps(This, lpDDSCaps);
		if (SUCCEEDED(result))
		{
			restoreOrigCaps(lpDDSCaps->dwCaps);
		}
		return result;
	}

	template <typename TSurface>
	HRESULT SurfaceImpl<TSurface>::GetDC(TSurface* This, HDC* lphDC)
	{
		HRESULT result = getOrigVtable(This).GetDC(This, lphDC);
		if (SUCCEEDED(result))
		{
			RealPrimarySurface::waitForFlip(m_data);
			Dll::g_origProcs.ReleaseDDThreadLock();
		}
		return result;
	}

	template <typename TSurface>
	HRESULT SurfaceImpl<TSurface>::GetFlipStatus(TSurface* This, DWORD dwFlags)
	{
		HRESULT result = getOrigVtable(This).GetFlipStatus(This, dwFlags);
		if (SUCCEEDED(result))
		{
			const bool wait = false;
			if (!RealPrimarySurface::waitForFlip(m_data, wait))
			{
				return DDERR_WASSTILLDRAWING;
			}
		}
		return result;
	}

	template <typename TSurface>
	HRESULT SurfaceImpl<TSurface>::GetSurfaceDesc(TSurface* This, TSurfaceDesc* lpDDSurfaceDesc)
	{
		HRESULT result = getOrigVtable(This).GetSurfaceDesc(This, lpDDSurfaceDesc);
		if (SUCCEEDED(result))
		{
			if (0 != m_data->m_sizeOverride.cx)
			{
				lpDDSurfaceDesc->dwWidth = m_data->m_sizeOverride.cx;
				lpDDSurfaceDesc->dwHeight = m_data->m_sizeOverride.cy;
				m_data->m_sizeOverride = {};
			}
			restoreOrigCaps(lpDDSurfaceDesc->ddsCaps.dwCaps);
		}
		return result;
	}

	template <typename TSurface>
	HRESULT SurfaceImpl<TSurface>::IsLost(TSurface* This)
	{
		return getOrigVtable(This).IsLost(This);
	}

	template <typename TSurface>
	HRESULT SurfaceImpl<TSurface>::Lock(
		TSurface* This, LPRECT lpDestRect, TSurfaceDesc* lpDDSurfaceDesc,
		DWORD dwFlags, HANDLE hEvent)
	{
		if (!waitForFlip(This, dwFlags, DDLOCK_WAIT, DDLOCK_DONOTWAIT))
		{
			return DDERR_WASSTILLDRAWING;
		}

		HRESULT result = getOrigVtable(This).Lock(This, lpDestRect, lpDDSurfaceDesc, dwFlags, hEvent);
		if (SUCCEEDED(result))
		{
			restoreOrigCaps(lpDDSurfaceDesc->ddsCaps.dwCaps);
		}
		else if (DDERR_SURFACELOST == result)
		{
			TSurfaceDesc desc = {};
			desc.dwSize = sizeof(desc);
			if (SUCCEEDED(getOrigVtable(This).GetSurfaceDesc(This, &desc)) && !(desc.dwFlags & DDSD_HEIGHT))
			{
				// Fixes missing handling for lost vertex buffers in Messiah
				getOrigVtable(This).Restore(This);
				// Still, pass back DDERR_SURFACELOST to the application in case it handles it
			}
		}

		return result;
	}

	template <typename TSurface>
	HRESULT SurfaceImpl<TSurface>::QueryInterface(TSurface* This, REFIID riid, LPVOID* obp)
	{
		auto iid = (IID_IDirect3DRampDevice == riid) ? &IID_IDirect3DRGBDevice : &riid;
		HRESULT result = getOrigVtable(This).QueryInterface(This, *iid, obp);
		if (DDERR_INVALIDOBJECT == result)
		{
			m_data->setSizeOverride(1, 1);
			result = getOrigVtable(This).QueryInterface(This, *iid, obp);
			m_data->setSizeOverride(0, 0);
		}
		return result;
	}

	template <typename TSurface>
	HRESULT SurfaceImpl<TSurface>::ReleaseDC(TSurface* This, HDC hDC)
	{
		HRESULT result = getOrigVtable(This).ReleaseDC(This, hDC);
		if (SUCCEEDED(result))
		{
			Dll::g_origProcs.AcquireDDThreadLock();
		}
		return result;
	}

	template <typename TSurface>
	HRESULT SurfaceImpl<TSurface>::Restore(TSurface* This)
	{
		HRESULT result = getOrigVtable(This).Restore(This);
		if (SUCCEEDED(result))
		{
			m_data->restore();
		}
		return result;
	}

	template <typename TSurface>
	HRESULT SurfaceImpl<TSurface>::SetPalette(TSurface* This, LPDIRECTDRAWPALETTE lpDDPalette)
	{
		return getOrigVtable(This).SetPalette(This, lpDDPalette);
	}

	template <typename TSurface>
	HRESULT SurfaceImpl<TSurface>::Unlock(TSurface* This, TUnlockParam lpRect)
	{
		return getOrigVtable(This).Unlock(This, lpRect);
	}

	template <typename TSurface>
	void SurfaceImpl<TSurface>::restoreOrigCaps(DWORD& caps)
	{
		if (m_data->m_origCaps & DDSCAPS_3DDEVICE)
		{
			caps |= DDSCAPS_3DDEVICE;
		}
	}

	template <typename TSurface>
	bool SurfaceImpl<TSurface>::waitForFlip(TSurface* This, DWORD flags, DWORD waitFlag, DWORD doNotWaitFlag)
	{
		const bool wait = (flags & waitFlag) || !(flags & doNotWaitFlag) &&
			getSurface7VtablePtr(reinterpret_cast<IUnknown*>(This)) == This->lpVtbl;
		return DDraw::RealPrimarySurface::waitForFlip(m_data, wait);
	}

	template SurfaceImpl<IDirectDrawSurface>;
	template SurfaceImpl<IDirectDrawSurface2>;
	template SurfaceImpl<IDirectDrawSurface3>;
	template SurfaceImpl<IDirectDrawSurface4>;
	template SurfaceImpl<IDirectDrawSurface7>;
}
