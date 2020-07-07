/**
* Copyright (C) 2020 Elisha Riedlinger
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

HRESULT m_IDirect3DVertexBufferX::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (ppvObj && riid == IID_GetRealInterface)
	{
		*ppvObj = ProxyInterface;
		return DD_OK;
	}
	if (ppvObj && riid == IID_GetInterfaceX)
	{
		*ppvObj = this;
		return DD_OK;
	}

	if (ProxyDirectXVersion == 9)
	{
		if ((riid == IID_IDirect3DVertexBuffer || riid == IID_IDirect3DVertexBuffer7 || riid == IID_IUnknown) && ppvObj)
		{
			AddRef();

			*ppvObj = this;

			return D3D_OK;
		}
	}

	return ProxyQueryInterface(ProxyInterface, riid, ppvObj, GetWrapperType(GetGUIDVersion(riid)), GetWrapperInterfaceX(GetGUIDVersion(riid)));
}

void *m_IDirect3DVertexBufferX::GetWrapperInterfaceX(DWORD DirectXVersion)
{
	switch (DirectXVersion)
	{
	case 1:
		return WrapperInterface;
	case 7:
		return WrapperInterface7;
	default:
		LOG_LIMIT(100, __FUNCTION__ << " Error: wrapper interface version not found: " << DirectXVersion);
		return nullptr;
	}
}

ULONG m_IDirect3DVertexBufferX::AddRef()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (ProxyDirectXVersion == 9)
	{
		return InterlockedIncrement(&RefCount);
	}

	return ProxyInterface->AddRef();
}

ULONG m_IDirect3DVertexBufferX::Release()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	LONG ref;

	if (ProxyDirectXVersion == 9)
	{
		ref = InterlockedDecrement(&RefCount);
	}
	else
	{
		ref = ProxyInterface->Release();
	}

	if (ref == 0)
	{
		delete this;
	}

	return ref;
}

HRESULT m_IDirect3DVertexBufferX::Lock(DWORD dwFlags, LPVOID * lplpData, LPDWORD lpdwSize)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (ProxyDirectXVersion == 9)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	return ProxyInterface->Lock(dwFlags, lplpData, lpdwSize);
}

HRESULT m_IDirect3DVertexBufferX::Unlock()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (ProxyDirectXVersion == 9)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	return ProxyInterface->Unlock();
}

HRESULT m_IDirect3DVertexBufferX::ProcessVertices(DWORD dwVertexOp, DWORD dwDestIndex, DWORD dwCount, LPDIRECT3DVERTEXBUFFER7 lpSrcBuffer, DWORD dwSrcIndex, LPDIRECT3DDEVICE7 lpD3DDevice, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (ProxyDirectXVersion == 9)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	if (lpSrcBuffer)
	{
		lpSrcBuffer->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpSrcBuffer);
	}
	if (lpD3DDevice)
	{
		lpD3DDevice->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpD3DDevice);
	}

	return ProxyInterface->ProcessVertices(dwVertexOp, dwDestIndex, dwCount, lpSrcBuffer, dwSrcIndex, lpD3DDevice, dwFlags);
}

HRESULT m_IDirect3DVertexBufferX::GetVertexBufferDesc(LPD3DVERTEXBUFFERDESC lpVBDesc)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (ProxyDirectXVersion == 9)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	return ProxyInterface->GetVertexBufferDesc(lpVBDesc);
}

HRESULT m_IDirect3DVertexBufferX::Optimize(LPDIRECT3DDEVICE7 lpD3DDevice, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (ProxyDirectXVersion == 9)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	if (lpD3DDevice)
	{
		lpD3DDevice->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpD3DDevice);
	}

	return ProxyInterface->Optimize(lpD3DDevice, dwFlags);
}

HRESULT m_IDirect3DVertexBufferX::ProcessVerticesStrided(DWORD dwVertexOp, DWORD dwDestIndex, DWORD dwCount, LPD3DDRAWPRIMITIVESTRIDEDDATA lpVertexArray, DWORD dwSrcIndex, LPDIRECT3DDEVICE7 lpD3DDevice, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (lpD3DDevice)
	{
		lpD3DDevice->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpD3DDevice);
	}

	switch (ProxyDirectXVersion)
	{
	case 1:
		return DDERR_GENERIC;
	case 7:
		return ProxyInterface->ProcessVerticesStrided(dwVertexOp, dwDestIndex, dwCount, lpVertexArray, dwSrcIndex, lpD3DDevice, dwFlags);
	case 9:
		LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");
		return DDERR_UNSUPPORTED;
	default:
		return DDERR_GENERIC;
	}
}
