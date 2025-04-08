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

#include "ddraw.h"
#include "Utils\Utils.h"

namespace {
	bool GetD3DPath = true;
	char D3DImPath[MAX_PATH] = { '\0' };
	char D3DIm700Path[MAX_PATH] = { '\0' };
	HMODULE hD3DIm = nullptr;
	HMODULE hD3DIm700 = nullptr;
}

// ******************************
// IUnknown functions
// ******************************

HRESULT m_IDirect3DX::QueryInterface(REFIID riid, LPVOID FAR * ppvObj, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ") " << riid;

	if (!ppvObj)
	{
		return E_POINTER;
	}
	*ppvObj = nullptr;

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

	DWORD DxVersion = (CheckWrapperType(riid) && Config.Dd7to9) ? GetGUIDVersion(riid) : DirectXVersion;

	if ((riid == GetWrapperType(DxVersion) && (riid != IID_IDirect3D7 || DirectXVersion == 7)) || riid == IID_IUnknown)
	{
		if (riid != IID_IUnknown && ddrawParent && ((ddrawParent->IsCreatedEx() && riid != IID_IDirect3D7) || (!ddrawParent->IsCreatedEx() && riid == IID_IDirect3D7)))
		{
			return E_NOINTERFACE;
		}

		*ppvObj = GetWrapperInterfaceX(DxVersion);

		AddRef(DxVersion);

		return D3D_OK;
	}

	return ProxyQueryInterface(ProxyInterface, riid, ppvObj, GetWrapperType(DirectXVersion));
}

ULONG m_IDirect3DX::AddRef(DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		// Direct3D shares reference count with DirectDraw
		if (ddrawParent)
		{
			return ddrawParent->AddRef(DDrawVersion);
		}

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

	if (Config.Dd7to9)
	{
		// Direct3D shares reference count with DirectDraw
		if (ddrawParent)
		{
			return ddrawParent->Release(DDrawVersion);
		}

		ULONG ref;

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

		return ref;
	}

	ULONG ref = ProxyInterface->Release();

	if (ref == 0)
	{
		delete this;
	}

	return ref;
}

// ******************************
// IDirect3D v1 functions
// ******************************

HRESULT m_IDirect3DX::Initialize(REFCLSID rclsid)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		// The method returns DDERR_ALREADYINITIALIZED because the IDirect3D object is initialized when it is created.
		return DDERR_ALREADYINITIALIZED;
	}

	return GetProxyInterfaceV1()->Initialize(rclsid);
}

HRESULT m_IDirect3DX::EnumDevices(LPD3DENUMDEVICESCALLBACK lpEnumDevicesCallback, LPVOID lpUserArg, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		return EnumDevices7((LPD3DENUMDEVICESCALLBACK7)lpEnumDevicesCallback, lpUserArg, DirectXVersion);
	}

	switch (ProxyDirectXVersion)
	{
	case 1:
		return GetProxyInterfaceV1()->EnumDevices(lpEnumDevicesCallback, lpUserArg);
	case 2:
		return GetProxyInterfaceV2()->EnumDevices(lpEnumDevicesCallback, lpUserArg);
	case 3:
		return GetProxyInterfaceV3()->EnumDevices(lpEnumDevicesCallback, lpUserArg);
	case 7:
	default:
		return DDERR_GENERIC;
	}
}

HRESULT m_IDirect3DX::CreateLight(LPDIRECT3DLIGHT* lplpDirect3DLight, LPUNKNOWN pUnkOuter)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lplpDirect3DLight)
		{
			return DDERR_INVALIDPARAMS;
		}
		*lplpDirect3DLight = nullptr;

		// Check for device
		if (!ddrawParent)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: no ddraw parent!");
			return DDERR_INVALIDOBJECT;
		}

		m_IDirect3DLight* Interface = m_IDirect3DLight::CreateDirect3DLight(nullptr, this);

		*lplpDirect3DLight = (LPDIRECT3DLIGHT)Interface;

		return D3D_OK;
	}

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
	default:
		if (lplpDirect3DLight)
		{
			*lplpDirect3DLight = nullptr;
		}
		return DDERR_GENERIC;
	}

	if (SUCCEEDED(hr) && lplpDirect3DLight)
	{
		*lplpDirect3DLight = m_IDirect3DLight::CreateDirect3DLight(*lplpDirect3DLight, nullptr);
	}

	return hr;
}

