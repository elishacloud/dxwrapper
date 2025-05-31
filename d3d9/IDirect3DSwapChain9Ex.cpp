/**
* Copyright (C) 2025 Elisha Riedlinger
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

HRESULT m_IDirect3DSwapChain9Ex::QueryInterface(THIS_ REFIID riid, void** ppvObj)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ") " << riid;

	if (riid == IID_IUnknown || riid == WrapperID)
	{
		HRESULT hr = ProxyInterface->QueryInterface(WrapperID, ppvObj);

		if (SUCCEEDED(hr))
		{
			*ppvObj = this;
		}

		return hr;
	}

	HRESULT hr = ProxyInterface->QueryInterface(riid, ppvObj);

	if (SUCCEEDED(hr))
	{
		D3d9Wrapper::genericQueryInterface(riid, ppvObj, m_pDeviceEx);
	}

	return hr;
}

ULONG m_IDirect3DSwapChain9Ex::AddRef(THIS)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->AddRef();
}

ULONG m_IDirect3DSwapChain9Ex::Release(THIS)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	ULONG ref = ProxyInterface->Release();

	if (ref == 0)
	{
		m_pDeviceEx->GetLookupTable()->DeleteAddress(this);

		delete this;
	}

	return ref;
}

HRESULT m_IDirect3DSwapChain9Ex::Present(THIS_ CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion, dwFlags);
}

HRESULT m_IDirect3DSwapChain9Ex::GetFrontBufferData(THIS_ IDirect3DSurface9* pDestSurface)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (pDestSurface)
	{
		pDestSurface = static_cast<m_IDirect3DSurface9 *>(pDestSurface)->GetProxyInterface();
	}

	return ProxyInterface->GetFrontBufferData(pDestSurface);
}

HRESULT m_IDirect3DSwapChain9Ex::GetBackBuffer(THIS_ UINT BackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface9** ppBackBuffer)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	HRESULT hr = ProxyInterface->GetBackBuffer(BackBuffer, Type, ppBackBuffer);

	if (SUCCEEDED(hr) && ppBackBuffer)
	{
		*ppBackBuffer = m_pDeviceEx->GetLookupTable()->FindAddress<m_IDirect3DSurface9, m_IDirect3DDevice9Ex, LPVOID>(*ppBackBuffer, m_pDeviceEx, IID_IDirect3DSurface9, nullptr);
	}

	return hr;
}

HRESULT m_IDirect3DSwapChain9Ex::GetRasterStatus(THIS_ D3DRASTER_STATUS* pRasterStatus)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->GetRasterStatus(pRasterStatus);
}

HRESULT m_IDirect3DSwapChain9Ex::GetDisplayMode(THIS_ D3DDISPLAYMODE* pMode)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.D3d9to9Ex && ProxyInterfaceEx)
	{
		D3DDISPLAYMODEEX* pModeEx = nullptr;
		D3DDISPLAYMODEEX ModeEx = {};

		if (pMode)
		{
			ModeEx.Size = sizeof(D3DDISPLAYMODEEX);
			ModeEx.Width = pMode->Width;
			ModeEx.Height = pMode->Height;
			ModeEx.RefreshRate = pMode->RefreshRate;
			ModeEx.Format = pMode->Format;
			ModeEx.ScanLineOrdering = D3DSCANLINEORDERING_PROGRESSIVE;
			pModeEx = &ModeEx;
		}

		D3DDISPLAYROTATION Rotation = D3DDISPLAYROTATION_IDENTITY;

		return GetDisplayModeEx(pModeEx, &Rotation);
	}

	return ProxyInterface->GetDisplayMode(pMode);
}

HRESULT m_IDirect3DSwapChain9Ex::GetDevice(THIS_ IDirect3DDevice9** ppDevice)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ppDevice)
	{
		return D3DERR_INVALIDCALL;
	}

	return m_pDeviceEx->QueryInterface(WrapperID == IID_IDirect3DSwapChain9Ex ? IID_IDirect3DDevice9Ex : m_pDeviceEx->GetIID(), (LPVOID*)ppDevice);
}

HRESULT m_IDirect3DSwapChain9Ex::GetPresentParameters(THIS_ D3DPRESENT_PARAMETERS* pPresentationParameters)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->GetPresentParameters(pPresentationParameters);
}

HRESULT m_IDirect3DSwapChain9Ex::GetLastPresentCount(THIS_ UINT* pLastPresentCount)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterfaceEx)
	{
		Logging::Log() << __FUNCTION__ << " Error: Calling extension function from a non-extension device!";
		return D3DERR_INVALIDCALL;
	}

	return ProxyInterfaceEx->GetLastPresentCount(pLastPresentCount);
}

HRESULT m_IDirect3DSwapChain9Ex::GetPresentStats(THIS_ D3DPRESENTSTATS* pPresentationStatistics)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterfaceEx)
	{
		Logging::Log() << __FUNCTION__ << " Error: Calling extension function from a non-extension device!";
		return D3DERR_INVALIDCALL;
	}

	return ProxyInterfaceEx->GetPresentStats(pPresentationStatistics);
}

HRESULT m_IDirect3DSwapChain9Ex::GetDisplayModeEx(THIS_ D3DDISPLAYMODEEX* pMode, D3DDISPLAYROTATION* pRotation)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterfaceEx)
	{
		Logging::Log() << __FUNCTION__ << " Error: Calling extension function from a non-extension device!";
		return D3DERR_INVALIDCALL;
	}

	return ProxyInterfaceEx->GetDisplayModeEx(pMode, pRotation);
}
