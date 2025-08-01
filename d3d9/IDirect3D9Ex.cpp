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
#include "GDI\WndProc.h"

// Initial screen resolution
volatile LONG InitWidth = 0;
volatile LONG InitHeight = 0;

AddressLookupTableD3d9 ProxyAddressLookupTable9;		// Just used for m_IDirect3D9Ex interfaces only

void m_IDirect3D9Ex::InitInterface()
{
	ProxyAddressLookupTable9.SaveAddress(this, ProxyInterface);
}
void m_IDirect3D9Ex::ReleaseInterface()
{
	ProxyAddressLookupTable9.DeleteAddress(this);
}

HRESULT m_IDirect3D9Ex::QueryInterface(REFIID riid, void** ppvObj)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ") " << riid;

	if (!ppvObj)
	{
		return E_POINTER;
	}

	if (riid == IID_GetInterfaceX)
	{
		*ppvObj = this;
		return D3D_OK;
	}

	if (riid == IID_IUnknown || riid == WrapperID || (Config.D3d9to9Ex && riid == IID_IDirect3D9))
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
		if (riid == IID_IDirect3D9 || riid == IID_IDirect3D9Ex)
		{
			*ppvObj = ProxyAddressLookupTable9.FindAddress<m_IDirect3D9Ex, void, LPVOID>(*ppvObj, nullptr, riid, nullptr);
		}
		else
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: not wrapping interface: " << riid);
		}
	}

	return hr;
}

ULONG m_IDirect3D9Ex::AddRef()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->AddRef();
}

ULONG m_IDirect3D9Ex::Release()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	ULONG ref = ProxyInterface->Release();

	if (ref == 0)
	{
		delete this;
	}

	return ref;
}

void m_IDirect3D9Ex::LogAdapterNames()
{
#ifndef DEBUG
	static bool RunOnce = true;
	if (!RunOnce)
	{
		return;
	}
	RunOnce = false;
#endif // DEBUG
	UINT Adapter = ProxyInterface->GetAdapterCount();
	for (UINT x = 0; x < Adapter; x++)
	{
		D3DADAPTER_IDENTIFIER9 Identifier = {};
		if (SUCCEEDED(ProxyInterface->GetAdapterIdentifier(x, NULL, &Identifier)))
		{
			Logging::Log() << __FUNCTION__ << " Adapter: " << x << " " << Identifier.DeviceName << " " << Identifier.Description;
		}
	}
}

HRESULT m_IDirect3D9Ex::EnumAdapterModes(THIS_ UINT Adapter, D3DFORMAT Format, UINT Mode, D3DDISPLAYMODE* pMode)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.LimitDisplayModeCount || Config.OverrideRefreshRate)
	{
		if (!pMode)
		{
			return D3DERR_INVALIDCALL;
		}

		// Required to build the cache, if it doesn't exist
		if (Mode >= GetAdapterModeCache(Adapter, Format, false, nullptr))
		{
			return D3DERR_INVALIDCALL;
		}

		for (auto& entry : AdapterModesCache)
		{
			if (entry.Adapter == Adapter && !entry.IsEx && entry.Filter.Format == Format)
			{
				if (Mode < entry.DisplayModeList.size())
				{
					*pMode = entry.DisplayModeList[Mode].Data();

					return D3D_OK;
				}
				return D3DERR_INVALIDCALL;
			}
		}

		return D3DERR_INVALIDCALL;
	}

	return ProxyInterface->EnumAdapterModes(Adapter, Format, Mode, pMode);
}

UINT m_IDirect3D9Ex::GetAdapterCount()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->GetAdapterCount();
}

HRESULT m_IDirect3D9Ex::GetAdapterDisplayMode(UINT Adapter, D3DDISPLAYMODE *pMode)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->GetAdapterDisplayMode(Adapter, pMode);
}

HRESULT m_IDirect3D9Ex::GetAdapterIdentifier(UINT Adapter, DWORD Flags, D3DADAPTER_IDENTIFIER9 *pIdentifier)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->GetAdapterIdentifier(Adapter, Flags, pIdentifier);
}

