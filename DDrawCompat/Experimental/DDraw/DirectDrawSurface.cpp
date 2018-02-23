#include <set>

#include "DDraw/DirectDrawSurface.h"
#include "DDraw/Surfaces/Surface.h"
#include "DDraw/Surfaces/SurfaceImpl.h"

namespace
{
	template <typename CompatMethod, CompatMethod compatMethod,
		typename OrigMethod, OrigMethod origMethod,
		typename TSurface, typename... Params>
	HRESULT STDMETHODCALLTYPE callImpl(TSurface* This, Params... params)
	{
		DDraw::Surface* surface = This ? DDraw::Surface::getSurface(*This) : nullptr;
		if (!surface)
		{
			return (CompatVtable<Vtable<TSurface>>::s_origVtable.*origMethod)(This, params...);
		}
		return (surface->getImpl<TSurface>()->*compatMethod)(This, params...);
	}
}

#define SET_COMPAT_METHOD(method) \
	vtable.method = &callImpl<decltype(&SurfaceImpl<TSurface>::method), &SurfaceImpl<TSurface>::method, \
							  decltype(&Vtable<TSurface>::method), &Vtable<TSurface>::method>

namespace DDraw
{
	template <typename TSurface>
	void DirectDrawSurface<TSurface>::setCompatVtable(Vtable<TSurface>& vtable)
	{
		SET_COMPAT_METHOD(Blt);
		SET_COMPAT_METHOD(BltFast);
		SET_COMPAT_METHOD(Flip);
		SET_COMPAT_METHOD(GetCaps);
		SET_COMPAT_METHOD(GetSurfaceDesc);
		SET_COMPAT_METHOD(IsLost);
		SET_COMPAT_METHOD(Lock);
		SET_COMPAT_METHOD(ReleaseDC);
		SET_COMPAT_METHOD(Restore);
		SET_COMPAT_METHOD(SetPalette);
		SET_COMPAT_METHOD(Unlock);

		setCompatVtable2(vtable);
	}

	template <typename TSurface>
	void DirectDrawSurface<TSurface>::setCompatVtable2(Vtable<TSurface>& vtable)
	{
		SET_COMPAT_METHOD(GetDDInterface);
	}

	template <>
	void DirectDrawSurface<IDirectDrawSurface>::setCompatVtable2(Vtable<IDirectDrawSurface>&)
	{
	}

	template DirectDrawSurface<IDirectDrawSurface>;
	template DirectDrawSurface<IDirectDrawSurface2>;
	template DirectDrawSurface<IDirectDrawSurface3>;
	template DirectDrawSurface<IDirectDrawSurface4>;
	template DirectDrawSurface<IDirectDrawSurface7>;
}
