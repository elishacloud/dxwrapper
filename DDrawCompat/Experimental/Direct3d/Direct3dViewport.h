#pragma once

#include "Common/CompatVtable.h"
#include "Direct3d/Visitors/Direct3dViewportVtblVisitor.h"

namespace Direct3d
{
	template <typename TDirect3dViewport>
	class Direct3dViewport : public CompatVtable<Vtable<TDirect3dViewport>>
	{
	public:
		static void setCompatVtable(Vtable<TDirect3dViewport>& vtable);
	};
}

SET_COMPAT_VTABLE(IDirect3DViewportVtbl, Direct3d::Direct3dViewport<IDirect3DViewport>);
SET_COMPAT_VTABLE(IDirect3DViewport2Vtbl, Direct3d::Direct3dViewport<IDirect3DViewport2>);
SET_COMPAT_VTABLE(IDirect3DViewport3Vtbl, Direct3d::Direct3dViewport<IDirect3DViewport3>);
