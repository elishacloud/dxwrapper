/**
 * Copyright (C) 2015 Patrick Mours. All rights reserved.
 * License: https://github.com/crosire/d3d8to9#license
 */

#include "d3d8to9.hpp"

// IDirect3DSwapChain8
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

	return _proxy->QueryInterface(riid, ppvObj);
}
ULONG STDMETHODCALLTYPE Direct3DSwapChain8::AddRef()
{
	return _proxy->AddRef();
}
ULONG STDMETHODCALLTYPE Direct3DSwapChain8::Release()
{
	const auto ref = _proxy->Release();

	if (ref == 0)
	{
		delete this;
	}

	return ref;
}
HRESULT STDMETHODCALLTYPE Direct3DSwapChain8::Present(CONST RECT *pSourceRect, CONST RECT *pDestRect, HWND hDestWindowOverride, CONST RGNDATA *pDirtyRegion)
{
	UNREFERENCED_PARAMETER(pDirtyRegion);

	return _proxy->Present(pSourceRect, pDestRect, hDestWindowOverride, nullptr, 0);
}
HRESULT STDMETHODCALLTYPE Direct3DSwapChain8::GetBackBuffer(UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, Direct3DSurface8 **ppBackBuffer)
{
	if (ppBackBuffer == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	*ppBackBuffer = nullptr;

	IDirect3DSurface9 *surface = nullptr;

	const auto hr = _proxy->GetBackBuffer(iBackBuffer, Type, &surface);

	if (FAILED(hr))
	{
		return hr;
	}

	*ppBackBuffer = new Direct3DSurface8(_device, surface);

	return D3D_OK;
}
