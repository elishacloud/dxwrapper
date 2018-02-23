#include <algorithm>
#include <set>
#include <vector>

#include "CompatDirectDraw.h"
#include "CompatDirectDrawSurface.h"
#include "CompatGdi.h"
#include "CompatPrimarySurface.h"
#include "DDrawProcs.h"
#include "IReleaseNotifier.h"
#include "RealPrimarySurface.h"

namespace Compat20
{
	namespace
	{
		struct SimilarSurface
		{
			DWORD width;
			DWORD height;
			DDPIXELFORMAT pixelFormat;
			IDirectDrawSurface7* front;
			IDirectDrawSurface7* back;
		};

		SimilarSurface getSimilarSurface(const DDSURFACEDESC2& desc);
		bool mirrorBlt(IDirectDrawSurface7& dst, IDirectDrawSurface7& src, RECT srcRect, DWORD mirrorFx);

		IDirectDraw7* g_mirrorDirectDraw = nullptr;
		bool g_lockingPrimary = false;

		IDirectDraw7* createMirrorDirectDraw()
		{
			IDirectDraw7* dd = nullptr;
			CALL_ORIG_DDRAW(DirectDrawCreateEx, nullptr, reinterpret_cast<void**>(&dd), IID_IDirectDraw7, nullptr);
			if (!dd ||
				FAILED(CompatDirectDraw<IDirectDraw7>::s_origVtable.SetCooperativeLevel(dd, nullptr, DDSCL_NORMAL)))
			{
				Logging::Log() << "Failed to create a helper DirectDraw object for mirroring";
			}
			return dd;
		}

		void fixSurfacePtr(IDirectDrawSurface7& surface, const DDSURFACEDESC2& desc)
		{
			if ((desc.dwFlags & DDSD_CAPS) && (desc.ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY))
			{
				return;
			}

			DDSURFACEDESC2 tempSurfaceDesc = desc;
			tempSurfaceDesc.dwWidth = 1;
			tempSurfaceDesc.dwHeight = 1;
			SimilarSurface tempSurface = getSimilarSurface(desc);
			if (!tempSurface.front)
			{
				LOG_ONCE("Failed to fix a surface memory pointer");
				return;
			}

			RECT r = { 0, 0, 1, 1 };
			CompatDirectDrawSurface<IDirectDrawSurface7>::s_origVtable.Blt(
				&surface, &r, tempSurface.front, &r, DDBLT_WAIT, nullptr);
		}

		HRESULT WINAPI enumSurfacesCallback(
			LPDIRECTDRAWSURFACE7 lpDDSurface,
			LPDDSURFACEDESC2 lpDDSurfaceDesc,
			LPVOID lpContext)
		{
			auto& visitedSurfaces = *static_cast<std::set<IDirectDrawSurface7*>*>(lpContext);

			if (visitedSurfaces.find(lpDDSurface) == visitedSurfaces.end())
			{
				visitedSurfaces.insert(lpDDSurface);
				fixSurfacePtr(*lpDDSurface, *lpDDSurfaceDesc);
				CompatDirectDrawSurface<IDirectDrawSurface7>::s_origVtable.EnumAttachedSurfaces(
					lpDDSurface, lpContext, &enumSurfacesCallback);
			}

			CompatDirectDrawSurface<IDirectDrawSurface7>::s_origVtable.Release(lpDDSurface);
			return DDENUMRET_OK;
		}

		void fixSurfacePtrs(IDirectDrawSurface7& surface)
		{
			DDSURFACEDESC2 desc = {};
			desc.dwSize = sizeof(desc);
			CompatDirectDrawSurface<IDirectDrawSurface7>::s_origVtable.GetSurfaceDesc(&surface, &desc);

			fixSurfacePtr(surface, desc);
			std::set<IDirectDrawSurface7*> visitedSurfaces{ &surface };
			CompatDirectDrawSurface<IDirectDrawSurface7>::s_origVtable.EnumAttachedSurfaces(
				&surface, &visitedSurfaces, &enumSurfacesCallback);
		}