HRESULT m_IDirect3DX::CreateMaterial(LPDIRECT3DMATERIAL3* lplpDirect3DMaterial, LPUNKNOWN pUnkOuter, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lplpDirect3DMaterial)
		{
			return DDERR_INVALIDPARAMS;
		}
		*lplpDirect3DMaterial = nullptr;

		// Check for device
		if (!ddrawParent)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: no ddraw parent!");
			return DDERR_INVALIDOBJECT;
		}

		m_IDirect3DMaterialX* Interface = new m_IDirect3DMaterialX(this, DirectXVersion);

		*lplpDirect3DMaterial = (LPDIRECT3DMATERIAL3)Interface->GetWrapperInterfaceX(DirectXVersion);

		return D3D_OK;
	}

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
	default:
		if (lplpDirect3DMaterial)
		{
			*lplpDirect3DMaterial = nullptr;
		}
		return DDERR_GENERIC;
	}

	if (SUCCEEDED(hr) && lplpDirect3DMaterial)
	{
		m_IDirect3DMaterialX* Interface = new m_IDirect3DMaterialX((IDirect3DMaterial3*)*lplpDirect3DMaterial, DirectXVersion);

		*lplpDirect3DMaterial = (LPDIRECT3DMATERIAL3)Interface->GetWrapperInterfaceX(DirectXVersion);
	}

	return hr;
}

HRESULT m_IDirect3DX::CreateViewport(LPDIRECT3DVIEWPORT3* lplpD3DViewport, LPUNKNOWN pUnkOuter, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lplpD3DViewport)
		{
			return DDERR_INVALIDPARAMS;
		}
		*lplpD3DViewport = nullptr;

		// Check for device
		if (!ddrawParent)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: no ddraw parent!");
			return DDERR_INVALIDOBJECT;
		}

		m_IDirect3DViewportX* Interface = new m_IDirect3DViewportX(this, DirectXVersion);

		*lplpD3DViewport = (LPDIRECT3DVIEWPORT3)Interface->GetWrapperInterfaceX(DirectXVersion);

		return D3D_OK;
	}

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
	default:
		if (lplpD3DViewport)
		{
			*lplpD3DViewport = nullptr;
		}
		return DDERR_GENERIC;
	}

	if (SUCCEEDED(hr) && lplpD3DViewport)
	{
		m_IDirect3DViewportX* Interface = new m_IDirect3DViewportX((IDirect3DViewport3*)*lplpD3DViewport, DirectXVersion);

		*lplpD3DViewport = (LPDIRECT3DVIEWPORT3)Interface->GetWrapperInterfaceX(DirectXVersion);
	}

	return hr;
}

HRESULT m_IDirect3DX::FindDevice(LPD3DFINDDEVICESEARCH lpD3DFDS, LPD3DFINDDEVICERESULT lpD3DFDR)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lpD3DFDS || !lpD3DFDR)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: invalid parameters!");
			return DDERR_INVALIDPARAMS;
		}

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
			DWORD Size = (lpD3DFDR->dwSize - sizeof(DWORD) - sizeof(GUID)) / 2;
			if (Size != D3DDEVICEDESC1_SIZE && Size != D3DDEVICEDESC5_SIZE && Size != D3DDEVICEDESC6_SIZE)
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: Incorrect dwSize: " << Size);
				return DDERR_INVALIDPARAMS;
			}

			lpD3DFDR->guid = CallbackContext.DeviceDesc7.deviceGUID;

			LPD3DDEVICEDESC lpddHwDesc = &lpD3DFDR->ddHwDesc;
			lpddHwDesc->dwSize = Size;
			ConvertDeviceDesc(*lpddHwDesc, CallbackContext.DeviceDesc7);

			LPD3DDEVICEDESC lpddSwDesc = (LPD3DDEVICEDESC)((DWORD)lpddHwDesc + Size);
			lpddSwDesc->dwSize = Size;
			ConvertDeviceDesc(*lpddSwDesc, CallbackContext.DeviceDesc7);

			return D3D_OK;
		}

		return DDERR_NOTFOUND;
	}

	switch (ProxyDirectXVersion)
	{
	case 1:
		return GetProxyInterfaceV1()->FindDevice(lpD3DFDS, lpD3DFDR);
	case 2:
		return GetProxyInterfaceV2()->FindDevice(lpD3DFDS, lpD3DFDR);
	case 3:
		return GetProxyInterfaceV3()->FindDevice(lpD3DFDS, lpD3DFDR);
	case 7:
	default:
		return DDERR_GENERIC;
	}
}

