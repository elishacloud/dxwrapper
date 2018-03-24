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

HRESULT m_IDirect3DTexture::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
	return ProxyQueryInterface(ProxyInterface, riid, ppvObj, IID_IDirect3DTexture, this);
}

ULONG m_IDirect3DTexture::AddRef()
{
	return ProxyInterface->AddRef();
}

ULONG m_IDirect3DTexture::Release()
{
	ULONG x = ProxyInterface->Release();

	if (x == 0)
	{
		ProxyAddressLookupTable.DeleteAddress(this);

		delete this;
	}

	return x;
}

HRESULT m_IDirect3DTexture::Initialize(LPDIRECT3DDEVICE a, LPDIRECTDRAWSURFACE b)
{
	if (a)
	{
		a = static_cast<m_IDirect3DDevice *>(a)->GetProxyInterface();
	}
	if (b)
	{
		b = static_cast<m_IDirectDrawSurface *>(b)->GetProxyInterface();
	}

	return ProxyInterface->Initialize(a, b);
}

HRESULT m_IDirect3DTexture::GetHandle(LPDIRECT3DDEVICE a, LPD3DTEXTUREHANDLE b)
{
	if (a)
	{
		a = static_cast<m_IDirect3DDevice *>(a)->GetProxyInterface();
	}

	return ProxyInterface->GetHandle(a, b);
}

HRESULT m_IDirect3DTexture::PaletteChanged(DWORD a, DWORD b)
{
	return ProxyInterface->PaletteChanged(a, b);
}

HRESULT m_IDirect3DTexture::Load(LPDIRECT3DTEXTURE a)
{
	if (a)
	{
		a = static_cast<m_IDirect3DTexture *>(a)->GetProxyInterface();
	}

	return ProxyInterface->Load(a);
}

HRESULT m_IDirect3DTexture::Unload()
{
	return ProxyInterface->Unload();
}
