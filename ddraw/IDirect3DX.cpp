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
	if (Config.Dd7to9)
	{
		if ((riid == IID_IDirect3D || riid == IID_IDirect3D2 || riid == IID_IDirect3D3 || riid == IID_IDirect3D7 || riid == IID_IUnknown) && ppvObj)
		{
			AddRef();

			*ppvObj = this;

			return S_OK;
		}
	}
	return ProxyQueryInterface(ProxyInterface, riid, ppvObj, WrapperID, WrapperInterface);
}

ULONG m_IDirect3DX::AddRef()
{
	if (Config.Dd7to9)
	{
		return InterlockedIncrement(&RefCount);
	}

	return ProxyInterface->AddRef();
}

ULONG m_IDirect3DX::Release()
{
	ULONG ref;

	if (Config.Dd7to9)
	{
		ref = InterlockedDecrement(&RefCount);
	}
	else
	{
		ref = ProxyInterface->Release();
	}

	if (ref == 0)
	{
		if (WrapperInterface)
		{
			WrapperInterface->DeleteMe();
		}
		else
		{
			delete this;
		}
	}

	return ref;
}

HRESULT m_IDirect3DX::Initialize(REFCLSID rclsid)
{
	if (ProxyDirectXVersion != 1)
	{
		return D3D_OK;
	}

	return ((IDirect3D*)ProxyInterface)->Initialize(rclsid);
}

HRESULT m_IDirect3DX::EnumDevices(LPD3DENUMDEVICESCALLBACK7 lpEnumDevicesCallback, LPVOID lpUserArg)
{
	ENUMDEVICES CallbackContext;
	if (ProxyDirectXVersion > 3 && DirectXVersion < 4)
	{
		CallbackContext.lpContext = lpUserArg;
		CallbackContext.lpCallback = (LPD3DENUMDEVICESCALLBACK)lpEnumDevicesCallback;

		lpUserArg = &CallbackContext;
		lpEnumDevicesCallback = m_IDirect3DEnumDevices::ConvertCallback;
	}

	if (Config.Dd7to9)
	{
		// Check for device
		if (!ddrawParent)
		{
			Logging::Log() << __FUNCTION__ << " Error, ddraw closed!";
			return DDERR_INVALIDOBJECT;
		}

		// Get d3d9Object
		IDirect3D9 *d3d9Object = ddrawParent->GetDirect3D();
		UINT AdapterCount = d3d9Object->GetAdapterCount();

		// Loop through all adapters
		for (UINT i = 0; i < AdapterCount; i++)
		{
			for (D3DDEVTYPE Type : {D3DDEVTYPE_REF, D3DDEVTYPE_HAL, (D3DDEVTYPE)(D3DDEVTYPE_HAL + 0x10)})
			{
				// Get Device Caps
				D3DCAPS9 Caps9;
				HRESULT hr = d3d9Object->GetDeviceCaps(i, (D3DDEVTYPE)((DWORD)D3DDEVTYPE_HAL & 0xF), &Caps9);

				if (SUCCEEDED(hr))
				{
					// Convert device desc
					D3DDEVICEDESC7 DeviceDesc7;
					ConvertDeviceDesc(DeviceDesc7, Caps9);

					LPSTR lpDescription, lpName;
					switch ((DWORD)Type)
					{
					case D3DDEVTYPE_REF:
						lpDescription = "Microsoft Direct3D RGB Software Emulation";
						lpName = "RGB Emulation";
						break;
					case D3DDEVTYPE_HAL:
						lpDescription = "Microsoft Direct3D Hardware acceleration through Direct3D HAL";
						lpName = "Direct3D HAL";
						break;
					default:
					case D3DDEVTYPE_HAL + 0x10 :
						lpDescription = "Microsoft Direct3D Hardware Transform and Lighting acceleration capable device";
						lpName = "Direct3D T&L HAL";
						break;
					}

					if (lpEnumDevicesCallback(lpDescription, lpName, &DeviceDesc7, lpUserArg) != DDENUMRET_OK)
					{
						return D3D_OK;
					}
				}
			}
		}

		return D3D_OK;
	}

	return ProxyInterface->EnumDevices(lpEnumDevicesCallback, lpUserArg);
}

HRESULT m_IDirect3DX::CreateLight(LPDIRECT3DLIGHT * lplpDirect3DLight, LPUNKNOWN pUnkOuter)
{
	if (ProxyDirectXVersion > 3)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
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
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	if (ProxyDirectXVersion > 3)
	{
		if (lplpDirect3DMaterial && lpCurrentD3DDevice)
		{
			*lplpDirect3DMaterial = new m_IDirect3DMaterialX(lpCurrentD3DDevice, 7);
			return D3D_OK;
		}
		else if (!lplpDirect3DMaterial)
		{
			return DDERR_INVALIDPARAMS;
		}
		else if (!lpCurrentD3DDevice)
		{
			Logging::Log() << __FUNCTION__ << " No current IDirect3DDevice";
			return D3DERR_INVALID_DEVICE;
		}
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
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	if (ProxyDirectXVersion > 3)
	{
		if (lplpD3DViewport && lpCurrentD3DDevice)
		{
			*lplpD3DViewport = new m_IDirect3DViewportX(lpCurrentD3DDevice, 7);
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
	if (ProxyDirectXVersion > 3)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	return ((IDirect3D3*)ProxyInterface)->FindDevice(lpD3DFDS, lpD3DFDR);
}

HRESULT m_IDirect3DX::CreateDevice(REFCLSID rclsid, LPDIRECTDRAWSURFACE7 lpDDS, LPDIRECT3DDEVICE7 * lplpD3DDevice)
{
	if (Config.Dd7to9)
	{
		if (!lplpD3DDevice)
		{
			return DDERR_INVALIDPARAMS;
		}

		*lplpD3DDevice = new m_IDirect3DDeviceX(d3d9Device, ddrawParent, DirectXVersion);

		return DD_OK;
	}

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
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

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
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	if (ProxyDirectXVersion == 3)
	{
		return ((IDirect3D3*)ProxyInterface)->EnumZBufferFormats(riidDevice, lpEnumCallback, lpContext);
	}

	return ProxyInterface->EnumZBufferFormats(riidDevice, lpEnumCallback, lpContext);
}

HRESULT m_IDirect3DX::EvictManagedTextures()
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	return ProxyInterface->EvictManagedTextures();
}
