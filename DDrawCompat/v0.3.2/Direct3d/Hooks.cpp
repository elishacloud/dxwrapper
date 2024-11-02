#define WIN32_LEAN_AND_MEAN
#define CINTERFACE

#include <d3d.h>

#include <DDrawCompat/v0.3.2/Common/CompatRef.h>
#include <DDrawCompat/DDrawLog.h>
#include <DDrawCompat/v0.3.2/Direct3d/Direct3d.h>
#include <DDrawCompat/v0.3.2/Direct3d/Direct3dDevice.h>
#include <DDrawCompat/v0.3.2/Direct3d/Direct3dExecuteBuffer.h>
#include <DDrawCompat/v0.3.2/Direct3d/Direct3dLight.h>
#include <DDrawCompat/v0.3.2/Direct3d/Direct3dMaterial.h>
#include <DDrawCompat/v0.3.2/Direct3d/Direct3dTexture.h>
#include <DDrawCompat/v0.3.2/Direct3d/Direct3dVertexBuffer.h>
#include <DDrawCompat/v0.3.2/Direct3d/Direct3dViewport.h>
#include <DDrawCompat/v0.3.2/Direct3d/Hooks.h>

namespace
{
	void hookDirect3dDevice(CompatRef<IDirect3D3> d3d, CompatRef<IDirectDrawSurface4> renderTarget);
	void hookDirect3dExecuteBuffer(CompatRef<IDirect3DDevice> dev);
	void hookDirect3dLight(CompatRef<IDirect3D3> d3d);
	void hookDirect3dMaterial(CompatRef<IDirect3D3> d3d);
	void hookDirect3dTexture(CompatRef<IDirectDraw> dd);
	void hookDirect3dVertexBuffer(CompatRef<IDirect3D3> d3d);
	void hookDirect3dViewport(CompatRef<IDirect3D3> d3d);

	template <typename TDirect3d, typename TDirectDraw>
	CompatPtr<TDirect3d> createDirect3d(CompatRef<TDirectDraw> dd)
	{
		CompatPtr<TDirect3d> d3d;
		HRESULT result = dd->QueryInterface(&dd, Compat32::getIntfId<TDirect3d>(),
			reinterpret_cast<void**>(&d3d.getRef()));
		if (SUCCEEDED(result))
		{
			CompatVtable<Vtable<TDirect3d>>::s_origVtable = *d3d.get()->lpVtbl;
		}
		else
		{
			Compat32::Log() << "ERROR: Failed to create a Direct3D object for hooking: " << Compat32::hex(result);
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
		desc.ddsCaps.dwCaps = DDSCAPS_3DDEVICE;

		CompatPtr<IDirectDrawSurface7> renderTarget;
		HRESULT result = dd->CreateSurface(&dd, &desc, &renderTarget.getRef(), nullptr);
		if (FAILED(result))
		{
			Compat32::Log() << "ERROR: Failed to create a render target for hooking: " << Compat32::hex(result);
		}
		return renderTarget;
	}

	void hookDirect3d(CompatRef<IDirectDraw> dd, CompatRef<IDirectDrawSurface4> renderTarget)
	{
		CompatPtr<IDirect3D3> d3d(createDirect3d<IDirect3D3>(dd));
		if (d3d)
		{
			CompatVtable<IDirect3D3Vtbl>::s_origVtable = *d3d.get()->lpVtbl;
			Direct3d::Direct3d::hookVtable(*CompatPtr<IDirect3D>(d3d).get()->lpVtbl);
			Direct3d::Direct3d::hookVtable(*CompatPtr<IDirect3D2>(d3d).get()->lpVtbl);
			Direct3d::Direct3d::hookVtable(*CompatPtr<IDirect3D3>(d3d).get()->lpVtbl);
			hookDirect3dDevice(*d3d, renderTarget);
			hookDirect3dLight(*d3d);
			hookDirect3dMaterial(*d3d);
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
			Direct3d::Direct3d::hookVtable(*d3d.get()->lpVtbl);
		}
	}

	template <typename TDirect3dDevice>
	void hookDirect3dDevice(const CompatPtr<TDirect3dDevice>& d3d)
	{
		Direct3d::Direct3dDevice::hookVtable(*d3d.get()->lpVtbl);
	}

	void hookDirect3dDevice(CompatRef<IDirect3D3> d3d, CompatRef<IDirectDrawSurface4> renderTarget)
	{
		CompatPtr<IDirect3DDevice3> d3dDevice;
		HRESULT result = d3d->CreateDevice(
			&d3d, IID_IDirect3DRGBDevice, &renderTarget, &d3dDevice.getRef(), nullptr);
		if (FAILED(result))
		{
			Compat32::Log() << "ERROR: Failed to create a Direct3D device for hooking: " << Compat32::hex(result);
			return;
		}

		CompatVtable<IDirect3DDevice3Vtbl>::s_origVtable = *d3dDevice.get()->lpVtbl;
		Direct3d::Direct3dDevice::hookVtable(*CompatPtr<IDirect3DDevice>(d3dDevice).get()->lpVtbl);
		Direct3d::Direct3dDevice::hookVtable(*CompatPtr<IDirect3DDevice2>(d3dDevice).get()->lpVtbl);
		Direct3d::Direct3dDevice::hookVtable(*CompatPtr<IDirect3DDevice3>(d3dDevice).get()->lpVtbl);

		CompatPtr<IDirect3DDevice> dev(d3dDevice);
		if (dev)
		{
			hookDirect3dExecuteBuffer(*dev);
		}
	}

	void hookDirect3dExecuteBuffer(CompatRef<IDirect3DDevice> dev)
	{
		D3DEXECUTEBUFFERDESC desc = {};
		desc.dwSize = sizeof(desc);
		desc.dwFlags = D3DDEB_BUFSIZE;
		desc.dwBufferSize = 1;

		CompatPtr<IDirect3DExecuteBuffer> buffer;
		HRESULT result = dev->CreateExecuteBuffer(&dev, &desc, &buffer.getRef(), nullptr);
		if (FAILED(result))
		{
			Compat32::Log() << "ERROR: Failed to create an execute buffer for hooking: " << Compat32::hex(result);
			return;
		}

		Direct3d::Direct3dExecuteBuffer::hookVtable(*buffer.get()->lpVtbl);
	}

	void hookDirect3dLight(CompatRef<IDirect3D3> d3d)
	{
		CompatPtr<IDirect3DLight> light;
		HRESULT result = d3d->CreateLight(&d3d, &light.getRef(), nullptr);
		if (FAILED(result))
		{
			Compat32::Log() << "ERROR: Failed to create a light for hooking: " << Compat32::hex(result);
			return;
		}

		Direct3d::Direct3dLight::hookVtable(*light.get()->lpVtbl);
	}

	void hookDirect3dMaterial(CompatRef<IDirect3D3> d3d)
	{
		CompatPtr<IDirect3DMaterial3> material;
		HRESULT result = d3d->CreateMaterial(&d3d, &material.getRef(), nullptr);
		if (FAILED(result))
		{
			Compat32::Log() << "ERROR: Failed to create a material for hooking: " << Compat32::hex(result);
			return;
		}

		CompatVtable<IDirect3DMaterial3Vtbl>::s_origVtable = *material.get()->lpVtbl;
		Direct3d::Direct3dMaterial::hookVtable(*CompatPtr<IDirect3DMaterial>(material).get()->lpVtbl);
		Direct3d::Direct3dMaterial::hookVtable(*CompatPtr<IDirect3DMaterial2>(material).get()->lpVtbl);
		Direct3d::Direct3dMaterial::hookVtable(*CompatPtr<IDirect3DMaterial3>(material).get()->lpVtbl);
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
			Compat32::Log() << "ERROR: Failed to create a texture for hooking: " << Compat32::hex(result);
			return;
		}

		Direct3d::Direct3dTexture::hookVtable(*CompatPtr<IDirect3DTexture>(texture).get()->lpVtbl);
		Direct3d::Direct3dTexture::hookVtable(*CompatPtr<IDirect3DTexture2>(texture).get()->lpVtbl);
	}

