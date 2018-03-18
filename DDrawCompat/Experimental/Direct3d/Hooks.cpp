#define CINTERFACE

#include <functional>

#include <d3d.h>

#include "Common/CompatPtr.h"
#include "Common/CompatRef.h"
#include "DDrawCompat\DDrawLog.h"
#include "DDraw/Repository.h"
#include "Direct3d/Direct3d.h"
#include "Direct3d/Direct3dDevice.h"
#include "Direct3d/Direct3dTexture.h"
#include "Direct3d/Direct3dVertexBuffer.h"
#include "Direct3d/Direct3dViewport.h"
#include "Direct3d/Hooks.h"
#include "Dll/Procs.h"

namespace
{
	void hookDirect3dDevice(CompatRef<IDirect3D3> d3d, CompatRef<IDirectDrawSurface4> renderTarget);
	void hookDirect3dDevice7(CompatRef<IDirect3D7> d3d, CompatRef<IDirectDrawSurface7> renderTarget);
	void hookDirect3dTexture(CompatRef<IDirectDraw> dd);
	void hookDirect3dVertexBuffer(CompatRef<IDirect3D3> d3d);
	void hookDirect3dVertexBuffer7(CompatRef<IDirect3D7> d3d);
	void hookDirect3dViewport(CompatRef<IDirect3D3> d3d);

	template <typename TDirect3dVertexBuffer>
	void hookVertexBuffer(
		const std::function<HRESULT(D3DVERTEXBUFFERDESC&, TDirect3dVertexBuffer*&)> createVertexBuffer);

	template <typename Interface>
	void hookVtable(const CompatPtr<Interface>& intf);

	template <typename TDirect3d, typename TDirectDraw>
	CompatPtr<TDirect3d> createDirect3d(CompatRef<TDirectDraw> dd)
	{
		CompatPtr<TDirect3d> d3d;
		HRESULT result = dd->QueryInterface(&dd, Compat::getIntfId<TDirect3d>(),
			reinterpret_cast<void**>(&d3d.getRef()));
		if (FAILED(result))
		{
			Compat::Log() << "Failed to create a Direct3D object for hooking: " << result;
		}
		return d3d;
	}

	CompatPtr<IDirectDrawSurface7> createRenderTarget(CompatRef<IDirectDraw7> dd)
	{
		DDSURFACEDESC2 desc = {};
		desc.dwSize = sizeof(desc);
		desc.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT | DDSD_CAPS;
		desc.dwWidth = 1;
		desc.dwHeight = 1;
		desc.ddpfPixelFormat.dwSize = sizeof(desc.ddpfPixelFormat);
		desc.ddpfPixelFormat.dwFlags = DDPF_RGB;
		desc.ddpfPixelFormat.dwRGBBitCount = 32;
		desc.ddpfPixelFormat.dwRBitMask = 0x00FF0000;
		desc.ddpfPixelFormat.dwGBitMask = 0x0000FF00;
		desc.ddpfPixelFormat.dwBBitMask = 0x000000FF;
		desc.ddsCaps.dwCaps = DDSCAPS_3DDEVICE | DDSCAPS_SYSTEMMEMORY;

		CompatPtr<IDirectDrawSurface7> renderTarget;
		HRESULT result = dd->CreateSurface(&dd, &desc, &renderTarget.getRef(), nullptr);
		if (FAILED(result))
		{
			Compat::Log() << "Failed to create a render target for hooking: " << result;
		}
		return renderTarget;
	}

	void hookDirect3d(CompatRef<IDirectDraw> dd, CompatRef<IDirectDrawSurface4> renderTarget)
	{
		CompatPtr<IDirect3D3> d3d(createDirect3d<IDirect3D3>(dd));
		if (d3d)
		{
			hookVtable<IDirect3D>(d3d);
			hookVtable<IDirect3D2>(d3d);
			hookVtable<IDirect3D3>(d3d);
			hookDirect3dDevice(*d3d, renderTarget);
			hookDirect3dTexture(dd);
			hookDirect3dVertexBuffer(*d3d);
			hookDirect3dViewport(*d3d);
		}
	}

	void hookDirect3d7(CompatRef<IDirectDraw7> dd)
	{
		CompatPtr<IDirect3D7> d3d(createDirect3d<IDirect3D7>(dd));
		if (d3d)
		{
			hookVtable<IDirect3D7>(d3d);
			hookDirect3dVertexBuffer7(*d3d);
		}
	}

