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

#include "d3d9.h"

m_IDirect3DDevice9Ex *pD3DDeviceInterface = nullptr;
HWND DeviceWindow = nullptr;
LONG BufferWidth = 0, BufferHeight = 0;

HRESULT m_IDirect3D9Ex::QueryInterface(REFIID riid, void** ppvObj)
{
	Logging::LogDebug() << __FUNCTION__;

	if (riid == IID_IUnknown && ppvObj)
	{
		AddRef();

		*ppvObj = this;

		return D3D_OK;
	}
	else if ((riid == IID_IDirect3D9 || riid == IID_IDirect3D9Ex) && ppvObj)
	{
		HRESULT hr = ProxyInterface->QueryInterface(riid, ppvObj);

		if (SUCCEEDED(hr) && *ppvObj != ProxyInterface)
		{
			*ppvObj = new m_IDirect3D9Ex((LPDIRECT3D9EX)*ppvObj);
		}
		else if (SUCCEEDED(hr))
		{
			*ppvObj = this;
		}

		return hr;
	}

	return ProxyInterface->QueryInterface(riid, ppvObj);
}

ULONG m_IDirect3D9Ex::AddRef()
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->AddRef();
}

ULONG m_IDirect3D9Ex::Release()
{
	Logging::LogDebug() << __FUNCTION__;

	ULONG ref = ProxyInterface->Release();

	if (ref == 0)
	{
		delete this;
	}

	return ref;
}

HRESULT m_IDirect3D9Ex::EnumAdapterModes(THIS_ UINT Adapter, D3DFORMAT Format, UINT Mode, D3DDISPLAYMODE* pMode)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->EnumAdapterModes(Adapter, Format, Mode, pMode);
}

UINT m_IDirect3D9Ex::GetAdapterCount()
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->GetAdapterCount();
}

HRESULT m_IDirect3D9Ex::GetAdapterDisplayMode(UINT Adapter, D3DDISPLAYMODE *pMode)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->GetAdapterDisplayMode(Adapter, pMode);
}

HRESULT m_IDirect3D9Ex::GetAdapterIdentifier(UINT Adapter, DWORD Flags, D3DADAPTER_IDENTIFIER9 *pIdentifier)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->GetAdapterIdentifier(Adapter, Flags, pIdentifier);
}

UINT m_IDirect3D9Ex::GetAdapterModeCount(THIS_ UINT Adapter, D3DFORMAT Format)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->GetAdapterModeCount(Adapter, Format);
}

HMONITOR m_IDirect3D9Ex::GetAdapterMonitor(UINT Adapter)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->GetAdapterMonitor(Adapter);
}

HRESULT m_IDirect3D9Ex::GetDeviceCaps(UINT Adapter, D3DDEVTYPE DeviceType, D3DCAPS9 *pCaps)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->GetDeviceCaps(Adapter, DeviceType, pCaps);
}

HRESULT m_IDirect3D9Ex::RegisterSoftwareDevice(void *pInitializeFunction)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->RegisterSoftwareDevice(pInitializeFunction);
}

HRESULT m_IDirect3D9Ex::CheckDepthStencilMatch(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, D3DFORMAT RenderTargetFormat, D3DFORMAT DepthStencilFormat)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->CheckDepthStencilMatch(Adapter, DeviceType, AdapterFormat, RenderTargetFormat, DepthStencilFormat);
}

HRESULT m_IDirect3D9Ex::CheckDeviceFormat(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, DWORD Usage, D3DRESOURCETYPE RType, D3DFORMAT CheckFormat)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->CheckDeviceFormat(Adapter, DeviceType, AdapterFormat, Usage, RType, CheckFormat);
}

HRESULT m_IDirect3D9Ex::CheckDeviceMultiSampleType(THIS_ UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SurfaceFormat, BOOL Windowed, D3DMULTISAMPLE_TYPE MultiSampleType, DWORD* pQualityLevels)
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.EnableWindowMode)
	{
		Windowed = true;
	}

	return ProxyInterface->CheckDeviceMultiSampleType(Adapter, DeviceType, SurfaceFormat, Windowed, MultiSampleType, pQualityLevels);
}

HRESULT m_IDirect3D9Ex::CheckDeviceType(UINT Adapter, D3DDEVTYPE CheckType, D3DFORMAT DisplayFormat, D3DFORMAT BackBufferFormat, BOOL Windowed)
{
	Logging::LogDebug() << __FUNCTION__;

	if (Config.EnableWindowMode)
	{
		Windowed = true;
	}

	return ProxyInterface->CheckDeviceType(Adapter, CheckType, DisplayFormat, BackBufferFormat, Windowed);
}

