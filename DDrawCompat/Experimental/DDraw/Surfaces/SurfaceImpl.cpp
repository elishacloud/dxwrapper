#include <set>

#include "Common/CompatRef.h"
#include "DDraw/Repository.h"
#include "DDraw/Surfaces/Surface.h"
#include "DDraw/Surfaces/SurfaceImpl.h"

namespace
{
	struct DirectDrawInterface
	{
		const void* vtable;
		void* ddObject;
		DirectDrawInterface* next;
		DWORD refCount;
		DWORD unknown1;
		DWORD unknown2;
	};

	template <typename TSurface>
	void copyColorKey(CompatRef<TSurface> dst, CompatRef<TSurface> src, DWORD ckFlag)
	{
		DDCOLORKEY ck = {};
		if (SUCCEEDED(src->GetColorKey(&src, ckFlag, &ck)))
		{
			dst->SetColorKey(&dst, ckFlag, &ck);
		}
	}
}

namespace DDraw
{
	template <typename TSurface>
	SurfaceImpl<TSurface>::~SurfaceImpl()
	{
	}

	template <typename TSurface>
	bool SurfaceImpl<TSurface>::bltRetry(TSurface*& dstSurface, RECT*& dstRect,
		TSurface*& srcSurface, RECT*& srcRect, bool isTransparentBlt,
		const std::function<HRESULT()>& blt)
	{
		if (!dstSurface || !srcSurface)
		{
			return false;
		}

		TSurfaceDesc dstDesc = {};
		dstDesc.dwSize = sizeof(dstDesc);
		s_origVtable.GetSurfaceDesc(dstSurface, &dstDesc);

		TSurfaceDesc srcDesc = {};
		srcDesc.dwSize = sizeof(srcDesc);
		s_origVtable.GetSurfaceDesc(srcSurface, &srcDesc);

		if ((dstDesc.ddpfPixelFormat.dwFlags & DDPF_FOURCC) &&
			(dstDesc.ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY) &&
			(srcDesc.ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY))
		{
			const bool isCopyNeeded = true;
			return prepareBltRetrySurface(srcSurface, srcRect, srcDesc, isTransparentBlt, isCopyNeeded) &&
				SUCCEEDED(blt());
		}
		else if ((srcDesc.ddpfPixelFormat.dwFlags & DDPF_FOURCC) &&
			(srcDesc.ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY) &&
			(dstDesc.ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY))
		{
			TSurface* origDstSurface = dstSurface;
			RECT* origDstRect = dstRect;
			const bool isCopyNeeded = isTransparentBlt;
			return prepareBltRetrySurface(dstSurface, dstRect, dstDesc, isTransparentBlt, isCopyNeeded) &&
				SUCCEEDED(blt()) &&
				SUCCEEDED(s_origVtable.Blt(
					origDstSurface, origDstRect, dstSurface, dstRect, DDBLT_WAIT, nullptr));
		}

		return false;
	}

	template <typename TSurface>
	bool SurfaceImpl<TSurface>::prepareBltRetrySurface(TSurface*& surface, RECT*& rect,
		const TSurfaceDesc& desc, bool isTransparentBlt, bool isCopyNeeded)
	{
		TSurface* replSurface = surface;
		RECT* replRect = rect;
		replaceWithVidMemSurface(replSurface, replRect, desc);
		if (replSurface == surface)
		{
			return false;
		}

		if (isCopyNeeded && FAILED(s_origVtable.Blt(
			replSurface, replRect, surface, rect, DDBLT_WAIT, nullptr)))
		{
			return false;
		}

		if (isTransparentBlt)
		{
			copyColorKey<TSurface>(*replSurface, *surface, DDCKEY_SRCBLT);
			copyColorKey<TSurface>(*replSurface, *surface, DDCKEY_DESTBLT);
		}
		surface = replSurface;
		rect = replRect;
		return true;
	}

