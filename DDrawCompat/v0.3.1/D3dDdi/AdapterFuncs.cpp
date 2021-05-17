#define WIN32_LEAN_AND_MEAN
#define CINTERFACE

#include <guiddef.h>
#include <d3dnthal.h>

#include <DDrawCompat/v0.3.1/Common/CompatVtable.h>
#include <DDrawCompat/v0.3.1/D3dDdi/Adapter.h>
#include <DDrawCompat/v0.3.1/D3dDdi/AdapterFuncs.h>
#include <DDrawCompat/v0.3.1/D3dDdi/DeviceCallbacks.h>
#include <DDrawCompat/v0.3.1/D3dDdi/DeviceFuncs.h>
#include <DDrawCompat/v0.3.1/D3dDdi/ScopedCriticalSection.h>
#include <DDrawCompat/v0.3.1/D3dDdi/Visitors/AdapterFuncsVisitor.h>

namespace
{
	template <auto adapterMethod, typename... Params>
	HRESULT WINAPI adapterFunc(HANDLE adapter, Params... params)
	{
		return (D3dDdi::Adapter::get(adapter).*adapterMethod)(params...);
	}

	const D3DDDI_ADAPTERFUNCS& getOrigVtable(HANDLE adapter)
	{
		return D3dDdi::Adapter::get(adapter).getOrigVtable();
	}

	constexpr void setCompatVtable(D3DDDI_ADAPTERFUNCS& vtable)
	{
#define SET_ADAPTER_FUNC(func) vtable.func = &adapterFunc<&D3dDdi::Adapter::func>

		SET_ADAPTER_FUNC(pfnCloseAdapter);
		SET_ADAPTER_FUNC(pfnCreateDevice);
		SET_ADAPTER_FUNC(pfnGetCaps);
	}
}

namespace D3dDdi
{
	namespace AdapterFuncs
	{
		void hookVtable(const D3DDDI_ADAPTERFUNCS& vtable, UINT version)
		{
			CompatVtable<D3DDDI_ADAPTERFUNCS>::s_origVtable = {};
			CompatVtable<D3DDDI_ADAPTERFUNCS>::hookVtable<ScopedCriticalSection>(vtable, version);
		}
	}
}
