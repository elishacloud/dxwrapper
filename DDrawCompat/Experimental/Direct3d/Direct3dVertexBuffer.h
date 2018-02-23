#pragma once

#include "Common/CompatVtable.h"
#include "Direct3d/Visitors/Direct3dVertexBufferVtblVisitor.h"

namespace Direct3d
{
	template <typename TDirect3dVertexBuffer>
	class Direct3dVertexBuffer : public CompatVtable<Vtable<TDirect3dVertexBuffer>>
	{
	public:
		static void setCompatVtable(Vtable<TDirect3dVertexBuffer>& vtable);
	};
}

SET_COMPAT_VTABLE(IDirect3DVertexBufferVtbl, Direct3d::Direct3dVertexBuffer<IDirect3DVertexBuffer>);
SET_COMPAT_VTABLE(IDirect3DVertexBuffer7Vtbl, Direct3d::Direct3dVertexBuffer<IDirect3DVertexBuffer7>);
