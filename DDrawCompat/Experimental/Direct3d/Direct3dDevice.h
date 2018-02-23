#pragma once

#include "Common/CompatVtable.h"
#include "Direct3d/Visitors/Direct3dDeviceVtblVisitor.h"

namespace Direct3d
{
	template <typename TDirect3dDevice>
	class Direct3dDevice : public CompatVtable<Vtable<TDirect3dDevice>>
	{
	public:
		static void setCompatVtable(Vtable<TDirect3dDevice>& vtable);
	};
}

SET_COMPAT_VTABLE(IDirect3DDeviceVtbl, Direct3d::Direct3dDevice<IDirect3DDevice>);
SET_COMPAT_VTABLE(IDirect3DDevice2Vtbl, Direct3d::Direct3dDevice<IDirect3DDevice2>);
SET_COMPAT_VTABLE(IDirect3DDevice3Vtbl, Direct3d::Direct3dDevice<IDirect3DDevice3>);
SET_COMPAT_VTABLE(IDirect3DDevice7Vtbl, Direct3d::Direct3dDevice<IDirect3DDevice7>);
