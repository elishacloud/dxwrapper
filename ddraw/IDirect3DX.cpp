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

#include "ddraw.h"

HRESULT m_IDirect3DX::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
	return ProxyQueryInterface(ProxyInterface, riid, ppvObj, WrapperID, WrapperInterface);
}

ULONG m_IDirect3DX::AddRef()
{
	return ProxyInterface->AddRef();
}

ULONG m_IDirect3DX::Release()
{
	ULONG x = ProxyInterface->Release();

	if (x == 0)
	{
		WrapperInterface->DeleteMe();
	}

	return x;
}

HRESULT m_IDirect3DX::Initialize(REFCLSID rclsid)
{
	if (ProxyDirectXVersion != 1)
	{
		Logging::Log() << __FUNCTION__ << " Not Implimented";
		return D3D_OK;	// Should not matter for newer versions of DirectX
	}

	return ((IDirect3D*)ProxyInterface)->Initialize(rclsid);
}

HRESULT m_IDirect3DX::EnumDevices(LPD3DENUMDEVICESCALLBACK7 lpEnumDevicesCallback, LPVOID lpUserArg)
{
	ENUMDEVICES CallbackContext;
	if (ProxyDirectXVersion == 7 && DirectXVersion < 4)
	{
		CallbackContext.lpContext = lpUserArg;
		CallbackContext.lpCallback = (LPD3DENUMDEVICESCALLBACK)lpEnumDevicesCallback;

		lpUserArg = &CallbackContext;
		lpEnumDevicesCallback = m_IDirect3DEnumDevices::ConvertCallback;
	}

	return ProxyInterface->EnumDevices(lpEnumDevicesCallback, lpUserArg);
}

HRESULT m_IDirect3DX::CreateLight(LPDIRECT3DLIGHT * lplpDirect3DLight, LPUNKNOWN pUnkOuter)
{
	if (ProxyDirectXVersion == 7)
	{
		Logging::Log() << __FUNCTION__ << " Not Implimented";
		return E_NOTIMPL;
	}

	HRESULT hr = ((IDirect3D3*)ProxyInterface)->CreateLight(lplpDirect3DLight, pUnkOuter);

	if (SUCCEEDED(hr) && lplpDirect3DLight)
	{
		*lplpDirect3DLight = ProxyAddressLookupTable.FindAddress<m_IDirect3DLight>(*lplpDirect3DLight);
	}

	return hr;
}

HRESULT m_IDirect3DX::CreateMaterial(LPDIRECT3DMATERIAL3 * lplpDirect3DMaterial, LPUNKNOWN pUnkOuter)
{
	if (ProxyDirectXVersion == 7)
	{
		Logging::Log() << __FUNCTION__ << " Not Implimented";
		return E_NOTIMPL;
	}

	HRESULT hr = ((IDirect3D3*)ProxyInterface)->CreateMaterial(lplpDirect3DMaterial, pUnkOuter);

	if (SUCCEEDED(hr) && lplpDirect3DMaterial)
	{
		*lplpDirect3DMaterial = ProxyAddressLookupTable.FindAddress<m_IDirect3DMaterial3>(*lplpDirect3DMaterial, DirectXVersion);
	}

	return hr;
}

HRESULT m_IDirect3DX::CreateViewport(LPDIRECT3DVIEWPORT3 * lplpD3DViewport, LPUNKNOWN pUnkOuter)
{
	if (ProxyDirectXVersion == 7)
	{
		if (lplpD3DViewport && lpCurrentD3DDevice)
		{
			*lplpD3DViewport = new m_IDirect3DViewportX((IDirect3DViewport3*)lpCurrentD3DDevice->GetProxyInterface(), 7, (m_IDirect3DViewport3*)lpCurrentD3DDevice);
			return D3D_OK;
		}
		else if (!lplpD3DViewport)
		{
			return DDERR_INVALIDPARAMS;
		}
		else if (!lpCurrentD3DDevice)
		{
			Logging::Log() << __FUNCTION__ << " No current IDirect3DDevice";
			return D3DERR_INVALID_DEVICE;
		}
	}

	HRESULT hr = ((IDirect3D3*)ProxyInterface)->CreateViewport(lplpD3DViewport, pUnkOuter);

	if (SUCCEEDED(hr) && lplpD3DViewport)
	{
		*lplpD3DViewport = ProxyAddressLookupTable.FindAddress<m_IDirect3DViewport3>(*lplpD3DViewport, DirectXVersion);
	}

	return hr;
}

