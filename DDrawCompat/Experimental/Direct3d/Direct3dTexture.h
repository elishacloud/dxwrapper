#pragma once

#include "Common/CompatVtable.h"
#include "Direct3d/Visitors/Direct3dTextureVtblVisitor.h"

namespace Direct3d
{
	template <typename TDirect3dTexture>
	class Direct3dTexture : public CompatVtable<Vtable<TDirect3dTexture>>
	{
	public:
		static void setCompatVtable(Vtable<TDirect3dTexture>& vtable);
	};
}

SET_COMPAT_VTABLE(IDirect3DTextureVtbl, Direct3d::Direct3dTexture<IDirect3DTexture>);
SET_COMPAT_VTABLE(IDirect3DTexture2Vtbl, Direct3d::Direct3dTexture<IDirect3DTexture2>);
