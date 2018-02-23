#pragma once

#define CINTERFACE

#include <ddraw.h>

#include "Common/CompatPtr.h"
#include "Common/CompatRef.h"
#include "Common/CompatVtable.h"
#include "DDraw/Visitors/DirectDrawVtblVisitor.h"
#include "DDraw/Types.h"

namespace DDraw
{
	CompatPtr<IDirectDrawSurface7> createCompatibleSurface(DWORD bpp);
	
	template <typename TDirectDraw>
	void* getDdObject(TDirectDraw& dd);

	DDSURFACEDESC2 getDisplayMode(CompatRef<IDirectDraw7> dd);
	void suppressEmulatedDirectDraw(GUID*& guid);

	template <typename TDirectDraw>
	class DirectDraw: public CompatVtable<Vtable<TDirectDraw>>
	{
	public:
		typedef typename Types<TDirectDraw>::TCreatedSurface TSurface;
		typedef typename Types<TDirectDraw>::TSurfaceDesc TSurfaceDesc;

		static void setCompatVtable(Vtable<TDirectDraw>& vtable);

		static HRESULT STDMETHODCALLTYPE CreateSurface(
			TDirectDraw* This,
			TSurfaceDesc* lpDDSurfaceDesc,
			TSurface** lplpDDSurface,
			IUnknown* pUnkOuter);

		static HRESULT STDMETHODCALLTYPE FlipToGDISurface(TDirectDraw* This);
		static HRESULT STDMETHODCALLTYPE GetGDISurface(TDirectDraw* This, TSurface** lplpGDIDDSSurface);
		static HRESULT STDMETHODCALLTYPE Initialize(TDirectDraw* This, GUID* lpGUID);
		static HRESULT STDMETHODCALLTYPE SetCooperativeLevel(TDirectDraw* This, HWND hWnd, DWORD dwFlags);

		template <typename... Params>
		static HRESULT STDMETHODCALLTYPE SetDisplayMode(
			TDirectDraw* This,
			DWORD dwWidth,
			DWORD dwHeight,
			DWORD dwBPP,
			Params... params);
	};
}

SET_COMPAT_VTABLE(IDirectDrawVtbl, DDraw::DirectDraw<IDirectDraw>);
SET_COMPAT_VTABLE(IDirectDraw2Vtbl, DDraw::DirectDraw<IDirectDraw2>);
SET_COMPAT_VTABLE(IDirectDraw4Vtbl, DDraw::DirectDraw<IDirectDraw4>);
SET_COMPAT_VTABLE(IDirectDraw7Vtbl, DDraw::DirectDraw<IDirectDraw7>);
