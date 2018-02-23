#include "DDraw/DirectDrawPalette.h"
#include "DDraw/RealPrimarySurface.h"
#include "DDraw/Surfaces/PrimarySurface.h"
#include "DDraw/Surfaces/PrimarySurfaceImpl.h"
#include "Gdi/Gdi.h"

namespace
{
	void restorePrimaryCaps(DWORD& caps)
	{
		caps &= ~DDSCAPS_OFFSCREENPLAIN;
		caps |= DDSCAPS_PRIMARYSURFACE | DDSCAPS_VISIBLE;

		if ((caps & DDSCAPS_SYSTEMMEMORY) &&
			!(DDraw::PrimarySurface::getOrigCaps() & DDSCAPS_SYSTEMMEMORY))
		{
			caps &= ~DDSCAPS_SYSTEMMEMORY;
			caps |= DDSCAPS_VIDEOMEMORY | DDSCAPS_LOCALVIDMEM;
		}
	}
}

namespace DDraw
{
	template <typename TSurface>
	PrimarySurfaceImpl<TSurface>::PrimarySurfaceImpl(SurfaceImpl& impl) : m_impl(impl)
	{
	}

	template <typename TSurface>
	HRESULT PrimarySurfaceImpl<TSurface>::Blt(
		TSurface* This, LPRECT lpDestRect, TSurface* lpDDSrcSurface, LPRECT lpSrcRect,
		DWORD dwFlags, LPDDBLTFX lpDDBltFx)
	{
		if (RealPrimarySurface::isLost())
		{
			return DDERR_SURFACELOST;
		}

		HRESULT result = m_impl.Blt(This, lpDestRect, lpDDSrcSurface, lpSrcRect, dwFlags, lpDDBltFx);
		if (SUCCEEDED(result))
		{
			RealPrimarySurface::update();
		}
		return result;
	}

	template <typename TSurface>
	HRESULT PrimarySurfaceImpl<TSurface>::BltFast(
		TSurface* This, DWORD dwX, DWORD dwY, TSurface* lpDDSrcSurface, LPRECT lpSrcRect, DWORD dwTrans)
	{
		if (RealPrimarySurface::isLost())
		{
			return DDERR_SURFACELOST;
		}

		HRESULT result = m_impl.BltFast(This, dwX, dwY, lpDDSrcSurface, lpSrcRect, dwTrans);
		if (SUCCEEDED(result))
		{
			const LONG x = dwX;
			const LONG y = dwY;
			RECT destRect = { x, y, x, y };
			if (lpSrcRect)
			{
				destRect.right += lpSrcRect->right - lpSrcRect->left;
				destRect.bottom += lpSrcRect->bottom - lpSrcRect->top;
			}
			else
			{
				TSurfaceDesc desc = {};
				desc.dwSize = sizeof(desc);
				CompatVtable<Vtable<TSurface>>::s_origVtable.GetSurfaceDesc(lpDDSrcSurface, &desc);
				destRect.right += desc.dwWidth;
				destRect.bottom += desc.dwHeight;
			}
			RealPrimarySurface::update();
		}
		return result;
	}

	template <typename TSurface>
	HRESULT PrimarySurfaceImpl<TSurface>::Flip(TSurface* This, TSurface* lpDDSurfaceTargetOverride, DWORD dwFlags)
	{
		if (RealPrimarySurface::isLost())
		{
			return DDERR_SURFACELOST;
		}

		HRESULT result = m_impl.Flip(This, lpDDSurfaceTargetOverride, dwFlags);
		if (FAILED(result))
		{
			return result;
		}

		const bool isFlipEmulated = 0 != (PrimarySurface::getOrigCaps() & DDSCAPS_SYSTEMMEMORY);
		result = RealPrimarySurface::flip(dwFlags);
		if (SUCCEEDED(result) && !isFlipEmulated)
		{
			return DD_OK;
		}

		undoFlip(This, lpDDSurfaceTargetOverride);

		if (SUCCEEDED(result) && isFlipEmulated)
		{
			if (lpDDSurfaceTargetOverride)
			{
				s_origVtable.BltFast(This, 0, 0, lpDDSurfaceTargetOverride, nullptr, DDBLTFAST_WAIT);
			}
			else
			{
				TDdsCaps caps = {};
				caps.dwCaps = DDSCAPS_BACKBUFFER;
				CompatPtr<TSurface> backBuffer;
				s_origVtable.GetAttachedSurface(This, &caps, &backBuffer.getRef());

				s_origVtable.BltFast(This, 0, 0, backBuffer, nullptr, DDBLTFAST_WAIT);
			}
		}

		return result;
	}