		IDirectDrawSurface7* getMirroredSurface(IDirectDrawSurface7& surface, RECT* srcRect, DWORD mirrorFx)
		{
			auto& origVtable = CompatDirectDrawSurface<IDirectDrawSurface7>::s_origVtable;

			DDSURFACEDESC2 desc = {};
			desc.dwSize = sizeof(desc);
			HRESULT result = origVtable.GetSurfaceDesc(&surface, &desc);
			if (FAILED(result))
			{
				LOG_ONCE("Failed to get surface description for mirroring: " << result);
				return nullptr;
			}

			if (srcRect)
			{
				desc.dwWidth = srcRect->right - srcRect->left;
				desc.dwHeight = srcRect->bottom - srcRect->top;
			}

			SimilarSurface similarSurface = getSimilarSurface(desc);
			if (!similarSurface.front)
			{
				return nullptr;
			}

			RECT rect = { 0, 0, static_cast<LONG>(desc.dwWidth), static_cast<LONG>(desc.dwHeight) };
			if ((mirrorFx & DDBLTFX_MIRRORLEFTRIGHT) && (mirrorFx & DDBLTFX_MIRRORUPDOWN))
			{
				if (!mirrorBlt(*similarSurface.back, surface, srcRect ? *srcRect : rect, DDBLTFX_MIRRORLEFTRIGHT) ||
					!mirrorBlt(*similarSurface.front, *similarSurface.back, rect, DDBLTFX_MIRRORUPDOWN))
				{
					return nullptr;
				}
			}
			else if (!mirrorBlt(*similarSurface.front, surface, srcRect ? *srcRect : rect, mirrorFx))
			{
				return nullptr;
			}

			origVtable.AddRef(similarSurface.front);
			return similarSurface.front;
		}

		template <typename TSurface>
		TSurface* getMirroredSurface(TSurface& surface, RECT* rect, DWORD mirrorFx)
		{
			auto& origVtable = CompatDirectDrawSurface<TSurface>::s_origVtable;

			IDirectDrawSurface7* surface7 = nullptr;
			origVtable.QueryInterface(&surface, IID_IDirectDrawSurface7, reinterpret_cast<void**>(&surface7));
			IDirectDrawSurface7* mirroredSurface7 = getMirroredSurface(*surface7, rect, mirrorFx);
			surface7->lpVtbl->Release(surface7);

			if (!mirroredSurface7)
			{
				return nullptr;
			}

			auto& origVtable7 = CompatDirectDrawSurface<IDirectDrawSurface7>::s_origVtable;
			TSurface* mirroredSurface = nullptr;
			origVtable7.QueryInterface(mirroredSurface7,
				CompatDirectDrawSurface<TSurface>::s_iid, reinterpret_cast<void**>(&mirroredSurface));
			origVtable7.Release(mirroredSurface7);
			return mirroredSurface;
		}