	void hookDirect3dVertexBuffer(CompatRef<IDirect3D3> d3d)
	{
		D3DVERTEXBUFFERDESC desc = {};
		desc.dwSize = sizeof(desc);
		desc.dwCaps = D3DVBCAPS_SYSTEMMEMORY;
		desc.dwFVF = D3DFVF_VERTEX;
		desc.dwNumVertices = 1;

		CompatPtr<IDirect3DVertexBuffer> vertexBuffer = nullptr;
		HRESULT result = d3d->CreateVertexBuffer(&d3d, &desc, &vertexBuffer.getRef(), 0, nullptr);
		if (FAILED(result))
		{
			Compat32::Log() << "ERROR: Failed to create a vertex buffer for hooking: " << Compat32::hex(result);
		}

		Direct3d::Direct3dVertexBuffer::hookVtable(*vertexBuffer.get()->lpVtbl);
	}

	void hookDirect3dViewport(CompatRef<IDirect3D3> d3d)
	{
		CompatPtr<IDirect3DViewport3> viewport;
		HRESULT result = d3d->CreateViewport(&d3d, &viewport.getRef(), nullptr);
		if (FAILED(result))
		{
			Compat32::Log() << "ERROR: Failed to create a Direct3D viewport for hooking: " << Compat32::hex(result);
			return;
		}

		CompatVtable<IDirect3DViewport3Vtbl>::s_origVtable = *viewport.get()->lpVtbl;
		Direct3d::Direct3dViewport::hookVtable(*CompatPtr<IDirect3DViewport>(viewport).get()->lpVtbl);
		Direct3d::Direct3dViewport::hookVtable(*CompatPtr<IDirect3DViewport2>(viewport).get()->lpVtbl);
		Direct3d::Direct3dViewport::hookVtable(*CompatPtr<IDirect3DViewport3>(viewport).get()->lpVtbl);
	}
}

namespace Direct3d
{
	void installHooks(CompatPtr<IDirectDraw> dd, CompatPtr<IDirectDraw7> dd7)
	{
		CompatPtr<IDirectDrawSurface7> renderTarget7(createRenderTarget(*dd7));
		if (renderTarget7)
		{
			CompatPtr<IDirectDrawSurface4> renderTarget4(renderTarget7);
			hookDirect3d(*dd, *renderTarget4);
			hookDirect3d7(*dd7);
		}
	}
}
