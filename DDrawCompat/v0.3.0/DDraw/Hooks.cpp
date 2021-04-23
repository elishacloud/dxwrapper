#define WIN32_LEAN_AND_MEAN
#define CINTERFACE

#include <DDrawCompat/v0.3.0/Common/CompatRef.h>
#include <DDrawCompat/DDrawLog.h>
#include <DDrawCompat/v0.3.0/DDraw/DirectDraw.h>
#include <DDrawCompat/v0.3.0/DDraw/DirectDrawClipper.h>
#include <DDrawCompat/v0.3.0/DDraw/DirectDrawGammaControl.h>
#include <DDrawCompat/v0.3.0/DDraw/DirectDrawPalette.h>
#include <DDrawCompat/v0.3.0/DDraw/DirectDrawSurface.h>
#include <DDrawCompat/v0.3.0/DDraw/Hooks.h>
#include <DDrawCompat/v0.3.0/DDraw/RealPrimarySurface.h>
#include <DDrawCompat/v0.3.0/DDraw/ScopedThreadLock.h>
#include <DDrawCompat/v0.3.0/Win32/Registry.h>

namespace
{
	void hookDirectDraw(CompatPtr<IDirectDraw7> dd)
	{
		DDraw::DirectDraw::hookVtable(*CompatPtr<IDirectDraw>(dd).get()->lpVtbl);
		DDraw::DirectDraw::hookVtable(*CompatPtr<IDirectDraw2>(dd).get()->lpVtbl);
		DDraw::DirectDraw::hookVtable(*CompatPtr<IDirectDraw4>(dd).get()->lpVtbl);
		DDraw::DirectDraw::hookVtable(*CompatPtr<IDirectDraw7>(dd).get()->lpVtbl);
	}

	void hookDirectDrawClipper(CompatRef<IDirectDraw7> dd)
	{
		CompatPtr<IDirectDrawClipper> clipper;
		HRESULT result = dd->CreateClipper(&dd, 0, &clipper.getRef(), nullptr);
		if (SUCCEEDED(result))
		{
			DDraw::DirectDrawClipper::hookVtable(*clipper.get()->lpVtbl);
		}
		else
		{
			Compat30::Log() << "ERROR: Failed to create a DirectDraw clipper for hooking: " << result;
		}
	}

	void hookDirectDrawPalette(CompatRef<IDirectDraw7> dd)
	{
		PALETTEENTRY paletteEntries[2] = {};
		CompatPtr<IDirectDrawPalette> palette;
		HRESULT result = dd->CreatePalette(&dd, DDPCAPS_1BIT, paletteEntries, &palette.getRef(), nullptr);
		if (SUCCEEDED(result))
		{
			DDraw::DirectDrawPalette::hookVtable(*palette.get()->lpVtbl);
		}
		else
		{
			Compat30::Log() << "ERROR: Failed to create a DirectDraw palette for hooking: " << result;
		}
	}

	void hookDirectDrawSurface(CompatRef<IDirectDraw7> dd)
	{
		DDSURFACEDESC2 desc = {};
		desc.dwSize = sizeof(desc);
		desc.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
		desc.dwWidth = 1;
		desc.dwHeight = 1;
		desc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;

		CompatPtr<IDirectDrawSurface7> surface;
		HRESULT result = dd->CreateSurface(&dd, &desc, &surface.getRef(), nullptr);
		if (SUCCEEDED(result))
		{
			CompatVtable<IDirectDrawSurface7Vtbl>::s_origVtable = *surface.get()->lpVtbl;
			DDraw::DirectDrawSurface::hookVtable(*CompatPtr<IDirectDrawSurface>(surface).get()->lpVtbl);
			DDraw::DirectDrawSurface::hookVtable(*CompatPtr<IDirectDrawSurface2>(surface).get()->lpVtbl);
			DDraw::DirectDrawSurface::hookVtable(*CompatPtr<IDirectDrawSurface3>(surface).get()->lpVtbl);
			DDraw::DirectDrawSurface::hookVtable(*CompatPtr<IDirectDrawSurface4>(surface).get()->lpVtbl);
			DDraw::DirectDrawSurface::hookVtable(*CompatPtr<IDirectDrawSurface7>(surface).get()->lpVtbl);

			CompatPtr<IDirectDrawGammaControl> gammaControl(surface);
			DDraw::DirectDrawGammaControl::hookVtable(*gammaControl.get()->lpVtbl);
		}
		else
		{
			Compat30::Log() << "ERROR: Failed to create a DirectDraw surface for hooking: " << result;
		}
	}
}

namespace DDraw
{
	void installHooks(CompatPtr<IDirectDraw7> dd7)
	{
		RealPrimarySurface::init();

		Win32::Registry::unsetValue(
			HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\DirectDraw", "EmulationOnly");
		Win32::Registry::unsetValue(
			HKEY_LOCAL_MACHINE, "SOFTWARE\\WOW6432Node\\Microsoft\\DirectDraw", "EmulationOnly");

		hookDirectDraw(dd7);
		hookDirectDrawClipper(*dd7);
		hookDirectDrawPalette(*dd7);
		hookDirectDrawSurface(*dd7);
	}
}