		SimilarSurface getSimilarSurface(const DDSURFACEDESC2& desc)
		{
			static std::vector<SimilarSurface> similarSurfacesVidMem;
			static std::vector<SimilarSurface> similarSurfacesSysMem;

			std::vector<SimilarSurface>& similarSurfaces =
				(desc.ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY) ? similarSurfacesSysMem : similarSurfacesVidMem;

			DDPIXELFORMAT pf = desc.ddpfPixelFormat;
			if (!(pf.dwFlags & DDPF_FOURCC))
			{
				pf.dwFourCC = 0;
			}
			if (!(pf.dwFlags & (DDPF_ALPHAPIXELS | DDPF_ZPIXELS)))
			{
				pf.dwRGBAlphaBitMask = 0;
			}
			auto it = std::find_if(similarSurfaces.begin(), similarSurfaces.end(),
				[&](SimilarSurface& s) { return 0 == memcmp(&s.pixelFormat, &pf, sizeof(pf)); });

			auto& origVtable = CompatDirectDrawSurface<IDirectDrawSurface7>::s_origVtable;
			SimilarSurface similarSurface = {};
			if (it != similarSurfaces.end())
			{
				if (DDERR_SURFACELOST == origVtable.IsLost(it->front) ||
					DDERR_SURFACELOST == origVtable.IsLost(it->back))
				{
					origVtable.Release(it->front);
					origVtable.Release(it->back);
					similarSurfaces.erase(it);
					it = similarSurfaces.end();
				}
				else
				{
					similarSurface = *it;
				}
			}

			if (similarSurface.width >= desc.dwWidth && similarSurface.height >= desc.dwHeight)
			{
				return similarSurface;
			}

			DDSURFACEDESC2 similarDesc = {};
			similarDesc.dwSize = sizeof(similarDesc);
			similarDesc.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT | DDSD_CAPS;
			similarDesc.dwWidth = max(similarSurface.width, desc.dwWidth);
			similarDesc.dwHeight = max(similarSurface.height, desc.dwHeight);
			similarDesc.ddpfPixelFormat = pf;
			similarDesc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
			if (desc.ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY)
			{
				similarDesc.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
			}

			HRESULT result = CompatDirectDraw<IDirectDraw7>::s_origVtable.CreateSurface(
				g_mirrorDirectDraw, &similarDesc, &similarSurface.front, nullptr);
			if (FAILED(result))
			{
				LOG_ONCE("Failed to create a similar front surface");
				similarSurface.front = nullptr;
				return similarSurface;
			}

			result = CompatDirectDraw<IDirectDraw7>::s_origVtable.CreateSurface(
				g_mirrorDirectDraw, &similarDesc, &similarSurface.back, nullptr);
			if (FAILED(result))
			{
				LOG_ONCE("Failed to create a similar back surface");
				origVtable.Release(similarSurface.front);
				similarSurface.front = nullptr;
				return similarSurface;
			}

			similarSurface.width = similarDesc.dwWidth;
			similarSurface.height = similarDesc.dwHeight;
			similarSurface.pixelFormat = pf;

			if (it != similarSurfaces.end())
			{
				origVtable.Release(it->front);
				origVtable.Release(it->back);
				*it = similarSurface;
			}
			else
			{
				similarSurfaces.push_back(similarSurface);
			}

			return similarSurface;
		}

		bool mirrorBlt(IDirectDrawSurface7& dst, IDirectDrawSurface7& src, RECT srcRect, DWORD mirrorFx)
		{
			if (DDBLTFX_MIRRORLEFTRIGHT == mirrorFx)
			{
				LONG width = srcRect.right - srcRect.left;
				srcRect.left = srcRect.right - 1;
				for (LONG x = 0; x < width; ++x)
				{
					HRESULT result = CompatDirectDrawSurface<IDirectDrawSurface7>::s_origVtable.BltFast(
						&dst, x, 0, &src, &srcRect, DDBLTFAST_WAIT);
					if (FAILED(result))
					{
						LOG_ONCE("Failed BltFast for mirroring: " << result);
						return false;
					}
					--srcRect.left;
					--srcRect.right;
				}
			}
			else
			{
				LONG height = srcRect.bottom - srcRect.top;
				srcRect.top = srcRect.bottom - 1;
				for (LONG y = 0; y < height; ++y)
				{
					HRESULT result = CompatDirectDrawSurface<IDirectDrawSurface7>::s_origVtable.BltFast(
						&dst, 0, y, &src, &srcRect, DDBLTFAST_WAIT);
					if (FAILED(result))
					{
						LOG_ONCE("Failed BltFast for mirroring: " << result);
						return false;
					}
					--srcRect.top;
					--srcRect.bottom;
				}
			}

			return true;
		}
	}

	template <typename TSurface>
	void CompatDirectDrawSurface<TSurface>::setCompatVtable(Vtable<TSurface>& vtable)
	{
		vtable.Blt = &Blt;
		vtable.BltFast = &BltFast;
		vtable.Flip = &Flip;
		vtable.GetCaps = &GetCaps;
		vtable.GetSurfaceDesc = &GetSurfaceDesc;
		vtable.IsLost = &IsLost;
		vtable.Lock = &Lock;
		vtable.QueryInterface = &QueryInterface;
		vtable.ReleaseDC = &ReleaseDC;
		vtable.Restore = &Restore;
		vtable.SetClipper = &SetClipper;
		vtable.SetPalette = &SetPalette;
		vtable.Unlock = &Unlock;
	}

