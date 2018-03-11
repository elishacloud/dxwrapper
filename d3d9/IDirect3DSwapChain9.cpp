/**
* Copyright (C) 2018 Elisha Riedlinger
*
* This software is  provided 'as-is', without any express  or implied  warranty. In no event will the
* authors be held liable for any damages arising from the use of this software.
* Permission  is granted  to anyone  to use  this software  for  any  purpose,  including  commercial
* applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*   1. The origin of this software must not be misrepresented; you must not claim that you  wrote the
*      original  software. If you use this  software  in a product, an  acknowledgment in the product
*      documentation would be appreciated but is not required.
*   2. Altered source versions must  be plainly  marked as such, and  must not be  misrepresented  as
*      being the original software.
*   3. This notice may not be removed or altered from any source distribution.
*/

#include "d3d9.h"

HRESULT m_IDirect3DSwapChain9::QueryInterface(THIS_ REFIID riid, void** ppvObj)
{
	if ((riid == IID_IDirect3DSwapChain9 || riid == IID_IUnknown) && ppvObj)
	{
		AddRef();

		*ppvObj = this;

		return S_OK;
	}

	return ProxyInterface->QueryInterface(riid, ppvObj);
}

ULONG m_IDirect3DSwapChain9::AddRef(THIS)
{
	return ProxyInterface->AddRef();
}

ULONG m_IDirect3DSwapChain9::Release(THIS)
{
	return ProxyInterface->Release();
}

HRESULT m_IDirect3DSwapChain9::Present(THIS_ CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion, DWORD dwFlags)
{
	return ProxyInterface->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion, dwFlags);
}

HRESULT m_IDirect3DSwapChain9::GetFrontBufferData(THIS_ IDirect3DSurface9* pDestSurface)
{
	if (pDestSurface)
	{
		pDestSurface = static_cast<m_IDirect3DSurface9 *>(pDestSurface)->GetProxyInterface();
	}

	return GetFrontBufferData(pDestSurface);
}

HRESULT m_IDirect3DSwapChain9::GetBackBuffer(THIS_ UINT BackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface9** ppBackBuffer)
{
	HRESULT hr = ProxyInterface->GetBackBuffer(BackBuffer, Type, ppBackBuffer);

	if (SUCCEEDED(hr))
	{
		if (m_pDevice)
		{
			*ppBackBuffer = m_pDevice->ProxyAddressLookupTable->FindAddress<m_IDirect3DSurface9>(*ppBackBuffer);
		}
		else if (m_pDeviceEx)
		{
			*ppBackBuffer = m_pDeviceEx->ProxyAddressLookupTable->FindAddress<m_IDirect3DSurface9>(*ppBackBuffer);
		}
	}

	return hr;
}

HRESULT m_IDirect3DSwapChain9::GetRasterStatus(THIS_ D3DRASTER_STATUS* pRasterStatus)
{
	return GetRasterStatus(pRasterStatus);
}

HRESULT m_IDirect3DSwapChain9::GetDisplayMode(THIS_ D3DDISPLAYMODE* pMode)
{
	return GetDisplayMode(pMode);
}

HRESULT m_IDirect3DSwapChain9::GetDevice(THIS_ IDirect3DDevice9** ppDevice)
{
	if (ppDevice)
	{
		if (m_pDevice)
		{
			m_pDevice->AddRef();

			*ppDevice = m_pDevice;

			return D3D_OK;
		}
		else if (m_pDeviceEx)
		{
			m_pDeviceEx->AddRef();

			*ppDevice = m_pDeviceEx;

			return D3D_OK;
		}
	}
	return D3DERR_INVALIDCALL;
}

HRESULT m_IDirect3DSwapChain9::GetPresentParameters(THIS_ D3DPRESENT_PARAMETERS* pPresentationParameters)
{
	return GetPresentParameters(pPresentationParameters);
}
