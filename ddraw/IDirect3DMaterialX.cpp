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

HRESULT m_IDirect3DMaterialX::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
	if (ProxyDirectXVersion == 7)
	{
		if ((riid == IID_IDirect3DMaterial || riid == IID_IDirect3DMaterial2 || riid == IID_IDirect3DMaterial3 || riid == IID_IUnknown) && ppvObj)
		{
			AddRef();

			*ppvObj = this;

			return S_OK;
		}
	}

	return ProxyQueryInterface(ProxyInterface, riid, ppvObj, WrapperID, WrapperInterface);
}

ULONG m_IDirect3DMaterialX::AddRef()
{
	if (ProxyDirectXVersion == 7)
	{
		return InterlockedIncrement(&RefCount);
	}

	return ProxyInterface->AddRef();
}

ULONG m_IDirect3DMaterialX::Release()
{
	if (ProxyDirectXVersion == 7)
	{
		LONG ref = InterlockedDecrement(&RefCount);

		if (ref == 0)
		{
			delete this;
			return 0;
		}

		return ref;
	}

	ULONG x = ProxyInterface->Release();

	if (x == 0)
	{
		WrapperInterface->DeleteMe();
	}

	return x;
}

HRESULT m_IDirect3DMaterialX::Initialize(LPDIRECT3D lplpD3D)
{
	if (ProxyDirectXVersion != 1)
	{
		Logging::Log() << __FUNCTION__ << " Not Implimented";
		return D3D_OK;	// Should not matter for newer versions of DirectX
	}

	if (lplpD3D)
	{
		lplpD3D = static_cast<m_IDirect3D *>(lplpD3D)->GetProxyInterface();
	}

	return ((IDirect3DMaterial*)ProxyInterface)->Initialize(lplpD3D);
}

HRESULT m_IDirect3DMaterialX::SetMaterial(LPD3DMATERIAL lpMat)
{
	if (ProxyDirectXVersion == 7)
	{
		D3DMATERIAL7 tmpMaterial;

		if (lpMat->hTexture)
		{
			Logging::Log() << __FUNCTION__ << " D3DMATERIALHANDLE Not Implimented";
		}

		ConvertMaterial(tmpMaterial, *lpMat);

		return ((m_IDirect3DDeviceX*)WrapperInterface)->SetMaterial(&tmpMaterial);
	}

	return ProxyInterface->SetMaterial(lpMat);
}

HRESULT m_IDirect3DMaterialX::GetMaterial(LPD3DMATERIAL lpMat)
{
	if (ProxyDirectXVersion == 7)
	{
		D3DMATERIAL7 tmpMaterial;

		HRESULT hr = ((m_IDirect3DDeviceX*)WrapperInterface)->GetMaterial(&tmpMaterial);

		Logging::Log() << __FUNCTION__ << " D3DMATERIALHANDLE Not Implimented";
		ConvertMaterial(tmpMaterial, *lpMat);

		return hr;
	}

	return ProxyInterface->GetMaterial(lpMat);
}

HRESULT m_IDirect3DMaterialX::GetHandle(LPDIRECT3DDEVICE3 lpDirect3DDevice, LPD3DMATERIALHANDLE lpHandle)
{
	if (ProxyDirectXVersion == 7)
	{
		lpDirect3DDevice = (IDirect3DDevice3*)ProxyInterface;

		lpHandle = nullptr;

		Logging::Log() << __FUNCTION__ << " D3DMATERIALHANDLE Not Implimented";
		return D3D_OK;
	}

	if (lpDirect3DDevice)
	{
		lpDirect3DDevice = static_cast<m_IDirect3DDevice3 *>(lpDirect3DDevice)->GetProxyInterface();
	}

	return ProxyInterface->GetHandle(lpDirect3DDevice, lpHandle);
}

HRESULT m_IDirect3DMaterialX::Reserve()
{
	if (ProxyDirectXVersion != 1)
	{
		Logging::Log() << __FUNCTION__ << " Not Implimented";
		return E_NOTIMPL;
	}

	return ((IDirect3DMaterial*)ProxyInterface)->Reserve();
}

HRESULT m_IDirect3DMaterialX::Unreserve()
{
	if (ProxyDirectXVersion != 1)
	{
		Logging::Log() << __FUNCTION__ << " Not Implimented";
		return E_NOTIMPL;
	}

	return ((IDirect3DMaterial*)ProxyInterface)->Unreserve();
}
