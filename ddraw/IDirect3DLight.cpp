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

HRESULT m_IDirect3DLight::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyQueryInterface(ProxyInterface, riid, ppvObj, WrapperID, this);
}

ULONG m_IDirect3DLight::AddRef()
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->AddRef();
}

ULONG m_IDirect3DLight::Release()
{
	Logging::LogDebug() << __FUNCTION__;

	ULONG x = ProxyInterface->Release();

	if (x == 0)
	{
		delete this;
	}

	return x;
}

HRESULT m_IDirect3DLight::Initialize(LPDIRECT3D lpDirect3D)
{
	Logging::LogDebug() << __FUNCTION__;

	if (lpDirect3D)
	{
		lpDirect3D = static_cast<m_IDirect3D *>(lpDirect3D)->GetProxyInterface();
	}

	return ProxyInterface->Initialize(lpDirect3D);
}

HRESULT m_IDirect3DLight::SetLight(LPD3DLIGHT lpLight)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->SetLight(lpLight);
}

HRESULT m_IDirect3DLight::GetLight(LPD3DLIGHT lpLight)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->GetLight(lpLight);
}