UINT m_IDirect3D9Ex::GetAdapterModeCache(THIS_ UINT Adapter, D3DFORMAT Format, bool IsEx, CONST D3DDISPLAYMODEFILTER* pFilter)
{
	if (IsEx && (!pFilter || !ProxyInterfaceEx))
	{
		return 0;
	}

	// For games that require limited resolution return
	const SIZE LimitedResolutionList[] = {
		{ 512, 384 },
		{ 640, 400 },
		{ 640, 480 },
		{ 720, 480 },
		{ 800, 600 },
		{ 1024, 768 },
		{ 1152, 864 },
		{ 1280, 720 },
		{ 1280, 1024 },
		{ 1366, 768 },
		{ 1440, 900 },
		{ 1600, 1200 },
		{ InitWidth, InitHeight },
		{ (LONG)Config.CustomDisplayWidth, (LONG)Config.CustomDisplayHeight } };

	for (auto& entry : AdapterModesCache)
	{
		if (entry.Adapter == Adapter && entry.IsEx == IsEx &&
			(!IsEx && entry.Filter.Format == Format) ||
			(IsEx && entry.Filter.Format == pFilter->Format && entry.Filter.ScanLineOrdering == pFilter->ScanLineOrdering))
		{
			return entry.DisplayModeList.size();
		}
	}

	ENUM_ADAPTERS_CACHE NewCacheEntry;
	NewCacheEntry.Adapter = Adapter;
	NewCacheEntry.IsEx = IsEx;
	NewCacheEntry.Filter.Format = Format;
	if (pFilter)
	{
		NewCacheEntry.Filter = *pFilter;
	}

	UINT Count = 0;
	if (!IsEx)
	{
		Count = ProxyInterface->GetAdapterModeCount(Adapter, Format);
	}
	else
	{
		Count = ProxyInterfaceEx->GetAdapterModeCountEx(Adapter, pFilter);
	}

	UINT RefreshRate = 0;
	std::vector<D3DDISPLAYMODEEX_CONVERT> NewDisplayModeList;

	// Cache all adapter modes
	for (UINT x = 0; x < Count; x++)
	{
		D3DDISPLAYMODEEX_CONVERT DisplayMode;
		if (SUCCEEDED(!IsEx ? ProxyInterface->EnumAdapterModes(Adapter, Format, x, DisplayMode.Ptr()) :
			ProxyInterfaceEx->EnumAdapterModesEx(Adapter, pFilter, x, DisplayMode.PtrEx())))
		{
			if (Config.OverrideRefreshRate &&
				(RefreshRate == 0 || std::abs((INT)Config.OverrideRefreshRate - (INT)DisplayMode.RefreshRate) < std::abs((INT)Config.OverrideRefreshRate - (INT)RefreshRate)))
			{
				RefreshRate = DisplayMode.RefreshRate;
			}
			NewDisplayModeList.push_back(DisplayMode);
		}
	}

	// Filter cached adapter modes
	for (auto& entry : NewDisplayModeList)
	{
		// Check if resolution has already been sent
		bool IsResolutionAlreadySent = std::any_of(NewCacheEntry.DisplayModeList.begin(), NewCacheEntry.DisplayModeList.end(),
			[&](const auto& res) {
				return (res.Width == entry.Width && res.Height == entry.Height && res.RefreshRate == entry.RefreshRate);
			});

		// Check if the resolution is on the LimitedResolutionList
		bool IsResolutionSupported = (!Config.LimitDisplayModeCount ||
			std::any_of(std::begin(LimitedResolutionList), std::end(LimitedResolutionList),
				[&](const auto& res) {
					return ((DWORD)res.cx == entry.Width && (DWORD)res.cy == entry.Height);
				}));

		// Check if refresh rate is suported
		bool IsRefreshSupported = (!Config.OverrideRefreshRate || entry.RefreshRate == RefreshRate);

		// Store entry
		if (!IsResolutionAlreadySent && IsResolutionSupported && IsRefreshSupported)
		{
			NewCacheEntry.DisplayModeList.push_back(entry);
		}
	}

	AdapterModesCache.push_back(NewCacheEntry);

	return NewCacheEntry.DisplayModeList.size();
}

UINT m_IDirect3D9Ex::GetAdapterModeCount(THIS_ UINT Adapter, D3DFORMAT Format)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.LimitDisplayModeCount || Config.OverrideRefreshRate)
	{
		return GetAdapterModeCache(Adapter, Format, false, nullptr);
	}

	return ProxyInterface->GetAdapterModeCount(Adapter, Format);
}

HMONITOR m_IDirect3D9Ex::GetAdapterMonitor(UINT Adapter)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->GetAdapterMonitor(Adapter);
}

HRESULT m_IDirect3D9Ex::GetDeviceCaps(UINT Adapter, D3DDEVTYPE DeviceType, D3DCAPS9 *pCaps)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	HRESULT hr = ProxyInterface->GetDeviceCaps(Adapter, DeviceType, pCaps);

	if (SUCCEEDED(hr))
	{
		pCaps->TextureCaps = AdjustPOW2Caps(pCaps->TextureCaps);
	}

	return hr;
}

HRESULT m_IDirect3D9Ex::RegisterSoftwareDevice(void *pInitializeFunction)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->RegisterSoftwareDevice(pInitializeFunction);
}

HRESULT m_IDirect3D9Ex::CheckDepthStencilMatch(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, D3DFORMAT RenderTargetFormat, D3DFORMAT DepthStencilFormat)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->CheckDepthStencilMatch(Adapter, DeviceType, AdapterFormat, RenderTargetFormat, DepthStencilFormat);
}

HRESULT m_IDirect3D9Ex::CheckDeviceFormat(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, DWORD Usage, D3DRESOURCETYPE RType, D3DFORMAT CheckFormat)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->CheckDeviceFormat(Adapter, DeviceType, AdapterFormat, Usage, RType, CheckFormat);
}

HRESULT m_IDirect3D9Ex::CheckDeviceMultiSampleType(THIS_ UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SurfaceFormat, BOOL Windowed, D3DMULTISAMPLE_TYPE MultiSampleType, DWORD* pQualityLevels)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.EnableWindowMode)
	{
		Windowed = TRUE;
	}

	return ProxyInterface->CheckDeviceMultiSampleType(Adapter, DeviceType, SurfaceFormat, Windowed, MultiSampleType, pQualityLevels);
}

HRESULT m_IDirect3D9Ex::CheckDeviceType(UINT Adapter, D3DDEVTYPE CheckType, D3DFORMAT DisplayFormat, D3DFORMAT BackBufferFormat, BOOL Windowed)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.EnableWindowMode)
	{
		Windowed = TRUE;
	}

	return ProxyInterface->CheckDeviceType(Adapter, CheckType, DisplayFormat, BackBufferFormat, Windowed);
}

HRESULT m_IDirect3D9Ex::CheckDeviceFormatConversion(THIS_ UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SourceFormat, D3DFORMAT TargetFormat)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->CheckDeviceFormatConversion(Adapter, DeviceType, SourceFormat, TargetFormat);
}

