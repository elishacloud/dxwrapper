#include "Common/CompatPtr.h"
#include "DDraw/ActivateAppHandler.h"
#include "DDraw/DirectDraw.h"
#include "DDraw/Repository.h"
#include "DDraw/Surfaces/TagSurface.h"
#include "DDraw/Surfaces/PrimarySurface.h"
#include "Win32/DisplayMode.h"

namespace
{
	DDPIXELFORMAT getRgbPixelFormat(DWORD bpp)
	{
		DDPIXELFORMAT pf = {};
		pf.dwSize = sizeof(pf);
		pf.dwFlags = DDPF_RGB;
		pf.dwRGBBitCount = bpp;

		switch (bpp)
		{
		case 1:
			pf.dwFlags |= DDPF_PALETTEINDEXED1;
			break;
		case 2:
			pf.dwFlags |= DDPF_PALETTEINDEXED2;
			break;
		case 4:
			pf.dwFlags |= DDPF_PALETTEINDEXED4;
			break;
		case 8:
			pf.dwFlags |= DDPF_PALETTEINDEXED8;
			break;
		case 16:
			pf.dwRBitMask = 0xF800;
			pf.dwGBitMask = 0x07E0;
			pf.dwBBitMask = 0x001F;
			break;
		case 24:
		case 32:
			pf.dwRBitMask = 0xFF0000;
			pf.dwGBitMask = 0x00FF00;
			pf.dwBBitMask = 0x0000FF;
			break;
		}

		return pf;
	}

	template <typename TDirectDraw>
	HRESULT setDisplayMode(TDirectDraw* This, DWORD width, DWORD height, DWORD bpp)
	{
		return DDraw::DirectDraw<TDirectDraw>::s_origVtable.SetDisplayMode(This, width, height, bpp);
	}

	template <typename TDirectDraw>
	HRESULT setDisplayMode(TDirectDraw* This, DWORD width, DWORD height, DWORD bpp,
		DWORD refreshRate, DWORD flags)
	{
		Win32::DisplayMode::setDDrawBpp(bpp);
		HRESULT result = DDraw::DirectDraw<TDirectDraw>::s_origVtable.SetDisplayMode(
			This, width, height, bpp, refreshRate, flags);
		Win32::DisplayMode::setDDrawBpp(0);
		return result;
	}
}

namespace DDraw
{
	CompatPtr<IDirectDrawSurface7> createCompatibleSurface(DWORD bpp)
	{
		DDSURFACEDESC2 desc = {};
		desc.dwSize = sizeof(desc);
		desc.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS | DDSD_PIXELFORMAT;
		desc.dwWidth = 1;
		desc.dwHeight = 1;
		desc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
		desc.ddpfPixelFormat = getRgbPixelFormat(bpp);

		CompatPtr<IDirectDrawSurface7> surface;
		auto dd = DDraw::Repository::getDirectDraw();
		dd->CreateSurface(dd, &desc, &surface.getRef(), nullptr);
		return surface;
	}

	template <typename TDirectDraw>
	void* getDdObject(TDirectDraw& dd)
	{
		return reinterpret_cast<void**>(&dd)[1];
	}

	template void* getDdObject(IDirectDraw&);
	template void* getDdObject(IDirectDraw2&);
	template void* getDdObject(IDirectDraw4&);
	template void* getDdObject(IDirectDraw7&);

	DDSURFACEDESC2 getDisplayMode(CompatRef<IDirectDraw7> dd)
	{
		DDSURFACEDESC2 dm = {};
		dm.dwSize = sizeof(dm);
		dd->GetDisplayMode(&dd, &dm);
		return dm;
	}

	void suppressEmulatedDirectDraw(GUID*& guid)
	{
		if (reinterpret_cast<GUID*>(DDCREATE_EMULATIONONLY) == guid)
		{
			LOG_ONCE("Suppressed a request to create an emulated DirectDraw object");
			guid = nullptr;
		}
	}