HRESULT m_IDirect3DX::FindDevice(LPD3DFINDDEVICESEARCH lpD3DFDS, LPD3DFINDDEVICERESULT lpD3DFDR)
{
	if (ProxyDirectXVersion == 7)
	{
		Logging::Log() << __FUNCTION__ << " Not Implimented";
		return E_NOTIMPL;
	}

	return ((IDirect3D3*)ProxyInterface)->FindDevice(lpD3DFDS, lpD3DFDR);
}

HRESULT m_IDirect3DX::CreateDevice(REFCLSID rclsid, LPDIRECTDRAWSURFACE7 lpDDS, LPDIRECT3DDEVICE7 * lplpD3DDevice)
{
	if (lpDDS)
	{
		lpDDS = static_cast<m_IDirectDrawSurface7 *>(lpDDS)->GetProxyInterface();
	}

	HRESULT hr;

	if (ProxyDirectXVersion == 3)
	{
		hr = ((IDirect3D3*)ProxyInterface)->CreateDevice(rclsid, (LPDIRECTDRAWSURFACE4)lpDDS, (LPDIRECT3DDEVICE3*)lplpD3DDevice, nullptr);
	}
	else
	{
		hr = ProxyInterface->CreateDevice(rclsid, lpDDS, lplpD3DDevice);
	}

	if (SUCCEEDED(hr) && lplpD3DDevice)
	{
		*lplpD3DDevice = ProxyAddressLookupTable.FindAddress<m_IDirect3DDevice7>(*lplpD3DDevice, DirectXVersion);
	}

	return hr;
}

HRESULT m_IDirect3DX::CreateVertexBuffer(LPD3DVERTEXBUFFERDESC lpVBDesc, LPDIRECT3DVERTEXBUFFER7 * lplpD3DVertexBuffer, DWORD dwFlags)
{
	HRESULT hr;

	if (ProxyDirectXVersion == 3)
	{
		hr = ((IDirect3D3*)ProxyInterface)->CreateVertexBuffer(lpVBDesc, (LPDIRECT3DVERTEXBUFFER*)lplpD3DVertexBuffer, dwFlags, nullptr);
	}
	else
	{
		hr = ProxyInterface->CreateVertexBuffer(lpVBDesc, lplpD3DVertexBuffer, dwFlags);
	}	

	if (SUCCEEDED(hr) && lplpD3DVertexBuffer)
	{
		*lplpD3DVertexBuffer = ProxyAddressLookupTable.FindAddress<m_IDirect3DVertexBuffer7>(*lplpD3DVertexBuffer, DirectXVersion);
	}

	return hr;
}

HRESULT m_IDirect3DX::EnumZBufferFormats(REFCLSID riidDevice, LPD3DENUMPIXELFORMATSCALLBACK lpEnumCallback, LPVOID lpContext)
{
	if (ProxyDirectXVersion != 7)
	{
		return ((IDirect3D3*)ProxyInterface)->EnumZBufferFormats(riidDevice, lpEnumCallback, lpContext);
	}

	return ProxyInterface->EnumZBufferFormats(riidDevice, lpEnumCallback, lpContext);
}

HRESULT m_IDirect3DX::EvictManagedTextures()
{
	return ProxyInterface->EvictManagedTextures();
}
