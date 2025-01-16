#define WIN32_LEAN_AND_MEAN
#define CINTERFACE

#include <DDrawCompat/v0.3.2/Common/CompatVtable.h>
#include <DDrawCompat/v0.3.2/D3dDdi/Device.h>
#include <DDrawCompat/v0.3.2/D3dDdi/DeviceFuncs.h>
#include <DDrawCompat/v0.3.2/D3dDdi/ScopedCriticalSection.h>
#include <DDrawCompat/v0.3.2/D3dDdi/Visitors/DeviceFuncsVisitor.h>

namespace
{
	template <auto deviceMethod, typename... Params>
	HRESULT WINAPI deviceFunc(HANDLE device, Params... params)
	{
		return (D3dDdi::Device::get(device).*deviceMethod)(params...);
	}

	template <auto deviceMethod, typename... Params>
	HRESULT WINAPI deviceStateFunc(HANDLE device, Params... params)
	{
		return (D3dDdi::Device::get(device).getState().*deviceMethod)(params...);
	}

	template <auto memberPtr, typename... Params>
	HRESULT WINAPI flushPrimitivesFunc(HANDLE hDevice, Params... params)
	{
		auto& device = D3dDdi::Device::get(hDevice);
		device.flushPrimitives();
		return (device.getOrigVtable().*memberPtr)(hDevice, params...);
	}

	template <auto memberPtr, typename... Params>
	auto WINAPI origDeviceFunc(HANDLE device, Params... params)
	{
		return (D3dDdi::Device::get(device).getOrigVtable().*memberPtr)(device, params...);
	}

	template <auto memberPtr>
	constexpr auto getCompatFunc(D3DDDI_DEVICEFUNCS*)
	{
		auto func = getCompatVtable<D3DDDI_DEVICEFUNCS>().*memberPtr;
		if (!func)
		{
			func = &origDeviceFunc<memberPtr>;
		}
		return func;
	}

	constexpr void setCompatVtable(D3DDDI_DEVICEFUNCS& vtable)
	{
#define SET_DEVICE_FUNC(func) vtable.func = &deviceFunc<&D3dDdi::Device::func>
#define SET_DEVICE_STATE_FUNC(func) vtable.func = &deviceStateFunc<&D3dDdi::DeviceState::func>
#define SET_FLUSH_PRIMITIVES_FUNC(func) vtable.func = &flushPrimitivesFunc<&D3DDDI_DEVICEFUNCS::func>

		SET_DEVICE_FUNC(pfnBlt);
		SET_DEVICE_FUNC(pfnClear);
		SET_DEVICE_FUNC(pfnColorFill);
		SET_DEVICE_FUNC(pfnCreateResource);
		SET_DEVICE_FUNC(pfnCreateResource2);
		SET_DEVICE_FUNC(pfnDestroyDevice);
		SET_DEVICE_FUNC(pfnDestroyResource);
		SET_DEVICE_FUNC(pfnDrawIndexedPrimitive2);
		SET_DEVICE_FUNC(pfnDrawPrimitive);
		SET_DEVICE_FUNC(pfnFlush);
		SET_DEVICE_FUNC(pfnFlush1);
		SET_DEVICE_FUNC(pfnLock);
		SET_DEVICE_FUNC(pfnOpenResource);
		SET_DEVICE_FUNC(pfnPresent);
		SET_DEVICE_FUNC(pfnPresent1);
		SET_DEVICE_FUNC(pfnSetRenderTarget);
		SET_DEVICE_FUNC(pfnSetStreamSource);
		SET_DEVICE_FUNC(pfnSetStreamSourceUm);
		SET_DEVICE_FUNC(pfnUnlock);

		SET_DEVICE_STATE_FUNC(pfnCreateVertexShaderDecl);
		SET_DEVICE_STATE_FUNC(pfnDeletePixelShader);
		SET_DEVICE_STATE_FUNC(pfnDeleteVertexShaderDecl);
		SET_DEVICE_STATE_FUNC(pfnDeleteVertexShaderFunc);
		SET_DEVICE_STATE_FUNC(pfnSetPixelShader);
		SET_DEVICE_STATE_FUNC(pfnSetPixelShaderConst);
		SET_DEVICE_STATE_FUNC(pfnSetPixelShaderConstB);
		SET_DEVICE_STATE_FUNC(pfnSetPixelShaderConstI);
		SET_DEVICE_STATE_FUNC(pfnSetRenderState);
		SET_DEVICE_STATE_FUNC(pfnSetTexture);
		SET_DEVICE_STATE_FUNC(pfnSetTextureStageState);
		SET_DEVICE_STATE_FUNC(pfnSetVertexShaderConst);
		SET_DEVICE_STATE_FUNC(pfnSetVertexShaderConstB);
		SET_DEVICE_STATE_FUNC(pfnSetVertexShaderConstI);
		SET_DEVICE_STATE_FUNC(pfnSetVertexShaderDecl);
		SET_DEVICE_STATE_FUNC(pfnSetVertexShaderFunc);
		SET_DEVICE_STATE_FUNC(pfnSetZRange);
		SET_DEVICE_STATE_FUNC(pfnUpdateWInfo);

		SET_FLUSH_PRIMITIVES_FUNC(pfnBufBlt);
		SET_FLUSH_PRIMITIVES_FUNC(pfnBufBlt1);
		SET_FLUSH_PRIMITIVES_FUNC(pfnDepthFill);
		SET_FLUSH_PRIMITIVES_FUNC(pfnDiscard);
		SET_FLUSH_PRIMITIVES_FUNC(pfnGenerateMipSubLevels);
		SET_FLUSH_PRIMITIVES_FUNC(pfnSetClipPlane);
		SET_FLUSH_PRIMITIVES_FUNC(pfnSetDepthStencil);
		SET_FLUSH_PRIMITIVES_FUNC(pfnSetPalette);
		SET_FLUSH_PRIMITIVES_FUNC(pfnSetScissorRect);
		SET_FLUSH_PRIMITIVES_FUNC(pfnSetViewport);
		SET_FLUSH_PRIMITIVES_FUNC(pfnStateSet);
		SET_FLUSH_PRIMITIVES_FUNC(pfnTexBlt);
		SET_FLUSH_PRIMITIVES_FUNC(pfnTexBlt1);
		SET_FLUSH_PRIMITIVES_FUNC(pfnUpdatePalette);
	}
}

namespace D3dDdi
{
	namespace DeviceFuncs
	{
		void hookVtable(const D3DDDI_DEVICEFUNCS& vtable, UINT version)
		{
			CompatVtable<D3DDDI_DEVICEFUNCS>::s_origVtable = {};
			CompatVtable<D3DDDI_DEVICEFUNCS>::hookVtable<ScopedCriticalSection>(vtable, version);
		}
	}
}
