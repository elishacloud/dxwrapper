#include "CompatDirectDraw.h"
#include "CompatDirectDrawSurface.h"
#include "CompatPrimarySurface.h"

namespace Compat20
{
	namespace
	{
		template <typename TSurfaceDesc>
		HRESULT PASCAL enumDisplayModesCallback(
			TSurfaceDesc* lpDDSurfaceDesc,
			LPVOID lpContext)
		{
			if (lpDDSurfaceDesc)
			{
				*static_cast<DDPIXELFORMAT*>(lpContext) = lpDDSurfaceDesc->ddpfPixelFormat;
			}
			return DDENUMRET_CANCEL;
		}
	}

	template <typename TDirectDraw>
	void CompatDirectDraw<TDirectDraw>::setCompatVtable(Vtable<TDirectDraw>& vtable)
	{
		vtable.CreateSurface = &CreateSurface;
		vtable.RestoreDisplayMode = &RestoreDisplayMode;
		vtable.SetDisplayMode = &SetDisplayMode;
	}

	template <typename TDirectDraw>
	HRESULT STDMETHODCALLTYPE CompatDirectDraw<TDirectDraw>::CreateSurface(
		TDirectDraw* This,
		TSurfaceDesc* lpDDSurfaceDesc,
		TSurface** lplpDDSurface,
		IUnknown* pUnkOuter)
	{
		HRESULT result = DD_OK;

		const bool isPrimary = lpDDSurfaceDesc &&
			(lpDDSurfaceDesc->dwFlags & DDSD_CAPS) &&
			(lpDDSurfaceDesc->ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE);

		if (isPrimary)
		{
			result = CompatDirectDrawSurface<TSurface>::createCompatPrimarySurface(
				*This, *lpDDSurfaceDesc, *lplpDDSurface);
		}
		else
		{
			if (CompatPrimarySurface::displayMode.pixelFormat.dwSize != 0 &&
				!(lpDDSurfaceDesc->dwFlags & DDSD_PIXELFORMAT) &&
				(lpDDSurfaceDesc->dwFlags & DDSD_WIDTH) &&
				(lpDDSurfaceDesc->dwFlags & DDSD_HEIGHT) &&
				!((lpDDSurfaceDesc->dwFlags & DDSD_CAPS) &&
				(lpDDSurfaceDesc->ddsCaps.dwCaps & (DDSCAPS_ALPHA | DDSCAPS_ZBUFFER))))
			{
				TSurfaceDesc desc = *lpDDSurfaceDesc;
				desc.dwFlags |= DDSD_PIXELFORMAT;
				desc.ddpfPixelFormat = CompatPrimarySurface::displayMode.pixelFormat;
				result = s_origVtable.CreateSurface(This, &desc, lplpDDSurface, pUnkOuter);
			}
			else
			{
				result = s_origVtable.CreateSurface(This, lpDDSurfaceDesc, lplpDDSurface, pUnkOuter);
			}
		}

		if (SUCCEEDED(result))
		{
			CompatDirectDrawSurface<TSurface>::fixSurfacePtrs(**lplpDDSurface);
		}

		return result;
	}

	template <typename TDirectDraw>
	HRESULT STDMETHODCALLTYPE CompatDirectDraw<TDirectDraw>::RestoreDisplayMode(TDirectDraw* This)
	{
		HRESULT result = s_origVtable.RestoreDisplayMode(This);
		if (SUCCEEDED(result))
		{
			CompatPrimarySurface::displayMode = CompatPrimarySurface::getDisplayMode(*This);
		}
		return result;
	}

	template <typename TDirectDraw>
	template <typename... Params>
	static HRESULT STDMETHODCALLTYPE CompatDirectDraw<TDirectDraw>::SetDisplayMode(
		TDirectDraw* This,
		DWORD dwWidth,
		DWORD dwHeight,
		DWORD dwBPP,
		Params... params)
	{
		TSurfaceDesc desc = {};
		desc.dwSize = sizeof(desc);
		desc.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;
		desc.dwWidth = dwWidth;
		desc.dwHeight = dwHeight;
		desc.ddpfPixelFormat.dwSize = sizeof(desc.ddpfPixelFormat);
		desc.ddpfPixelFormat.dwFlags = DDPF_RGB;
		desc.ddpfPixelFormat.dwRGBBitCount = dwBPP;

		switch (dwBPP)
		{
		case 1: desc.ddpfPixelFormat.dwFlags |= DDPF_PALETTEINDEXED1; break;
		case 2: desc.ddpfPixelFormat.dwFlags |= DDPF_PALETTEINDEXED2; break;
		case 4: desc.ddpfPixelFormat.dwFlags |= DDPF_PALETTEINDEXED4; break;
		case 8: desc.ddpfPixelFormat.dwFlags |= DDPF_PALETTEINDEXED8; break;
		}

		DDPIXELFORMAT pf = {};
		if (dwBPP > 8)
		{
			if (FAILED(s_origVtable.EnumDisplayModes(This, 0, &desc, &pf, &enumDisplayModesCallback)) ||
				0 == pf.dwSize)
			{
				//********** Begin Edit *************
				// Try again with 32bit mode
				desc.ddpfPixelFormat.dwRGBBitCount = 32;
				if (FAILED(CompatDirectDraw<TDirectDraw>::s_origVtable.EnumDisplayModes(
					This, 0, &desc, &pf, &enumDisplayModesCallback)) || 0 == pf.dwSize)
				{
					Logging::Log() << "Failed to find the requested display mode: " <<
						dwWidth << "x" << dwHeight << "x" << dwBPP;
					return DDERR_INVALIDMODE;
				}
				//********** End Edit ***************
			}
		}
		else
		{
			pf = desc.ddpfPixelFormat;
		}

		HRESULT result = s_origVtable.SetDisplayMode(This, dwWidth, dwHeight, 32, params...);
		if (SUCCEEDED(result))
		{
			CompatPrimarySurface::displayMode.width = dwWidth;
			CompatPrimarySurface::displayMode.height = dwHeight;
			CompatPrimarySurface::displayMode.pixelFormat = pf;
		}
		else
		{
			Logging::Log() << "Failed to set the display mode to " << dwWidth << "x" << dwHeight << "x32";
		}

		return result;
	}

	template CompatDirectDraw<IDirectDraw>;
	template CompatDirectDraw<IDirectDraw2>;
	template CompatDirectDraw<IDirectDraw4>;
	template CompatDirectDraw<IDirectDraw7>;
}