template <typename T>
HRESULT m_IDirect3D9Ex::CreateDeviceT(DEVICEDETAILS& DeviceDetails, UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, D3DDISPLAYMODEEX* pFullscreenDisplayMode, T ppReturnedDeviceInterface)
{
	if (!pPresentationParameters || !ppReturnedDeviceInterface)
	{
		return D3DERR_INVALIDCALL;
	}

	// Hook WndProc before creating device
	HWND hwnd = (hFocusWindow ? hFocusWindow : pPresentationParameters ? pPresentationParameters->hDeviceWindow : nullptr);
	WndProc::DATASTRUCT* WndDataStruct = WndProc::AddWndProc(hwnd);
	if (WndDataStruct)
	{
		WndDataStruct->IsDirect3D9 = true;
		if (!WndDataStruct->IsDirectDraw)
		{
			// Already set by DirectDraw
			WndDataStruct->IsCreatingDevice = true;
			WndDataStruct->IsExclusiveMode = !pPresentationParameters->Windowed;
			WndDataStruct->DirectXVersion = ClientDirectXVersion;
		}
		DeviceDetails.IsDirectDrawDevice = WndDataStruct->IsDirectDraw;
	}

	BehaviorFlags = UpdateBehaviorFlags(BehaviorFlags);

	// Create new d3d9 device
	HRESULT hr = D3DERR_INVALIDCALL;

	// Get monitor handle
	HMONITOR hMonitor = nullptr;
	D3DADAPTER_IDENTIFIER9 Identifier = {};
	if (SUCCEEDED(ProxyInterface->GetAdapterIdentifier(Adapter, 0, &Identifier)))
	{
		hMonitor = Utils::GetMonitorFromDeviceName(Identifier.DeviceName);
	}
	if (hMonitor == nullptr)
	{
		Logging::Log() << __FUNCTION__ << " Warning: Failed to get monitor handle!";
	}

	// Needs to be set before updating present parameters
	DeviceDetails.ClientDirectXVersion = ClientDirectXVersion;
	DeviceDetails.Adapter = Adapter;
	DeviceDetails.DeviceType = DeviceType;
	DeviceDetails.hMonitor = hMonitor;

	// Check fullscreen
	bool ForceFullscreen = TestResolution(Adapter, pPresentationParameters->BackBufferWidth, pPresentationParameters->BackBufferHeight);

	// Setup presentation parameters
	D3DPRESENT_PARAMETERS d3dpp;
	CopyMemory(&d3dpp, pPresentationParameters, sizeof(D3DPRESENT_PARAMETERS));
	UpdatePresentParameter(&d3dpp, hFocusWindow, DeviceDetails, ForceFullscreen, true);

	bool IsWindowMode = d3dpp.Windowed != FALSE;

	bool MultiSampleFlag = false;

	// Check for AntiAliasing
	if (Config.AntiAliasing != 0)
	{
		DWORD QualityLevels = 0;

		// Check AntiAliasing quality
		for (int x = min(D3DMULTISAMPLE_16_SAMPLES, Config.AntiAliasing); x > 0; x--)
		{
			D3DMULTISAMPLE_TYPE Samples = (D3DMULTISAMPLE_TYPE)x;
			D3DFORMAT BufferFormat = (d3dpp.BackBufferFormat) ? d3dpp.BackBufferFormat : D3DFMT_X8R8G8B8;
			D3DFORMAT StencilFormat = (d3dpp.AutoDepthStencilFormat) ? d3dpp.AutoDepthStencilFormat : D3DFMT_X8R8G8B8;

			if (SUCCEEDED(ProxyInterface->CheckDeviceMultiSampleType(Adapter, DeviceType, BufferFormat, d3dpp.Windowed, Samples, &QualityLevels)) &&
				SUCCEEDED(ProxyInterface->CheckDeviceMultiSampleType(Adapter, DeviceType, StencilFormat, d3dpp.Windowed, Samples, &QualityLevels)))
			{
				// Update Present Parameter for Multisample
				UpdatePresentParameterForMultisample(&d3dpp, Samples, (QualityLevels > 0) ? QualityLevels - 1 : 0);

				// Create Device
				hr = CreateDeviceT(Adapter, DeviceType, hFocusWindow, BehaviorFlags, &d3dpp, (d3dpp.Windowed ? nullptr : pFullscreenDisplayMode), ppReturnedDeviceInterface);

				// Check if device was created successfully
				if (SUCCEEDED(hr))
				{
					MultiSampleFlag = true;
					(*ppReturnedDeviceInterface)->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, TRUE);
					LOG_LIMIT(3, "Setting MultiSample " << d3dpp.MultiSampleType << " Quality " << d3dpp.MultiSampleQuality);
					break;
				}
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
		// Update presentation parameters
		CopyMemory(&d3dpp, pPresentationParameters, sizeof(D3DPRESENT_PARAMETERS));
		UpdatePresentParameter(&d3dpp, hFocusWindow, DeviceDetails, ForceFullscreen, false);

		// Create Device
		hr = CreateDeviceT(Adapter, DeviceType, hFocusWindow, BehaviorFlags, &d3dpp, (d3dpp.Windowed ? nullptr : pFullscreenDisplayMode), ppReturnedDeviceInterface);
	}

	if (SUCCEEDED(hr))
	{
		GetFinalPresentParameter(&d3dpp, DeviceDetails);

		if (WndDataStruct && WndDataStruct->IsExclusiveMode)
		{
			d3dpp.Windowed = FALSE;
		}

		if (MultiSampleFlag)
		{
			DeviceDetails.DeviceMultiSampleFlag = true;
			DeviceDetails.DeviceMultiSampleType = d3dpp.MultiSampleType;
			DeviceDetails.DeviceMultiSampleQuality = d3dpp.MultiSampleQuality;
		}

		DeviceDetails.IsWindowMode = IsWindowMode;

		CopyMemory(pPresentationParameters, &d3dpp, sizeof(D3DPRESENT_PARAMETERS));
	}

	// Update WndProc after creating device
	if (WndDataStruct && !WndDataStruct->IsDirectDraw)
	{
		// Already set by DirectDraw
		WndDataStruct->IsCreatingDevice = false;
	}

	return hr;
}

HRESULT m_IDirect3D9Ex::CreateDevice(UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS *pPresentationParameters, IDirect3DDevice9 **ppReturnedDeviceInterface)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!pPresentationParameters || !ppReturnedDeviceInterface)
	{
		return D3DERR_INVALIDCALL;
	}

	if (Config.D3d9to9Ex)
	{
		D3DDISPLAYMODEEX* pFullscreenMode = nullptr;
		D3DDISPLAYMODEEX FullscreenDisplayMode = {};

		// Fill fullscreen display mode only in fullscreen mode
		if (!pPresentationParameters->Windowed)
		{
			GetFullscreenDisplayMode(*pPresentationParameters, FullscreenDisplayMode);
			pFullscreenMode = &FullscreenDisplayMode;
		}

		if (SUCCEEDED(CreateDeviceEx(Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, pFullscreenMode, reinterpret_cast<IDirect3DDevice9Ex**>(ppReturnedDeviceInterface))))
		{
			return D3D_OK;
		}
	}

	DEVICEDETAILS DeviceDetails;

	HRESULT hr = CreateDeviceT(DeviceDetails, Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, nullptr, ppReturnedDeviceInterface);

	if (SUCCEEDED(hr))
	{
		GUID riid = IID_IDirect3DDevice9;

		if (Config.D3d9to9Ex)
		{
			IDirect3DDevice9* pD3DD = *ppReturnedDeviceInterface;
			IDirect3DDevice9Ex* pD3DDEx = nullptr;

			if (SUCCEEDED(pD3DD->QueryInterface(IID_IDirect3DDevice9Ex, reinterpret_cast<LPVOID*>(&pD3DDEx))))
			{
				pD3DD->Release();
				*ppReturnedDeviceInterface = pD3DDEx;
				riid = IID_IDirect3DDevice9Ex;
			}
		}

		UINT DDKey = (UINT)ppReturnedDeviceInterface + (UINT)*ppReturnedDeviceInterface + (UINT)DeviceDetails.DeviceWindow;
		DeviceDetailsMap[DDKey] = DeviceDetails;

		*ppReturnedDeviceInterface = new m_IDirect3DDevice9Ex((LPDIRECT3DDEVICE9EX)*ppReturnedDeviceInterface, this, riid, DDKey);

		return D3D_OK;
	}

	Logging::LogDebug() << __FUNCTION__ << " FAILED! " << (D3DERR)hr << " " << Adapter << " " << DeviceType << " " << hFocusWindow << " " << BehaviorFlags << " " << pPresentationParameters;
	
	return hr;
}

