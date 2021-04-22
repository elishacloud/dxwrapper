#define WIN32_LEAN_AND_MEAN
#define CINTERFACE

#include <Common/CompatVtable.h>
#include <D3dDdi/AdapterCallbacks.h>
#include <D3dDdi/Visitors/AdapterCallbacksVisitor.h>

namespace
{
	const D3DDDI_ADAPTERCALLBACKS& getOrigVtable(HANDLE /*adapter*/)
	{
		return CompatVtable<D3DDDI_ADAPTERCALLBACKS>::s_origVtable;
	}

	constexpr void setCompatVtable(D3DDDI_ADAPTERCALLBACKS& /*vtable*/)
	{
	}
}

namespace D3dDdi
{
	namespace AdapterCallbacks
	{
		void hookVtable(const D3DDDI_ADAPTERCALLBACKS& vtable, UINT version)
		{
			CompatVtable<D3DDDI_ADAPTERCALLBACKS>::hookCallbackVtable(vtable, version);
		}
	}
}
