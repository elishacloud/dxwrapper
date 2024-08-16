/**
* Copyright (C) 2023 Elisha Riedlinger
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
#include "Utils\Utils.h"

bool GetD3DPath = true;
char D3DImPath[MAX_PATH] = { '\0' };
char D3DIm700Path[MAX_PATH] = { '\0' };
HMODULE hD3DIm = nullptr;
HMODULE hD3DIm700 = nullptr;

HRESULT m_IDirect3DX::QueryInterface(REFIID riid, LPVOID FAR * ppvObj, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ") " << riid;

	if (!ppvObj)
	{
		return E_POINTER;
	}

	if (riid == IID_GetRealInterface)
	{
		*ppvObj = ProxyInterface;
		return D3D_OK;
	}
	if (riid == IID_GetInterfaceX)
	{
		*ppvObj = this;
		return D3D_OK;
	}

	if (riid == IID_IDirectDraw || riid == IID_IDirectDraw2 || riid == IID_IDirectDraw3 || riid == IID_IDirectDraw4 || riid == IID_IDirectDraw7)
	{
		DWORD DxVersion = GetGUIDVersion(riid);

		if (ddrawParent)
		{
			*ppvObj = ddrawParent->GetWrapperInterfaceX(DxVersion);

			ddrawParent->AddRef(DxVersion);

			return DD_OK;
		}
	}

	if (DirectXVersion != 1 && DirectXVersion != 2 && DirectXVersion != 3 && DirectXVersion != 7)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: wrapper interface version not found: " << DirectXVersion);
		return E_NOINTERFACE;
	}

	DWORD DxVersion = (CheckWrapperType(riid) && (Config.Dd7to9 || Config.ConvertToDirect3D7)) ? GetGUIDVersion(riid) : DirectXVersion;

	if (riid == GetWrapperType(DxVersion) || riid == IID_IUnknown)
	{
		*ppvObj = GetWrapperInterfaceX(DxVersion);

		AddRef(DxVersion);

		return D3D_OK;
	}

	return ProxyQueryInterface(ProxyInterface, riid, ppvObj, GetWrapperType(DxVersion));
}

void *m_IDirect3DX::GetWrapperInterfaceX(DWORD DirectXVersion)
{
	switch (DirectXVersion)
	{
	case 1:
		return WrapperInterface;
	case 2:
		return WrapperInterface2;
	case 3:
		return WrapperInterface3;
	case 7:
		return WrapperInterface7;
	default:
		LOG_LIMIT(100, __FUNCTION__ << " Error: wrapper interface version not found: " << DirectXVersion);
		return nullptr;
	}
}

ULONG m_IDirect3DX::AddRef(DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		switch (DirectXVersion)
		{
		case 1:
			return InterlockedIncrement(&RefCount1);
		case 2:
			return InterlockedIncrement(&RefCount2);
		case 3:
			return InterlockedIncrement(&RefCount3);
		case 7:
			return InterlockedIncrement(&RefCount7);
		default:
			LOG_LIMIT(100, __FUNCTION__ << " Error: wrapper interface version not found: " << DirectXVersion);
			return 0;
		}
	}

	return ProxyInterface->AddRef();
}

ULONG m_IDirect3DX::Release(DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	ULONG ref;

	if (Config.Dd7to9)
	{
		switch (DirectXVersion)
		{
		case 1:
			ref = (InterlockedCompareExchange(&RefCount1, 0, 0)) ? InterlockedDecrement(&RefCount1) : 0;
			break;
		case 2:
			ref = (InterlockedCompareExchange(&RefCount2, 0, 0)) ? InterlockedDecrement(&RefCount2) : 0;
			break;
		case 3:
			ref = (InterlockedCompareExchange(&RefCount3, 0, 0)) ? InterlockedDecrement(&RefCount3) : 0;
			break;
		case 7:
			ref = (InterlockedCompareExchange(&RefCount7, 0, 0)) ? InterlockedDecrement(&RefCount7) : 0;
			break;
		default:
			LOG_LIMIT(100, __FUNCTION__ << " Error: wrapper interface version not found: " << DirectXVersion);
			ref = 0;
		}

		if (InterlockedCompareExchange(&RefCount1, 0, 0) + InterlockedCompareExchange(&RefCount2, 0, 0) +
			InterlockedCompareExchange(&RefCount3, 0, 0) + InterlockedCompareExchange(&RefCount7, 0, 0) == 0)
		{
			delete this;
		}
	}
	else
	{
		ref = ProxyInterface->Release();

		if (ref == 0)
		{
			delete this;
		}
	}

	return ref;
}

HRESULT m_IDirect3DX::Initialize(REFCLSID rclsid)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (ProxyDirectXVersion != 1)
	{
		// The method returns DDERR_ALREADYINITIALIZED because the IDirect3D object is initialized when it is created.
		return DDERR_ALREADYINITIALIZED;
	}

	return GetProxyInterfaceV1()->Initialize(rclsid);
}

HRESULT m_IDirect3DX::EnumDevices(LPD3DENUMDEVICESCALLBACK lpEnumDevicesCallback, LPVOID lpUserArg, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

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
		if (!lpEnumDevicesCallback)
		{
			return DDERR_INVALIDPARAMS;
		}

		struct EnumDevicesStruct7
		{
			LPVOID lpContext;
			LPD3DENUMDEVICESCALLBACK lpCallback;

			static HRESULT CALLBACK ConvertCallback(LPSTR lpDeviceDescription, LPSTR lpDeviceName, LPD3DDEVICEDESC7 lpDeviceDesc, LPVOID lpContext)
			{
				EnumDevicesStruct7* self = (EnumDevicesStruct7*)lpContext;

				D3DDEVICEDESC D3DDRVDevDesc = {}, D3DSWDevDesc = {};
				D3DDRVDevDesc.dwSize = sizeof(D3DDEVICEDESC);
				D3DSWDevDesc.dwSize = sizeof(D3DDEVICEDESC);
				ConvertDeviceDesc(D3DDRVDevDesc, *lpDeviceDesc);
				ConvertDeviceDesc(D3DSWDevDesc, *lpDeviceDesc);

				return self->lpCallback(&lpDeviceDesc->deviceGUID, lpDeviceDescription, lpDeviceName, &D3DDRVDevDesc, &D3DSWDevDesc, self->lpContext);
			}
		} CallbackContext7 = {};
		CallbackContext7.lpContext = lpUserArg;
		CallbackContext7.lpCallback = lpEnumDevicesCallback;

		return EnumDevices7(EnumDevicesStruct7::ConvertCallback, &CallbackContext7, 7);
	}
	case 9:
		return EnumDevices7((LPD3DENUMDEVICESCALLBACK7)lpEnumDevicesCallback, lpUserArg, DirectXVersion);
	default:
		return DDERR_GENERIC;
	}
}

void m_IDirect3DX::GetCap9Cache()
{
	// Check for device
	if (ddrawParent)
	{
		// Get d3d9Object
		IDirect3D9* d3d9Object = ddrawParent->GetDirect3D9Object();
		if (d3d9Object)
		{
			UINT AdapterCount = d3d9Object->GetAdapterCount();
			if (AdapterCount)
			{
				Cap9Cache.clear();

				// Loop through all adapters
				for (UINT i = 0; i < AdapterCount; i++)
				{
					// Get Device Caps
					DUALCAP9 DCaps9;
					if (SUCCEEDED(d3d9Object->GetDeviceCaps(i, D3DDEVTYPE_REF, &DCaps9.REF)) && SUCCEEDED(d3d9Object->GetDeviceCaps(i, D3DDEVTYPE_HAL, &DCaps9.HAL)))
					{
						Cap9Cache.push_back(DCaps9);
					}
				}
			}
		}
	}
}

HRESULT m_IDirect3DX::EnumDevices7(LPD3DENUMDEVICESCALLBACK7 lpEnumDevicesCallback7, LPVOID lpUserArg, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lpEnumDevicesCallback7)
		{
			return DDERR_INVALIDPARAMS;
		}

		// Conversion callback
		LPD3DENUMDEVICESCALLBACK lpEnumDevicesCallback = (LPD3DENUMDEVICESCALLBACK)lpEnumDevicesCallback7;

		// Update Cap9 cache
		GetCap9Cache();

		// Check cache
		if (Cap9Cache.empty())
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: could not get Cap9 cache!");
			return DDERR_INVALIDOBJECT;
		}

		// Loop through all adapters
		for (auto& entry : Cap9Cache)
		{
			for (int x = 0; x < 3; x++)
			{
				// Get Device Caps
				D3DCAPS9 Caps9 = (x == 0) ? entry.REF : entry.HAL;
				D3DDEVTYPE Type = (x == 0) ? D3DDEVTYPE_REF : (x == 1) ? D3DDEVTYPE_HAL : (D3DDEVTYPE)(D3DDEVTYPE_HAL + 0x10);

				// Convert device desc
				D3DDEVICEDESC7 DeviceDesc7;
				Caps9.DeviceType = Type;
				ConvertDeviceDesc(DeviceDesc7, Caps9);

				GUID deviceGUID = DeviceDesc7.deviceGUID;
				LPSTR lpDescription = nullptr, lpName = nullptr;

				// For conversion
				D3DDEVICEDESC D3DDRVDevDesc = {}, D3DSWDevDesc = {};
				DWORD DevSize = (DirectXVersion == 1) ? D3DDEVICEDESC1_SIZE :
					(DirectXVersion == 2) ? D3DDEVICEDESC5_SIZE :
					(DirectXVersion == 3) ? D3DDEVICEDESC6_SIZE : sizeof(D3DDEVICEDESC);
				D3DDRVDevDesc.dwSize = DevSize;
				D3DSWDevDesc.dwSize = DevSize;

				switch ((DWORD)Type)
				{
				case D3DDEVTYPE_REF:
					lpName = "RGB Emulation";
					lpDescription = "Microsoft Direct3D RGB Software Emulation";
					if (DirectXVersion < 7)
					{
						// Get D3DSWDevDesc data (D3DDEVTYPE_REF)
						ConvertDeviceDesc(D3DSWDevDesc, DeviceDesc7);

						// Get D3DDRVDevDesc data (D3DDEVTYPE_HAL)
						Caps9 = entry.HAL;
						Caps9.DeviceType = D3DDEVTYPE_HAL;
						ConvertDeviceDesc(DeviceDesc7, Caps9);
						ConvertDeviceDesc(D3DDRVDevDesc, DeviceDesc7);

						if (lpEnumDevicesCallback(&deviceGUID, lpDescription, lpName, &D3DDRVDevDesc, &D3DSWDevDesc, lpUserArg) == DDENUMRET_CANCEL)
						{
							return D3D_OK;
						}
					}
					break;
				case D3DDEVTYPE_HAL:
					lpName = "Direct3D HAL";
					lpDescription = "Microsoft Direct3D Hardware acceleration through Direct3D HAL";
					if (DirectXVersion < 7)
					{
						// Get D3DDRVDevDesc data (D3DDEVTYPE_HAL)
						ConvertDeviceDesc(D3DDRVDevDesc, DeviceDesc7);

						// Get D3DSWDevDesc data (D3DDEVTYPE_REF)
						Caps9 = entry.REF;
						Caps9.DeviceType = D3DDEVTYPE_REF;
						ConvertDeviceDesc(DeviceDesc7, Caps9);
						ConvertDeviceDesc(D3DSWDevDesc, DeviceDesc7);

						if (lpEnumDevicesCallback(&deviceGUID, lpDescription, lpName, &D3DDRVDevDesc, &D3DSWDevDesc, lpUserArg) == DDENUMRET_CANCEL)
						{
							return D3D_OK;
						}
					}
					break;
				default:
				case D3DDEVTYPE_HAL + 0x10:
					lpName = "Direct3D T&L HAL";
					lpDescription = "Microsoft Direct3D Hardware Transform and Lighting acceleration capable device";
					break;
				}

				if (DirectXVersion == 7)
				{
					if (lpEnumDevicesCallback7(lpDescription, lpName, &DeviceDesc7, lpUserArg) == DDENUMRET_CANCEL)
					{
						return D3D_OK;
					}
				}
			}
		}

		return D3D_OK;
	}

	return GetProxyInterfaceV7()->EnumDevices(lpEnumDevicesCallback7, lpUserArg);
}

HRESULT m_IDirect3DX::CreateLight(LPDIRECT3DLIGHT * lplpDirect3DLight, LPUNKNOWN pUnkOuter)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

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
	{
		if (!lplpDirect3DLight)
		{
			return DDERR_INVALIDPARAMS;
		}

		// Check for device
		if (!ddrawParent)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: no ddraw parent!");
			return DDERR_INVALIDOBJECT;
		}

		*lplpDirect3DLight = (LPDIRECT3DLIGHT)new m_IDirect3DLight(ddrawParent->GetCurrentD3DDevice());

		return D3D_OK;
	}
	default:
		return DDERR_GENERIC;
	}

	if (SUCCEEDED(hr) && lplpDirect3DLight)
	{
		*lplpDirect3DLight = new m_IDirect3DLight(*lplpDirect3DLight);
	}

	return hr;
}

HRESULT m_IDirect3DX::CreateMaterial(LPDIRECT3DMATERIAL3 * lplpDirect3DMaterial, LPUNKNOWN pUnkOuter, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	DirectXVersion = (DirectXVersion < 3) ? DirectXVersion : 3;

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
	case 9:
	{
		if (!lplpDirect3DMaterial)
		{
			return DDERR_INVALIDPARAMS;
		}

		// Check for device
		if (!ddrawParent)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: no ddraw parent!");
			return DDERR_INVALIDOBJECT;
		}

		m_IDirect3DMaterialX *Interface = new m_IDirect3DMaterialX(ddrawParent->GetCurrentD3DDevice(), DirectXVersion);

		*lplpDirect3DMaterial = (LPDIRECT3DMATERIAL3)Interface->GetWrapperInterfaceX(DirectXVersion);

		return D3D_OK;
	}
	default:
		return DDERR_GENERIC;
	}

	if (SUCCEEDED(hr) && lplpDirect3DMaterial)
	{
		m_IDirect3DMaterialX *Interface = new m_IDirect3DMaterialX((IDirect3DMaterial3*)*lplpDirect3DMaterial, DirectXVersion);

		*lplpDirect3DMaterial = (LPDIRECT3DMATERIAL3)Interface->GetWrapperInterfaceX(DirectXVersion);
	}

	return hr;
}

HRESULT m_IDirect3DX::CreateViewport(LPDIRECT3DVIEWPORT3 * lplpD3DViewport, LPUNKNOWN pUnkOuter, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	DirectXVersion = (DirectXVersion < 3) ? DirectXVersion : 3;

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
	case 9:
	{
		if (!lplpD3DViewport)
		{
			return DDERR_INVALIDPARAMS;
		}

		// Check for device
		if (!ddrawParent)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: no ddraw parent!");
			return DDERR_INVALIDOBJECT;
		}

		m_IDirect3DViewportX *Interface = new m_IDirect3DViewportX(ddrawParent->GetCurrentD3DDevice(), DirectXVersion);

		*lplpD3DViewport = (LPDIRECT3DVIEWPORT3)Interface->GetWrapperInterfaceX(DirectXVersion);

		return D3D_OK;
	}
	default:
		return DDERR_GENERIC;
	}

	if (SUCCEEDED(hr) && lplpD3DViewport)
	{
		m_IDirect3DViewportX *Interface = new m_IDirect3DViewportX((IDirect3DViewport3*)*lplpD3DViewport, DirectXVersion);

		*lplpD3DViewport = (LPDIRECT3DVIEWPORT3)Interface->GetWrapperInterfaceX(DirectXVersion);
	}

	return hr;
}

HRESULT m_IDirect3DX::FindDevice(LPD3DFINDDEVICESEARCH lpD3DFDS, LPD3DFINDDEVICERESULT lpD3DFDR)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

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
	{
		struct EnumFindDevice
		{
			bool Found = false;
			GUID guid = IID_IUnknown;
			D3DDEVICEDESC7 DeviceDesc7 = {};

			static HRESULT CALLBACK ConvertCallback(LPSTR lpDeviceDescription, LPSTR lpDeviceName, LPD3DDEVICEDESC7 lpDeviceDesc7, LPVOID lpContext)
			{
				UNREFERENCED_PARAMETER(lpDeviceDescription);
				UNREFERENCED_PARAMETER(lpDeviceName);

				EnumFindDevice* self = (EnumFindDevice*)lpContext;

				if (lpDeviceDesc7->deviceGUID == self->guid)
				{
					self->Found = true;
					self->DeviceDesc7 = *lpDeviceDesc7;

					return DDENUMRET_CANCEL;
				}

				return DDENUMRET_OK;
			}
		} CallbackContext;
		CallbackContext.guid = lpD3DFDS->guid;

		EnumDevices7(EnumFindDevice::ConvertCallback, &CallbackContext, 7);

		if (CallbackContext.Found)
		{
			lpD3DFDR->guid = CallbackContext.DeviceDesc7.deviceGUID;
			lpD3DFDR->ddHwDesc.dwSize = (lpD3DFDR->dwSize - sizeof(DWORD) - sizeof(GUID)) / 2;
			ConvertDeviceDesc(lpD3DFDR->ddHwDesc, CallbackContext.DeviceDesc7);
			lpD3DFDR->ddSwDesc.dwSize = (lpD3DFDR->dwSize - sizeof(DWORD) - sizeof(GUID)) / 2;
			ConvertDeviceDescSoft(lpD3DFDR->ddSwDesc);

			return D3D_OK;
		}

		return DDERR_NOTFOUND;
	}
	default:
		return DDERR_GENERIC;
	}
}

HRESULT m_IDirect3DX::CreateDevice(REFCLSID rclsid, LPDIRECTDRAWSURFACE7 lpDDS, LPDIRECT3DDEVICE7 * lplpD3DDevice, LPUNKNOWN pUnkOuter, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	REFCLSID riid = (rclsid == IID_IDirect3DRampDevice) ? IID_IDirect3DRGBDevice : (ProxyDirectXVersion != 7) ? rclsid :
		(rclsid == IID_IDirect3DTnLHalDevice || rclsid == IID_IDirect3DHALDevice || rclsid == IID_IDirect3DMMXDevice || rclsid == IID_IDirect3DRGBDevice) ? rclsid : IID_IDirect3DRGBDevice;

	if (Config.Dd7to9)
	{
		if (!lplpD3DDevice || !lpDDS)
		{
			return DDERR_INVALIDPARAMS;
		}

		// Check for device
		if (!ddrawParent)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: no ddraw parent!");
			return DDERR_INVALIDOBJECT;
		}

		// Get surfaceX
		m_IDirectDrawSurfaceX *DdrawSurface3D = nullptr;
		lpDDS->QueryInterface(IID_GetInterfaceX, (LPVOID*)&DdrawSurface3D);

		// Check for Direct3D surface
		if (!DdrawSurface3D || !DdrawSurface3D->IsSurface3D())
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: surface is not a Direct3D surface!");
			return DDERR_INVALIDPARAMS;
		}

		m_IDirect3DDeviceX *p_IDirect3DDeviceX = new m_IDirect3DDeviceX(ddrawParent, lpDDS, riid, DirectXVersion);

		*lplpD3DDevice = (LPDIRECT3DDEVICE7)p_IDirect3DDeviceX->GetWrapperInterfaceX(DirectXVersion);

		return D3D_OK;
	}

	if (lpDDS)
	{
		lpDDS->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpDDS);
	}

	HRESULT hr = DDERR_GENERIC;

	switch (ProxyDirectXVersion)
	{
	case 1:
	default:
		return DDERR_GENERIC;
	case 2:
		hr = GetProxyInterfaceV2()->CreateDevice(riid, (LPDIRECTDRAWSURFACE)lpDDS, (LPDIRECT3DDEVICE2*)lplpD3DDevice);
		break;
	case 3:
		hr = GetProxyInterfaceV3()->CreateDevice(riid, (LPDIRECTDRAWSURFACE4)lpDDS, (LPDIRECT3DDEVICE3*)lplpD3DDevice, pUnkOuter);
		break;
	case 7:
		hr = GetProxyInterfaceV7()->CreateDevice(riid, lpDDS, lplpD3DDevice);
		break;
	}

	if (SUCCEEDED(hr) && lplpD3DDevice)
	{
		m_IDirect3DDeviceX *Interface = new m_IDirect3DDeviceX((m_IDirect3DDevice7*)*lplpD3DDevice, DirectXVersion);

		*lplpD3DDevice = (LPDIRECT3DDEVICE7)Interface->GetWrapperInterfaceX(DirectXVersion);

		if (Config.ConvertToDirect3D7 && ddrawParent)
		{
			Interface->SetDdrawParent(ddrawParent);
		}
	}

	return hr;
}

HRESULT m_IDirect3DX::CreateVertexBuffer(LPD3DVERTEXBUFFERDESC lpVBDesc, LPDIRECT3DVERTEXBUFFER7* lplpD3DVertexBuffer, DWORD dwFlags, LPUNKNOWN pUnkOuter, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	DirectXVersion = (DirectXVersion < 7) ? 1 : 7;

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
	{
		if (!lplpD3DVertexBuffer || !lpVBDesc)
		{
			return DDERR_INVALIDPARAMS;
		}

		if (!lpVBDesc->dwNumVertices)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: invalid number of vertices: " << *lpVBDesc);
			return DDERR_INVALIDPARAMS;
		}

		// Check FVF format
		if (!lpVBDesc->dwFVF || (lpVBDesc->dwFVF != D3DFVF_LVERTEX && (lpVBDesc->dwFVF & ~(D3DFVF_POSITION_MASK | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEXCOUNT_MASK))))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: invalid or unsupported vertex buffer FVF: " << Logging::hex(lpVBDesc->dwFVF));
			return D3DERR_INVALIDVERTEXFORMAT;
		}

		// Check for device
		if (!ddrawParent)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: no ddraw parent!");
			return DDERR_INVALIDOBJECT;
		}

		m_IDirect3DVertexBufferX *Interface = new m_IDirect3DVertexBufferX(ddrawParent, lpVBDesc, DirectXVersion);

		*lplpD3DVertexBuffer = (LPDIRECT3DVERTEXBUFFER7)Interface->GetWrapperInterfaceX(DirectXVersion);

		return D3D_OK;
	}
	}

	if (SUCCEEDED(hr) && lplpD3DVertexBuffer)
	{
		m_IDirect3DVertexBufferX *Interface = new m_IDirect3DVertexBufferX((m_IDirect3DVertexBuffer7*)*lplpD3DVertexBuffer, DirectXVersion);

		*lplpD3DVertexBuffer = (LPDIRECT3DVERTEXBUFFER7)Interface->GetWrapperInterfaceX(DirectXVersion);
	}

	return hr;
}

HRESULT m_IDirect3DX::EnumZBufferFormats(REFCLSID riidDevice, LPD3DENUMPIXELFORMATSCALLBACK lpEnumCallback, LPVOID lpContext)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

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
		if (!lpEnumCallback)
		{
			return DDERR_INVALIDPARAMS;
		}

		// Check for device
		if (!ddrawParent)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: no ddraw parent!");
			return DDERR_INVALIDOBJECT;
		}

		// Get d3d9Object
		IDirect3D9 *d3d9Object = ddrawParent->GetDirect3D9Object();

		if (riidDevice == IID_IDirect3DRGBDevice || riidDevice == IID_IDirect3DMMXDevice || riidDevice == IID_IDirect3DRefDevice ||
			riidDevice == IID_IDirect3DHALDevice || riidDevice == IID_IDirect3DTnLHalDevice)
		{
			D3DDEVICEDESC7 Desc7 = {};
			D3DCAPS9 Caps9;
			ZeroMemory(&Caps9, sizeof(D3DCAPS9));
			Caps9.DeviceType = (riidDevice == IID_IDirect3DRGBDevice || riidDevice == IID_IDirect3DMMXDevice || riidDevice == IID_IDirect3DRefDevice) ? D3DDEVTYPE_REF :
				(riidDevice == IID_IDirect3DHALDevice) ? D3DDEVTYPE_HAL :
				(riidDevice == IID_IDirect3DTnLHalDevice) ? (D3DDEVTYPE)(D3DDEVTYPE_HAL + 0x10) :
				D3DDEVTYPE_NULLREF;

			ConvertDeviceDesc(Desc7, Caps9);

			DDPIXELFORMAT PixelFormat = {};
			PixelFormat.dwSize = sizeof(DDPIXELFORMAT);

			// Handle 16bit zBuffer
			if (Desc7.dwDeviceZBufferBitDepth & DDBD_16)
			{
				for (D3DFORMAT Format : { D3DFMT_D16, D3DFMT_D15S1 })
				{
					if (SUCCEEDED(d3d9Object->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, D9DisplayFormat, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, Format)))
					{
						SetPixelDisplayFormat(Format, PixelFormat);

						if (PixelFormat.dwFlags && lpEnumCallback(&PixelFormat, lpContext) == DDENUMRET_CANCEL)
						{
							return D3D_OK;
						}
					}
				}
			}

			// Handle 32bit zBuffer
			if (Desc7.dwDeviceZBufferBitDepth & DDBD_32)
			{
				for (D3DFORMAT Format : { D3DFMT_D32, D3DFMT_D24S8, D3DFMT_D24X8, D3DFMT_D24X4S4 })
				{
					if (SUCCEEDED(d3d9Object->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, D9DisplayFormat, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, Format)))
					{
						SetPixelDisplayFormat(Format, PixelFormat);

						if (PixelFormat.dwFlags && lpEnumCallback(&PixelFormat, lpContext) == DDENUMRET_CANCEL)
						{
							return D3D_OK;
						}
					}
				}
			}
			return D3D_OK;
		}
		else if (riidDevice == IID_IDirect3DRampDevice)
		{
			// The IID_IDirect3DRampDevice, used for the ramp emulation device, is not supported by interfaces later than IDirect3D2
			return DDERR_NOZBUFFERHW;
		}
		else if (riidDevice == IID_IDirect3DNullDevice)
		{
			// NullDevice renders nothing
			return D3D_OK;
		}
		else
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Not Implemented " << riidDevice);
			return DDERR_NOZBUFFERHW;
		}
	}
}

HRESULT m_IDirect3DX::EvictManagedTextures()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

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
		// Check for device
		if (!ddrawParent)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: no ddraw parent!");
			return DDERR_INVALIDOBJECT;
		}

		// Evict managed texture surfaces
		ddrawParent->EvictManagedTextures();
		return D3D_OK;
	}
}

/************************/
/*** Helper functions ***/
/************************/