UINT m_IDirect3D9Ex::GetAdapterModeCountEx(THIS_ UINT Adapter, CONST D3DDISPLAYMODEFILTER* pFilter)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterfaceEx)
	{
		Logging::Log() << __FUNCTION__ << " Error: Calling extension function from a non-extension device!";
		return 0;
	}

	if (Config.LimitDisplayModeCount || Config.OverrideRefreshRate)
	{
		return GetAdapterModeCache(Adapter, D3DFMT_UNKNOWN, true, pFilter);
	}

	return ProxyInterfaceEx->GetAdapterModeCountEx(Adapter, pFilter);
}

HRESULT m_IDirect3D9Ex::EnumAdapterModesEx(THIS_ UINT Adapter, CONST D3DDISPLAYMODEFILTER* pFilter, UINT Mode, D3DDISPLAYMODEEX* pMode)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterfaceEx)
	{
		Logging::Log() << __FUNCTION__ << " Error: Calling extension function from a non-extension device!";
		return D3DERR_INVALIDCALL;
	}

	if (Config.LimitDisplayModeCount || Config.OverrideRefreshRate)
	{
		if (!pMode)
		{
			return D3DERR_INVALIDCALL;
		}

		// Required to build the cache, if it doesn't exist
		if (Mode >= GetAdapterModeCache(Adapter, D3DFMT_UNKNOWN, true, pFilter))
		{
			return D3DERR_INVALIDCALL;
		}

		for (auto& entry : AdapterModesCache)
		{
			if (entry.Adapter == Adapter && entry.IsEx && entry.Filter.Format == pFilter->Format && entry.Filter.ScanLineOrdering == pFilter->ScanLineOrdering)
			{
				if (Mode < entry.DisplayModeList.size())
				{
					*pMode = entry.DisplayModeList[Mode].DataEx();

					return D3D_OK;
				}
				return D3DERR_INVALIDCALL;
			}
		}

		return D3DERR_INVALIDCALL;
	}

	return ProxyInterfaceEx->EnumAdapterModesEx(Adapter, pFilter, Mode, pMode);
}