// ******************************
// IDirect3D v2 functions
// ******************************

HRESULT m_IDirect3DX::CreateDevice(REFCLSID rclsid, LPDIRECTDRAWSURFACE7 lpDDS, LPDIRECT3DDEVICE7* lplpD3DDevice, LPUNKNOWN pUnkOuter, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lplpD3DDevice || !lpDDS)
		{
			return DDERR_INVALIDPARAMS;
		}
		*lplpD3DDevice = nullptr;

		// Check for device
		if (!ddrawParent)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: no ddraw parent!");
			return DDERR_INVALIDOBJECT;
		}

		// Get surfaceX
		m_IDirectDrawSurfaceX* DdrawSurface3D = nullptr;
		lpDDS->QueryInterface(IID_GetInterfaceX, (LPVOID*)&DdrawSurface3D);

		// Check for Direct3D surface
		if (!DdrawSurface3D || !DdrawSurface3D->IsSurface3D())
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: surface is not a Direct3D surface!");
			return DDERR_INVALIDPARAMS;
		}

		// Check for existing device
		if (!D3DDeviceList.empty())
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Direct3DDevice is already setup. Multiple Direct3DDevice's are not implemented!");
		}

		m_IDirect3DDeviceX* Interface = new m_IDirect3DDeviceX(ddrawParent, this, lpDDS, rclsid, DirectXVersion);

		if (ddrawParent && ddrawParent->IsCreatedEx())
		{
			for (auto& entry : D3DDeviceList)
			{
				if (entry.Interface == Interface)
				{
					entry.RefCount = 7;
					entry.DxVersion = DirectXVersion;

					for (UINT x = 0; x < entry.RefCount; x++)
					{
						AddRef(DirectXVersion);
					}

					break;
				}
			}
		}

		*lplpD3DDevice = (LPDIRECT3DDEVICE7)Interface->GetWrapperInterfaceX(DirectXVersion);

		return D3D_OK;
	}

	REFCLSID riid = (rclsid == IID_IDirect3DRampDevice) ? IID_IDirect3DRGBDevice : (ProxyDirectXVersion != 7) ? rclsid :
		(rclsid == IID_IDirect3DTnLHalDevice || rclsid == IID_IDirect3DHALDevice || rclsid == IID_IDirect3DMMXDevice || rclsid == IID_IDirect3DRGBDevice) ? rclsid : IID_IDirect3DRGBDevice;

	HRESULT hr = DDERR_GENERIC;

	if (lpDDS)
	{
		lpDDS->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpDDS);
	}

	switch (ProxyDirectXVersion)
	{
	case 1:
	default:
		if (lplpD3DDevice)
		{
			*lplpD3DDevice = nullptr;
		}
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
		m_IDirect3DDeviceX* Interface = new m_IDirect3DDeviceX((m_IDirect3DDevice7*)*lplpD3DDevice, DirectXVersion);

		*lplpD3DDevice = (LPDIRECT3DDEVICE7)Interface->GetWrapperInterfaceX(DirectXVersion);
	}

	return hr;
}

// ******************************
// IDirect3D v3 functions
// ******************************

