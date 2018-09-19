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

HRESULT m_IDirect3DTextureX::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
	Logging::LogDebug() << __FUNCTION__;

	if (ProxyDirectXVersion > 3)
	{
		if ((riid == IID_IDirect3DTexture || riid == IID_IDirect3DTexture2 || riid == IID_IUnknown) && ppvObj)
		{
			AddRef();

			*ppvObj = this;

			return S_OK;
		}
	}

	return ProxyQueryInterface(ProxyInterface, riid, ppvObj, WrapperID, WrapperInterface);
}

ULONG m_IDirect3DTextureX::AddRef()
{
	Logging::LogDebug() << __FUNCTION__;

	if (ProxyDirectXVersion > 3)
	{
		return InterlockedIncrement(&RefCount);
	}

	return ProxyInterface->AddRef();
}

ULONG m_IDirect3DTextureX::Release()
{
	Logging::LogDebug() << __FUNCTION__;

	LONG ref;

	if (ProxyDirectXVersion > 3)
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

HRESULT m_IDirect3DTextureX::Initialize(LPDIRECT3DDEVICE lpDirect3DDevice, LPDIRECTDRAWSURFACE lplpDDSurface)
{
	Logging::LogDebug() << __FUNCTION__;

	if (ProxyDirectXVersion != 1)
	{
		return D3D_OK;
	}

	if (lpDirect3DDevice)
	{
		lpDirect3DDevice = static_cast<m_IDirect3DDevice *>(lpDirect3DDevice)->GetProxyInterface();
	}
	if (lplpDDSurface)
	{
		lplpDDSurface = static_cast<m_IDirectDrawSurface *>(lplpDDSurface)->GetProxyInterface();
	}

	return ((IDirect3DTexture*)ProxyInterface)->Initialize(lpDirect3DDevice, lplpDDSurface);
}

HRESULT m_IDirect3DTextureX::GetHandle(LPDIRECT3DDEVICE2 lpDirect3DDevice2, LPD3DTEXTUREHANDLE lpHandle)
{
	Logging::LogDebug() << __FUNCTION__;

	if (ProxyDirectXVersion > 3)
	{
		lpDirect3DDevice2 = (IDirect3DDevice2*)D3DDeviceInterface;

		lpHandle = nullptr;

		Logging::Log() << __FUNCTION__ << " LPD3DTEXTUREHANDLE Not Implemented";
		return D3D_OK;
	}

	if (lpDirect3DDevice2)
	{
		lpDirect3DDevice2 = static_cast<m_IDirect3DDevice2 *>(lpDirect3DDevice2)->GetProxyInterface();
	}

	return ProxyInterface->GetHandle(lpDirect3DDevice2, lpHandle);
}

HRESULT m_IDirect3DTextureX::PaletteChanged(DWORD dwStart, DWORD dwCount)
{
	Logging::LogDebug() << __FUNCTION__;

	if (ProxyDirectXVersion > 3)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	return ProxyInterface->PaletteChanged(dwStart, dwCount);
}

HRESULT m_IDirect3DTextureX::Load(LPDIRECT3DTEXTURE2 lpD3DTexture2)
{
	Logging::LogDebug() << __FUNCTION__;

	if (ProxyDirectXVersion > 3)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	if (lpD3DTexture2)
	{
		lpD3DTexture2 = static_cast<m_IDirect3DTexture2 *>(lpD3DTexture2)->GetProxyInterface();
	}

	return ProxyInterface->Load(lpD3DTexture2);
}

HRESULT m_IDirect3DTextureX::Unload()
{
	Logging::LogDebug() << __FUNCTION__;

	if (ProxyDirectXVersion != 1)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	return ((IDirect3DTexture*)ProxyInterface)->Unload();
}
