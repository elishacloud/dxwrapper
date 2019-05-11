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

#include "d3d9.h"

HRESULT m_IDirect3DTexture9::QueryInterface(THIS_ REFIID riid, void** ppvObj)
{
	Logging::LogDebug() << __FUNCTION__;

	if ((riid == IID_IDirect3DTexture9 || riid == IID_IUnknown || riid == IID_IDirect3DResource9 || riid == IID_IDirect3DBaseTexture9) && ppvObj)
	{
		AddRef();

		*ppvObj = this;

		return D3D_OK;
	}

	HRESULT hr = ProxyInterface->QueryInterface(riid, ppvObj);

	if (SUCCEEDED(hr))
	{
		D3d9Wrapper::genericQueryInterface(riid, ppvObj, m_pDeviceEx);
	}

	return hr;
}

ULONG m_IDirect3DTexture9::AddRef(THIS)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->AddRef();
}

ULONG m_IDirect3DTexture9::Release(THIS)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->Release();
}

HRESULT m_IDirect3DTexture9::GetDevice(THIS_ IDirect3DDevice9** ppDevice)
{
	Logging::LogDebug() << __FUNCTION__;

	if (!ppDevice)
	{
		return D3DERR_INVALIDCALL;
	}

	m_pDeviceEx->AddRef();

	*ppDevice = m_pDeviceEx;

	return D3D_OK;
}

HRESULT m_IDirect3DTexture9::SetPrivateData(THIS_ REFGUID refguid, CONST void* pData, DWORD SizeOfData, DWORD Flags)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->SetPrivateData(refguid, pData, SizeOfData, Flags);
}

HRESULT m_IDirect3DTexture9::GetPrivateData(THIS_ REFGUID refguid, void* pData, DWORD* pSizeOfData)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->GetPrivateData(refguid, pData, pSizeOfData);
}

HRESULT m_IDirect3DTexture9::FreePrivateData(THIS_ REFGUID refguid)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->FreePrivateData(refguid);
}

DWORD m_IDirect3DTexture9::SetPriority(THIS_ DWORD PriorityNew)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->SetPriority(PriorityNew);
}

DWORD m_IDirect3DTexture9::GetPriority(THIS)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->GetPriority();
}

void m_IDirect3DTexture9::PreLoad(THIS)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->PreLoad();
}

D3DRESOURCETYPE m_IDirect3DTexture9::GetType(THIS)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->GetType();
}

DWORD m_IDirect3DTexture9::SetLOD(THIS_ DWORD LODNew)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->SetLOD(LODNew);
}

DWORD m_IDirect3DTexture9::GetLOD(THIS)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->GetLOD();
}

DWORD m_IDirect3DTexture9::GetLevelCount(THIS)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->GetLevelCount();
}

HRESULT m_IDirect3DTexture9::SetAutoGenFilterType(THIS_ D3DTEXTUREFILTERTYPE FilterType)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->SetAutoGenFilterType(FilterType);
}

D3DTEXTUREFILTERTYPE m_IDirect3DTexture9::GetAutoGenFilterType(THIS)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->GetAutoGenFilterType();
}

void m_IDirect3DTexture9::GenerateMipSubLevels(THIS)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->GenerateMipSubLevels();
}

HRESULT m_IDirect3DTexture9::GetLevelDesc(THIS_ UINT Level, D3DSURFACE_DESC *pDesc)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->GetLevelDesc(Level, pDesc);
}

HRESULT m_IDirect3DTexture9::GetSurfaceLevel(THIS_ UINT Level, IDirect3DSurface9** ppSurfaceLevel)
{
	Logging::LogDebug() << __FUNCTION__;

	HRESULT hr = ProxyInterface->GetSurfaceLevel(Level, ppSurfaceLevel);

	if (SUCCEEDED(hr) && ppSurfaceLevel)
	{
		*ppSurfaceLevel = m_pDeviceEx->ProxyAddressLookupTable->FindAddress<m_IDirect3DSurface9>(*ppSurfaceLevel);
	}

	return hr;
}

HRESULT m_IDirect3DTexture9::LockRect(THIS_ UINT Level, D3DLOCKED_RECT* pLockedRect, CONST RECT* pRect, DWORD Flags)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->LockRect(Level, pLockedRect, pRect, Flags);
}

HRESULT m_IDirect3DTexture9::UnlockRect(THIS_ UINT Level)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->UnlockRect(Level);
}

HRESULT m_IDirect3DTexture9::AddDirtyRect(THIS_ CONST RECT* pDirtyRect)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->AddDirtyRect(pDirtyRect);
}
