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

void WINAPI D3d9Wrapper::genericQueryInterface(REFIID riid, LPVOID *ppvObj, m_IDirect3DDevice9Ex* m_pDeviceEx)
{
	if (!ppvObj || !*ppvObj || !m_pDeviceEx)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Warning: not wrapping interface because of nullptr: " << riid);
		return;
	}

	if (riid == IID_IDirect3D9 || riid == IID_IDirect3D9Ex)
	{
		ComPtr<IDirect3D9> pD3D9;
		if (SUCCEEDED(m_pDeviceEx->GetDirect3D(pD3D9.GetAddressOf())))
		{
			ComPtr<IDirect3D9> pD3D9Ex;
			if (SUCCEEDED(pD3D9->QueryInterface(riid, (LPVOID*)pD3D9Ex.GetAddressOf())))
			{
				*ppvObj = pD3D9Ex.Get();
				return;
			}
		}
		LOG_LIMIT(100, __FUNCTION__ << " Warning: not wrapping interface: " << riid);
		return;
	}

	if (riid == IID_IDirect3DDevice9 || riid == IID_IDirect3DDevice9Ex)
	{
		ComPtr<IDirect3DDevice9> pD3DDevice9;
		if (SUCCEEDED(m_pDeviceEx->QueryInterface(riid, (LPVOID*)pD3DDevice9.GetAddressOf())))
		{
			*ppvObj = pD3DDevice9.Get();
			return;
		}
		LOG_LIMIT(100, __FUNCTION__ << " Warning: not wrapping interface: " << riid);
		return;
	}

	if (riid == IID_IDirect3DSwapChain9 || riid == IID_IDirect3DSwapChain9Ex)
	{
		*ppvObj = m_pDeviceEx->GetLookupTable()->FindAddress<m_IDirect3DSwapChain9Ex, m_IDirect3DDevice9Ex, LPVOID>(*ppvObj, m_pDeviceEx, riid, nullptr);
		return;
	}

#define QUERYINTERFACE(x) \
	if (riid == IID_ ## x) \
		{ \
			*ppvObj = m_pDeviceEx->GetLookupTable()->FindAddress<m_ ## x, m_IDirect3DDevice9Ex, LPVOID>(*ppvObj, m_pDeviceEx, riid, nullptr); \
			return; \
		}

	QUERYINTERFACE(IDirect3DCubeTexture9);
	QUERYINTERFACE(IDirect3DIndexBuffer9);
	QUERYINTERFACE(IDirect3DPixelShader9);
	QUERYINTERFACE(IDirect3DQuery9);
	QUERYINTERFACE(IDirect3DStateBlock9);
	QUERYINTERFACE(IDirect3DSurface9);
	QUERYINTERFACE(IDirect3DTexture9);
	QUERYINTERFACE(IDirect3DVertexBuffer9);
	QUERYINTERFACE(IDirect3DVertexDeclaration9);
	QUERYINTERFACE(IDirect3DVertexShader9);
	QUERYINTERFACE(IDirect3DVolume9);
	QUERYINTERFACE(IDirect3DVolumeTexture9);
	QUERYINTERFACE(IDirect3DVideoDevice9);
	QUERYINTERFACE(IDirect3DDXVADevice9);

	LOG_LIMIT(100, __FUNCTION__ << " Warning: not wrapping interface: " << riid);
}
