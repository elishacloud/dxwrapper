/**
* Copyright (C) 2019 Elisha Riedlinger
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

bool GetD3DPath = true;
char D3DImPath[MAX_PATH] = { '\0' };
char D3DIm700Path[MAX_PATH] = { '\0' };
HMODULE hD3DIm = nullptr;
HMODULE hD3DIm700 = nullptr;

HRESULT m_IDirect3DX::QueryInterface(REFIID riid, LPVOID * ppvObj, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		if ((riid == IID_IDirect3D || riid == IID_IDirect3D2 || riid == IID_IDirect3D3 || riid == IID_IDirect3D7 || riid == IID_IUnknown) && ppvObj)
		{
			DWORD DxVersion = (riid == IID_IUnknown) ? DirectXVersion : GetIIDVersion(riid);

			*ppvObj = GetWrapperInterfaceX(DxVersion);

			::AddRef(*ppvObj);

			return S_OK;
		}
	}
	return ProxyQueryInterface(ProxyInterface, riid, ppvObj, GetWrapperType(DirectXVersion), WrapperInterface);
}

void *m_IDirect3DX::GetWrapperInterfaceX(DWORD DirectXVersion)
{
	// Check for device
	if (!ddrawParent)
	{
		Logging::Log() << __FUNCTION__ << " Error no ddraw parent!";
		return nullptr;
	}

	switch (DirectXVersion)
	{
	case 1:
		if (!UniqueProxyInterface.get())
		{
			UniqueProxyInterface = std::make_unique<m_IDirect3D>(this);
		}
		return UniqueProxyInterface.get();
	case 2:
		if (!UniqueProxyInterface2.get())
		{
			UniqueProxyInterface2 = std::make_unique<m_IDirect3D2>(this);
		}
		return UniqueProxyInterface2.get();
	case 3:
		if (!UniqueProxyInterface3.get())
		{
			UniqueProxyInterface3 = std::make_unique<m_IDirect3D3>(this);
		}
		return UniqueProxyInterface3.get();
	case 7:
		if (!UniqueProxyInterface7.get())
		{
			UniqueProxyInterface7 = std::make_unique<m_IDirect3D7>(this);
		}
		return UniqueProxyInterface7.get();
	default:
		Logging::Log() << __FUNCTION__ << " Error, wrapper interface version not found: " << DirectXVersion;
		return nullptr;
	}
}

ULONG m_IDirect3DX::AddRef()
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		return InterlockedIncrement(&RefCount);
	}

	return ProxyInterface->AddRef();
}

ULONG m_IDirect3DX::Release()
{
	Logging::LogDebug() << __FUNCTION__;

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
		// ToDo: Release all m_IDirect3DDeviceX devices when using Dd7to9.
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
	Logging::LogDebug() << __FUNCTION__;

	if (ProxyDirectXVersion != 1)
	{
		return D3D_OK;
	}

	return GetProxyInterfaceV1()->Initialize(rclsid);
}

HRESULT m_IDirect3DX::EnumDevices(LPD3DENUMDEVICESCALLBACK lpEnumDevicesCallback, LPVOID lpUserArg)
{
	Logging::LogDebug() << __FUNCTION__;

	switch (ProxyDirectXVersion)
	{
	case 1:
		return GetProxyInterfaceV1()->EnumDevices(lpEnumDevicesCallback, lpUserArg);
	case 2:
		return GetProxyInterfaceV2()->EnumDevices(lpEnumDevicesCallback, lpUserArg);
	case 3:
		return GetProxyInterfaceV3()->EnumDevices(lpEnumDevicesCallback, lpUserArg);
	case 7:
	{
		ENUMDEVICES CallbackContext;
		CallbackContext.lpContext = lpUserArg;
		CallbackContext.lpCallback = lpEnumDevicesCallback;

		return EnumDevices7(m_IDirect3DEnumDevices::ConvertCallback, &CallbackContext);
	}
	case 9:
		return EnumDevices7((LPD3DENUMDEVICESCALLBACK7)lpEnumDevicesCallback, lpUserArg, true);
	default:
		return DDERR_GENERIC;
	}
}

HRESULT m_IDirect3DX::EnumDevices7(LPD3DENUMDEVICESCALLBACK7 lpEnumDevicesCallback7, LPVOID lpUserArg, bool ConvertCallback)
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		// Check for device
		if (!ddrawParent)
		{
			Logging::Log() << __FUNCTION__ << " Error no ddraw parent!";
			return DDERR_GENERIC;
		}

		// Get d3d9Object
		IDirect3D9 *d3d9Object = ddrawParent->GetDirect3DObject();
		UINT AdapterCount = d3d9Object->GetAdapterCount();

		// Conversion callback
		LPD3DENUMDEVICESCALLBACK lpEnumDevicesCallback = (LPD3DENUMDEVICESCALLBACK)lpEnumDevicesCallback7;

		// Loop through all adapters
		for (UINT i = 0; i < AdapterCount; i++)
		{
			for (D3DDEVTYPE Type : {D3DDEVTYPE_REF, D3DDEVTYPE_HAL, (D3DDEVTYPE)(D3DDEVTYPE_HAL + 0x10)})
			{
				// Get Device Caps
				D3DCAPS9 Caps9;
				HRESULT hr = d3d9Object->GetDeviceCaps(i, (D3DDEVTYPE)((DWORD)Type & ~0x10), &Caps9);

				if (SUCCEEDED(hr))
				{
					// Convert device desc
					D3DDEVICEDESC7 DeviceDesc7;
					Caps9.DeviceType = Type;
					ConvertDeviceDesc(DeviceDesc7, Caps9);

					GUID deviceGUID = DeviceDesc7.deviceGUID;
					D3DDEVICEDESC D3DHWDevDesc, D3DHELDevDesc;
					D3DHWDevDesc.dwSize = sizeof(D3DDEVICEDESC);
					D3DHELDevDesc.dwSize = sizeof(D3DDEVICEDESC);

					LPSTR lpDescription, lpName;

					switch ((DWORD)Type)
					{
					case D3DDEVTYPE_REF:
						lpDescription = "Microsoft Direct3D RGB Software Emulation";
						lpName = "RGB Emulation";
						if (ConvertCallback)
						{
							// Get Device Caps D3DDEVTYPE_REF
							ConvertDeviceDesc(D3DHWDevDesc, DeviceDesc7);

							// Get Device Caps D3DDEVTYPE_HAL
							if (SUCCEEDED(d3d9Object->GetDeviceCaps(i, D3DDEVTYPE_HAL, &Caps9)))
							{
								Caps9.DeviceType = D3DDEVTYPE_HAL;
								ConvertDeviceDesc(DeviceDesc7, Caps9);
								ConvertDeviceDesc(D3DHELDevDesc, DeviceDesc7);

								if (lpEnumDevicesCallback(&deviceGUID, lpDescription, lpName, &D3DHWDevDesc, &D3DHELDevDesc, lpUserArg) == DDENUMRET_CANCEL)
								{
									return D3D_OK;
								}
							}
						}
						break;
					case D3DDEVTYPE_HAL:
						lpDescription = "Microsoft Direct3D Hardware acceleration through Direct3D HAL";
						lpName = "Direct3D HAL";
						if (ConvertCallback)
						{
							// Get Device Caps D3DDEVTYPE_HAL
							ConvertDeviceDesc(D3DHELDevDesc, DeviceDesc7);

							// Get Device Caps D3DDEVTYPE_REF
							if (SUCCEEDED(d3d9Object->GetDeviceCaps(i, D3DDEVTYPE_REF, &Caps9)))
							{
								Caps9.DeviceType = D3DDEVTYPE_REF;
								ConvertDeviceDesc(DeviceDesc7, Caps9);
								ConvertDeviceDesc(D3DHWDevDesc, DeviceDesc7);

								if (lpEnumDevicesCallback(&deviceGUID, lpDescription, lpName, &D3DHWDevDesc, &D3DHELDevDesc, lpUserArg) == DDENUMRET_CANCEL)
								{
									return D3D_OK;
								}
							}
						}
						break;
					default:
					case D3DDEVTYPE_HAL + 0x10:
						lpDescription = "Microsoft Direct3D Hardware Transform and Lighting acceleration capable device";
						lpName = "Direct3D T&L HAL";
						break;
					}

					if (!ConvertCallback)
					{
						if (lpEnumDevicesCallback7(lpDescription, lpName, &DeviceDesc7, lpUserArg) == DDENUMRET_CANCEL)
						{
							return D3D_OK;
						}
					}
				}
				else
				{
					Logging::Log() << __FUNCTION__ << " Error failed to GetCaps!";
				}
			}
		}

		return D3D_OK;
	}

	return GetProxyInterfaceV7()->EnumDevices(lpEnumDevicesCallback7, lpUserArg);
}

HRESULT m_IDirect3DX::CreateLight(LPDIRECT3DLIGHT * lplpDirect3DLight, LPUNKNOWN pUnkOuter)
{
	Logging::LogDebug() << __FUNCTION__;

	HRESULT hr = DDERR_GENERIC;

	switch (ProxyDirectXVersion)
	{
	case 1:
		hr = GetProxyInterfaceV1()->CreateLight(lplpDirect3DLight, pUnkOuter);
		break;
	case 2:
		hr = GetProxyInterfaceV2()->CreateLight(lplpDirect3DLight, pUnkOuter);
		break;
	case 3:
		hr = GetProxyInterfaceV3()->CreateLight(lplpDirect3DLight, pUnkOuter);
		break;
	case 7:
	case 9:
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	default:
		return DDERR_GENERIC;
	}

	if (SUCCEEDED(hr) && lplpDirect3DLight)
	{
		*lplpDirect3DLight = ProxyAddressLookupTable.FindAddress<m_IDirect3DLight>(*lplpDirect3DLight);
	}

	return hr;
}

HRESULT m_IDirect3DX::CreateMaterial(LPDIRECT3DMATERIAL3 * lplpDirect3DMaterial, LPUNKNOWN pUnkOuter, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__;

	HRESULT hr = DDERR_GENERIC;

	switch (ProxyDirectXVersion)
	{
	case 1:
		hr = GetProxyInterfaceV1()->CreateMaterial((LPDIRECT3DMATERIAL*)lplpDirect3DMaterial, pUnkOuter);
		break;
	case 2:
		hr = GetProxyInterfaceV2()->CreateMaterial((LPDIRECT3DMATERIAL2*)lplpDirect3DMaterial, pUnkOuter);
		break;
	case 3:
		hr = GetProxyInterfaceV3()->CreateMaterial(lplpDirect3DMaterial, pUnkOuter);
		break;
	case 7:
		if (lplpDirect3DMaterial && lpCurrentD3DDevice)
		{
			*lplpDirect3DMaterial = (LPDIRECT3DMATERIAL3)new m_IDirect3DMaterialX(lpCurrentD3DDevice, 7);
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
		break;
	case 9:
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	default:
		return DDERR_GENERIC;
	}

	if (SUCCEEDED(hr) && lplpDirect3DMaterial)
	{
		*lplpDirect3DMaterial = ProxyAddressLookupTable.FindAddress<m_IDirect3DMaterial3>(*lplpDirect3DMaterial, DirectXVersion);
	}

	return hr;
}

HRESULT m_IDirect3DX::CreateViewport(LPDIRECT3DVIEWPORT3 * lplpD3DViewport, LPUNKNOWN pUnkOuter, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__;

	HRESULT hr = DDERR_GENERIC;

	switch (ProxyDirectXVersion)
	{
	case 1:
		hr = GetProxyInterfaceV1()->CreateViewport((LPDIRECT3DVIEWPORT*)lplpD3DViewport, pUnkOuter);
		break;
	case 2:
		hr = GetProxyInterfaceV2()->CreateViewport((LPDIRECT3DVIEWPORT2*)lplpD3DViewport, pUnkOuter);
		break;
	case 3:
		hr = GetProxyInterfaceV3()->CreateViewport(lplpD3DViewport, pUnkOuter);
		break;
	case 7:
		if (lplpD3DViewport && lpCurrentD3DDevice)
		{
			*lplpD3DViewport = (LPDIRECT3DVIEWPORT3)new m_IDirect3DViewportX(lpCurrentD3DDevice, 7);
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
		break;
	case 9:
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	default:
		return DDERR_GENERIC;
	}

	if (SUCCEEDED(hr) && lplpD3DViewport)
	{
		*lplpD3DViewport = ProxyAddressLookupTable.FindAddress<m_IDirect3DViewport3>(*lplpD3DViewport, DirectXVersion);
	}

	return hr;
}

HRESULT m_IDirect3DX::FindDevice(LPD3DFINDDEVICESEARCH lpD3DFDS, LPD3DFINDDEVICERESULT lpD3DFDR)
{
	Logging::LogDebug() << __FUNCTION__;

	switch (ProxyDirectXVersion)
	{
	case 1:
		return GetProxyInterfaceV1()->FindDevice(lpD3DFDS, lpD3DFDR);
	case 2:
		return GetProxyInterfaceV2()->FindDevice(lpD3DFDS, lpD3DFDR);
	case 3:
		return GetProxyInterfaceV3()->FindDevice(lpD3DFDS, lpD3DFDR);
	case 7:
	case 9:
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	default:
		return DDERR_GENERIC;
	}
}

HRESULT m_IDirect3DX::CreateDevice(REFCLSID rclsid, LPDIRECTDRAWSURFACE7 lpDDS, LPDIRECT3DDEVICE7 * lplpD3DDevice, LPUNKNOWN pUnkOuter, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.Dd7to9)
	{
		// Check for device
		if (!ddrawParent)
		{
			Logging::Log() << __FUNCTION__ << " Error no ddraw parent!";
			return DDERR_GENERIC;
		}

		m_IDirect3DDeviceX *p_IDirect3DDeviceX = new m_IDirect3DDeviceX(ddrawParent, DirectXVersion);

		*lplpD3DDevice = (LPDIRECT3DDEVICE7)p_IDirect3DDeviceX->GetWrapperInterfaceX(DirectXVersion);

		return S_OK;
	}

	if (lpDDS)
	{
		lpDDS = static_cast<m_IDirectDrawSurface7 *>(lpDDS)->GetProxyInterface();
	}

	HRESULT hr = DDERR_GENERIC;

	switch (ProxyDirectXVersion)
	{
	case 1:
	default:
		return DDERR_GENERIC;
	case 2:
		hr = GetProxyInterfaceV2()->CreateDevice(rclsid, (LPDIRECTDRAWSURFACE)lpDDS, (LPDIRECT3DDEVICE2*)lplpD3DDevice);
		break;
	case 3:
		hr = GetProxyInterfaceV3()->CreateDevice(rclsid, (LPDIRECTDRAWSURFACE4)lpDDS, (LPDIRECT3DDEVICE3*)lplpD3DDevice, pUnkOuter);
		break;
	case 7:
		hr = GetProxyInterfaceV7()->CreateDevice(rclsid, lpDDS, lplpD3DDevice);
		break;
	}

	if (SUCCEEDED(hr) && lplpD3DDevice)
	{
		*lplpD3DDevice = ProxyAddressLookupTable.FindAddress<m_IDirect3DDevice7>(*lplpD3DDevice, DirectXVersion);
	}

	return hr;
}

HRESULT m_IDirect3DX::CreateVertexBuffer(LPD3DVERTEXBUFFERDESC lpVBDesc, LPDIRECT3DVERTEXBUFFER7 * lplpD3DVertexBuffer, DWORD dwFlags, LPUNKNOWN pUnkOuter, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__;

	HRESULT hr = DDERR_GENERIC;

	switch (ProxyDirectXVersion)
	{
	case 1:
	case 2:
	default:
		return DDERR_GENERIC;
	case 3:
		hr = GetProxyInterfaceV3()->CreateVertexBuffer(lpVBDesc, (LPDIRECT3DVERTEXBUFFER*)lplpD3DVertexBuffer, dwFlags, pUnkOuter);
		break;
	case 7:
		hr = GetProxyInterfaceV7()->CreateVertexBuffer(lpVBDesc, lplpD3DVertexBuffer, dwFlags);
		break;
	case 9:
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	if (SUCCEEDED(hr) && lplpD3DVertexBuffer)
	{
		*lplpD3DVertexBuffer = ProxyAddressLookupTable.FindAddress<m_IDirect3DVertexBuffer7>(*lplpD3DVertexBuffer, (DirectXVersion == 7) ? 7 : 1);
	}

	return hr;
}

HRESULT m_IDirect3DX::EnumZBufferFormats(REFCLSID riidDevice, LPD3DENUMPIXELFORMATSCALLBACK lpEnumCallback, LPVOID lpContext)
{
	Logging::LogDebug() << __FUNCTION__;

	switch (ProxyDirectXVersion)
	{
	case 1:
	case 2:
	default:
		return DDERR_GENERIC;
	case 3:
		return GetProxyInterfaceV3()->EnumZBufferFormats(riidDevice, lpEnumCallback, lpContext);
	case 7:
		return GetProxyInterfaceV7()->EnumZBufferFormats(riidDevice, lpEnumCallback, lpContext);
	case 9:
		if (riidDevice == IID_IDirect3DRGBDevice || riidDevice == IID_IDirect3DRampDevice)
		{
			DDPIXELFORMAT PixelFormat = { NULL };
			PixelFormat.dwSize = sizeof(DDPIXELFORMAT);

			for (DWORD Num : {0, 2})
			{
				GetBufferFormat(PixelFormat, Num);

				if (lpEnumCallback(&PixelFormat, lpContext) == DDENUMRET_CANCEL)
				{
					return D3D_OK;
				}
			}
			return D3D_OK;
		}
		else if (riidDevice == IID_IDirect3DHALDevice || riidDevice == IID_IDirect3DNullDevice)
		{
			DDPIXELFORMAT PixelFormat = { NULL };
			PixelFormat.dwSize = sizeof(DDPIXELFORMAT);

			for (DWORD Num = 0; Num < 6; Num++)
			{
				GetBufferFormat(PixelFormat, Num);

				if (lpEnumCallback(&PixelFormat, lpContext) == DDENUMRET_CANCEL)
				{
					return D3D_OK;
				}
			}
			return D3D_OK;
		}
		else if (riidDevice == IID_IDirect3DMMXDevice || riidDevice == IID_IDirect3DRefDevice || riidDevice == IID_IDirect3DTnLHalDevice)
		{
			return D3D_OK;
		}
		else
		{
			Logging::Log() << __FUNCTION__ << " Not Implemented " << riidDevice;
			return DDERR_NOZBUFFERHW;
		}
	}
}

HRESULT m_IDirect3DX::EvictManagedTextures()
{
	Logging::LogDebug() << __FUNCTION__;

	switch (ProxyDirectXVersion)
	{
	case 1:
	case 2:
	default:
		return DDERR_GENERIC;
	case 3:
		return GetProxyInterfaceV3()->EvictManagedTextures();
	case 7:
		return GetProxyInterfaceV7()->EvictManagedTextures();
	case 9:
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}
}

void m_IDirect3DX::ReleaseD3DInterface()
{
	while (ThreadSyncFlag)
	{
		Sleep(0);
	}

	ThreadSyncFlag = true;

	if (ddrawParent)
	{
		ddrawParent->ClearD3DInterface();
	}

	ThreadSyncFlag = false;
}