	template <typename TSurface>
	void SurfaceImpl<TSurface>::replaceWithVidMemSurface(TSurface*& surface, RECT*& rect,
		const TSurfaceDesc& desc)
	{
		static RECT replRect = {};
		replRect = rect ? RECT{ 0, 0, rect->right - rect->left, rect->bottom - rect->top } :
			RECT{ 0, 0, static_cast<LONG>(desc.dwWidth), static_cast<LONG>(desc.dwHeight) };
			
		DDSURFACEDESC2 replDesc = {};
		replDesc.dwSize = sizeof(replDesc);
		replDesc.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT | DDSD_CAPS;
		replDesc.dwWidth = replRect.right;
		replDesc.dwHeight = replRect.bottom;
		replDesc.ddpfPixelFormat = desc.ddpfPixelFormat;
		replDesc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_VIDEOMEMORY;

		CompatPtr<IUnknown> ddUnk;
		GetDDInterface(surface, reinterpret_cast<void**>(&ddUnk.getRef()));
		CompatPtr<IDirectDraw7> dd;
		ddUnk->QueryInterface(ddUnk, IID_IDirectDraw7, reinterpret_cast<void**>(&dd.getRef()));

		DDraw::Repository::ScopedSurface replacementSurface(*dd, replDesc);
		if (replacementSurface.surface)
		{
			surface = CompatPtr<TSurface>::from(replacementSurface.surface.get());
			rect = &replRect;
		}
	}

	template <typename TSurface>
	void SurfaceImpl<TSurface>::undoFlip(TSurface* This, TSurface* targetOverride)
	{
		if (targetOverride)
		{
			SurfaceImpl::Flip(This, targetOverride, DDFLIP_WAIT);
		}
		else
		{
			TSurfaceDesc desc = {};
			desc.dwSize = sizeof(desc);
			s_origVtable.GetSurfaceDesc(This, &desc);

			for (DWORD i = 0; i < desc.dwBackBufferCount; ++i)
			{
				SurfaceImpl::Flip(This, nullptr, DDFLIP_WAIT);
			}
		}
	}

	template <typename TSurface>
	HRESULT SurfaceImpl<TSurface>::Blt(
		TSurface* This, LPRECT lpDestRect, TSurface* lpDDSrcSurface, LPRECT lpSrcRect,
		DWORD dwFlags, LPDDBLTFX lpDDBltFx)
	{
		HRESULT result = s_origVtable.Blt(This, lpDestRect, lpDDSrcSurface, lpSrcRect, dwFlags, lpDDBltFx);
		if (DDERR_UNSUPPORTED == result || DDERR_GENERIC == result)
		{
			const bool isTransparentBlt = 0 !=
				(dwFlags & (DDBLT_KEYDEST | DDBLT_KEYSRC | DDBLT_KEYDESTOVERRIDE | DDBLT_KEYSRCOVERRIDE));
			if (bltRetry(This, lpDestRect, lpDDSrcSurface, lpSrcRect, isTransparentBlt,
				[&]() { return s_origVtable.Blt(
					This, lpDestRect, lpDDSrcSurface, lpSrcRect, dwFlags, lpDDBltFx); }))
			{
				return DD_OK;
			}
		}
		return result;
	}

	template <typename TSurface>
	HRESULT SurfaceImpl<TSurface>::BltFast(
		TSurface* This, DWORD dwX, DWORD dwY, TSurface* lpDDSrcSurface, LPRECT lpSrcRect, DWORD dwTrans)
	{
		HRESULT result = s_origVtable.BltFast(This, dwX, dwY, lpDDSrcSurface, lpSrcRect, dwTrans);
		if (DDERR_UNSUPPORTED == result || DDERR_GENERIC == result)
		{
			RECT dstRect = { static_cast<LONG>(dwX), static_cast<LONG>(dwY) };
			if (lpSrcRect)
			{
				dstRect.right = dwX + lpSrcRect->right - lpSrcRect->left;
				dstRect.bottom = dwY + lpSrcRect->bottom - lpSrcRect->top;
			}
			else
			{
				TSurfaceDesc desc = {};
				desc.dwSize = sizeof(desc);
				s_origVtable.GetSurfaceDesc(lpDDSrcSurface, &desc);

				dstRect.right = dwX + desc.dwWidth;
				dstRect.bottom = dwY + desc.dwHeight;
			}

			RECT* dstRectPtr = &dstRect;
			const bool isTransparentBlt = 0 != (dwTrans & (DDBLTFAST_DESTCOLORKEY | DDBLTFAST_SRCCOLORKEY));
			if (bltRetry(This, dstRectPtr, lpDDSrcSurface, lpSrcRect, isTransparentBlt,
				[&]() { return s_origVtable.BltFast(
					This, dstRectPtr->left, dstRectPtr->top, lpDDSrcSurface, lpSrcRect, dwTrans); }))
			{
				return DD_OK;
			}
		}
		return result;
	}