HRESULT m_IDirect3D9Ex::GetAdapterDisplayModeEx(THIS_ UINT Adapter, D3DDISPLAYMODEEX* pMode, D3DDISPLAYROTATION* pRotation)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterfaceEx)
	{
		Logging::Log() << __FUNCTION__ << " Error: Calling extension function from a non-extension device!";
		return D3DERR_INVALIDCALL;
	}

	return ProxyInterfaceEx->GetAdapterDisplayModeEx(Adapter, pMode, pRotation);
}

HRESULT m_IDirect3D9Ex::CreateDeviceEx(THIS_ UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, D3DDISPLAYMODEEX* pFullscreenDisplayMode, IDirect3DDevice9Ex** ppReturnedDeviceInterface)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!pPresentationParameters || !ppReturnedDeviceInterface)
	{
		return D3DERR_INVALIDCALL;
	}

	DEVICEDETAILS DeviceDetails;

	HRESULT hr = CreateDeviceT(DeviceDetails, Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, pFullscreenDisplayMode, ppReturnedDeviceInterface);

	if (SUCCEEDED(hr))
	{
		UINT DDKey = (UINT)ppReturnedDeviceInterface + (UINT)*ppReturnedDeviceInterface + (UINT)DeviceDetails.DeviceWindow;
		DeviceDetailsMap[DDKey] = DeviceDetails;

		*ppReturnedDeviceInterface = new m_IDirect3DDevice9Ex(*ppReturnedDeviceInterface, this, IID_IDirect3DDevice9Ex, DDKey);

		return D3D_OK;
	}

	Logging::LogDebug() << __FUNCTION__ << " FAILED! " << (D3DERR)hr << " " << Adapter << " " << DeviceType << " " << hFocusWindow << " " << BehaviorFlags << " " << pPresentationParameters << " " << pFullscreenDisplayMode;
	return hr;
}

HRESULT m_IDirect3D9Ex::GetAdapterLUID(THIS_ UINT Adapter, LUID * pLUID)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterfaceEx)
	{
		Logging::Log() << __FUNCTION__ << " Error: Calling extension function from a non-extension device!";
		return D3DERR_INVALIDCALL;
	}

	return ProxyInterfaceEx->GetAdapterLUID(Adapter, pLUID);
}

void m_IDirect3D9Ex::SetDirectXVersion(DWORD DxVersion)
{
	ClientDirectXVersion = DxVersion;
}

bool m_IDirect3D9Ex::TestResolution(UINT Adapter, DWORD BackBufferWidth, DWORD BackBufferHeight)
{
	if (Config.ForceExclusiveFullscreen && BackBufferWidth && BackBufferHeight)
	{
		// Enumerate modes for format XRGB
		UINT modeCount = ProxyInterface->GetAdapterModeCount(Adapter, D3DFMT_X8R8G8B8);

		D3DDISPLAYMODE d3ddispmode;

		// Loop through all modes looking for our requested resolution
		for (UINT i = 0; i < modeCount; i++)
		{
			// Get display modes here
			ZeroMemory(&d3ddispmode, sizeof(D3DDISPLAYMODE));
			if (FAILED(ProxyInterface->EnumAdapterModes(Adapter, D3DFMT_X8R8G8B8, i, &d3ddispmode)))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: EnumAdapterModes failed");
				break;
			}
			// Check exact height and width match
			if (d3ddispmode.Width == BackBufferWidth && d3ddispmode.Height == BackBufferHeight)
			{
				return true;
			}
		}
	}
	return false;
}

DWORD m_IDirect3D9Ex::AdjustPOW2Caps(DWORD OriginalCaps)
{
	DWORD Caps = OriginalCaps & ~(D3DPTEXTURECAPS_POW2 | D3DPTEXTURECAPS_NONPOW2CONDITIONAL);

	switch (Config.SetPOW2Caps)
	{
	case 0: return OriginalCaps;														// 0 = return original caps
	case 1: return Caps | D3DPTEXTURECAPS_POW2 | D3DPTEXTURECAPS_NONPOW2CONDITIONAL;	// 1 = force both
	case 2: return Caps | D3DPTEXTURECAPS_NONPOW2CONDITIONAL;							// 2 = force D3DPTEXTURECAPS_NONPOW2CONDITIONAL
	case 3: return Caps | D3DPTEXTURECAPS_POW2;											// 3 = force D3DPTEXTURECAPS_POW2
	default: return Caps;																// 4 = remove both
	}
}

DWORD m_IDirect3D9Ex::UpdateBehaviorFlags(DWORD BehaviorFlags)
{
	if (Config.ForceMixedVertexProcessing || (BehaviorFlags & D3DCREATE_MIXED_VERTEXPROCESSING) ||
		((BehaviorFlags & D3DCREATE_SOFTWARE_VERTEXPROCESSING) && (BehaviorFlags & D3DCREATE_HARDWARE_VERTEXPROCESSING)))
	{
		BehaviorFlags &= ~(D3DCREATE_PUREDEVICE | D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_SOFTWARE_VERTEXPROCESSING);
		BehaviorFlags |= D3DCREATE_MIXED_VERTEXPROCESSING;
	}
	else if (BehaviorFlags & D3DCREATE_SOFTWARE_VERTEXPROCESSING)
	{
		BehaviorFlags &= ~(D3DCREATE_PUREDEVICE | D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_MIXED_VERTEXPROCESSING);
		BehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	}
	else
	{
		BehaviorFlags &= ~(D3DCREATE_PUREDEVICE | D3DCREATE_MIXED_VERTEXPROCESSING | D3DCREATE_SOFTWARE_VERTEXPROCESSING);
		BehaviorFlags |= D3DCREATE_HARDWARE_VERTEXPROCESSING;
	}
	return BehaviorFlags;
}

