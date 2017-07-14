/**
 * Copyright (C) 2015 Patrick Mours. All rights reserved.
 * License: https://github.com/crosire/d3d8to9#license
 */

#include "d3d8to9.hpp"

// IDirect3DSwapChain8
Direct3DSwapChain8::Direct3DSwapChain8(Direct3DDevice8 *Device, IDirect3DSwapChain9 *ProxyInterface) :
	Device(Device), ProxyInterface(ProxyInterface)
{
	Device->AddRef();
	Device->ProxyAddressLookupTable->SaveAddress(this, ProxyInterface);
}
Direct3DSwapChain8::~Direct3DSwapChain8()
{
	if (CleanUpFlag)
	{
		Device->ProxyAddressLookupTable->DeleteAddress(this);
		if (Active)
		{
			Active = false;
			Device->Release();
		}
	}
}

HRESULT STDMETHODCALLTYPE Direct3DSwapChain8::QueryInterface(REFIID riid, void **ppvObj)
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

	return ProxyInterface->QueryInterface(riid, ppvObj);
}
ULONG STDMETHODCALLTYPE Direct3DSwapChain8::AddRef()
{
	return ProxyInterface->AddRef();
}
ULONG STDMETHODCALLTYPE Direct3DSwapChain8::Release()
{
	const ULONG LastRefCount = ProxyInterface->Release();

	if (LastRefCount == 0)
	{
		if (Active)
		{
			Active = false;
			Device->Release();
		}
		//delete this;
	}

	return LastRefCount;
}

HRESULT STDMETHODCALLTYPE Direct3DSwapChain8::Present(const RECT *pSourceRect, const RECT *pDestRect, HWND hDestWindowOverride, const RGNDATA *pDirtyRegion)
{
	UNREFERENCED_PARAMETER(pDirtyRegion);

	return ProxyInterface->Present(pSourceRect, pDestRect, hDestWindowOverride, nullptr, 0);
}
HRESULT STDMETHODCALLTYPE Direct3DSwapChain8::GetBackBuffer(UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, Direct3DSurface8 **ppBackBuffer)
{
	if (ppBackBuffer == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	*ppBackBuffer = nullptr;

	IDirect3DSurface9 *SurfaceInterface = nullptr;

	const HRESULT hr = ProxyInterface->GetBackBuffer(iBackBuffer, Type, &SurfaceInterface);

	if (FAILED(hr))
	{
		return hr;
	}

	*ppBackBuffer = Device->ProxyAddressLookupTable->FindAddress(SurfaceInterface);

	return D3D_OK;
}