	template <typename TSurface>
	HRESULT SurfaceImpl<TSurface>::Flip(TSurface* This, TSurface* lpDDSurfaceTargetOverride, DWORD dwFlags)
	{
		return s_origVtable.Flip(This, lpDDSurfaceTargetOverride, dwFlags);
	}

	template <typename TSurface>
	HRESULT SurfaceImpl<TSurface>::GetCaps(TSurface* This, TDdsCaps* lpDDSCaps)
	{
		return s_origVtable.GetCaps(This, lpDDSCaps);
	}
	
	template <typename TSurface>
	HRESULT SurfaceImpl2<TSurface>::GetDDInterface(TSurface* /*This*/, LPVOID* lplpDD)
	{
		DirectDrawInterface dd = {};
		dd.vtable = IID_IDirectDraw7 == m_data->m_ddId
			? static_cast<const void*>(CompatVtable<IDirectDrawVtbl>::s_origVtablePtr)
			: static_cast<const void*>(CompatVtable<IDirectDraw7Vtbl>::s_origVtablePtr);
		dd.ddObject = m_data->m_ddObject;
		return CompatVtable<IDirectDrawVtbl>::s_origVtable.QueryInterface(
			reinterpret_cast<IDirectDraw*>(&dd), m_data->m_ddId, lplpDD);
	}

	template <typename TSurface>
	HRESULT SurfaceImpl<TSurface>::GetSurfaceDesc(TSurface* This, TSurfaceDesc* lpDDSurfaceDesc)
	{
		return s_origVtable.GetSurfaceDesc(This, lpDDSurfaceDesc);
	}

	template <typename TSurface>
	HRESULT SurfaceImpl<TSurface>::IsLost(TSurface* This)
	{
		return s_origVtable.IsLost(This);
	}

	template <typename TSurface>
	HRESULT SurfaceImpl<TSurface>::Lock(
		TSurface* This, LPRECT lpDestRect, TSurfaceDesc* lpDDSurfaceDesc,
		DWORD dwFlags, HANDLE hEvent)
	{
		HRESULT result = s_origVtable.Lock(This, lpDestRect, lpDDSurfaceDesc, dwFlags, hEvent);
		if (DDERR_SURFACELOST == result)
		{
			TSurfaceDesc desc = {};
			desc.dwSize = sizeof(desc);
			if (SUCCEEDED(s_origVtable.GetSurfaceDesc(This, &desc)) && !(desc.dwFlags & DDSD_HEIGHT))
			{
				// Fixes missing handling for lost vertex buffers in Messiah
				s_origVtable.Restore(This);
				// Still, pass back DDERR_SURFACELOST to the application in case it handles it
			}
		}

		return result;
	}

	template <typename TSurface>
	HRESULT SurfaceImpl<TSurface>::ReleaseDC(TSurface* This, HDC hDC)
	{
		return s_origVtable.ReleaseDC(This, hDC);
	}

	template <typename TSurface>
	HRESULT SurfaceImpl<TSurface>::Restore(TSurface* This)
	{
		return s_origVtable.Restore(This);
	}

	template <typename TSurface>
	HRESULT SurfaceImpl<TSurface>::SetPalette(TSurface* This, LPDIRECTDRAWPALETTE lpDDPalette)
	{
		return s_origVtable.SetPalette(This, lpDDPalette);
	}

	template <typename TSurface>
	HRESULT SurfaceImpl<TSurface>::Unlock(TSurface* This, TUnlockParam lpRect)
	{
		return s_origVtable.Unlock(This, lpRect);
	}

	template <typename TSurface>
	const Vtable<TSurface>& SurfaceImpl<TSurface>::s_origVtable =
		CompatVtable<Vtable<TSurface>>::s_origVtable;

	template SurfaceImpl<IDirectDrawSurface>;
	template SurfaceImpl<IDirectDrawSurface2>;
	template SurfaceImpl<IDirectDrawSurface3>;
	template SurfaceImpl<IDirectDrawSurface4>;
	template SurfaceImpl<IDirectDrawSurface7>;
}
