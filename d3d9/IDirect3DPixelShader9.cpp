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

#include "d3d9.h"

HRESULT m_IDirect3DPixelShader9::QueryInterface(THIS_ REFIID riid, void** ppvObj)
{
	if ((riid == IID_IDirect3DPixelShader9 || riid == IID_IUnknown) && ppvObj)
	{
		AddRef();

		*ppvObj = this;

		return S_OK;
	}

	return ProxyInterface->QueryInterface(riid, ppvObj);
}

ULONG m_IDirect3DPixelShader9::AddRef(THIS)
{
	return ProxyInterface->AddRef();
}

ULONG m_IDirect3DPixelShader9::Release(THIS)
{
	return ProxyInterface->Release();
}

HRESULT m_IDirect3DPixelShader9::GetDevice(THIS_ IDirect3DDevice9** ppDevice)
{
	if (ppDevice)
	{
		if (m_pDevice)
		{
			m_pDevice->AddRef();

			*ppDevice = m_pDevice;

			return D3D_OK;
		}
		else if (m_pDeviceEx)
		{
			m_pDeviceEx->AddRef();

			*ppDevice = m_pDeviceEx;

			return D3D_OK;
		}
	}
	return D3DERR_INVALIDCALL;
}

HRESULT m_IDirect3DPixelShader9::GetFunction(THIS_ void* pData, UINT* pSizeOfData)
{
	return ProxyInterface->GetFunction(pData, pSizeOfData);
}
