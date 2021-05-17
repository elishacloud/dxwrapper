#define WIN32_LEAN_AND_MEAN
#define CINTERFACE

#include <DDrawCompat/v0.3.1/Common/CompatVtable.h>
#include <DDrawCompat/v0.3.1/D3dDdi/DeviceCallbacks.h>
#include <DDrawCompat/v0.3.1/D3dDdi/Visitors/DeviceCallbacksVisitor.h>

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
