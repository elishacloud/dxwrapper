#pragma once

#include "CompatVtable.h"
#include "DDrawTypes.h"
#include "DirectDrawSurfaceVtblVisitor.h"

namespace Compat20
{
	template <typename TSurface>
	class CompatDirectDrawSurface : public CompatVtable<CompatDirectDrawSurface<TSurface>, TSurface>
	{
	public:
		typedef typename Types<TSurface>::TSurfaceDesc TSurfaceDesc;
		typedef typename Types<TSurface>::TDdsCaps TDdsCaps;
		typedef typename Types<TSurface>::TUnlockParam TUnlockParam;

		static void setCompatVtable(Vtable<TSurface>& vtable);

		template <typename TDirectDraw>
		static HRESULT createCompatPrimarySurface(
			TDirectDraw& dd,
			TSurfaceDesc compatDesc,
			TSurface*& compatSurface);

		static void fixSurfacePtrs(TSurface& surface);
		static void initPrimarySurfacePtr(const GUID& guid, IUnknown& surface);
		static void resetPrimarySurfacePtr();

		static HRESULT STDMETHODCALLTYPE Blt(
			TSurface* This,
			LPRECT lpDestRect,
			TSurface* lpDDSrcSurface,
			LPRECT lpSrcRect,
			DWORD dwFlags,
			LPDDBLTFX lpDDBltFx);

		static HRESULT STDMETHODCALLTYPE BltFast(
			TSurface* This,
			DWORD dwX,
			DWORD dwY,
			TSurface* lpDDSrcSurface,
			LPRECT lpSrcRect,
			DWORD dwTrans);

		static HRESULT STDMETHODCALLTYPE Flip(
			TSurface* This,
			TSurface* lpDDSurfaceTargetOverride,
			DWORD dwFlags);

		static HRESULT STDMETHODCALLTYPE GetCaps(TSurface* This, TDdsCaps* lpDDSCaps);
		static HRESULT STDMETHODCALLTYPE GetSurfaceDesc(TSurface* This, TSurfaceDesc* lpDDSurfaceDesc);
		static HRESULT STDMETHODCALLTYPE IsLost(TSurface* This);

		static HRESULT STDMETHODCALLTYPE Lock(
			TSurface* This,
			LPRECT lpDestRect,
			TSurfaceDesc* lpDDSurfaceDesc,
			DWORD dwFlags,
			HANDLE hEvent);

		static HRESULT STDMETHODCALLTYPE QueryInterface(TSurface* This, REFIID riid, LPVOID* obp);
		static HRESULT STDMETHODCALLTYPE ReleaseDC(TSurface* This, HDC hDC);
		static HRESULT STDMETHODCALLTYPE Restore(TSurface* This);
		static HRESULT STDMETHODCALLTYPE SetClipper(TSurface* This, LPDIRECTDRAWCLIPPER lpDDClipper);
		static HRESULT STDMETHODCALLTYPE SetPalette(TSurface* This, LPDIRECTDRAWPALETTE lpDDPalette);
		static HRESULT STDMETHODCALLTYPE Unlock(TSurface* This, TUnlockParam lpRect);

		static const IID& s_iid;

	private:
		static void initCompatPrimarySurface();
		static void restorePrimaryCaps(TDdsCaps& caps);
		static TSurface* s_compatPrimarySurface;
	};
}