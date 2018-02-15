/**
* Copyright (C) 2017 Elisha Riedlinger
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

HRESULT m_IDirect3D9::QueryInterface(REFIID riid, void** ppvObj)
{
	if ((riid == IID_IDirect3D9 || riid == IID_IUnknown) && ppvObj)
	{
		AddRef();

		*ppvObj = this;

		return S_OK;
	}

	return ProxyInterface->QueryInterface(riid, ppvObj);
}

ULONG m_IDirect3D9::AddRef()
{
	return ProxyInterface->AddRef();
}

ULONG m_IDirect3D9::Release()
{
	ULONG count = ProxyInterface->Release();

	if (count == 0)
	{
		delete this;
	}

	return count;
}

HRESULT m_IDirect3D9::EnumAdapterModes(THIS_ UINT Adapter, D3DFORMAT Format, UINT Mode, D3DDISPLAYMODE* pMode)
{
	return ProxyInterface->EnumAdapterModes(Adapter, Format, Mode, pMode);
}

UINT m_IDirect3D9::GetAdapterCount()
{
	return ProxyInterface->GetAdapterCount();
}

HRESULT m_IDirect3D9::GetAdapterDisplayMode(UINT Adapter, D3DDISPLAYMODE *pMode)
{
	return ProxyInterface->GetAdapterDisplayMode(Adapter, pMode);
}

HRESULT m_IDirect3D9::GetAdapterIdentifier(UINT Adapter, DWORD Flags, D3DADAPTER_IDENTIFIER9 *pIdentifier)
{
	return ProxyInterface->GetAdapterIdentifier(Adapter, Flags, pIdentifier);
}

UINT m_IDirect3D9::GetAdapterModeCount(THIS_ UINT Adapter, D3DFORMAT Format)
{
	return ProxyInterface->GetAdapterModeCount(Adapter, Format);
}

HMONITOR m_IDirect3D9::GetAdapterMonitor(UINT Adapter)
{
	return ProxyInterface->GetAdapterMonitor(Adapter);
}

HRESULT m_IDirect3D9::GetDeviceCaps(UINT Adapter, D3DDEVTYPE DeviceType, D3DCAPS9 *pCaps)
{
	return ProxyInterface->GetDeviceCaps(Adapter, DeviceType, pCaps);
}

HRESULT m_IDirect3D9::RegisterSoftwareDevice(void *pInitializeFunction)
{
	return ProxyInterface->RegisterSoftwareDevice(pInitializeFunction);
}

HRESULT m_IDirect3D9::CheckDepthStencilMatch(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, D3DFORMAT RenderTargetFormat, D3DFORMAT DepthStencilFormat)
{
	return ProxyInterface->CheckDepthStencilMatch(Adapter, DeviceType, AdapterFormat, RenderTargetFormat, DepthStencilFormat);
}

HRESULT m_IDirect3D9::CheckDeviceFormat(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, DWORD Usage, D3DRESOURCETYPE RType, D3DFORMAT CheckFormat)
{
	return ProxyInterface->CheckDeviceFormat(Adapter, DeviceType, AdapterFormat, Usage, RType, CheckFormat);
}

HRESULT m_IDirect3D9::CheckDeviceMultiSampleType(THIS_ UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SurfaceFormat, BOOL Windowed, D3DMULTISAMPLE_TYPE MultiSampleType, DWORD* pQualityLevels)
{
	return ProxyInterface->CheckDeviceMultiSampleType(Adapter, DeviceType, SurfaceFormat, Windowed, MultiSampleType, pQualityLevels);
}

HRESULT m_IDirect3D9::CheckDeviceType(UINT Adapter, D3DDEVTYPE CheckType, D3DFORMAT DisplayFormat, D3DFORMAT BackBufferFormat, BOOL Windowed)
{
	return ProxyInterface->CheckDeviceType(Adapter, CheckType, DisplayFormat, BackBufferFormat, Windowed);
}

HRESULT m_IDirect3D9::CheckDeviceFormatConversion(THIS_ UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SourceFormat, D3DFORMAT TargetFormat)
{
	return ProxyInterface->CheckDeviceFormatConversion(Adapter, DeviceType, SourceFormat, TargetFormat);
}

HRESULT m_IDirect3D9::CreateDevice(UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS *pPresentationParameters, IDirect3DDevice9 **ppReturnedDeviceInterface)
{
	// Check for AntiAliasing
	bool MultiSampleFlag = false;
	DWORD QualityLevels = 0;
	DWORD SampleType = 0;
	D3DPRESENT_PARAMETERS d3dpp;
	CopyMemory(&d3dpp, pPresentationParameters, sizeof(d3dpp));
	DWORD bFlags = BehaviorFlags;
	if (Config.AntiAliasing != 0)
	{
		// Check AntiAliasing quality
		for (int x = min(16, Config.AntiAliasing); x > 0; x--)
		{
			ProxyInterface->CheckDeviceMultiSampleType(Adapter,
				DeviceType, (*pPresentationParameters).BackBufferFormat, (*pPresentationParameters).Windowed,
				(D3DMULTISAMPLE_TYPE)x, &QualityLevels);
			if (QualityLevels > 1)
			{
				MultiSampleFlag = true;
				SampleType = x;
				QualityLevels -= 1;
				break;
			}
		}

		// Enable AntiAliasing
		if (MultiSampleFlag)
		{
			// Full-scene AntiAliasing is supported. Enable it here.
			(*pPresentationParameters).Flags = (*pPresentationParameters).Flags & ~D3DPRESENTFLAG_LOCKABLE_BACKBUFFER & ~D3DPRESENTFLAG_VIDEO;
			(*pPresentationParameters).MultiSampleType = (D3DMULTISAMPLE_TYPE)SampleType;
			(*pPresentationParameters).MultiSampleQuality = QualityLevels;
			(*pPresentationParameters).SwapEffect = D3DSWAPEFFECT_DISCARD;
			BehaviorFlags = D3DCREATE_HARDWARE_VERTEXPROCESSING | (BehaviorFlags & ~D3DCREATE_SOFTWARE_VERTEXPROCESSING);
		}
	}

	// Create Device
	HRESULT hr = ProxyInterface->CreateDevice(Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);

	// Check for AntiAliasing
	if (MultiSampleFlag)
	{
		// Check if device was created successfully
		if (SUCCEEDED(hr))
		{
			Logging::Log() << "Setting MultiSample " << SampleType << " Quality " << QualityLevels;
			(*ppReturnedDeviceInterface)->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, TRUE);
		}
		// Try creating device with original settings
		else
		{
			Logging::Log() << "Error enabling AntiAliasing!";
			BehaviorFlags = bFlags;
			CopyMemory(pPresentationParameters, &d3dpp, sizeof(d3dpp));
			hr = ProxyInterface->CreateDevice(Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);
		}
	}

	if (SUCCEEDED(hr))
	{
		*ppReturnedDeviceInterface = new m_IDirect3DDevice9(*ppReturnedDeviceInterface, this);
	}

	return hr;
}
