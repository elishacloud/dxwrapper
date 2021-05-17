#define WIN32_LEAN_AND_MEAN
#define CINTERFACE

#include <DDrawCompat/v0.3.0/Common/CompatVtable.h>
#include <DDrawCompat/v0.3.0/DDraw/DirectDrawSurface.h>
#include <DDrawCompat/v0.3.0/DDraw/ScopedThreadLock.h>
#include <DDrawCompat/v0.3.0/DDraw/Surfaces/Surface.h>
#include <DDrawCompat/v0.3.0/DDraw/Surfaces/SurfaceImpl.h>
#include <DDrawCompat/v0.3.0/DDraw/Visitors/DirectDrawSurfaceVtblVisitor.h>

#define SET_COMPAT_METHOD(method) \
	vtable.method = &callImpl<decltype(&DDraw::SurfaceImpl<TSurface>::method), &DDraw::SurfaceImpl<TSurface>::method, \
							  decltype(&Vtable::method), &Vtable::method>

namespace
{
	template <typename Vtable> struct GetSurfaceType {};
	template <> struct GetSurfaceType<IDirectDrawSurfaceVtbl> { typedef IDirectDrawSurface Type; };
	template <> struct GetSurfaceType<IDirectDrawSurface2Vtbl> { typedef IDirectDrawSurface2 Type; };
	template <> struct GetSurfaceType<IDirectDrawSurface3Vtbl> { typedef IDirectDrawSurface3 Type; };
	template <> struct GetSurfaceType<IDirectDrawSurface4Vtbl> { typedef IDirectDrawSurface4 Type; };
	template <> struct GetSurfaceType<IDirectDrawSurface7Vtbl> { typedef IDirectDrawSurface7 Type; };

	struct AddAttachedSurfacesContext
	{
		IDirectDrawSurface7* rootSurface;
		std::vector<CompatPtr<IDirectDrawSurface7>> surfaces;
	};

	HRESULT WINAPI addAttachedSurfaces(
		LPDIRECTDRAWSURFACE7 lpDDSurface, LPDDSURFACEDESC2 /*lpDDSurfaceDesc*/, LPVOID lpContext)
	{
		CompatPtr<IDirectDrawSurface7> surface(lpDDSurface);
		auto& context(*static_cast<AddAttachedSurfacesContext*>(lpContext));
		if (surface == context.rootSurface)
		{
			return DD_OK;
		}
		context.surfaces.push_back(surface);
		surface->EnumAttachedSurfaces(surface, &context, &addAttachedSurfaces);
		return DD_OK;
	}

	template <typename CompatMethod, CompatMethod compatMethod,
		typename OrigMethod, OrigMethod origMethod,
		typename TSurface, typename... Params>
	HRESULT STDMETHODCALLTYPE callImpl(TSurface* This, Params... params)
	{
		DDraw::Surface* surface = DDraw::Surface::getSurface(*This);
		if (!surface || !(surface->getImpl<TSurface>()))
		{
			return (getOrigVtable(This).*origMethod)(This, params...);
		}
		return (surface->getImpl<TSurface>()->*compatMethod)(This, params...);
	}

	template <typename Vtable>
	constexpr void setCompatVtable(Vtable& vtable)
	{
		typedef GetSurfaceType<Vtable>::Type TSurface;
		SET_COMPAT_METHOD(Blt);
		SET_COMPAT_METHOD(BltFast);
		SET_COMPAT_METHOD(Flip);
		SET_COMPAT_METHOD(GetBltStatus);
		SET_COMPAT_METHOD(GetCaps);
		SET_COMPAT_METHOD(GetDC);
		SET_COMPAT_METHOD(GetFlipStatus);
		SET_COMPAT_METHOD(GetSurfaceDesc);
		SET_COMPAT_METHOD(IsLost);
		SET_COMPAT_METHOD(Lock);
		SET_COMPAT_METHOD(QueryInterface);
		SET_COMPAT_METHOD(ReleaseDC);
		SET_COMPAT_METHOD(Restore);
		SET_COMPAT_METHOD(SetClipper);
		SET_COMPAT_METHOD(SetPalette);
		SET_COMPAT_METHOD(Unlock);
	}
}

namespace DDraw
{
	namespace DirectDrawSurface
	{
		std::vector<CompatPtr<IDirectDrawSurface7>> getAllAttachedSurfaces(CompatRef<IDirectDrawSurface7> surface)
		{
			AddAttachedSurfacesContext context = { &surface };
			surface->EnumAttachedSurfaces(&surface, &context, &addAttachedSurfaces);
			return context.surfaces;
		}

		template <typename Vtable>
		void hookVtable(const Vtable& vtable)
		{
			CompatVtable<Vtable>::hookVtable<ScopedThreadLock>(vtable);
		}

		template void hookVtable(const IDirectDrawSurfaceVtbl&);
		template void hookVtable(const IDirectDrawSurface2Vtbl&);
		template void hookVtable(const IDirectDrawSurface3Vtbl&);
		template void hookVtable(const IDirectDrawSurface4Vtbl&);
		template void hookVtable(const IDirectDrawSurface7Vtbl&);
	}
}
