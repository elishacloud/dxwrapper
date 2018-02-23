#include "Direct3d/Direct3dViewport.h"

namespace Direct3d
{
	template <typename TDirect3dViewport>
	void Direct3dViewport<TDirect3dViewport>::setCompatVtable(Vtable<TDirect3dViewport>& /*vtable*/)
	{
	}

	template Direct3dViewport<IDirect3DViewport>;
	template Direct3dViewport<IDirect3DViewport2>;
	template Direct3dViewport<IDirect3DViewport3>;
}