	template <typename TSurface>
	template <typename TDirectDraw>
	HRESULT CompatDirectDrawSurface<TSurface>::createCompatPrimarySurface(
		TDirectDraw& dd,
		TSurfaceDesc compatDesc,
		TSurface*& compatSurface)
	{
		if (0 == CompatPrimarySurface::displayMode.pixelFormat.dwSize)
		{
			CompatPrimarySurface::displayMode = CompatPrimarySurface::getDisplayMode(dd);
		}

		HRESULT result = RealPrimarySurface::create(dd);
		if (FAILED(result))
		{
			return result;
		}

		CompatPrimarySurface::width = CompatPrimarySurface::displayMode.width;
		CompatPrimarySurface::height = CompatPrimarySurface::displayMode.height;
		CompatPrimarySurface::pixelFormat = CompatPrimarySurface::displayMode.pixelFormat;

		compatDesc.dwFlags |= DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;
		compatDesc.dwWidth = CompatPrimarySurface::width;
		compatDesc.dwHeight = CompatPrimarySurface::height;
		compatDesc.ddsCaps.dwCaps ^= DDSCAPS_PRIMARYSURFACE;
		compatDesc.ddsCaps.dwCaps |= DDSCAPS_OFFSCREENPLAIN;
		compatDesc.ddpfPixelFormat = CompatPrimarySurface::pixelFormat;

		result = CompatDirectDraw<TDirectDraw>::s_origVtable.CreateSurface(
			&dd, &compatDesc, &compatSurface, nullptr);
		if (FAILED(result))
		{
			Logging::Log() << "Failed to create the compat primary surface!";
			RealPrimarySurface::release();
			return result;
		}

		g_mirrorDirectDraw = createMirrorDirectDraw();

		s_compatPrimarySurface = compatSurface;
		initCompatPrimarySurface();
		return DD_OK;
	}

	template <typename TSurface>
	void CompatDirectDrawSurface<TSurface>::fixSurfacePtrs(TSurface& surface)
	{
		IDirectDrawSurface7* surface7 = nullptr;
		surface.lpVtbl->QueryInterface(&surface, IID_IDirectDrawSurface7, reinterpret_cast<LPVOID*>(&surface7));
		Compat20::fixSurfacePtrs(*surface7);
		surface7->lpVtbl->Release(surface7);
	}

	template <typename TSurface>
	void CompatDirectDrawSurface<TSurface>::initPrimarySurfacePtr(const GUID& guid, IUnknown& surface)
	{
		if (SUCCEEDED(surface.lpVtbl->QueryInterface(
			&surface, guid, reinterpret_cast<LPVOID*>(&s_compatPrimarySurface))))
		{
			s_compatPrimarySurface->lpVtbl->Release(s_compatPrimarySurface);
		}
	}

	template <typename TSurface>
	void CompatDirectDrawSurface<TSurface>::resetPrimarySurfacePtr()
	{
		s_compatPrimarySurface = nullptr;
	}

	template <typename TSurface>
	HRESULT STDMETHODCALLTYPE CompatDirectDrawSurface<TSurface>::Blt(
		TSurface* This,
		LPRECT lpDestRect,
		TSurface* lpDDSrcSurface,
		LPRECT lpSrcRect,
		DWORD dwFlags,
		LPDDBLTFX lpDDBltFx)
	{
		if ((This == s_compatPrimarySurface || lpDDSrcSurface == s_compatPrimarySurface) &&
			RealPrimarySurface::isLost())
		{
			return DDERR_SURFACELOST;
		}

		HRESULT result = DD_OK;
		TSurface* mirroredSrcSurface = nullptr;

		if (lpDDSrcSurface && (dwFlags & DDBLT_DDFX) && lpDDBltFx &&
			(lpDDBltFx->dwDDFX & (DDBLTFX_MIRRORLEFTRIGHT | DDBLTFX_MIRRORUPDOWN)))
		{
			mirroredSrcSurface = getMirroredSurface(*lpDDSrcSurface, lpSrcRect, lpDDBltFx->dwDDFX);
		}

		if (mirroredSrcSurface)
		{
			DWORD flags = dwFlags;
			DDBLTFX fx = *lpDDBltFx;
			fx.dwDDFX &= ~(DDBLTFX_MIRRORLEFTRIGHT | DDBLTFX_MIRRORUPDOWN);
			if (0 == fx.dwDDFX)
			{
				flags ^= DDBLT_DDFX;
			}
			if (flags & DDBLT_KEYSRC)
			{
				DDCOLORKEY srcColorKey = {};
				s_origVtable.GetColorKey(lpDDSrcSurface, DDCKEY_SRCBLT, &srcColorKey);
				s_origVtable.SetColorKey(mirroredSrcSurface, DDCKEY_SRCBLT, &srcColorKey);
			}

			if (lpSrcRect)
			{
				RECT srcRect = { 0, 0, lpSrcRect->right - lpSrcRect->left, lpSrcRect->bottom - lpSrcRect->top };
				result = s_origVtable.Blt(This, lpDestRect, mirroredSrcSurface, &srcRect, flags, &fx);
			}
			else
			{
				result = s_origVtable.Blt(This, lpDestRect, mirroredSrcSurface, nullptr, flags, &fx);
			}

			s_origVtable.Release(mirroredSrcSurface);
		}
		else
		{
			result = s_origVtable.Blt(This, lpDestRect, lpDDSrcSurface, lpSrcRect, dwFlags, lpDDBltFx);
		}

		if (This == s_compatPrimarySurface && SUCCEEDED(result))
		{
			RealPrimarySurface::update();
		}

		return result;
	}

