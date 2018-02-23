#pragma once

#include "Common/CompatPtr.h"
#include "Common/CompatRef.h"
#include "Common/CompatVtable.h"
#include "DDraw/Visitors/DirectDrawSurfaceVtblVisitor.h"
#include "DDraw/Types.h"

namespace DDraw
{
	template <typename TSurface>
	class DirectDrawSurface : public CompatVtable<Vtable<TSurface>>
	{
	public:
		typedef typename Types<TSurface>::TSurfaceDesc TSurfaceDesc;

		static void setCompatVtable(Vtable<TSurface>& vtable);

	private:
		static void setCompatVtable2(Vtable<TSurface>& vtable);
	};
}

SET_COMPAT_VTABLE(IDirectDrawSurfaceVtbl, DDraw::DirectDrawSurface<IDirectDrawSurface>);
SET_COMPAT_VTABLE(IDirectDrawSurface2Vtbl, DDraw::DirectDrawSurface<IDirectDrawSurface2>);
SET_COMPAT_VTABLE(IDirectDrawSurface3Vtbl, DDraw::DirectDrawSurface<IDirectDrawSurface3>);
SET_COMPAT_VTABLE(IDirectDrawSurface4Vtbl, DDraw::DirectDrawSurface<IDirectDrawSurface4>);
SET_COMPAT_VTABLE(IDirectDrawSurface7Vtbl, DDraw::DirectDrawSurface<IDirectDrawSurface7>);
