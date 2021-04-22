#define WIN32_LEAN_AND_MEAN
#define CINTERFACE

#include <Common/CompatVtable.h>
#include <D3dDdi/DeviceCallbacks.h>
#include <D3dDdi/Visitors/DeviceCallbacksVisitor.h>

namespace
{
	const D3DDDI_DEVICECALLBACKS& getOrigVtable(HANDLE /*device*/)
	{
		return CompatVtable<D3DDDI_DEVICECALLBACKS>::s_origVtable;
	}

	constexpr void setCompatVtable(D3DDDI_DEVICECALLBACKS& /*vtable*/)
	{
	}
}

namespace D3dDdi
{
	namespace DeviceCallbacks
	{
		void hookVtable(const D3DDDI_DEVICECALLBACKS & vtable, UINT version)
		{
			CompatVtable<D3DDDI_DEVICECALLBACKS>::hookCallbackVtable(vtable, version);
		}
	}
}