	template <typename TDirectDraw>
	void DirectDraw<TDirectDraw>::setCompatVtable(Vtable<TDirectDraw>& vtable)
	{
		vtable.CreateSurface = &CreateSurface;
		vtable.FlipToGDISurface = &FlipToGDISurface;
		vtable.GetGDISurface = &GetGDISurface;
		vtable.SetCooperativeLevel = &SetCooperativeLevel;
		vtable.SetDisplayMode = &SetDisplayMode;
	}

	template <typename TDirectDraw>
	HRESULT STDMETHODCALLTYPE DirectDraw<TDirectDraw>::CreateSurface(
		TDirectDraw* This,
		TSurfaceDesc* lpDDSurfaceDesc,
		TSurface** lplpDDSurface,
		IUnknown* pUnkOuter)
	{
		if (!This || !lpDDSurfaceDesc || !lplpDDSurface)
		{
			return s_origVtable.CreateSurface(This, lpDDSurfaceDesc, lplpDDSurface, pUnkOuter);
		}

		if (lpDDSurfaceDesc->ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE)
		{
			return PrimarySurface::create<TDirectDraw>(*This, *lpDDSurfaceDesc, *lplpDDSurface);
		}
		else
		{
			return Surface::create<TDirectDraw>(*This, *lpDDSurfaceDesc, *lplpDDSurface);
		}
	}

	template <typename TDirectDraw>
	HRESULT STDMETHODCALLTYPE DirectDraw<TDirectDraw>::FlipToGDISurface(TDirectDraw* /*This*/)
	{
		return PrimarySurface::flipToGdiSurface();
	}

	template <typename TDirectDraw>
	HRESULT STDMETHODCALLTYPE DirectDraw<TDirectDraw>::GetGDISurface(
		TDirectDraw* /*This*/, TSurface** lplpGDIDDSSurface)
	{
		if (!lplpGDIDDSSurface)
		{
			return DDERR_INVALIDPARAMS;
		}

		auto gdiSurface(PrimarySurface::getGdiSurface());
		if (!gdiSurface)
		{
			return DDERR_NOTFOUND;
		}

		*lplpGDIDDSSurface = CompatPtr<TSurface>::from(gdiSurface.get()).detach();
		return DD_OK;
	}

	template <typename TDirectDraw>
	HRESULT STDMETHODCALLTYPE DirectDraw<TDirectDraw>::Initialize(TDirectDraw* This, GUID* lpGUID)
	{
		suppressEmulatedDirectDraw(lpGUID);
		return s_origVtable.Initialize(This, lpGUID);
	}

	template <typename TDirectDraw>
	HRESULT STDMETHODCALLTYPE DirectDraw<TDirectDraw>::SetCooperativeLevel(
		TDirectDraw* This, HWND hWnd, DWORD dwFlags)
	{
		HRESULT result = s_origVtable.SetCooperativeLevel(This, hWnd, dwFlags);
		if (SUCCEEDED(result))
		{
			void* ddObject = getDdObject(*This);
			TagSurface* tagSurface = TagSurface::get(ddObject);
			if (!tagSurface)
			{
				CompatPtr<IDirectDraw> dd(Compat::queryInterface<IDirectDraw>(This));
				TagSurface::create(*dd);
				tagSurface = TagSurface::get(ddObject);
			}

			ActivateAppHandler::setCooperativeLevel(hWnd, dwFlags);
		}
		return result;
	}

	template <typename TDirectDraw>
	template <typename... Params>
	HRESULT STDMETHODCALLTYPE DirectDraw<TDirectDraw>::SetDisplayMode(
		TDirectDraw* This,
		DWORD dwWidth,
		DWORD dwHeight,
		DWORD dwBPP,
		Params... params)
	{
		return setDisplayMode(This, dwWidth, dwHeight, dwBPP, params...);
	}

	template DirectDraw<IDirectDraw>;
	template DirectDraw<IDirectDraw2>;
	template DirectDraw<IDirectDraw4>;
	template DirectDraw<IDirectDraw7>;
}
