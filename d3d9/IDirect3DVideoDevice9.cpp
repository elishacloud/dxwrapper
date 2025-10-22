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

HRESULT m_IDirect3DVideoDevice9::QueryInterface(THIS_ REFIID riid, void** ppvObj)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ") " << riid;

	if (riid == IID_IUnknown || riid == WrapperID || riid == IID_IDirect3DBaseTexture9 || riid == IID_IDirect3DResource9)
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

ULONG m_IDirect3DVideoDevice9::AddRef(THIS)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->AddRef();
}

ULONG m_IDirect3DVideoDevice9::Release(THIS)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	ULONG ref = ProxyInterface->Release();

	if (ref == 0 && m_pDeviceEx->GetClientDXVersion() < 8)
	{
		m_pDeviceEx->GetLookupTable()->DeleteAddress(this);

		delete this;
	}

	return ref;
}

HRESULT m_IDirect3DVideoDevice9::CreateSurface(THIS_ UINT Width, UINT Height, UINT BackBuffers, D3DFORMAT Format, D3DPOOL Pool, DWORD Usage, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	HRESULT hr = ProxyInterface->CreateSurface(Width, Height, BackBuffers, Format, Pool, Usage, ppSurface, pSharedHandle);

	if (SUCCEEDED(hr) && ppSurface)
	{
		*ppSurface = m_pDeviceEx->GetLookupTable()->FindCreateAddress<m_IDirect3DSurface9, m_IDirect3DDevice9Ex, LPVOID>(*ppSurface, m_pDeviceEx, IID_IDirect3DSurface9, nullptr);
		return D3D_OK;
	}

	Logging::LogDebug() << __FUNCTION__ << " FAILED! " << (D3DERR)hr << " " << Width << " " << Height << " " << BackBuffers << " " << Format << " " << Pool << " " << Usage << " " << pSharedHandle;
	return hr;
}

HRESULT m_IDirect3DVideoDevice9::GetDXVACompressedBufferInfo(THIS_ GUID* pGuid, DXVAUncompDataInfo* pUncompData, DWORD* pNumBuffers, DXVACompBufferInfo* pBufferInfo)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->GetDXVACompressedBufferInfo(pGuid, pUncompData, pNumBuffers, pBufferInfo);
}

HRESULT m_IDirect3DVideoDevice9::GetDXVAGuids(THIS_ DWORD* pNumGuids, GUID* pGuids)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->GetDXVAGuids(pNumGuids, pGuids);
}

HRESULT m_IDirect3DVideoDevice9::GetDXVAInternalInfo(THIS_ GUID* pGuid, DXVAUncompDataInfo* pUncompData, DWORD* pMemoryUsed)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->GetDXVAInternalInfo(pGuid, pUncompData, pMemoryUsed);
}

HRESULT m_IDirect3DVideoDevice9::GetUncompressedDXVAFormats(THIS_ GUID* pGuid, DWORD* pNumFormats, D3DFORMAT* pFormats)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->GetUncompressedDXVAFormats(pGuid, pNumFormats, pFormats);
}

HRESULT m_IDirect3DVideoDevice9::CreateDXVADevice(THIS_ GUID* pGuid, DXVAUncompDataInfo* pUncompData, LPVOID pData, DWORD DataSize, IDirect3DDXVADevice9** ppDXVADevice)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	HRESULT hr = ProxyInterface->CreateDXVADevice(pGuid, pUncompData, pData, DataSize, ppDXVADevice);

	if (SUCCEEDED(hr) && ppDXVADevice)
	{
		*ppDXVADevice = m_pDeviceEx->GetLookupTable()->FindCreateAddress<m_IDirect3DDXVADevice9, m_IDirect3DDevice9Ex, LPVOID>(*ppDXVADevice, m_pDeviceEx, IID_IDirect3DDXVADevice9, nullptr);
		return D3D_OK;
	}

	return hr;
}
