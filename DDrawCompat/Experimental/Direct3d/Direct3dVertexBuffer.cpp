#include "Direct3d/Direct3dVertexBuffer.h"

namespace Direct3d
{
	template <typename TDirect3dVertexBuffer>
	void Direct3dVertexBuffer<TDirect3dVertexBuffer>::setCompatVtable(
		Vtable<TDirect3dVertexBuffer>& /*vtable*/)
	{
	}

	template Direct3dVertexBuffer<IDirect3DVertexBuffer>;
	template Direct3dVertexBuffer<IDirect3DVertexBuffer7>;
}