// Update Presentation Parameters
void m_IDirect3D9Ex::UpdatePresentParameter(D3DPRESENT_PARAMETERS* pPresentationParameters, HWND hFocusWindow, DEVICEDETAILS& DeviceDetails, bool ForceExclusiveFullscreen, bool SetWindow)
{
	if (!pPresentationParameters)
	{
		return;
	}

	// Set vsync
	if (Config.EnableVSync && (Config.ForceVsyncMode || pPresentationParameters->PresentationInterval == D3DPRESENT_INTERVAL_IMMEDIATE))
	{
		pPresentationParameters->PresentationInterval = D3DPRESENT_INTERVAL_ONE;
	}
	else if (Config.ForceVsyncMode)
	{
		pPresentationParameters->PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
	}

	// Set windowed mode if enabled
	if (ForceExclusiveFullscreen)
	{
		pPresentationParameters->Windowed = FALSE;
		if (!pPresentationParameters->FullScreen_RefreshRateInHz)
		{
			pPresentationParameters->FullScreen_RefreshRateInHz = Utils::GetRefreshRate(DeviceDetails.hMonitor);
		}
		if (pPresentationParameters->BackBufferFormat == D3DFMT_UNKNOWN)
		{
			pPresentationParameters->BackBufferFormat = D3DFMT_X8R8G8B8;
		}
	}
	else if (Config.EnableWindowMode)
	{
		pPresentationParameters->Windowed = TRUE;
		pPresentationParameters->FullScreen_RefreshRateInHz = 0;
	}

	// Store last window data
	LONG LastBufferWidth = DeviceDetails.BufferWidth;
	LONG LastBufferHeight = DeviceDetails.BufferHeight;
	HWND LastDeviceWindow = DeviceDetails.DeviceWindow;

	// Get current window data
	if (IsWindow(pPresentationParameters->hDeviceWindow) || IsWindow(DeviceDetails.DeviceWindow) || IsWindow(hFocusWindow))
	{
		DeviceDetails.BufferWidth = (pPresentationParameters->BackBufferWidth) ? pPresentationParameters->BackBufferWidth : DeviceDetails.BufferWidth;
		DeviceDetails.BufferHeight = (pPresentationParameters->BackBufferHeight) ? pPresentationParameters->BackBufferHeight : DeviceDetails.BufferHeight;
		DeviceDetails.DeviceWindow = (IsWindow(hFocusWindow)) ? hFocusWindow :
			(IsWindow(pPresentationParameters->hDeviceWindow)) ? pPresentationParameters->hDeviceWindow :
			DeviceDetails.DeviceWindow;

		// Check if window is minimized
		if (IsIconic(DeviceDetails.DeviceWindow))
		{
			ShowWindow(DeviceDetails.DeviceWindow, SW_RESTORE);
		}

		// Remove tool and topmost window
		if (DeviceDetails.DeviceWindow != LastDeviceWindow)
		{
			LONG lExStyle = GetWindowLong(DeviceDetails.DeviceWindow, GWL_EXSTYLE);
			if (lExStyle & (WS_EX_TOOLWINDOW | WS_EX_TOPMOST))
			{
				LOG_LIMIT(3, __FUNCTION__ << " Removing window" << ((lExStyle & WS_EX_TOOLWINDOW) ? " WS_EX_TOOLWINDOW" : "") << ((lExStyle & WS_EX_TOPMOST) ? " WS_EX_TOPMOST" : ""));

				SetWindowLong(DeviceDetails.DeviceWindow, GWL_EXSTYLE, lExStyle & ~(WS_EX_TOOLWINDOW | WS_EX_TOPMOST));
				SetWindowPos(DeviceDetails.DeviceWindow, ((lExStyle & WS_EX_TOPMOST) ? HWND_NOTOPMOST : HWND_TOP),
					0, 0, 0, 0, ((lExStyle & WS_EX_TOPMOST) ? NULL : SWP_NOZORDER) | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
			}
		}

		// Get window width and height
		if (!DeviceDetails.BufferWidth || !DeviceDetails.BufferHeight)
		{
			RECT tempRect;
			GetClientRect(DeviceDetails.DeviceWindow, &tempRect);
			DeviceDetails.BufferWidth = tempRect.right;
			DeviceDetails.BufferHeight = tempRect.bottom;
		}

		// Set window size
		if (SetWindow && pPresentationParameters->Windowed)
		{
			bool AnyChange = (LastBufferWidth != DeviceDetails.BufferWidth || LastBufferHeight != DeviceDetails.BufferHeight || LastDeviceWindow != DeviceDetails.DeviceWindow);

			// Adjust window
			RECT Rect;
			GetClientRect(DeviceDetails.DeviceWindow, &Rect);
			if (AnyChange || Rect.right - Rect.left != DeviceDetails.BufferWidth || Rect.bottom - Rect.top != DeviceDetails.BufferHeight)
			{
				AdjustWindow(DeviceDetails.hMonitor, DeviceDetails.DeviceWindow, DeviceDetails.BufferWidth, DeviceDetails.BufferHeight, pPresentationParameters->Windowed, Config.EnableWindowMode, Config.FullscreenWindowMode);
			}

			// Set fullscreen resolution
			if (AnyChange && Config.FullscreenWindowMode)
			{
				Utils::SetDisplaySettings(DeviceDetails.hMonitor, DeviceDetails.BufferWidth, DeviceDetails.BufferHeight);
			}
		}
	}
}

void m_IDirect3D9Ex::GetFinalPresentParameter(D3DPRESENT_PARAMETERS* pPresentationParameters, DEVICEDETAILS& DeviceDetails)
{
	if (pPresentationParameters && (IsWindow(pPresentationParameters->hDeviceWindow) || IsWindow(DeviceDetails.DeviceWindow)))
	{
		DeviceDetails.BufferWidth = (pPresentationParameters->BackBufferWidth) ? pPresentationParameters->BackBufferWidth : DeviceDetails.BufferWidth;
		DeviceDetails.BufferHeight = (pPresentationParameters->BackBufferHeight) ? pPresentationParameters->BackBufferHeight : DeviceDetails.BufferHeight;
		DeviceDetails.DeviceWindow = (IsWindow(pPresentationParameters->hDeviceWindow)) ? pPresentationParameters->hDeviceWindow : DeviceDetails.DeviceWindow;
	}
}

// Set Presentation Parameters for Multisample
void m_IDirect3D9Ex::UpdatePresentParameterForMultisample(D3DPRESENT_PARAMETERS* pPresentationParameters, D3DMULTISAMPLE_TYPE MultiSampleType, DWORD MultiSampleQuality)
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

	pPresentationParameters->BackBufferCount = (pPresentationParameters->BackBufferCount) ? pPresentationParameters->BackBufferCount : 1;
}