HRESULT m_IDirect3DX::CreateVertexBuffer(LPD3DVERTEXBUFFERDESC lpVBDesc, LPDIRECT3DVERTEXBUFFER7* lplpD3DVertexBuffer, DWORD dwFlags, LPUNKNOWN pUnkOuter, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	DWORD DxVersion = DirectXVersion == 7 ? DirectXVersion : 1;

	if (Config.Dd7to9)
	{
		if (!lplpD3DVertexBuffer || !lpVBDesc)
		{
			return DDERR_INVALIDPARAMS;
		}
		*lplpD3DVertexBuffer = nullptr;

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

		m_IDirect3DVertexBufferX* Interface = new m_IDirect3DVertexBufferX(ddrawParent, this, lpVBDesc, DxVersion);

		if (DirectXVersion > 3)
		{
			for (auto& entry : VertexBufferList)
			{
				if (entry.Interface == Interface)
				{
					entry.DxVersion = DirectXVersion;
					entry.RefCount = 1;

					AddRef(entry.DxVersion);

					break;
				}
			}
		}

		*lplpD3DVertexBuffer = (LPDIRECT3DVERTEXBUFFER7)Interface->GetWrapperInterfaceX(DxVersion);

		return D3D_OK;
	}

	HRESULT hr = DDERR_GENERIC;

	switch (ProxyDirectXVersion)
	{
	case 1:
	case 2:
	default:
		if (lplpD3DVertexBuffer)
		{
			*lplpD3DVertexBuffer = nullptr;
		}
		return DDERR_GENERIC;
	case 3:
		hr = GetProxyInterfaceV3()->CreateVertexBuffer(lpVBDesc, (LPDIRECT3DVERTEXBUFFER*)lplpD3DVertexBuffer, dwFlags, pUnkOuter);
		break;
	case 7:
		hr = GetProxyInterfaceV7()->CreateVertexBuffer(lpVBDesc, lplpD3DVertexBuffer, dwFlags);
		break;
	}

	if (SUCCEEDED(hr) && lplpD3DVertexBuffer)
	{
		m_IDirect3DVertexBufferX* Interface = new m_IDirect3DVertexBufferX((m_IDirect3DVertexBuffer7*)*lplpD3DVertexBuffer, DxVersion);

		*lplpD3DVertexBuffer = (LPDIRECT3DVERTEXBUFFER7)Interface->GetWrapperInterfaceX(DxVersion);
	}

	return hr;
}

HRESULT m_IDirect3DX::EnumZBufferFormats(REFCLSID riidDevice, LPD3DENUMPIXELFORMATSCALLBACK lpEnumCallback, LPVOID lpContext)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
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
		IDirect3D9* d3d9Object = ddrawParent->GetDirectD9Object();

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
	}
}

HRESULT m_IDirect3DX::EvictManagedTextures()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		// Textures are loaded as managed in Direct3D9, so there is no need to manualy evict the texture
		return D3D_OK;
	}

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
	}
}

