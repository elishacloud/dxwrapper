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

// ******************************
// IUnknown functions
// ******************************

HRESULT m_IDirect3DDXVADevice9::QueryInterface(THIS_ REFIID riid, void** ppvObj)
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

ULONG m_IDirect3DDXVADevice9::AddRef(THIS)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->AddRef();
}

ULONG m_IDirect3DDXVADevice9::Release(THIS)
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

// ******************************
// IDirect3DDXVADevice9 methods
// ******************************

HRESULT m_IDirect3DDXVADevice9::BeginFrame(THIS_ IDirect3DSurface9* pDstSurface, DWORD SizeInputData, VOID* pInputData, DWORD* pSizeOutputData, VOID* pOutputData)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (pDstSurface)
	{
		pDstSurface = static_cast<m_IDirect3DSurface9*>(pDstSurface)->GetProxyInterface();
	}

	return ProxyInterface->BeginFrame(pDstSurface, SizeInputData, pInputData, pSizeOutputData, pOutputData);
}

HRESULT m_IDirect3DDXVADevice9::EndFrame(THIS_ DWORD SizeMiscData, VOID* pMiscData)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->EndFrame(SizeMiscData, pMiscData);
}

HRESULT m_IDirect3DDXVADevice9::Execute(THIS_ DWORD FunctionNum, VOID* pInputData, DWORD InputSize, VOID* OuputData, DWORD OutputSize, DWORD NumBuffers, DXVABufferInfo* pBufferInfo)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	std::vector<DXVABufferInfo> localBuffers;

	if (pBufferInfo && NumBuffers)
	{
		localBuffers.resize(NumBuffers);

		for (DWORD i = 0; i < NumBuffers; i++)
		{
			localBuffers[i] = pBufferInfo[i];

			if (localBuffers[i].pCompSurface)
			{
				IDirect3DSurface9* pSurface = m_pDeviceEx->GetLookupTable()->GetSafeProxyInterface<m_IDirect3DSurface9, IDirect3DSurface9>(static_cast<m_IDirect3DSurface9*>(localBuffers[i].pCompSurface));
				if (pSurface)
				{
					localBuffers[i].pCompSurface = pSurface;
				}
				else
				{
					LOG_LIMIT(100, __FUNCTION__ << " Warning: surface interface not found: " << Logging::hex(localBuffers[i].pCompSurface));
				}
			}
		}
	}

	DXVABufferInfo* buffers = (pBufferInfo && NumBuffers) ? localBuffers.data() : pBufferInfo;

	return ProxyInterface->Execute(FunctionNum, pInputData, InputSize, OuputData, OutputSize, NumBuffers, buffers);
}

HRESULT m_IDirect3DDXVADevice9::QueryStatus(THIS_ IDirect3DSurface9* pSurface, DWORD Flags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (pSurface)
	{
		pSurface = static_cast<m_IDirect3DSurface9*>(pSurface)->GetProxyInterface();
	}

	return ProxyInterface->QueryStatus(pSurface, Flags);
}
