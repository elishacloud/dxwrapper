/**
* Copyright (C) 2026 Elisha Riedlinger
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

// ******************************
// IUnknown functions
// ******************************

HRESULT m_IDirect3DQuery9::QueryInterface(THIS_ REFIID riid, void** ppvObj)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ") " << riid;

	if (!ppvObj)
	{
		return E_POINTER;
	}

	if (riid == IID_IUnknown || riid == WrapperID)
	{
		AddRef();

		*ppvObj = this;

		return D3D_OK;
	}

	if (!ProxyInterface)
	{
		*ppvObj = nullptr;
		return D3DERR_INVALIDCALL;
	}

	HRESULT hr = ProxyInterface->QueryInterface(riid, ppvObj);

	if (SUCCEEDED(hr))
	{
		D3d9Wrapper::genericQueryInterface(riid, WrapperID, ppvObj, m_pDeviceEx);
	}

	return hr;
}

ULONG m_IDirect3DQuery9::AddRef(THIS)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface)
	{
		return InterlockedIncrement(&RefCount);
	}

	return ProxyInterface->AddRef();
}

ULONG m_IDirect3DQuery9::Release(THIS)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface)
	{
		return InterlockedDecrementIfNotNull(&RefCount);
	}

	return ProxyInterface->Release();
}

// ******************************
// IDirect3DQuery9 methods
// ******************************

HRESULT m_IDirect3DQuery9::GetDevice(THIS_ IDirect3DDevice9** ppDevice)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (FAILED(m_pDeviceEx->QueryInterface(m_pDeviceEx->GetIID(), (LPVOID*)ppDevice)))
	{
		return D3DERR_INVALIDCALL;
	}

	return D3D_OK;
}

D3DQUERYTYPE m_IDirect3DQuery9::GetType(THIS)
{
	if (m_IsFakeQuery)
	{
		return m_FakeQueryType;
	}

	if (!ProxyInterface)
	{
		return (D3DQUERYTYPE)0;
	}

	return ProxyInterface->GetType();
}

DWORD m_IDirect3DQuery9::GetDataSize(THIS)
{
	if (m_IsFakeQuery)
	{
		switch (m_FakeQueryType)
		{
		case D3DQUERYTYPE_EVENT:
			return sizeof(BOOL);

		case D3DQUERYTYPE_OCCLUSION:
			return sizeof(DWORD);

		default:
			return 0;
		}
	}

	if (!ProxyInterface)
	{
		return 0;
	}

	return ProxyInterface->GetDataSize();
}

HRESULT m_IDirect3DQuery9::Issue(THIS_ DWORD dwIssueFlags)
{
	if (m_IsFakeQuery)
	{
		switch (m_FakeQueryType)
		{
		case D3DQUERYTYPE_EVENT:
			if (dwIssueFlags == D3DISSUE_BEGIN)
			{
				return D3D_OK;
			}
			if (dwIssueFlags == D3DISSUE_END)
			{
				m_QueryEnded = true;
				return D3D_OK;
			}
			return D3DERR_INVALIDCALL;

		case D3DQUERYTYPE_OCCLUSION:
			if (dwIssueFlags == D3DISSUE_BEGIN)
			{
				m_QueryBegun = true;
				m_QueryEnded = false;
				return D3D_OK;
			}
			if (dwIssueFlags == D3DISSUE_END && m_QueryBegun)
			{
				m_QueryBegun = false;
				m_QueryEnded = true;
				return D3D_OK;
			}
			return D3DERR_INVALIDCALL;

		default:
			return D3DERR_INVALIDCALL;
		}
	}

	if (!ProxyInterface)
	{
		return D3DERR_INVALIDCALL;
	}

	return ProxyInterface->Issue(dwIssueFlags);
}

HRESULT m_IDirect3DQuery9::GetData(THIS_ void* pData, DWORD dwSize, DWORD dwGetDataFlags)
{
	if (m_IsFakeQuery)
	{
		if (m_QueryBegun && !m_QueryEnded)
		{
			// Still in Issue() - D3DISSUE_BEGIN
			return S_FALSE;
		}

		if (!pData)
		{
			// Polling without requesting the result.
			return S_OK;
		}

		if (m_FakeQueryType == D3DQUERYTYPE_EVENT)
		{
			if (dwSize != sizeof(BOOL))
			{
				return D3DERR_INVALIDCALL;
			}

			*static_cast<BOOL*>(pData) = TRUE;
			return S_OK;
		}

		if (m_FakeQueryType == D3DQUERYTYPE_OCCLUSION)
		{
			if (dwSize != sizeof(DWORD))
			{
				return D3DERR_INVALIDCALL;
			}

			*static_cast<DWORD*>(pData) = 1;
			return S_OK;
		}

		return D3DERR_INVALIDCALL;
	}

	if (!ProxyInterface)
	{
		return D3DERR_INVALIDCALL;
	}

	return ProxyInterface->GetData(pData, dwSize, dwGetDataFlags);
}