HRESULT m_IDirect3D9Ex::CheckDeviceFormatConversion(THIS_ UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SourceFormat, D3DFORMAT TargetFormat)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->CheckDeviceFormatConversion(Adapter, DeviceType, SourceFormat, TargetFormat);
}

HRESULT m_IDirect3D9Ex::CreateDevice(UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS *pPresentationParameters, IDirect3DDevice9 **ppReturnedDeviceInterface)
{
	Logging::LogDebug() << __FUNCTION__;

	if (!pPresentationParameters || !ppReturnedDeviceInterface)
	{
		return D3DERR_INVALIDCALL;
	}

	if (Config.Dd7to9 || Config.D3d8to9)
	{
		PVOID NullValue = nullptr;
		IDirect3DDevice9 *pReturnedDevice = (IDirect3DDevice9*)InterlockedCompareExchangePointer((PVOID*)&pD3DDeviceInterface, NullValue, NullValue);

		if (pReturnedDevice)
		{
			pReturnedDevice->AddRef();

			pReturnedDevice->Reset(pPresentationParameters);

			*ppReturnedDeviceInterface = pReturnedDevice;

			return D3D_OK;
		}
	}

	HRESULT hr = D3DERR_INVALIDCALL;

	// Update presentation parameters
	UpdatePresentParameter(pPresentationParameters, hFocusWindow, true);

	// Check for AntiAliasing
	bool MultiSampleFlag = false;
	if (Config.AntiAliasing != 0)
	{
		DWORD QualityLevels = 0;
		D3DPRESENT_PARAMETERS d3dpp;
		CopyMemory(&d3dpp, pPresentationParameters, sizeof(D3DPRESENT_PARAMETERS));
		d3dpp.BackBufferCount = (d3dpp.BackBufferCount) ? d3dpp.BackBufferCount : 1;

		// Check AntiAliasing quality
		for (int x = min(16, Config.AntiAliasing); x > 0; x--)
		{
			if (ProxyInterface->CheckDeviceMultiSampleType(Adapter,
				DeviceType, (d3dpp.BackBufferFormat) ? d3dpp.BackBufferFormat : D3DFMT_X8R8G8B8, d3dpp.Windowed,
				(D3DMULTISAMPLE_TYPE)x, &QualityLevels) == D3D_OK ||
				ProxyInterface->CheckDeviceMultiSampleType(Adapter,
					DeviceType, d3dpp.AutoDepthStencilFormat, d3dpp.Windowed,
					(D3DMULTISAMPLE_TYPE)x, &QualityLevels) == D3D_OK)
			{
				// Update Present Parameter for Multisample
				UpdatePresentParameterForMultisample(&d3dpp, (D3DMULTISAMPLE_TYPE)x, (QualityLevels > 0) ? QualityLevels - 1 : 0);

				// Create Device
				hr = ProxyInterface->CreateDevice(Adapter, DeviceType, hFocusWindow, BehaviorFlags, &d3dpp, ppReturnedDeviceInterface);

				// Check if device was created successfully
				if (SUCCEEDED(hr))
				{
					MultiSampleFlag = true;
					(*ppReturnedDeviceInterface)->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, TRUE);
					LOG_LIMIT(3, "Setting MultiSample " << d3dpp.MultiSampleType << " Quality " << d3dpp.MultiSampleQuality);
				}
				break;
			}
		}
		if (FAILED(hr))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Failed to enable AntiAliasing!");
		}
	}

	// Create Device
	if (FAILED(hr))
	{
		hr = ProxyInterface->CreateDevice(Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);
	}

	if (SUCCEEDED(hr))
	{
		m_IDirect3DDevice9Ex *pReturnedDevice = new m_IDirect3DDevice9Ex((LPDIRECT3DDEVICE9EX)*ppReturnedDeviceInterface, (m_IDirect3D9Ex*)this);

		*ppReturnedDeviceInterface = pReturnedDevice;

		pReturnedDevice->SetDefaults(pPresentationParameters, hFocusWindow, MultiSampleFlag);

		InterlockedExchangePointer((PVOID*)&pD3DDeviceInterface, pReturnedDevice);
	}

	return hr;
}

UINT m_IDirect3D9Ex::GetAdapterModeCountEx(THIS_ UINT Adapter, CONST D3DDISPLAYMODEFILTER* pFilter)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->GetAdapterModeCountEx(Adapter, pFilter);
}

HRESULT m_IDirect3D9Ex::EnumAdapterModesEx(THIS_ UINT Adapter, CONST D3DDISPLAYMODEFILTER* pFilter, UINT Mode, D3DDISPLAYMODEEX* pMode)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->EnumAdapterModesEx(Adapter, pFilter, Mode, pMode);
}