	void hookDirect3dDevice(CompatRef<IDirect3D3> d3d, CompatRef<IDirectDrawSurface4> renderTarget)
	{
		CompatPtr<IDirect3DDevice3> d3dDevice;
		HRESULT result = d3d->CreateDevice(
			&d3d, IID_IDirect3DRGBDevice, &renderTarget, &d3dDevice.getRef(), nullptr);
		if (FAILED(result))
		{
			Compat::Log() << "Failed to create a Direct3D device for hooking: " << result;
			return;
		}

		hookVtable<IDirect3DDevice>(d3dDevice);
		hookVtable<IDirect3DDevice2>(d3dDevice);
		hookVtable<IDirect3DDevice3>(d3dDevice);
	}

	void hookDirect3dTexture(CompatRef<IDirectDraw> dd)
	{
		DDSURFACEDESC desc = {};
		desc.dwSize = sizeof(desc);
		desc.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
		desc.dwWidth = 1;
		desc.dwHeight = 1;
		desc.ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_SYSTEMMEMORY;

		CompatPtr<IDirectDrawSurface> texture;
		HRESULT result = dd->CreateSurface(&dd, &desc, &texture.getRef(), nullptr);
		if (FAILED(result))
		{
			Compat::Log() << "Failed to create a texture for hooking: " << result;
			return;
		}

		hookVtable<IDirect3DTexture>(texture);
		hookVtable<IDirect3DTexture2>(texture);
	}

	void hookDirect3dVertexBuffer(CompatRef<IDirect3D3> d3d)
	{
		hookVertexBuffer<IDirect3DVertexBuffer>(
			[&](D3DVERTEXBUFFERDESC& desc, IDirect3DVertexBuffer*& vb) {
			return d3d->CreateVertexBuffer(&d3d, &desc, &vb, 0, nullptr); });
	}

	void hookDirect3dVertexBuffer7(CompatRef<IDirect3D7> d3d)
	{
		hookVertexBuffer<IDirect3DVertexBuffer7>(
			[&](D3DVERTEXBUFFERDESC& desc, IDirect3DVertexBuffer7*& vb) {
			return d3d->CreateVertexBuffer(&d3d, &desc, &vb, 0); });
	}

	void hookDirect3dViewport(CompatRef<IDirect3D3> d3d)
	{
		CompatPtr<IDirect3DViewport3> viewport;
		HRESULT result = d3d->CreateViewport(&d3d, &viewport.getRef(), nullptr);
		if (FAILED(result))
		{
			Compat::Log() << "Failed to create a Direct3D viewport for hooking: " << result;
			return;
		}

		hookVtable<IDirect3DViewport>(viewport);
		hookVtable<IDirect3DViewport2>(viewport);
		hookVtable<IDirect3DViewport3>(viewport);
	}

	template <typename TDirect3dVertexBuffer>
	void hookVertexBuffer(
		const std::function<HRESULT(D3DVERTEXBUFFERDESC&, TDirect3dVertexBuffer*&)> createVertexBuffer)
	{
		D3DVERTEXBUFFERDESC desc = {};
		desc.dwSize = sizeof(desc);
		desc.dwCaps = D3DVBCAPS_SYSTEMMEMORY;
		desc.dwFVF = D3DFVF_VERTEX;
		desc.dwNumVertices = 1;

		CompatPtr<TDirect3dVertexBuffer> vertexBuffer = nullptr;
		HRESULT result = createVertexBuffer(desc, vertexBuffer.getRef());
		if (FAILED(result))
		{
			Compat::Log() << "Failed to create a vertex buffer for hooking: " << result;
		}

		hookVtable<TDirect3dVertexBuffer>(vertexBuffer);
	}

	template <typename Interface>
	void hookVtable(const CompatPtr<Interface>& intf)
	{
		CompatVtable<Vtable<Interface>>::hookVtable(intf.get()->lpVtbl);
	}
}

namespace Direct3d
{
	void installHooks()
	{
		auto dd7(DDraw::Repository::getDirectDraw());
		CompatPtr<IDirectDraw> dd;
		CALL_ORIG_PROC(DirectDrawCreate, nullptr, &dd.getRef(), nullptr);
		if (!dd || !dd7 || FAILED(dd->SetCooperativeLevel(dd, nullptr, DDSCL_NORMAL)))
		{
			Compat::Log() << "Failed to hook Direct3d interfaces";
			return;
		}

		CompatPtr<IDirectDrawSurface7> renderTarget7(createRenderTarget(*dd7));
		if (renderTarget7)
		{
			CompatPtr<IDirectDrawSurface4> renderTarget4(renderTarget7);
			hookDirect3d(*dd, *renderTarget4);
			hookDirect3d7(*dd7);
		}
	}
}