void m_IDirect3D9Ex::GetFullscreenDisplayMode(D3DPRESENT_PARAMETERS& d3dpp, D3DDISPLAYMODEEX& Mode)
{
	Mode.Size = sizeof(D3DDISPLAYMODEEX);

	// Derive width and height
	Mode.Width = d3dpp.BackBufferWidth;
	Mode.Height = d3dpp.BackBufferHeight;

	// Derive format
	Mode.Format = d3dpp.BackBufferFormat;

	// RefreshRate
	Mode.RefreshRate = d3dpp.FullScreen_RefreshRateInHz;

	// ScanLineOrdering and Scaling are optional and can usually be defaulted
	Mode.ScanLineOrdering = D3DSCANLINEORDERING_PROGRESSIVE;
}

// Adjusting the window position for WindowMode
void m_IDirect3D9Ex::AdjustWindow(HMONITOR hMonitor, HWND MainhWnd, LONG displayWidth, LONG displayHeight, bool isWindowed, bool EnableWindowMode, bool FullscreenWindowMode)
{
	if (!IsWindow(MainhWnd) || !displayWidth || !displayHeight)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: could not set window size, nullptr.");
		return;
	}

	// Verify monitor handle
	if (!Utils::IsMonitorValid(hMonitor))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Warning: monitor handle is invalid, using window location instead.");
		hMonitor = Utils::GetMonitorFromWindow(MainhWnd);
	}

	// Remove clip children for popup windows
	LONG lStyle = GetWindowLong(MainhWnd, GWL_STYLE);
	if ((lStyle & WS_POPUP) && (lStyle & WS_CLIPCHILDREN))
	{
		SetWindowLong(MainhWnd, GWL_STYLE, lStyle & ~WS_CLIPCHILDREN);
		SetWindowPos(MainhWnd, HWND_TOP, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);
	}

	// Set window active and focus
	if (EnableWindowMode || isWindowed)
	{
		// Move window to top if not already topmost
		LONG lExStyle = GetWindowLong(MainhWnd, GWL_EXSTYLE);
		if (!(lExStyle & WS_EX_TOPMOST))
		{
			SetWindowLong(MainhWnd, GWL_EXSTYLE, lExStyle | WS_EX_TOPMOST);
			SetWindowPos(MainhWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
			SetWindowLong(MainhWnd, GWL_EXSTYLE, lExStyle & ~WS_EX_TOPMOST);
			SetWindowPos(MainhWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
		}

		// Set active and foreground if needed
		if (MainhWnd != GetForegroundWindow() || MainhWnd != GetFocus() || MainhWnd != GetActiveWindow())
		{
			DWORD currentThreadId = GetCurrentThreadId();
			DWORD foregroundThreadId = GetWindowThreadProcessId(GetForegroundWindow(), NULL);

			bool isForeground = (MainhWnd == GetForegroundWindow());
			bool shouldAttachThreadID = !isForeground && (currentThreadId != foregroundThreadId);

			// Attach the input of the foreground window and current window
			if (shouldAttachThreadID)
			{
				AttachThreadInput(currentThreadId, foregroundThreadId, TRUE);
			}
			if (!isForeground)
			{
				SetForegroundWindow(MainhWnd);
			}

			SetFocus(MainhWnd);
			SetActiveWindow(MainhWnd);
			BringWindowToTop(MainhWnd);

			// Detach the input from the foreground window
			if (shouldAttachThreadID)
			{
				AttachThreadInput(currentThreadId, foregroundThreadId, FALSE);
			}
		}
	}

	// Get screen area and width and height
	LONG screenWidth = 0, screenHeight = 0;
	Utils::GetScreenSize(hMonitor, screenWidth, screenHeight);
	RECT screenClientRect = {};
	Utils::GetScreenClientRect(hMonitor, screenClientRect);
	LONG screenClientWidth = screenClientRect.right - screenClientRect.left;
	LONG screenClientHeight = screenClientRect.bottom - screenClientRect.top;

	// Get window style
	LONG lOrgStyle = GetWindowLong(MainhWnd, GWL_STYLE);
	lStyle = lOrgStyle;
	LONG lExStyle = GetWindowLong(MainhWnd, GWL_EXSTYLE);
	BOOL HasMenu = (GetMenu(MainhWnd) != NULL);

	// Set window style
	bool clientWidthOverlap = false, clientHeightOverlap = false;
	if (EnableWindowMode)
	{
		// New window size
		LONG windowWidth = displayWidth;
		LONG windowHeight = displayHeight;

		// Get border style and size
		LONG lBorderStyle = lStyle;
		int borderWidth = 0, borderHeight = 0;
		if (Config.EnableWindowMode && Config.WindowModeBorder && !FullscreenWindowMode)
		{
			// Apply windowed border to popup and child window
			if (lBorderStyle & (WS_POPUP | WS_CHILD | WS_BORDER))
			{
				lBorderStyle |= (WS_BORDER | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);
				if (!(lBorderStyle & WS_CHILD))
				{
					lBorderStyle |= WS_POPUP;
				}
			}
			// Apply windowed border to other window types
			else
			{
				lBorderStyle |= WS_OVERLAPPEDWINDOW;
			}

			// Get window size with border
			RECT rc = { 0, 0, displayWidth, displayHeight };
			AdjustWindowRectEx(&rc, lBorderStyle, HasMenu, lExStyle);

			int borderLeft = -rc.left;
			int borderTop = -rc.top;    // Includes the title bar
			int borderRight = rc.right - displayWidth;
			int borderBottom = rc.bottom - displayHeight;

			borderWidth = borderLeft + borderRight;
			borderHeight = borderTop + borderBottom;
		}

		// Check for overlapping
		clientWidthOverlap = screenClientWidth < windowWidth + borderWidth;
		clientHeightOverlap = screenClientHeight < windowHeight + borderHeight;

		// Update style
		if (Config.EnableWindowMode)
		{
			// Apply window border
			if (Config.WindowModeBorder && !FullscreenWindowMode && !clientWidthOverlap && !clientHeightOverlap)
			{
				lStyle = lBorderStyle;
			}
			// Remove window border
			else
			{
				lStyle &= ~(WS_CAPTION | WS_BORDER | WS_THICKFRAME | WS_DLGFRAME);
			}
		}
		// Used when ddraw exclusive mode fails and switches to fullscreen windowed mode
		else
		{
			// Remove window border only
			lStyle &= ~WS_BORDER;
		}

		// Set style if it needs to change
		if (lOrgStyle != lStyle)
		{
			SetWindowLong(MainhWnd, GWL_STYLE, lStyle);
			SetWindowPos(MainhWnd, HWND_TOP, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);
		}
	}

	// Get new window rect
	RECT Rect = { 0, 0, displayWidth, displayHeight };
	AdjustWindowRectEx(&Rect, lStyle, HasMenu, lExStyle);
	Rect = { 0, 0, Rect.right - Rect.left, Rect.bottom - Rect.top };

	// Get upper left window position
	bool SetWindowPositionFlag = FullscreenWindowMode;
	LONG xLoc = 0, yLoc = 0;
	if (Config.SetInitialWindowPosition && !FullscreenWindowMode &&
		(Config.InitialWindowPositionLeft == 0 || Rect.right + (LONG)Config.InitialWindowPositionLeft <= screenWidth) &&
		(Config.InitialWindowPositionTop == 0 || Rect.bottom + (LONG)Config.InitialWindowPositionTop <= screenHeight))
	{
		SetWindowPositionFlag = true;
		xLoc = Config.InitialWindowPositionLeft;
		yLoc = Config.InitialWindowPositionTop;
	}
	else if (EnableWindowMode && !FullscreenWindowMode && screenWidth >= Rect.right && screenHeight >= Rect.bottom)
	{
		SetWindowPositionFlag = true;
		xLoc = (screenWidth - Rect.right) / 2;
		yLoc = (screenHeight - Rect.bottom) / 2;
	}

	// Ensure the window doesn't cross over the desktop client area
	if (!FullscreenWindowMode)
	{
		if (!clientWidthOverlap)
		{
			if (xLoc < screenClientRect.left)
			{
				xLoc = screenClientRect.left;
			}
			if (xLoc + Rect.right > screenClientRect.right)
			{
				xLoc -= (xLoc + Rect.right) - screenClientRect.right;
			}
		}
		if (!clientHeightOverlap)
		{
			if (yLoc < screenClientRect.top)
			{
				yLoc = screenClientRect.top;
			}
			if (yLoc + Rect.bottom > screenClientRect.bottom)
			{
				yLoc -= (yLoc + Rect.bottom) - screenClientRect.bottom;
			}
		}
	}

	// Center and adjust size of window
	if (SetWindowPositionFlag)
	{
		// Use SetWindowPlacement to center and adjust size
		WINDOWPLACEMENT wndpl = {};
		wndpl.length = sizeof(WINDOWPLACEMENT);
		if (GetWindowPlacement(MainhWnd, &wndpl))
		{
			// Force restore first if maximized/fullscreen
			if (wndpl.showCmd == SW_MAXIMIZE || IsZoomed(MainhWnd))
			{
				ShowWindow(MainhWnd, SW_RESTORE);
			}

			wndpl.showCmd = SW_NORMAL;
			wndpl.rcNormalPosition = { xLoc, yLoc, Rect.right + xLoc, Rect.bottom + yLoc };
			Utils::SetWindowPlacementToMonitor(hMonitor, MainhWnd, &wndpl);
		}

		// Use SetWindowPos to center and adjust size
		Utils::SetWindowPosToMonitor(hMonitor, MainhWnd, HWND_TOP, xLoc, yLoc, Rect.right, Rect.bottom, SWP_NOZORDER | SWP_NOACTIVATE);
	}
}