HRESULT m_IDirect3D9Ex::GetAdapterDisplayModeEx(THIS_ UINT Adapter, D3DDISPLAYMODEEX* pMode, D3DDISPLAYROTATION* pRotation)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->GetAdapterDisplayModeEx(Adapter, pMode, pRotation);
}

HRESULT m_IDirect3D9Ex::CreateDeviceEx(THIS_ UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, D3DDISPLAYMODEEX* pFullscreenDisplayMode, IDirect3DDevice9Ex** ppReturnedDeviceInterface)
{
	Logging::LogDebug() << __FUNCTION__;

	if (!pPresentationParameters || !ppReturnedDeviceInterface)
	{
		return D3DERR_INVALIDCALL;
	}

	HRESULT hr = D3DERR_INVALIDCALL;

	// Update presentation parameters
	UpdatePresentParameter(pPresentationParameters, hFocusWindow, true);

	// Check for AntiAliasing
	bool MultiSampleFlag = false;
	if (Config.AntiAliasing != 0)
	{
		DWORD QualityLevels = 0;
		D3DPRESENT_PARAMETERS d3dpp;
		CopyMemory(&d3dpp, pPresentationParameters, sizeof(D3DPRESENT_PARAMETERS));
		d3dpp.BackBufferCount = (d3dpp.BackBufferCount) ? d3dpp.BackBufferCount : 1;

		// Check AntiAliasing quality
		for (int x = min(16, Config.AntiAliasing); x > 0; x--)
		{
			if (ProxyInterface->CheckDeviceMultiSampleType(Adapter,
				DeviceType, (d3dpp.BackBufferFormat) ? d3dpp.BackBufferFormat : D3DFMT_X8R8G8B8, d3dpp.Windowed,
				(D3DMULTISAMPLE_TYPE)x, &QualityLevels) == D3D_OK ||
				ProxyInterface->CheckDeviceMultiSampleType(Adapter,
					DeviceType, d3dpp.AutoDepthStencilFormat, d3dpp.Windowed,
					(D3DMULTISAMPLE_TYPE)x, &QualityLevels) == D3D_OK)
			{
				// Update Present Parameter for Multisample
				UpdatePresentParameterForMultisample(&d3dpp, (D3DMULTISAMPLE_TYPE)x, (QualityLevels > 0) ? QualityLevels - 1 : 0);

				// Create Device
				hr = ProxyInterface->CreateDeviceEx(Adapter, DeviceType, hFocusWindow, BehaviorFlags, &d3dpp, pFullscreenDisplayMode, ppReturnedDeviceInterface);

				// Check if device was created successfully
				if (SUCCEEDED(hr))
				{
					MultiSampleFlag = true;
					(*ppReturnedDeviceInterface)->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, TRUE);
					LOG_LIMIT(3, "Setting MultiSample " << d3dpp.MultiSampleType << " Quality " << d3dpp.MultiSampleQuality);
				}
				break;
			}
		}
		if (FAILED(hr))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Failed to enable AntiAliasing!");
		}
	}

	// Create Device
	if (FAILED(hr))
	{
		hr = ProxyInterface->CreateDeviceEx(Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, pFullscreenDisplayMode, ppReturnedDeviceInterface);
	}

	if (SUCCEEDED(hr))
	{
		m_IDirect3DDevice9Ex *pReturnedDevice = new m_IDirect3DDevice9Ex(*ppReturnedDeviceInterface, this);

		*ppReturnedDeviceInterface = pReturnedDevice;

		pReturnedDevice->SetDefaults(pPresentationParameters, hFocusWindow, MultiSampleFlag);
	}

	return hr;
}

HRESULT m_IDirect3D9Ex::GetAdapterLUID(THIS_ UINT Adapter, LUID * pLUID)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->GetAdapterLUID(Adapter, pLUID);
}

