/**
 * Copyright (C) 2015 Patrick Mours. All rights reserved.
 * License: https://github.com/crosire/d3d8to9#license
 *
 * Updated 2017 by Elisha Riedlinger
 */

#include "d3d8to9.hpp"

// IDirect3D8
HRESULT STDMETHODCALLTYPE Direct3D8::QueryInterface(REFIID riid, void **ppvObj)
{
	if (ppvObj == nullptr)
	{
		return E_POINTER;
	}

	if (riid == __uuidof(this) ||
		riid == __uuidof(IUnknown))
	{
		AddRef();

		*ppvObj = this;

		return S_OK;
	}

	return _proxy->QueryInterface(riid, ppvObj);
}
ULONG STDMETHODCALLTYPE Direct3D8::AddRef()
{
	return _proxy->AddRef();
}
ULONG STDMETHODCALLTYPE Direct3D8::Release()
{
	const auto ref = _proxy->Release();

	if (ref == 0)
	{
		delete this;
	}

	return ref;
}
HRESULT STDMETHODCALLTYPE Direct3D8::RegisterSoftwareDevice(void *pInitializeFunction)
{
	return _proxy->RegisterSoftwareDevice(pInitializeFunction);
}
UINT STDMETHODCALLTYPE Direct3D8::GetAdapterCount()
{
	return _proxy->GetAdapterCount();
}
HRESULT STDMETHODCALLTYPE Direct3D8::GetAdapterIdentifier(UINT Adapter, DWORD Flags, D3DADAPTER_IDENTIFIER8 *pIdentifier)
{
	if (pIdentifier == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	D3DADAPTER_IDENTIFIER9 identifier;

	if ((Flags & D3DENUM_NO_WHQL_LEVEL) == 0)
	{
		Flags |= D3DENUM_WHQL_LEVEL;
	}
	else
	{
		Flags ^= D3DENUM_NO_WHQL_LEVEL;
	}

	const auto hr = _proxy->GetAdapterIdentifier(Adapter, Flags, &identifier);

	if (FAILED(hr))
	{
		return hr;
	}

	convert_adapter_identifier(identifier, *pIdentifier);

	return D3D_OK;
}

static const D3DFORMAT AdapterFormats[] = { D3DFMT_A8R8G8B8, D3DFMT_X8R8G8B8, D3DFMT_R5G6B5, D3DFMT_X1R5G5B5, D3DFMT_A1R5G5B5, D3DFMT_A2R10G10B10 };
UINT STDMETHODCALLTYPE Direct3D8::GetAdapterModeCount(UINT Adapter)
{
	UINT count = 0;

	for (auto format : AdapterFormats)
	{
		count += _proxy->GetAdapterModeCount(Adapter, format);
	}

	return count;
}
HRESULT STDMETHODCALLTYPE Direct3D8::EnumAdapterModes(UINT Adapter, UINT Mode, D3DDISPLAYMODE *pMode)
{
	if (pMode == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	UINT offset = 0;

	for (auto format : AdapterFormats)
	{
		const auto modes = _proxy->GetAdapterModeCount(Adapter, format);

		if (modes == 0)
		{
			continue;
		}

		if (Mode < offset + modes)
		{
			return _proxy->EnumAdapterModes(Adapter, format, Mode - offset, pMode);
		}

		offset += modes;
	}

	return D3DERR_INVALIDCALL;
}
HRESULT STDMETHODCALLTYPE Direct3D8::GetAdapterDisplayMode(UINT Adapter, D3DDISPLAYMODE *pMode)
{
	return _proxy->GetAdapterDisplayMode(Adapter, pMode);
}
HRESULT STDMETHODCALLTYPE Direct3D8::CheckDeviceType(UINT Adapter, D3DDEVTYPE CheckType, D3DFORMAT DisplayFormat, D3DFORMAT BackBufferFormat, BOOL bWindowed)
{
	return _proxy->CheckDeviceType(Adapter, CheckType, DisplayFormat, BackBufferFormat, bWindowed);
}
HRESULT STDMETHODCALLTYPE Direct3D8::CheckDeviceFormat(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, DWORD Usage, D3DRESOURCETYPE RType, D3DFORMAT CheckFormat)
{
	return _proxy->CheckDeviceFormat(Adapter, DeviceType, AdapterFormat, Usage, RType, CheckFormat);
}
HRESULT STDMETHODCALLTYPE Direct3D8::CheckDeviceMultiSampleType(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SurfaceFormat, BOOL Windowed, D3DMULTISAMPLE_TYPE MultiSampleType)
{
	return _proxy->CheckDeviceMultiSampleType(Adapter, DeviceType, SurfaceFormat, Windowed, MultiSampleType, nullptr);
}
HRESULT STDMETHODCALLTYPE Direct3D8::CheckDepthStencilMatch(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, D3DFORMAT RenderTargetFormat, D3DFORMAT DepthStencilFormat)
{
	return _proxy->CheckDepthStencilMatch(Adapter, DeviceType, AdapterFormat, RenderTargetFormat, DepthStencilFormat);
}
HRESULT STDMETHODCALLTYPE Direct3D8::GetDeviceCaps(UINT Adapter, D3DDEVTYPE DeviceType, D3DCAPS8 *pCaps)
{
	if (pCaps == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	D3DCAPS9 caps;

	const auto hr = _proxy->GetDeviceCaps(Adapter, DeviceType, &caps);

	if (FAILED(hr))
	{
		return hr;
	}

	convert_caps(caps, *pCaps);

	return D3D_OK;
}
HMONITOR STDMETHODCALLTYPE Direct3D8::GetAdapterMonitor(UINT Adapter)
{
	return _proxy->GetAdapterMonitor(Adapter);
}
HRESULT STDMETHODCALLTYPE Direct3D8::CreateDevice(UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS8 *pPresentationParameters, Direct3DDevice8 **ppReturnedDeviceInterface)
{

#ifdef _DEBUG
	Compat::Log() << "Redirecting '" << "IDirect3D8::CreateDevice" << "(" << this << ", " << Adapter << ", " << DeviceType << ", " << hFocusWindow << ", " << BehaviorFlags << ", " << pPresentationParameters << ", " << ppReturnedDeviceInterface << ")' ...";
#endif

	if (pPresentationParameters == nullptr || ppReturnedDeviceInterface == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	*ppReturnedDeviceInterface = nullptr;

	D3DPRESENT_PARAMETERS pp;
	IDirect3DDevice9 *device = nullptr;

	convert_present_parameters(*pPresentationParameters, pp);

	const auto hr = _proxy->CreateDevice(Adapter, DeviceType, hFocusWindow, BehaviorFlags, &pp, &device);

	if (FAILED(hr))
	{
		return hr;
	}

	const auto device_proxy = new Direct3DDevice8(this, device, (pp.Flags & D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL) != 0);

	// Set default vertex declaration
	//device->SetFVF(D3DFVF_XYZ);		// ** Disabled to fix an issue with game Haegemonia Legions of Iron **
	//										 It seems there is no need to declare a defualt vertex here.
	//										 According to Microsoft documentation D3D8 applicaitons should
	//										 call SetVertexShader API before using a vertex and SetFVF
	//										 already called when an application calls SetVertexShader.
	//										 https://msdn.microsoft.com/en-us/library/windows/desktop/bb204851(v=vs.85).aspx#Vertex_Declaration_Changes

	// Set default render target
	IDirect3DSurface9 *rendertarget = nullptr, *depthstencil = nullptr;
	Direct3DSurface8 *rendertarget_proxy = nullptr, *depthstencil_proxy = nullptr;

	device->GetRenderTarget(0, &rendertarget);
	device->GetDepthStencilSurface(&depthstencil);

	if (rendertarget != nullptr)
	{
		rendertarget_proxy = new Direct3DSurface8(device_proxy, rendertarget);
	}
	if (depthstencil != nullptr)
	{
		depthstencil_proxy = new Direct3DSurface8(device_proxy, depthstencil);
	}

	device_proxy->SetRenderTarget(rendertarget_proxy, depthstencil_proxy);

	if (rendertarget_proxy != nullptr)
	{
		rendertarget_proxy->Release();
	}
	if (depthstencil_proxy != nullptr)
	{
		depthstencil_proxy->Release();
	}

	*ppReturnedDeviceInterface = device_proxy;

	return D3D_OK;
}
