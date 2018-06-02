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

#include "..\ddraw.h"

HRESULT m_IDirect3D2::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
	return ProxyInterface->QueryInterface(riid, ppvObj);
}

ULONG m_IDirect3D2::AddRef()
{
	return ProxyInterface->AddRef();
}

ULONG m_IDirect3D2::Release()
{
	return ProxyInterface->Release();
}

HRESULT m_IDirect3D2::EnumDevices(LPD3DENUMDEVICESCALLBACK a, LPVOID b)
{
	return ProxyInterface->EnumDevices((LPD3DENUMDEVICESCALLBACK7)a, b);
}

HRESULT m_IDirect3D2::CreateLight(LPDIRECT3DLIGHT * a, IUnknown * b)
{
	return ProxyInterface->CreateLight(a, b);
}

HRESULT m_IDirect3D2::CreateMaterial(LPDIRECT3DMATERIAL2 * a, IUnknown * b)
{
	return ProxyInterface->CreateMaterial((LPDIRECT3DMATERIAL3*)a, b);
}

HRESULT m_IDirect3D2::CreateViewport(LPDIRECT3DVIEWPORT2 * a, IUnknown * b)
{
	return ProxyInterface->CreateViewport((LPDIRECT3DVIEWPORT3*)a, b);
}

HRESULT m_IDirect3D2::FindDevice(LPD3DFINDDEVICESEARCH a, LPD3DFINDDEVICERESULT b)
{
	return ProxyInterface->FindDevice(a, b);
}

HRESULT m_IDirect3D2::CreateDevice(REFCLSID a, LPDIRECTDRAWSURFACE b, LPDIRECT3DDEVICE2 * c)
{
	return ProxyInterface->CreateDevice(a, (LPDIRECTDRAWSURFACE7)b, (LPDIRECT3DDEVICE7*)c);
}