	template <typename TSurface>
	HRESULT STDMETHODCALLTYPE CompatDirectDrawSurface<TSurface>::BltFast(
		TSurface* This,
		DWORD dwX,
		DWORD dwY,
		TSurface* lpDDSrcSurface,
		LPRECT lpSrcRect,
		DWORD dwTrans)
	{
		if ((This == s_compatPrimarySurface || lpDDSrcSurface == s_compatPrimarySurface) &&
			RealPrimarySurface::isLost())
		{
			return DDERR_SURFACELOST;
		}

		HRESULT result = s_origVtable.BltFast(This, dwX, dwY, lpDDSrcSurface, lpSrcRect, dwTrans);
		if (This == s_compatPrimarySurface && SUCCEEDED(result))
		{
			RealPrimarySurface::update();
		}
		return result;
	}

	template <typename TSurface>
	HRESULT STDMETHODCALLTYPE CompatDirectDrawSurface<TSurface>::Flip(
		TSurface* This,
		TSurface* lpDDSurfaceTargetOverride,
		DWORD dwFlags)
	{
		HRESULT result = s_origVtable.Flip(This, lpDDSurfaceTargetOverride, dwFlags);
		if (This == s_compatPrimarySurface && SUCCEEDED(result))
		{
			result = RealPrimarySurface::flip(dwFlags);
		}
		return result;
	}

	template <typename TSurface>
	HRESULT STDMETHODCALLTYPE CompatDirectDrawSurface<TSurface>::GetCaps(
		TSurface* This,
		TDdsCaps* lpDDSCaps)
	{
		HRESULT result = s_origVtable.GetCaps(This, lpDDSCaps);
		if (This == s_compatPrimarySurface && SUCCEEDED(result))
		{
			restorePrimaryCaps(*lpDDSCaps);
		}
		return result;
	}

	template <typename TSurface>
	HRESULT STDMETHODCALLTYPE CompatDirectDrawSurface<TSurface>::GetSurfaceDesc(
		TSurface* This,
		TSurfaceDesc* lpDDSurfaceDesc)
	{
		HRESULT result = s_origVtable.GetSurfaceDesc(This, lpDDSurfaceDesc);
		if (This == s_compatPrimarySurface && SUCCEEDED(result) && !g_lockingPrimary)
		{
			restorePrimaryCaps(lpDDSurfaceDesc->ddsCaps);
		}
		return result;
	}

	template <typename TSurface>
	HRESULT STDMETHODCALLTYPE CompatDirectDrawSurface<TSurface>::IsLost(TSurface* This)
	{
		HRESULT result = s_origVtable.IsLost(This);
		if (This == s_compatPrimarySurface && SUCCEEDED(result))
		{
			result = RealPrimarySurface::isLost() ? DDERR_SURFACELOST : DD_OK;
		}
		return result;
	}

