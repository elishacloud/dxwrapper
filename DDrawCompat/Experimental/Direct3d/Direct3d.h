#pragma once

#include "Common/CompatVtable.h"
#include "Direct3d/Visitors/Direct3dVtblVisitor.h"

namespace Direct3d
{
	template <typename TDirect3d>
	class Direct3d : public CompatVtable<Vtable<TDirect3d>>
	{
	public:
		static void setCompatVtable(Vtable<TDirect3d>& vtable);
	};
}

SET_COMPAT_VTABLE(IDirect3DVtbl, Direct3d::Direct3d<IDirect3D>);
SET_COMPAT_VTABLE(IDirect3D2Vtbl, Direct3d::Direct3d<IDirect3D2>);
SET_COMPAT_VTABLE(IDirect3D3Vtbl, Direct3d::Direct3d<IDirect3D3>);
SET_COMPAT_VTABLE(IDirect3D7Vtbl, Direct3d::Direct3d<IDirect3D7>);
