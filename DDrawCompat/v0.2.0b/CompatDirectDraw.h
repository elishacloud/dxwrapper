#pragma once

#include "CompatVtable.h"
#include "DDrawTypes.h"
#include "DirectDrawVtblVisitor.h"

namespace Compat20
{
	template <typename TDirectDraw>
	class CompatDirectDraw : public CompatVtable<CompatDirectDraw<TDirectDraw>, TDirectDraw>
	{
	public:
		typedef typename Types<TDirectDraw>::TCreatedSurface TSurface;
		typedef typename Types<TDirectDraw>::TSurfaceDesc TSurfaceDesc;

		static void setCompatVtable(Vtable<TDirectDraw>& vtable);

		static HRESULT STDMETHODCALLTYPE Initialize(TDirectDraw* This, GUID* unnamedParam1);

		static HRESULT STDMETHODCALLTYPE CreateSurface(
			TDirectDraw* This,
			TSurfaceDesc* lpDDSurfaceDesc,
			TSurface** lplpDDSurface,
			IUnknown* pUnkOuter);

		static HRESULT STDMETHODCALLTYPE RestoreDisplayMode(TDirectDraw* This);

		template <typename... Params>
		static HRESULT STDMETHODCALLTYPE SetDisplayMode(
			TDirectDraw* This,
			DWORD dwWidth,
			DWORD dwHeight,
			DWORD dwBPP,
			Params... params);
	};
}
