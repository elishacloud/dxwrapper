/**
* Copyright (C) 2025 Elisha Riedlinger
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

HRESULT m_IDirect3DSurface9::QueryInterface(THIS_ REFIID riid, void** ppvObj)
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

	if (riid == IID_IUnknown || riid == WrapperID || riid == IID_IDirect3DResource9)
	{
		HRESULT hr = ProxyInterface->QueryInterface(WrapperID, ppvObj);

		if (SUCCEEDED(hr))
		{
			*ppvObj = this;
		}

		return hr;
	}

	HRESULT hr = ProxyInterface->QueryInterface(riid, ppvObj);

	if (SUCCEEDED(hr))
	{
		D3d9Wrapper::genericQueryInterface(riid, ppvObj, m_pDeviceEx);
	}

	return hr;
}

ULONG m_IDirect3DSurface9::AddRef(THIS)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->AddRef();
}

ULONG m_IDirect3DSurface9::Release(THIS)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	ULONG ref = ProxyInterface->Release();

	if (ref == 0)
	{
		if (Emu.pSurface)
		{
			ULONG eref = Emu.pSurface->Release();
			if (eref)
			{
				Logging::Log() << __FUNCTION__ << " Error: there is still a reference to 'Emu.pSurface' " << eref;
			}
			Emu.pSurface = nullptr;
		}
    }

	return ref;
}

HRESULT m_IDirect3DSurface9::GetDevice(THIS_ IDirect3DDevice9** ppDevice)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ppDevice)
	{
		return D3DERR_INVALIDCALL;
	}

	return m_pDeviceEx->QueryInterface(m_pDeviceEx->GetIID(), (LPVOID*)ppDevice);
}

HRESULT m_IDirect3DSurface9::SetPrivateData(THIS_ REFGUID refguid, CONST void* pData, DWORD SizeOfData, DWORD Flags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->SetPrivateData(refguid, pData, SizeOfData, Flags);
}

HRESULT m_IDirect3DSurface9::GetPrivateData(THIS_ REFGUID refguid, void* pData, DWORD* pSizeOfData)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->GetPrivateData(refguid, pData, pSizeOfData);
}

HRESULT m_IDirect3DSurface9::FreePrivateData(THIS_ REFGUID refguid)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->FreePrivateData(refguid);
}

DWORD m_IDirect3DSurface9::SetPriority(THIS_ DWORD PriorityNew)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->SetPriority(PriorityNew);
}

DWORD m_IDirect3DSurface9::GetPriority(THIS)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->GetPriority();
}

void m_IDirect3DSurface9::PreLoad(THIS)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->PreLoad();
}

D3DRESOURCETYPE m_IDirect3DSurface9::GetType(THIS)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->GetType();
}

HRESULT m_IDirect3DSurface9::GetContainer(THIS_ REFIID riid, void** ppContainer)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	HRESULT hr = ProxyInterface->GetContainer(riid, ppContainer);

	if (SUCCEEDED(hr))
	{
		D3d9Wrapper::genericQueryInterface(riid, ppContainer, m_pDeviceEx);
	}

	return hr;
}

HRESULT m_IDirect3DSurface9::GetDesc(THIS_ D3DSURFACE_DESC *pDesc)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->GetDesc(pDesc);
}

m_IDirect3DSurface9* m_IDirect3DSurface9::m_GetNonMultiSampledSurface(const RECT* pRect, DWORD Flags)
{
	if (!Emu.pSurface)
	{
		if (SUCCEEDED((Desc.Usage & D3DUSAGE_RENDERTARGET) ? m_pDeviceEx->GetProxyInterface()->CreateRenderTarget(Desc.Width, Desc.Height, Desc.Format, D3DMULTISAMPLE_NONE, 0, TRUE, (LPDIRECT3DSURFACE9*)&Emu.pSurface, nullptr) :
			m_pDeviceEx->GetProxyInterface()->CreateOffscreenPlainSurface(Desc.Width, Desc.Height, Desc.Format, D3DPOOL_SYSTEMMEM, (LPDIRECT3DSURFACE9*)&Emu.pSurface, nullptr)))
		{
			Emu.pSurface = new m_IDirect3DSurface9(Emu.pSurface, m_pDeviceEx);
		}
	}
	if (Emu.pSurface)
	{
		Emu.ReadOnly = (Flags & D3DLOCK_READONLY);
		Emu.Rect = (pRect) ? *pRect : Emu.Rect;
		Emu.pRect = (pRect) ? &Emu.Rect : nullptr;

		if (FAILED(m_pDeviceEx->CopyRects(this, pRect, 1, Emu.pSurface, (LPPOINT)pRect)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: copying surface!");
		}
	}
	else
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: creating emulated surface!");
	}

	return Emu.pSurface;
}

HRESULT m_IDirect3DSurface9::RestoreMultiSampleData()
{
	if (Emu.pSurface && !Emu.ReadOnly)
	{
		if (FAILED(m_pDeviceEx->CopyRects(Emu.pSurface, Emu.pRect, 1, this, (LPPOINT)Emu.pRect)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: copying emulated surface!");
			return D3DERR_INVALIDCALL;
		}
	}
	return D3D_OK;
}

HRESULT m_IDirect3DSurface9::LockRect(THIS_ D3DLOCKED_RECT* pLockedRect, CONST RECT* pRect, DWORD Flags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!pLockedRect)
	{
		return D3DERR_INVALIDCALL;
	}

	return GetNonMultiSampledSurface(pRect, Flags)->LockRect(pLockedRect, pRect, Flags);
}

HRESULT m_IDirect3DSurface9::UnlockRect(THIS)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	HRESULT hr = D3DERR_INVALIDCALL;

	// Copy data back from emulated surface
	if (Emu.pSurface)
	{
		hr = Emu.pSurface->UnlockRect();

		// Copy emulated data to surface
		if (SUCCEEDED(hr))
		{
			RestoreMultiSampleData();
		}

		return hr;
	}

	return ProxyInterface->UnlockRect();
}

HRESULT m_IDirect3DSurface9::GetDC(THIS_ HDC *phdc)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return GetNonMultiSampledSurface(nullptr, 0)->GetDC(phdc);
}

HRESULT m_IDirect3DSurface9::ReleaseDC(THIS_ HDC hdc)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	HRESULT hr = D3DERR_INVALIDCALL;

	// Copy data back from emulated surface
	if (Emu.pSurface)
	{
		hr = Emu.pSurface->ReleaseDC(hdc);

		// Copy emulated data to surface
		if (SUCCEEDED(hr))
		{
			RestoreMultiSampleData();
		}

		return hr;
	}

	return ProxyInterface->ReleaseDC(hdc);
}
