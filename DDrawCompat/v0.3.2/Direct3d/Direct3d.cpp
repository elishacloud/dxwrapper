#define WIN32_LEAN_AND_MEAN
#define CINTERFACE

#include <set>

#include <DDrawCompat/v0.3.2/Common/CompatPtr.h>
#include <DDrawCompat/v0.3.2/Common/CompatVtable.h>
#include <DDrawCompat/v0.3.2/DDraw/ScopedThreadLock.h>
#include <DDrawCompat/v0.3.2/DDraw/Surfaces/Surface.h>
#include <DDrawCompat/v0.3.2/Direct3d/Direct3d.h>
#include <DDrawCompat/v0.3.2/Direct3d/Direct3dDevice.h>
#include <DDrawCompat/v0.3.2/Direct3d/Direct3dVertexBuffer.h>
#include <DDrawCompat/v0.3.2/Direct3d/Visitors/Direct3dVtblVisitor.h>

namespace
{
	template <typename TDirect3d, typename TDirectDrawSurface, typename TDirect3dDevice, typename... Params>
	HRESULT STDMETHODCALLTYPE createDevice(
		TDirect3d* This,
		REFCLSID rclsid,
		TDirectDrawSurface* lpDDS,
		TDirect3dDevice** lplpD3DDevice,
		Params... params)
	{
		auto iid = (IID_IDirect3DRampDevice == rclsid) ? &IID_IDirect3DRGBDevice : &rclsid;
		HRESULT result = getOrigVtable(This).CreateDevice(This, *iid, lpDDS, lplpD3DDevice, params...);
		if (DDERR_INVALIDOBJECT == result && lpDDS)
		{
			auto surface = DDraw::Surface::getSurface(*lpDDS);
			if (surface)
			{
				surface->setSizeOverride(1, 1);
				result = getOrigVtable(This).CreateDevice(This, *iid, lpDDS, lplpD3DDevice, params...);
				surface->setSizeOverride(0, 0);
			}
		}

		if constexpr (std::is_same_v<TDirect3d, IDirect3D7>)
		{
			if (SUCCEEDED(result))
			{
				Direct3d::Direct3dDevice::hookVtable(*(*lplpD3DDevice)->lpVtbl);
			}
		}
		return result;
	}

	HRESULT STDMETHODCALLTYPE createVertexBuffer(
		IDirect3D7* This,
		LPD3DVERTEXBUFFERDESC lpVBDesc,
		LPDIRECT3DVERTEXBUFFER7* lplpD3DVertexBuffer,
		DWORD dwFlags)
	{
		HRESULT result = getOrigVtable(This).CreateVertexBuffer(This, lpVBDesc, lplpD3DVertexBuffer, dwFlags);
		if (SUCCEEDED(result))
		{
			Direct3d::Direct3dVertexBuffer::hookVtable(*(*lplpD3DVertexBuffer)->lpVtbl);
		}
		return result;
	}

	template <typename Vtable>
	constexpr void setCompatVtable(Vtable& vtable)
	{
		if constexpr (!std::is_same_v<Vtable, IDirect3DVtbl>)
		{
			vtable.CreateDevice = &createDevice;
		}

		if constexpr (std::is_same_v<Vtable, IDirect3D7Vtbl>)
		{
			vtable.CreateVertexBuffer = &createVertexBuffer;
		}
	}
}

namespace Direct3d
{
	namespace Direct3d
	{
		template <typename Vtable>
		void hookVtable(const Vtable& vtable)
		{
			CompatVtable<Vtable>::hookVtable<DDraw::ScopedThreadLock>(vtable);
		}

		template void hookVtable(const IDirect3DVtbl&);
		template void hookVtable(const IDirect3D2Vtbl&);
		template void hookVtable(const IDirect3D3Vtbl&);
		template void hookVtable(const IDirect3D7Vtbl&);
	}
}
