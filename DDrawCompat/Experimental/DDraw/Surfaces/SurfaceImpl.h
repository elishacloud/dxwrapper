#pragma once

#define CINTERFACE

#include <functional>

#include <ddraw.h>

#include "Common/CompatVtable.h"
#include "DDraw/Types.h"

namespace DDraw
{
	class Surface;

	template <typename TSurface>
	class SurfaceImpl2
	{
	public:
		SurfaceImpl2() : m_data(nullptr) {}

		virtual HRESULT GetDDInterface(TSurface* This, LPVOID* lplpDD);

	protected:
		friend class Surface;

		Surface* m_data;
	};

	template <typename TSurface>
	class SurfaceImpl : public SurfaceImpl2<TSurface>
	{
	public:
		typedef typename Types<TSurface>::TSurfaceDesc TSurfaceDesc;
		typedef typename Types<TSurface>::TDdsCaps TDdsCaps;
		typedef typename Types<TSurface>::TUnlockParam TUnlockParam;

		virtual ~SurfaceImpl();

		virtual HRESULT Blt(TSurface* This, LPRECT lpDestRect, TSurface* lpDDSrcSurface, LPRECT lpSrcRect,
			DWORD dwFlags, LPDDBLTFX lpDDBltFx);
		virtual HRESULT BltFast(TSurface* This, DWORD dwX, DWORD dwY,
			TSurface* lpDDSrcSurface, LPRECT lpSrcRect, DWORD dwTrans);
		virtual HRESULT Flip(TSurface* This, TSurface* lpDDSurfaceTargetOverride, DWORD dwFlags);
		virtual HRESULT GetCaps(TSurface* This, TDdsCaps* lpDDSCaps);
		virtual HRESULT GetSurfaceDesc(TSurface* This, TSurfaceDesc* lpDDSurfaceDesc);
		virtual HRESULT IsLost(TSurface* This);
		virtual HRESULT Lock(TSurface* This, LPRECT lpDestRect, TSurfaceDesc* lpDDSurfaceDesc,
			DWORD dwFlags, HANDLE hEvent);
		virtual HRESULT ReleaseDC(TSurface* This, HDC hDC);
		virtual HRESULT Restore(TSurface* This);
		virtual HRESULT SetPalette(TSurface* This, LPDIRECTDRAWPALETTE lpDDPalette);
		virtual HRESULT Unlock(TSurface* This, TUnlockParam lpRect);

	protected:
		void undoFlip(TSurface* This, TSurface* targetOverride);

		static const Vtable<TSurface>& s_origVtable;

	private:
		bool bltRetry(TSurface*& dstSurface, RECT*& dstRect,
			TSurface*& srcSurface, RECT*& srcRect, bool isTransparentBlt,
			const std::function<HRESULT()>& blt);
		bool prepareBltRetrySurface(TSurface*& surface, RECT*& rect,
			const TSurfaceDesc& desc, bool isTransparentBlt, bool isCopyNeeded);
		void replaceWithVidMemSurface(TSurface*& surface, RECT*& rect, const TSurfaceDesc& desc);
	};
}
