/**
* Copyright (C) 2024 Elisha Riedlinger
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

namespace {
	m_IDirect3DMaterial* WrapperInterfaceBackup = nullptr;
	m_IDirect3DMaterial2* WrapperInterfaceBackup2 = nullptr;
	m_IDirect3DMaterial3* WrapperInterfaceBackup3 = nullptr;
}

// ******************************
// IUnknown functions
// ******************************

HRESULT m_IDirect3DMaterialX::QueryInterface(REFIID riid, LPVOID FAR * ppvObj, DWORD DirectXVersion)
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

	DWORD DxVersion = (CheckWrapperType(riid) && Config.Dd7to9) ? GetGUIDVersion(riid) : DirectXVersion;

	if (riid == GetWrapperType(DxVersion) || riid == IID_IUnknown)
	{
		*ppvObj = GetWrapperInterfaceX(DxVersion);

		AddRef(DxVersion);

		return D3D_OK;
	}

	return ProxyQueryInterface(ProxyInterface, riid, ppvObj, GetWrapperType(DirectXVersion));
}

ULONG m_IDirect3DMaterialX::AddRef(DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ") v" << DirectXVersion;

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
		default:
			LOG_LIMIT(100, __FUNCTION__ << " Error: wrapper interface version not found: " << DirectXVersion);
			return 0;
		}
	}

	return ProxyInterface->AddRef();
}

ULONG m_IDirect3DMaterialX::Release(DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ") v" << DirectXVersion;

	if (Config.Dd7to9)
	{
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
		default:
			LOG_LIMIT(100, __FUNCTION__ << " Error: wrapper interface version not found: " << DirectXVersion);
			ref = 0;
		}

		if (InterlockedCompareExchange(&RefCount1, 0, 0) + InterlockedCompareExchange(&RefCount2, 0, 0) +
			InterlockedCompareExchange(&RefCount3, 0, 0) == 0)
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
// IDirect3DMaterial functions
// ******************************

HRESULT m_IDirect3DMaterialX::Initialize(LPDIRECT3D lplpD3D)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		// The method returns DDERR_ALREADYINITIALIZED because the IDirect3DMaterial object is initialized when it is created.
		return DDERR_ALREADYINITIALIZED;
	}

	if (lplpD3D)
	{
		lplpD3D->QueryInterface(IID_GetRealInterface, (LPVOID*)&lplpD3D);
	}

	return GetProxyInterfaceV1()->Initialize(lplpD3D);
}

HRESULT m_IDirect3DMaterialX::SetMaterial(LPD3DMATERIAL lpMat)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lpMat || lpMat->dwSize != sizeof(D3DMATERIAL))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Incorrect dwSize: " << ((lpMat) ? lpMat->dwSize : -1));
			return DDERR_INVALIDPARAMS;
		}

		// If current material is set then use new material
		if (mHandle && SUCCEEDED(CheckInterface(__FUNCTION__)) && (*D3DDeviceInterface)->CheckIfMaterialSet(mHandle))
		{
			if (FAILED((*D3DDeviceInterface)->SetMaterial(lpMat)))
			{
				return DDERR_GENERIC;
			}
		}

		Material = *lpMat;

		return D3D_OK;
	}

	switch (ProxyDirectXVersion)
	{
	case 1:
		return GetProxyInterfaceV1()->SetMaterial(lpMat);
	case 2:
		return GetProxyInterfaceV2()->SetMaterial(lpMat);
	case 3:
		return GetProxyInterfaceV3()->SetMaterial(lpMat);
	default:
		return DDERR_GENERIC;
	}
}

HRESULT m_IDirect3DMaterialX::GetMaterial(LPD3DMATERIAL lpMat)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lpMat || lpMat->dwSize != sizeof(D3DMATERIAL))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Incorrect dwSize: " << ((lpMat) ? lpMat->dwSize : -1));
			return DDERR_INVALIDPARAMS;
		}

		*lpMat = Material;

		return D3D_OK;
	}

	switch (ProxyDirectXVersion)
	{
	case 1:
		return GetProxyInterfaceV1()->GetMaterial(lpMat);
	case 2:
		return GetProxyInterfaceV2()->GetMaterial(lpMat);
	case 3:
		return GetProxyInterfaceV3()->GetMaterial(lpMat);
	default:
		return DDERR_GENERIC;
	}
}

HRESULT m_IDirect3DMaterialX::GetHandle(LPDIRECT3DDEVICE3 lpDirect3DDevice, LPD3DMATERIALHANDLE lpHandle)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lpDirect3DDevice || !lpHandle)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: called with nullptr: " << lpDirect3DDevice << " " << lpHandle);
			return DDERR_INVALIDPARAMS;
		}

		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: D3DDevice does not exist!");
			return DDERR_GENERIC;
		}

		m_IDirect3DDeviceX* pDirect3DDeviceX = nullptr;
		lpDirect3DDevice->QueryInterface(IID_GetInterfaceX, (LPVOID*)&pDirect3DDeviceX);
		if (!pDirect3DDeviceX)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: could not get Direct3D Device wrapper!");
			return DDERR_INVALIDPARAMS;
		}

		if (*D3DDeviceInterface != pDirect3DDeviceX)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: Direct3D Device wrapper does not match! " << *D3DDeviceInterface << "->" << pDirect3DDeviceX);
		}

		if (!mHandle)
		{
			mHandle = (DWORD)this;
		}

		// Makes mHandle unique and then stores it
		(*D3DDeviceInterface)->SetMaterialHandle(mHandle, this);

		// Set lpHandle after setting material handle in D3D device
		*lpHandle = mHandle;

		return D3D_OK;
	}

	if (lpDirect3DDevice)
	{
		lpDirect3DDevice->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpDirect3DDevice);
	}

	switch (ProxyDirectXVersion)
	{
	case 1:
		return GetProxyInterfaceV1()->GetHandle((LPDIRECT3DDEVICE)lpDirect3DDevice, lpHandle);
	case 2:
		return GetProxyInterfaceV2()->GetHandle((LPDIRECT3DDEVICE2)lpDirect3DDevice, lpHandle);
	case 3:
		return GetProxyInterfaceV3()->GetHandle(lpDirect3DDevice, lpHandle);
	default:
		return DDERR_GENERIC;
	}
}

HRESULT m_IDirect3DMaterialX::Reserve()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		// Former stub method. This method was never implemented and is not supported in any interface.
		return D3D_OK;
	}

	return GetProxyInterfaceV1()->Reserve();
}

HRESULT m_IDirect3DMaterialX::Unreserve()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		// Former stub method. This method was never implemented and is not supported in any interface.
		return D3D_OK;
	}

	return GetProxyInterfaceV1()->Unreserve();
}

// ******************************
// Helper functions
// ******************************

void m_IDirect3DMaterialX::InitInterface(DWORD DirectXVersion)
{
	if (D3DInterface)
	{
		D3DInterface->AddMaterial(this);
	}

	if (Config.Dd7to9)
	{
		Material.dwSize = sizeof(D3DMATERIAL);

		AddRef(DirectXVersion);
	}
}

void m_IDirect3DMaterialX::ReleaseInterface()
{
	if (Config.Exiting)
	{
		return;
	}

	if (D3DInterface)
	{
		D3DInterface->ClearMaterial(this);
	}

	// Don't delete wrapper interface
	SaveInterfaceAddress(WrapperInterface, WrapperInterfaceBackup);
	SaveInterfaceAddress(WrapperInterface2, WrapperInterfaceBackup2);
	SaveInterfaceAddress(WrapperInterface3, WrapperInterfaceBackup3);

	if (mHandle && D3DDeviceInterface && *D3DDeviceInterface)
	{
		(*D3DDeviceInterface)->ClearMaterialHandle(mHandle);
	}
}

HRESULT m_IDirect3DMaterialX::CheckInterface(char* FunctionName)
{
	// Check D3DInterface device
	if (!D3DInterface)
	{
		LOG_LIMIT(100, FunctionName << " Error: no D3D parent!");
		return DDERR_INVALIDOBJECT;
	}

	// Check d3d9 device
	if (!D3DDeviceInterface || !*D3DDeviceInterface)
	{
		D3DDeviceInterface = D3DInterface->GetD3DDevice();
		if (!D3DDeviceInterface || !*D3DDeviceInterface)
		{
			LOG_LIMIT(100, FunctionName << " Error: could not get the D3DDevice!");
			return DDERR_INVALIDOBJECT;
		}
	}

	return D3D_OK;
}

void* m_IDirect3DMaterialX::GetWrapperInterfaceX(DWORD DirectXVersion)
{
	switch (DirectXVersion)
	{
	case 0:
		if (WrapperInterface3) return WrapperInterface3;
		if (WrapperInterface2) return WrapperInterface2;
		if (WrapperInterface) return WrapperInterface;
		break;
	case 1:
		return GetInterfaceAddress(WrapperInterface, WrapperInterfaceBackup, (LPDIRECT3DMATERIAL)ProxyInterface, this);
	case 2:
		return GetInterfaceAddress(WrapperInterface2, WrapperInterfaceBackup2, (LPDIRECT3DMATERIAL2)ProxyInterface, this);
	case 3:
		return GetInterfaceAddress(WrapperInterface3, WrapperInterfaceBackup3, (LPDIRECT3DMATERIAL3)ProxyInterface, this);
	}
	LOG_LIMIT(100, __FUNCTION__ << " Error: wrapper interface version not found: " << DirectXVersion);
	return nullptr;
}

m_IDirect3DDeviceX* m_IDirect3DMaterialX::GetD3DDevice()
{
	// Check for device interface
	if (FAILED(CheckInterface(__FUNCTION__)))
	{
		return nullptr;
	}

	return *D3DDeviceInterface;
}