	template <typename TSurface>
	HRESULT PrimarySurfaceImpl<TSurface>::GetCaps(TSurface* This, TDdsCaps* lpDDSCaps)
	{
		HRESULT result = m_impl.GetCaps(This, lpDDSCaps);
		if (SUCCEEDED(result))
		{
			restorePrimaryCaps(lpDDSCaps->dwCaps);
		}
		return result;
	}

	template <typename TSurface>
	HRESULT PrimarySurfaceImpl<TSurface>::GetSurfaceDesc(TSurface* This, TSurfaceDesc* lpDDSurfaceDesc)
	{
		HRESULT result = m_impl.GetSurfaceDesc(This, lpDDSurfaceDesc);
		if (SUCCEEDED(result))
		{
			restorePrimaryCaps(lpDDSurfaceDesc->ddsCaps.dwCaps);
		}
		return result;
	}

	template <typename TSurface>
	HRESULT PrimarySurfaceImpl<TSurface>::IsLost(TSurface* This)
	{
		HRESULT result = m_impl.IsLost(This);
		if (SUCCEEDED(result))
		{
			result = RealPrimarySurface::isLost() ? DDERR_SURFACELOST : DD_OK;
		}
		return result;
	}

	template <typename TSurface>
	HRESULT PrimarySurfaceImpl<TSurface>::Lock(
		TSurface* This, LPRECT lpDestRect, TSurfaceDesc* lpDDSurfaceDesc,
		DWORD dwFlags, HANDLE hEvent)
	{
		if (RealPrimarySurface::isLost())
		{
			return DDERR_SURFACELOST;
		}

		HRESULT result = m_impl.Lock(This, lpDestRect, lpDDSurfaceDesc, dwFlags, hEvent);
		if (SUCCEEDED(result))
		{
			restorePrimaryCaps(lpDDSurfaceDesc->ddsCaps.dwCaps);
		}
		return result;
	}

	template <typename TSurface>
	HRESULT PrimarySurfaceImpl<TSurface>::ReleaseDC(TSurface* This, HDC hDC)
	{
		HRESULT result = m_impl.ReleaseDC(This, hDC);
		if (SUCCEEDED(result))
		{
			RealPrimarySurface::update();
		}
		return result;
	}

	template <typename TSurface>
	HRESULT PrimarySurfaceImpl<TSurface>::Restore(TSurface* This)
	{
		HRESULT result = IsLost(This);
		if (FAILED(result))
		{
			result = RealPrimarySurface::restore();
			if (SUCCEEDED(result))
			{
				result = m_impl.Restore(This);
				if (SUCCEEDED(result))
				{
					Gdi::redraw(nullptr);
				}
			}
		}
		return result;
	}

	template <typename TSurface>
	HRESULT PrimarySurfaceImpl<TSurface>::SetPalette(TSurface* This, LPDIRECTDRAWPALETTE lpDDPalette)
	{
		if (lpDDPalette)
		{
			DirectDrawPalette::waitForNextUpdate();
		}
		if (lpDDPalette == PrimarySurface::s_palette)
		{
			return DD_OK;
		}

		HRESULT result = m_impl.SetPalette(This, lpDDPalette);
		if (SUCCEEDED(result))
		{
			PrimarySurface::s_palette = lpDDPalette;
			RealPrimarySurface::setPalette();
		}
		return result;
	}

	template <typename TSurface>
	HRESULT PrimarySurfaceImpl<TSurface>::Unlock(TSurface* This, TUnlockParam lpRect)
	{
		HRESULT result = m_impl.Unlock(This, lpRect);
		if (SUCCEEDED(result))
		{
			RealPrimarySurface::update();
		}
		return result;
	}

	template PrimarySurfaceImpl<IDirectDrawSurface>;
	template PrimarySurfaceImpl<IDirectDrawSurface2>;
	template PrimarySurfaceImpl<IDirectDrawSurface3>;
	template PrimarySurfaceImpl<IDirectDrawSurface4>;
	template PrimarySurfaceImpl<IDirectDrawSurface7>;
}
