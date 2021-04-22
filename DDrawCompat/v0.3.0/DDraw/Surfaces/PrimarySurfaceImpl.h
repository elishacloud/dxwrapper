#pragma once

#include <ddraw.h>

#include <Common/CompatRef.h>
#include <Common/CompatVtable.h>
#include <DDraw/Surfaces/SurfaceImpl.h>
#include <DDraw/Types.h>

namespace DDraw
{
	template <typename TSurface>
	class PrimarySurfaceImpl : public SurfaceImpl<TSurface>
	{
	public:
		PrimarySurfaceImpl(Surface* data);

		virtual HRESULT Blt(TSurface* This, LPRECT lpDestRect, TSurface* lpDDSrcSurface, LPRECT lpSrcRect,
			DWORD dwFlags, LPDDBLTFX lpDDBltFx) override;
		virtual HRESULT BltFast(TSurface* This, DWORD dwX, DWORD dwY,
			TSurface* lpDDSrcSurface, LPRECT lpSrcRect, DWORD dwTrans) override;
		virtual HRESULT Flip(TSurface* This, TSurface* lpDDSurfaceTargetOverride, DWORD dwFlags) override;
		virtual HRESULT GetCaps(TSurface* This, TDdsCaps* lpDDSCaps) override;
		virtual HRESULT GetSurfaceDesc(TSurface* This, TSurfaceDesc* lpDDSurfaceDesc) override;
		virtual HRESULT IsLost(TSurface* This) override;
		virtual HRESULT Lock(TSurface* This, LPRECT lpDestRect, TSurfaceDesc* lpDDSurfaceDesc,
			DWORD dwFlags, HANDLE hEvent) override;
		virtual HRESULT ReleaseDC(TSurface* This, HDC hDC) override;
		virtual HRESULT Restore(TSurface* This) override;
		virtual HRESULT SetPalette(TSurface* This, LPDIRECTDRAWPALETTE lpDDPalette) override;
		virtual HRESULT Unlock(TSurface* This, TUnlockParam lpRect) override;
	};
}