	template <typename TSurface>
	HRESULT STDMETHODCALLTYPE CompatDirectDrawSurface<TSurface>::Lock(
		TSurface* This,
		LPRECT lpDestRect,
		TSurfaceDesc* lpDDSurfaceDesc,
		DWORD dwFlags,
		HANDLE hEvent)
	{
		if (This == s_compatPrimarySurface)
		{
			if (RealPrimarySurface::isLost())
			{
				return DDERR_SURFACELOST;
			}
			g_lockingPrimary = true;
		}

		HRESULT result = s_origVtable.Lock(This, lpDestRect, lpDDSurfaceDesc, dwFlags, hEvent);
		if (SUCCEEDED(result) && g_lockingPrimary && lpDDSurfaceDesc)
		{
			restorePrimaryCaps(lpDDSurfaceDesc->ddsCaps);
		}
		else if (DDERR_SURFACELOST == result)
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

		g_lockingPrimary = false;
		return result;
	}

	template <typename TSurface>
	HRESULT STDMETHODCALLTYPE CompatDirectDrawSurface<TSurface>::QueryInterface(
		TSurface* This,
		REFIID riid,
		LPVOID* obp)
	{
		if (This == s_compatPrimarySurface && riid == IID_IDirectDrawGammaControl)
		{
			return RealPrimarySurface::getSurface()->lpVtbl->QueryInterface(
				RealPrimarySurface::getSurface(), riid, obp);
		}
		return s_origVtable.QueryInterface(This, riid, obp);
	}

	template <typename TSurface>
	HRESULT STDMETHODCALLTYPE CompatDirectDrawSurface<TSurface>::ReleaseDC(TSurface* This, HDC hDC)
	{
		if (This == s_compatPrimarySurface && RealPrimarySurface::isLost())
		{
			return DDERR_SURFACELOST;
		}

		HRESULT result = s_origVtable.ReleaseDC(This, hDC);
		if (This == s_compatPrimarySurface && SUCCEEDED(result))
		{
			RealPrimarySurface::update();
		}
		return result;
	}

	template <typename TSurface>
	HRESULT STDMETHODCALLTYPE CompatDirectDrawSurface<TSurface>::Restore(TSurface* This)
	{
		const bool wasLost = DDERR_SURFACELOST == s_origVtable.IsLost(This);
		HRESULT result = s_origVtable.Restore(This);
		if (SUCCEEDED(result))
		{
			if (wasLost)
			{
				fixSurfacePtrs(*This);
			}
			if (This == s_compatPrimarySurface)
			{
				result = RealPrimarySurface::restore();
				if (wasLost)
				{
					CompatGdi::invalidate();
				}
			}
		}
		return result;
	}

	template <typename TSurface>
	HRESULT STDMETHODCALLTYPE CompatDirectDrawSurface<TSurface>::SetClipper(
		TSurface* This,
		LPDIRECTDRAWCLIPPER lpDDClipper)
	{
		HRESULT result = s_origVtable.SetClipper(This, lpDDClipper);
		if (This == s_compatPrimarySurface && SUCCEEDED(result))
		{
			RealPrimarySurface::setClipper(lpDDClipper);
		}
		return result;
	}

	template <typename TSurface>
	HRESULT STDMETHODCALLTYPE CompatDirectDrawSurface<TSurface>::SetPalette(
		TSurface* This,
		LPDIRECTDRAWPALETTE lpDDPalette)
	{
		HRESULT result = s_origVtable.SetPalette(This, lpDDPalette);
		if (This == s_compatPrimarySurface && SUCCEEDED(result))
		{
			CompatPrimarySurface::palette = lpDDPalette;
			RealPrimarySurface::setPalette(lpDDPalette);
		}
		return result;
	}

	template <typename TSurface>
	HRESULT STDMETHODCALLTYPE CompatDirectDrawSurface<TSurface>::Unlock(TSurface* This, TUnlockParam lpRect)
	{
		HRESULT result = s_origVtable.Unlock(This, lpRect);
		if (This == s_compatPrimarySurface && SUCCEEDED(result))
		{
			RealPrimarySurface::update();
		}
		return result;
	}

