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

HRESULT m_IDirect3D9Ex::QueryInterface(REFIID riid, void** ppvObj)
{
	if ((riid == IID_IDirect3D9Ex || riid == IID_IDirect3D9 || riid == IID_IUnknown) && ppvObj)
	{
		AddRef();

		*ppvObj = this;

		return S_OK;
	}

	return ProxyInterface->QueryInterface(riid, ppvObj);
}

ULONG m_IDirect3D9Ex::AddRef()
{
	return ProxyInterface->AddRef();
}

ULONG m_IDirect3D9Ex::Release()
{
	ULONG count = ProxyInterface->Release();

	if (count == 0)
	{
		delete this;
	}

	return count;
}

HRESULT m_IDirect3D9Ex::EnumAdapterModes(THIS_ UINT Adapter, D3DFORMAT Format, UINT Mode, D3DDISPLAYMODE* pMode)
{
	return ProxyInterface->EnumAdapterModes(Adapter, Format, Mode, pMode);
}

UINT m_IDirect3D9Ex::GetAdapterCount()
{
	return ProxyInterface->GetAdapterCount();
}

HRESULT m_IDirect3D9Ex::GetAdapterDisplayMode(UINT Adapter, D3DDISPLAYMODE *pMode)
{
	return ProxyInterface->GetAdapterDisplayMode(Adapter, pMode);
}

HRESULT m_IDirect3D9Ex::GetAdapterIdentifier(UINT Adapter, DWORD Flags, D3DADAPTER_IDENTIFIER9 *pIdentifier)
{
	return ProxyInterface->GetAdapterIdentifier(Adapter, Flags, pIdentifier);
}

UINT m_IDirect3D9Ex::GetAdapterModeCount(THIS_ UINT Adapter, D3DFORMAT Format)
{
	return ProxyInterface->GetAdapterModeCount(Adapter, Format);
}

HMONITOR m_IDirect3D9Ex::GetAdapterMonitor(UINT Adapter)
{
	return ProxyInterface->GetAdapterMonitor(Adapter);
}

HRESULT m_IDirect3D9Ex::GetDeviceCaps(UINT Adapter, D3DDEVTYPE DeviceType, D3DCAPS9 *pCaps)
{
	return ProxyInterface->GetDeviceCaps(Adapter, DeviceType, pCaps);
}

HRESULT m_IDirect3D9Ex::RegisterSoftwareDevice(void *pInitializeFunction)
{
	return ProxyInterface->RegisterSoftwareDevice(pInitializeFunction);
}

HRESULT m_IDirect3D9Ex::CheckDepthStencilMatch(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, D3DFORMAT RenderTargetFormat, D3DFORMAT DepthStencilFormat)
{
	return ProxyInterface->CheckDepthStencilMatch(Adapter, DeviceType, AdapterFormat, RenderTargetFormat, DepthStencilFormat);
}

HRESULT m_IDirect3D9Ex::CheckDeviceFormat(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, DWORD Usage, D3DRESOURCETYPE RType, D3DFORMAT CheckFormat)
{
	return ProxyInterface->CheckDeviceFormat(Adapter, DeviceType, AdapterFormat, Usage, RType, CheckFormat);
}

HRESULT m_IDirect3D9Ex::CheckDeviceMultiSampleType(THIS_ UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SurfaceFormat, BOOL Windowed, D3DMULTISAMPLE_TYPE MultiSampleType, DWORD* pQualityLevels)
{
	return ProxyInterface->CheckDeviceMultiSampleType(Adapter, DeviceType, SurfaceFormat, Windowed, MultiSampleType, pQualityLevels);
}

HRESULT m_IDirect3D9Ex::CheckDeviceType(UINT Adapter, D3DDEVTYPE CheckType, D3DFORMAT DisplayFormat, D3DFORMAT BackBufferFormat, BOOL Windowed)
{
	return ProxyInterface->CheckDeviceType(Adapter, CheckType, DisplayFormat, BackBufferFormat, Windowed);
}

HRESULT m_IDirect3D9Ex::CheckDeviceFormatConversion(THIS_ UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SourceFormat, D3DFORMAT TargetFormat)
{
	return ProxyInterface->CheckDeviceFormatConversion(Adapter, DeviceType, SourceFormat, TargetFormat);
}

HRESULT m_IDirect3D9Ex::CreateDevice(UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS *pPresentationParameters, IDirect3DDevice9 **ppReturnedDeviceInterface)
{
	HRESULT hr = ProxyInterface->CreateDevice(Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);

	if (SUCCEEDED(hr))
	{
		*ppReturnedDeviceInterface = new m_IDirect3DDevice9(*ppReturnedDeviceInterface, this);
	}

	return hr;
}

