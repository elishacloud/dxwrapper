/**
* Copyright (C) 2022 Elisha Riedlinger
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
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if ((riid == IID_IDirect3DSurface9 || riid == IID_IUnknown || riid == IID_IDirect3DResource9) && ppvObj)
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

ULONG m_IDirect3DSurface9::AddRef(THIS)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->AddRef();
}

ULONG m_IDirect3DSurface9::Release(THIS)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	ULONG ref = ProxyInterface->Release();

	if (ref == 0 && pEmuSurface)
	{
		pEmuSurface->UnlockRect();
		pEmuSurface->Release();
		pEmuSurface = nullptr;
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

	m_pDeviceEx->AddRef();

	*ppDevice = m_pDeviceEx;

	return D3D_OK;
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

HRESULT m_IDirect3DSurface9::LockRect(THIS_ D3DLOCKED_RECT* pLockedRect, CONST RECT* pRect, DWORD Flags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!pLockedRect)
	{
		return D3DERR_INVALIDCALL;
	}

	HRESULT hr = ProxyInterface->LockRect(pLockedRect, pRect, Flags);

	if (hr == D3DERR_INVALIDCALL && !IsLocked && pLockedRect && !pEmuSurface && DeviceMultiSampleType != D3DMULTISAMPLE_NONE)
	{
		D3DSURFACE_DESC Desc;
		if (SUCCEEDED(GetDesc(&Desc)))
		{
			// Create new surface for lock
			if (SUCCEEDED(m_pDeviceEx->CreateOffscreenPlainSurface(Desc.Width, Desc.Height, Desc.Format, D3DPOOL_SCRATCH, &pEmuSurface, nullptr)))
			{
				EmuReadOnly = (Flags & D3DLOCK_READONLY);
				EmuRect.left = 0;
				EmuRect.top = 0;
				EmuRect.right = (LONG)Desc.Width;
				EmuRect.bottom = (LONG)Desc.Height;
				if (pRect) { memcpy(&EmuRect, pRect, sizeof(RECT)); }
				POINT Point = { EmuRect.left, EmuRect.top };

				// Copy surface data
				if (FAILED(m_pDeviceEx->CopyRects(this, &EmuRect, 1, pEmuSurface, &Point)))
				{
					LOG_LIMIT(100, __FUNCTION__ << " Error: copying surface!");
				}
				D3DLOCKED_RECT LockedRect = {};
				if (SUCCEEDED(pEmuSurface->LockRect(&LockedRect, &EmuRect, Flags)))
				{
					pLockedRect->pBits = LockedRect.pBits;
					pLockedRect->Pitch = LockedRect.Pitch;
					return D3D_OK;
				}
				else
				{
					LOG_LIMIT(100, __FUNCTION__ << " Error: locking emulated surface!");
				}
				pEmuSurface->Release();
				pEmuSurface = nullptr;
			}
			else
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: creating emulated surface!");
			}

			LOG_LIMIT(100, __FUNCTION__ << " Error: Surface Lock error: " << (D3DERR)hr);

			return hr;
		}
	}
	else if (SUCCEEDED(hr))
	{
		IsLocked = true;
	}

	return hr;
}

HRESULT m_IDirect3DSurface9::UnlockRect(THIS)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	HRESULT hr = D3DERR_INVALIDCALL;

	// Copy data back from emulated surface
	if (pEmuSurface)
	{
		hr = pEmuSurface->UnlockRect();
		POINT Point = { EmuRect.left, EmuRect.top };
		if (!EmuReadOnly)
		{
			// Copy emulated surface data
			if (FAILED(m_pDeviceEx->CopyRects(pEmuSurface, &EmuRect, 1, this, &Point)))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: copying emulated surface!");
			}
		}
		pEmuSurface->Release();
		pEmuSurface = nullptr;
	}
	else
	{
		hr = ProxyInterface->UnlockRect();

		if (SUCCEEDED(hr))
		{
			IsLocked = false;
		}
	}

	return hr;
}

HRESULT m_IDirect3DSurface9::GetDC(THIS_ HDC *phdc)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->GetDC(phdc);
}

HRESULT m_IDirect3DSurface9::ReleaseDC(THIS_ HDC hdc)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->ReleaseDC(hdc);
}