// Set Presentation Parameters
void UpdatePresentParameter(D3DPRESENT_PARAMETERS* pPresentationParameters, HWND hFocusWindow, bool SetWindow)
{
	if (!pPresentationParameters)
	{
		return;
	}

	if (Config.EnableVSync && pPresentationParameters->PresentationInterval == D3DPRESENT_INTERVAL_IMMEDIATE)
	{
		pPresentationParameters->PresentationInterval = D3DPRESENT_INTERVAL_ONE;
	}

	// Set window size if window mode is enabled
	if (Config.EnableWindowMode && (IsWindow(pPresentationParameters->hDeviceWindow) || IsWindow(DeviceWindow) || IsWindow(hFocusWindow)))
	{
		pPresentationParameters->Windowed = true;
		pPresentationParameters->FullScreen_RefreshRateInHz = 0;
		if (SetWindow)
		{
			BufferWidth = (pPresentationParameters->BackBufferWidth) ? pPresentationParameters->BackBufferWidth : BufferWidth;
			BufferHeight = (pPresentationParameters->BackBufferHeight) ? pPresentationParameters->BackBufferHeight : BufferHeight;
			DeviceWindow = (IsWindow(pPresentationParameters->hDeviceWindow)) ? pPresentationParameters->hDeviceWindow :
				(IsWindow(hFocusWindow)) ? hFocusWindow : DeviceWindow;
			if (!BufferWidth || !BufferHeight)
			{
				RECT tempRect;
				GetClientRect(DeviceWindow, &tempRect);
				BufferWidth = tempRect.right;
				BufferHeight = tempRect.bottom;
			}
			if (Config.FullscreenWindowMode)
			{
				DEVMODE newSettings;
				ZeroMemory(&newSettings, sizeof(newSettings));
				if (EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &newSettings) != 0)
				{
					newSettings.dmPelsWidth = BufferWidth;
					newSettings.dmPelsHeight = BufferHeight;
					newSettings.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;
					ChangeDisplaySettings(&newSettings, CDS_FULLSCREEN);
				}
			}
			AdjustWindow(DeviceWindow, BufferWidth, BufferHeight);
		}
	}
}

// Set Presentation Parameters
void UpdatePresentParameterForMultisample(D3DPRESENT_PARAMETERS* pPresentationParameters, D3DMULTISAMPLE_TYPE MultiSampleType, DWORD MultiSampleQuality)
{
	if (!pPresentationParameters)
	{
		return;
	}

	pPresentationParameters->MultiSampleType = MultiSampleType;
	pPresentationParameters->MultiSampleQuality = MultiSampleQuality;

	pPresentationParameters->Flags &= ~D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
	pPresentationParameters->SwapEffect = D3DSWAPEFFECT_DISCARD;

	if (!pPresentationParameters->EnableAutoDepthStencil)
	{
		pPresentationParameters->EnableAutoDepthStencil = true;
		pPresentationParameters->AutoDepthStencilFormat = D3DFMT_D24S8;
	}
}

// Adjusting the window position for WindowMode
void AdjustWindow(HWND MainhWnd, LONG displayWidth, LONG displayHeight)
{
	if (!IsWindow(MainhWnd) || !displayWidth || !displayHeight)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: could not set window size, nullptr.");
		return;
	}

	// Get screen width and height
	LONG screenWidth = GetSystemMetrics(SM_CXSCREEN);
	LONG screenHeight = GetSystemMetrics(SM_CYSCREEN);

	// Get window border
	LONG lStyle = GetWindowLong(MainhWnd, GWL_STYLE) | WS_VISIBLE;
	if (Config.WindowModeBorder && screenHeight > displayHeight + GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYFIXEDFRAME))
	{
		lStyle |= WS_OVERLAPPEDWINDOW;
	}
	else
	{
		lStyle &= ~WS_OVERLAPPEDWINDOW;
	}

	// Set window border
	SetWindowLong(MainhWnd, GWL_STYLE, lStyle);
	SetWindowPos(MainhWnd, NULL, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_DEFERERASE | SWP_NOSENDCHANGING | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOREDRAW | SWP_FRAMECHANGED);

	// Set window size
	SetWindowPos(MainhWnd, NULL, 0, 0, displayWidth, displayHeight, SWP_NOACTIVATE | SWP_DEFERERASE | SWP_NOSENDCHANGING | SWP_NOMOVE | SWP_NOZORDER | SWP_NOOWNERZORDER);

	// Adjust for window decoration to ensure client area matches display size
	RECT tempRect;
	GetClientRect(MainhWnd, &tempRect);
	LONG newDisplayWidth = (displayWidth - tempRect.right) + displayWidth;
	LONG newDisplayHeight = (displayHeight - tempRect.bottom) + displayHeight;

	// Move window to center and adjust size
	LONG xLoc = 0;
	LONG yLoc = 0;
	if (screenWidth >= newDisplayWidth && screenHeight >= newDisplayHeight)
	{
		xLoc = (screenWidth - newDisplayWidth) / 2;
		yLoc = (screenHeight - newDisplayHeight) / 2;
	}
	SetWindowPos(MainhWnd, NULL, xLoc, yLoc, newDisplayWidth, newDisplayHeight, SWP_NOACTIVATE | SWP_DEFERERASE | SWP_NOSENDCHANGING | SWP_NOZORDER | SWP_NOOWNERZORDER);
}