UINT m_IDirect3D9Ex::GetAdapterModeCountEx(THIS_ UINT Adapter, CONST D3DDISPLAYMODEFILTER* pFilter)
{
	return ProxyInterface->GetAdapterModeCountEx(Adapter, pFilter);
}

HRESULT m_IDirect3D9Ex::EnumAdapterModesEx(THIS_ UINT Adapter, CONST D3DDISPLAYMODEFILTER* pFilter, UINT Mode, D3DDISPLAYMODEEX* pMode)
{
	return ProxyInterface->EnumAdapterModesEx(Adapter, pFilter, Mode, pMode);
}

HRESULT m_IDirect3D9Ex::GetAdapterDisplayModeEx(THIS_ UINT Adapter, D3DDISPLAYMODEEX* pMode, D3DDISPLAYROTATION* pRotation)
{
	return ProxyInterface->GetAdapterDisplayModeEx(Adapter, pMode, pRotation);
}

HRESULT m_IDirect3D9Ex::CreateDeviceEx(THIS_ UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, D3DDISPLAYMODEEX* pFullscreenDisplayMode, IDirect3DDevice9Ex** ppReturnedDeviceInterface)
{
	HRESULT hr = D3DERR_INVALIDCALL;

	// Check for AntiAliasing
	bool MultiSampleFlag = false;
	if (Config.AntiAliasing != 0)
	{
		if (pPresentationParameters->SwapEffect == D3DSWAPEFFECT_DISCARD)
		{
			DWORD QualityLevels = 0;
			D3DPRESENT_PARAMETERS d3dpp;
			CopyMemory(&d3dpp, pPresentationParameters, sizeof(d3dpp));

			// Check AntiAliasing quality
			for (int x = min(16, Config.AntiAliasing); x > 0; x--)
			{
				if (ProxyInterface->CheckDeviceMultiSampleType(Adapter,
					DeviceType, d3dpp.BackBufferFormat, d3dpp.Windowed,
					(D3DMULTISAMPLE_TYPE)x, &QualityLevels) == S_OK &&
					ProxyInterface->CheckDeviceMultiSampleType(Adapter,
						DeviceType, d3dpp.AutoDepthStencilFormat, d3dpp.Windowed,
						(D3DMULTISAMPLE_TYPE)x, &QualityLevels) == S_OK)
				{
					d3dpp.Flags &= ~D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
					d3dpp.MultiSampleType = (D3DMULTISAMPLE_TYPE)x;
					d3dpp.MultiSampleQuality = (QualityLevels > 0) ? QualityLevels - 1 : 0;

					// Create Device
					hr = ProxyInterface->CreateDeviceEx(Adapter, DeviceType, hFocusWindow, BehaviorFlags, &d3dpp, pFullscreenDisplayMode, ppReturnedDeviceInterface);

					// Check if device was created successfully
					if (SUCCEEDED(hr))
					{
						MultiSampleFlag = true;
						Logging::Log() << "Setting MultiSample " << d3dpp.MultiSampleType << " Quality " << d3dpp.MultiSampleQuality;
					}
					else
					{
						Logging::Log() << "Failed to set AntiAliasing!";
					}
					break;
				}
			}
		}
		if (FAILED(hr))
		{
			Logging::Log() << "Could not enable AntiAliasing!";
		}
	}

	// Create Device
	if (FAILED(hr))
	{
		hr = ProxyInterface->CreateDeviceEx(Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, pFullscreenDisplayMode, ppReturnedDeviceInterface);
	}

	if (SUCCEEDED(hr))
	{
		*ppReturnedDeviceInterface = new m_IDirect3DDevice9Ex(*ppReturnedDeviceInterface, this);
		if (MultiSampleFlag)
		{
			((m_IDirect3DDevice9*)(*ppReturnedDeviceInterface))->SetMultiSampleType(pPresentationParameters->MultiSampleType);
			((m_IDirect3DDevice9*)(*ppReturnedDeviceInterface))->SetMultiSampleQuality(pPresentationParameters->MultiSampleQuality);
		}
	}

	return hr;
}

HRESULT m_IDirect3D9Ex::GetAdapterLUID(THIS_ UINT Adapter, LUID * pLUID)
{
	return ProxyInterface->GetAdapterLUID(Adapter, pLUID);
}
