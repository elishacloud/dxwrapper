#include "Direct3d/Direct3dTexture.h"

namespace Direct3d
{
	template <typename TDirect3dTexture>
	void Direct3dTexture<TDirect3dTexture>::setCompatVtable(Vtable<TDirect3dTexture>& /*vtable*/)
	{
	}

	template Direct3dTexture<IDirect3DTexture>;
	template Direct3dTexture<IDirect3DTexture2>;
}