void m_IDirect3DX::InitDirect3D(DWORD DirectXVersion)
{
	WrapperInterface = new m_IDirect3D((LPDIRECT3D)ProxyInterface, this);
	WrapperInterface2 = new m_IDirect3D2((LPDIRECT3D2)ProxyInterface, this);
	WrapperInterface3 = new m_IDirect3D3((LPDIRECT3D3)ProxyInterface, this);
	WrapperInterface7 = new m_IDirect3D7((LPDIRECT3D7)ProxyInterface, this);

	if (!Config.Dd7to9)
	{
		ResolutionHack();

		return;
	}

	if (ddrawParent)
	{
		ddrawParent->SetD3D(this);
	}

	// Get Cap9 cache
	GetCap9Cache();

	AddRef(DirectXVersion);
}

void m_IDirect3DX::ReleaseDirect3D()
{
	WrapperInterface->DeleteMe();
	WrapperInterface2->DeleteMe();
	WrapperInterface3->DeleteMe();
	WrapperInterface7->DeleteMe();

	if (ddrawParent && !Config.Exiting)
	{
		ddrawParent->ClearD3D();
	}
}

void m_IDirect3DX::ResolutionHack()
{
	if (Config.DdrawResolutionHack)
	{
		if (GetD3DPath)
		{
			GetD3DPath = false;
			GetSystemDirectory(D3DImPath, MAX_PATH);
			strcpy_s(D3DIm700Path, MAX_PATH, D3DImPath);
			strcat_s(D3DImPath, MAX_PATH, "\\d3dim.dll");
			strcat_s(D3DIm700Path, MAX_PATH, "\\d3dim700.dll");
		}

		if (!hD3DIm)
		{
			hD3DIm = GetModuleHandle(D3DImPath);
			if (hD3DIm)
			{
				LOG_LIMIT(3, __FUNCTION__ << " Found loaded dll: 'd3dim.dll'");
				Utils::DDrawResolutionHack(hD3DIm);
			}
		}

		if (!hD3DIm700)
		{
			hD3DIm700 = GetModuleHandle(D3DIm700Path);
			if (hD3DIm700)
			{
				LOG_LIMIT(3, __FUNCTION__ << " Found loaded dll: 'd3dim700.dll'");
				Utils::DDrawResolutionHack(hD3DIm700);
			}
		}
	}
}