	template <typename TSurface>
	void CompatDirectDrawSurface<TSurface>::initCompatPrimarySurface()
	{
		Logging::LogEnter("CompatDirectDrawSurface::initCompatPrimarySurface");

		IUnknown& unk = reinterpret_cast<IUnknown&>(*s_compatPrimarySurface);
		CompatDirectDrawSurface<IDirectDrawSurface>::initPrimarySurfacePtr(IID_IDirectDrawSurface, unk);
		CompatDirectDrawSurface<IDirectDrawSurface2>::initPrimarySurfacePtr(IID_IDirectDrawSurface2, unk);
		CompatDirectDrawSurface<IDirectDrawSurface3>::initPrimarySurfacePtr(IID_IDirectDrawSurface3, unk);
		CompatDirectDrawSurface<IDirectDrawSurface4>::initPrimarySurfacePtr(IID_IDirectDrawSurface4, unk);
		CompatDirectDrawSurface<IDirectDrawSurface7>::initPrimarySurfacePtr(IID_IDirectDrawSurface7, unk);

		if (SUCCEEDED(s_origVtable.QueryInterface(
			s_compatPrimarySurface,
			IID_IDirectDrawSurface7,
			reinterpret_cast<LPVOID*>(&CompatPrimarySurface::surface))))
		{
			IReleaseNotifier* releaseNotifier = &CompatPrimarySurface::releaseNotifier;
			CompatPrimarySurface::surface->lpVtbl->SetPrivateData(CompatPrimarySurface::surface,
				IID_IReleaseNotifier, releaseNotifier, sizeof(releaseNotifier), DDSPD_IUNKNOWNPOINTER);
			CompatPrimarySurface::surface->lpVtbl->Release(CompatPrimarySurface::surface);
		}

		Logging::LogLeave("CompatDirectDrawSurface::initCompatPrimarySurface");
	}

	template <typename TSurface>
	void CompatDirectDrawSurface<TSurface>::restorePrimaryCaps(TDdsCaps& caps)
	{
		caps.dwCaps ^= DDSCAPS_OFFSCREENPLAIN;
		caps.dwCaps |= DDSCAPS_PRIMARYSURFACE | DDSCAPS_VISIBLE;
	}

	template <typename TSurface>
	TSurface* CompatDirectDrawSurface<TSurface>::s_compatPrimarySurface = nullptr;

	template <> const IID& CompatDirectDrawSurface<IDirectDrawSurface>::s_iid = IID_IDirectDrawSurface;
	template <> const IID& CompatDirectDrawSurface<IDirectDrawSurface2>::s_iid = IID_IDirectDrawSurface2;
	template <> const IID& CompatDirectDrawSurface<IDirectDrawSurface3>::s_iid = IID_IDirectDrawSurface3;
	template <> const IID& CompatDirectDrawSurface<IDirectDrawSurface4>::s_iid = IID_IDirectDrawSurface4;
	template <> const IID& CompatDirectDrawSurface<IDirectDrawSurface7>::s_iid = IID_IDirectDrawSurface7;

	template CompatDirectDrawSurface<IDirectDrawSurface>;
	template CompatDirectDrawSurface<IDirectDrawSurface2>;
	template CompatDirectDrawSurface<IDirectDrawSurface3>;
	template CompatDirectDrawSurface<IDirectDrawSurface4>;
	template CompatDirectDrawSurface<IDirectDrawSurface7>;

	template HRESULT CompatDirectDrawSurface<IDirectDrawSurface>::createCompatPrimarySurface(
		IDirectDraw& dd,
		TSurfaceDesc compatDesc,
		IDirectDrawSurface*& compatSurface);
	template HRESULT CompatDirectDrawSurface<IDirectDrawSurface>::createCompatPrimarySurface(
		IDirectDraw2& dd,
		TSurfaceDesc compatDesc,
		IDirectDrawSurface*& compatSurface);
	template HRESULT CompatDirectDrawSurface<IDirectDrawSurface4>::createCompatPrimarySurface(
		IDirectDraw4& dd,
		TSurfaceDesc compatDesc,
		IDirectDrawSurface4*& compatSurface);
	template HRESULT CompatDirectDrawSurface<IDirectDrawSurface7>::createCompatPrimarySurface(
		IDirectDraw7& dd,
		TSurfaceDesc compatDesc,
		IDirectDrawSurface7*& compatSurface);
}