// ******************************
// IDirect3D v7 functions
// ******************************

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

		DWORD DevSize = (DirectXVersion == 1) ? D3DDEVICEDESC1_SIZE :
			(DirectXVersion == 2) ? D3DDEVICEDESC5_SIZE :
			(DirectXVersion == 3) ? D3DDEVICEDESC6_SIZE : sizeof(D3DDEVICEDESC);

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
				D3DDRVDevDesc.dwSize = sizeof(D3DDEVICEDESC);
				D3DSWDevDesc.dwSize = sizeof(D3DDEVICEDESC);

				char Desc[MAX_PATH] = {};
				char Name[MAX_PATH] = {};

				switch ((DWORD)Type)
				{
				case D3DDEVTYPE_REF:
					lpName = "RGB Emulation";
					lpDescription = "Microsoft Direct3D RGB Software Emulation";
					break;
				case D3DDEVTYPE_HAL:
					lpName = "Direct3D HAL";
					lpDescription = "Microsoft Direct3D Hardware acceleration through Direct3D HAL";
					break;
				default:
				case D3DDEVTYPE_HAL + 0x10:
					lpName = "Direct3D T&L HAL";
					lpDescription = "Microsoft Direct3D Hardware Transform and Lighting acceleration capable device";
					break;
				}

				strcpy_s(Desc, lpDescription);
				strcpy_s(Name, lpName);

				if (DirectXVersion < 7)
				{
					// Get D3DSWDevDesc data (D3DDEVTYPE_REF)
					ConvertDeviceDesc(D3DSWDevDesc, DeviceDesc7);

					// Get D3DDRVDevDesc data (D3DDEVTYPE_HAL)
					Caps9 = entry.HAL;
					Caps9.DeviceType = D3DDEVTYPE_HAL;
					ConvertDeviceDesc(DeviceDesc7, Caps9);
					ConvertDeviceDesc(D3DDRVDevDesc, DeviceDesc7);

					D3DDRVDevDesc.dwSize = DevSize;
					D3DSWDevDesc.dwSize = DevSize;

					if (lpEnumDevicesCallback(&deviceGUID, Desc, Name, &D3DDRVDevDesc, &D3DSWDevDesc, lpUserArg) == DDENUMRET_CANCEL)
					{
						return D3D_OK;
					}
				}
				else if (DirectXVersion == 7)
				{
					if (lpEnumDevicesCallback7(Desc, Name, &DeviceDesc7, lpUserArg) == DDENUMRET_CANCEL)
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

// ******************************
// Helper functions
// ******************************

void m_IDirect3DX::InitInterface(DWORD DirectXVersion)
{
	ScopedDDCriticalSection ThreadLockDD;

	if (!Config.Dd7to9)
	{
		ResolutionHack();

		return;
	}

	AddRef(DirectXVersion);

	// Get Cap9 cache
	GetCap9Cache();
}

void m_IDirect3DX::ReleaseInterface()
{
	if (Config.Exiting)
	{
		return;
	}

	ScopedDDCriticalSection ThreadLockDD;

	// Release device
	for (auto& entry : D3DDeviceList)
	{
		entry.Interface->ClearD3D(this);
	}

	// Don't delete wrapper interface
	SaveInterfaceAddress(WrapperInterface);
	SaveInterfaceAddress(WrapperInterface2);
	SaveInterfaceAddress(WrapperInterface3);
	SaveInterfaceAddress(WrapperInterface7);

	// Release Light
	for (auto& entry : LightList)
	{
		entry->ClearD3D();
	}

	// Release Material
	for (auto& entry : MaterialList)
	{
		entry->ClearD3D();
	}

	// Release Vertex Buffer
	for (auto& entry : VertexBufferList)
	{
		entry.Interface->ClearD3D();
	}

	// Release Viewport
	for (auto& entry : ViewportList)
	{
		entry->ClearD3D();
	}
}

void* m_IDirect3DX::GetWrapperInterfaceX(DWORD DirectXVersion)
{
	switch (DirectXVersion)
	{
	case 0:
		if (WrapperInterface7) return WrapperInterface7;
		if (WrapperInterface3) return WrapperInterface3;
		if (WrapperInterface2) return WrapperInterface2;
		if (WrapperInterface) return WrapperInterface;
		break;
	case 1:
		return GetInterfaceAddress(WrapperInterface, (LPDIRECT3D)ProxyInterface, this);
	case 2:
		return GetInterfaceAddress(WrapperInterface2, (LPDIRECT3D2)ProxyInterface, this);
	case 3:
		return GetInterfaceAddress(WrapperInterface3, (LPDIRECT3D3)ProxyInterface, this);
	case 7:
		return GetInterfaceAddress(WrapperInterface7, (LPDIRECT3D7)ProxyInterface, this);
	}
	LOG_LIMIT(100, __FUNCTION__ << " Error: wrapper interface version not found: " << DirectXVersion);
	return nullptr;
}

void m_IDirect3DX::GetCap9Cache()
{
	// Check for device
	if (ddrawParent)
	{
		// Get d3d9Object
		IDirect3D9* d3d9Object = ddrawParent->GetDirectD9Object();
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

void m_IDirect3DX::AddLight(m_IDirect3DLight* lpLight)
{
	if (!lpLight)
	{
		return;
	}

	LightList.push_back(lpLight);
}

void m_IDirect3DX::ClearLight(m_IDirect3DLight* lpLight)
{
	// Find and remove the light from the list
	auto it = std::find(LightList.begin(), LightList.end(), lpLight);
	if (it != LightList.end())
	{
		LightList.erase(it);
	}
	// Remove light for all D3D devices
	for (auto& entry : D3DDeviceList)
	{
		entry.Interface->ClearLight(lpLight);
	}
}

void m_IDirect3DX::AddMaterial(m_IDirect3DMaterialX* lpMaterialX)
{
	if (!lpMaterialX)
	{
		return;
	}

	MaterialList.push_back(lpMaterialX);
}

void m_IDirect3DX::ClearMaterial(m_IDirect3DMaterialX* lpMaterialX, D3DMATERIALHANDLE mHandle)
{
	// Find and remove the material from the list
	auto it = std::find(MaterialList.begin(), MaterialList.end(), lpMaterialX);
	if (it != MaterialList.end())
	{
		MaterialList.erase(it);
	}
	// Remove material handle for all D3D devices
	if (mHandle)
	{
		for (auto& entry : D3DDeviceList)
		{
			entry.Interface->ClearMaterialHandle(mHandle);
		}
	}
}

void m_IDirect3DX::AddViewport(m_IDirect3DViewportX* lpViewportX)
{
	if (!lpViewportX)
	{
		return;
	}

	ViewportList.push_back(lpViewportX);
}

void m_IDirect3DX::ClearViewport(m_IDirect3DViewportX* lpViewportX)
{
	// Find and remove the viewport from the list
	auto it = std::find(ViewportList.begin(), ViewportList.end(), lpViewportX);
	if (it != ViewportList.end())
	{
		ViewportList.erase(it);
	}
}

void m_IDirect3DX::GetViewportResolution(DWORD& Width, DWORD& Height)
{
	if (ddrawParent)
	{
		ddrawParent->GetViewportResolution(Width, Height);
	}
}

void m_IDirect3DX::AddD3DDevice(m_IDirect3DDeviceX* lpD3DDevice)
{
	if (!lpD3DDevice)
	{
		return;
	}

	D3DDeviceList.push_back({ lpD3DDevice, 0, 0 });
}

void m_IDirect3DX::ClearD3DDevice(m_IDirect3DDeviceX* lpD3DDevice)
{
	// Find and remove the device from the list
	auto it = std::find_if(D3DDeviceList.begin(), D3DDeviceList.end(),
		[lpD3DDevice](auto entry) {
			return entry.Interface == lpD3DDevice;
		});
	if (it != D3DDeviceList.end())
	{
		DWORD RefCount = it->RefCount;
		DWORD DxVersion = it->DxVersion;
		D3DDeviceList.erase(it);	// Erase from list before releasing
		if (ddrawParent && D3DDeviceList.empty())
		{
			ddrawParent->ClearD3DDevice();
		}
		if (RefCount)
		{
			for (UINT x = 0; x < RefCount; x++)
			{
				Release(DxVersion);
			}
		}
	}
}

void m_IDirect3DX::AddVertexBuffer(m_IDirect3DVertexBufferX* lpVertexBufferX)
{
	if (!lpVertexBufferX)
	{
		return;
	}

	VertexBufferList.push_back({ lpVertexBufferX, 0, 0 });
}

void m_IDirect3DX::ClearVertexBuffer(m_IDirect3DVertexBufferX* lpVertexBufferX)
{
	// Find and remove the buffer from the list
	auto it = std::find_if(VertexBufferList.begin(), VertexBufferList.end(),
		[lpVertexBufferX](auto entry) {
			return entry.Interface == lpVertexBufferX;
		});
	if (it != VertexBufferList.end())
	{
		DWORD RefCount = it->RefCount;
		DWORD DxVersion = it->DxVersion;
		VertexBufferList.erase(it);	// Erase from list before releasing
		if (RefCount == 1)
		{
			Release(DxVersion);
		}
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

void m_IDirect3DX::ClearDdraw()
{
	ddrawParent = nullptr;

	for (auto& entry : D3DDeviceList)
	{
		entry.Interface->ClearDdraw();
	}
}
