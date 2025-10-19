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
#include "ddraw\Shaders\GammaPixelShader.h"
#include "GDI\WndProc.h"
#include "Utils\Utils.h"

static inline void my_nop(void)
{
	__asm { nop }
}

#ifdef ENABLE_DEBUGOVERLAY
DebugOverlay DOverlay;
#endif

#define SHARED DeviceDetailsMap[DDKey]

std::unordered_map<UINT, DEVICEDETAILS> DeviceDetailsMap;

HRESULT m_IDirect3DDevice9Ex::QueryInterface(REFIID riid, void** ppvObj)
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

	if (riid == IID_IUnknown || riid == WrapperID || (Config.D3d9to9Ex && riid == IID_IDirect3DDevice9))
	{
		HRESULT hr = ProxyInterface->QueryInterface(WrapperID, ppvObj);

		if (SUCCEEDED(hr))
		{
			*ppvObj = this;

			SHARED.DeviceMap[this] = TRUE;
		}

		return hr;
	}

	// Check for unsupported IIDs
	if (riid == IID{ 0x126D0349, 0x4787, 0x4AA6, {0x8E, 0x1B, 0x40, 0xC1, 0x77, 0xC6, 0x0A, 0x01} } ||
		riid == IID{ 0x694036AC, 0x542A, 0x4A3A, {0x9A, 0x32, 0x53, 0xBC, 0x20, 0x00, 0x2C, 0x1B} })
	{
		LOG_LIMIT(100, __FUNCTION__ << " Warning: disabling unsupported interface: " << riid);

		return E_NOINTERFACE;
	}

	HRESULT hr = ProxyInterface->QueryInterface(riid, ppvObj);

	if (SUCCEEDED(hr))
	{
		if (riid == IID_IDirect3DDevice9 || riid == IID_IDirect3DDevice9Ex)
		{
			*ppvObj = SHARED.ProxyAddressLookupTable9.FindAddress<m_IDirect3DDevice9Ex>(*ppvObj, m_pD3DEx, riid, DDKey);
		}
		else
		{
			D3d9Wrapper::genericQueryInterface(riid, ppvObj, this);
		}
	}

	return hr;
}

ULONG m_IDirect3DDevice9Ex::AddRef()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->AddRef();
}

ULONG m_IDirect3DDevice9Ex::Release()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	ULONG ref = ProxyInterface->Release();

	ULONG UsedRef = GetResourceRefCount();

#ifdef ENABLE_DEBUGOVERLAY
	bool UsingDOverlay = (Config.EnableImgui && DOverlay.IsSetup() && DOverlay.Getd3d9Device() == ProxyInterface);
	UsedRef += (UsingDOverlay ? 1 : 0);
#endif

	// Teardown wrapper resources before destroying device
	if (UsedRef > 0 && ref == UsedRef)
	{
		ProxyInterface->AddRef();

		ReleaseResources(false);

#ifdef ENABLE_DEBUGOVERLAY
		if (UsingDOverlay)
		{
			DOverlay.Shutdown();
		}
#endif

		ref = ProxyInterface->Release();
	}
	else if (ref > 0 && ref < UsedRef)
	{
		Logging::Log() << __FUNCTION__ << " Error: device ref '" << ref << "' is less than used ref '" << UsedRef << "'.";
	}

	if (ref == 0)
	{
		SHARED.DeviceMap[this] = FALSE;

		// Check if any other interfaces are sharing this device details
		BOOL MoreInstances = FALSE;
		for (auto it = SHARED.DeviceMap.begin(); it != SHARED.DeviceMap.end(); ++it)
		{
			if (it->second == TRUE)
			{
				MoreInstances = TRUE;
				break;
			}
		}

		// Remove device details if no devices are using it
		if (!MoreInstances)
		{
			HMONITOR hMonitor = SHARED.hMonitor;
			for (auto it = DeviceDetailsMap.begin(); it != DeviceDetailsMap.end(); ++it)
			{
				if (it->first == DDKey)
				{
					while (SHARED.DeletedStateBlocks.size())
					{
						m_IDirect3DStateBlock9* Interface = SHARED.DeletedStateBlocks.back();
						SHARED.DeletedStateBlocks.RemoveStateBlock(Interface);
						Interface->DeleteMe();
					}
					DeviceDetailsMap.erase(it);
					break;
				}
			}
			// Reset display
			if (Config.FullscreenWindowMode)
			{
				Utils::ResetDisplaySettings(hMonitor);
			}
		}
	}

	return ref;
}

void m_IDirect3DDevice9Ex::ClearVars(D3DPRESENT_PARAMETERS* pPresentationParameters)
{
	UNREFERENCED_PARAMETER(pPresentationParameters);

	// Clear variables
	ZeroMemory(&Caps, sizeof(D3DCAPS9));
	MaxAnisotropy = 0;
	isAnisotropySet = false;
	AnisotropyDisabledFlag = false;
	isClipPlaneSet = false;
	ClipPlaneRenderState = 0;
}

template <typename T>
HRESULT m_IDirect3DDevice9Ex::ResetT(T func, D3DPRESENT_PARAMETERS* pPresentationParameters, bool IsEx, D3DDISPLAYMODEEX* pFullscreenDisplayMode)
{
	if (!pPresentationParameters)
	{
		return D3DERR_INVALIDCALL;
	}

	ReleaseResources(true);

#ifdef ENABLE_DEBUGOVERLAY
	// Teardown debug overlay before reset
	if (Config.EnableImgui)
	{
		DOverlay.Shutdown();
	}
#endif

	ProxyInterface->EndScene();		// Required for some games when using WineD3D

	// Get WndProc before resetting device
	WndProc::DATASTRUCT* WndDataStruct = WndProc::GetWndProctStruct(SHARED.DeviceWindow);
	if (WndDataStruct)
	{
		WndDataStruct->IsCreatingDevice = true;
		WndDataStruct->IsExclusiveMode = !pPresentationParameters->Windowed;
	}

	HRESULT hr;

	// Check fullscreen
	bool ForceFullscreen = false;
	if (m_pD3DEx)
	{
		ForceFullscreen = m_pD3DEx->TestResolution(SHARED.Adapter, pPresentationParameters->BackBufferWidth, pPresentationParameters->BackBufferHeight);
	}

	// Check if display needs to be reset
	bool IsSettingWindowMode = (pPresentationParameters->Windowed != FALSE);
	bool IsWindowModeChanging = (SHARED.IsWindowMode != IsSettingWindowMode);
	bool IsResolutionChanging = (SHARED.BufferWidth != (LONG)pPresentationParameters->BackBufferWidth || SHARED.BufferHeight != (LONG)pPresentationParameters->BackBufferHeight);
	if ((IsWindowModeChanging && IsSettingWindowMode) || (Config.FullscreenWindowMode && IsResolutionChanging))
	{
		Utils::ResetDisplaySettings(SHARED.hMonitor);
	}

	// Setup presentation parameters
	D3DPRESENT_PARAMETERS d3dpp;
	CopyMemory(&d3dpp, pPresentationParameters, sizeof(D3DPRESENT_PARAMETERS));
	m_IDirect3D9Ex::UpdatePresentParameter(&d3dpp, SHARED.DeviceWindow, SHARED, IsEx, ForceFullscreen, IsWindowModeChanging || IsResolutionChanging);

	// Test for Multisample
	if (SHARED.DeviceMultiSampleFlag)
	{
		do {

			// Update Present Parameter for Multisample
			m_IDirect3D9Ex::UpdatePresentParameterForMultisample(&d3dpp, SHARED.DeviceMultiSampleType, SHARED.DeviceMultiSampleQuality);

			// Reset device
			hr = ResetT(func, &d3dpp, pFullscreenDisplayMode);

			// Check if device was reset successfully
			if (SUCCEEDED(hr))
			{
				break;
			}

			// Reset presentation parameters
			CopyMemory(&d3dpp, pPresentationParameters, sizeof(D3DPRESENT_PARAMETERS));
			m_IDirect3D9Ex::UpdatePresentParameter(&d3dpp, SHARED.DeviceWindow, SHARED, IsEx, ForceFullscreen, false);

			// Reset device
			hr = ResetT(func, &d3dpp, pFullscreenDisplayMode);

			if (SUCCEEDED(hr))
			{
				LOG_LIMIT(3, __FUNCTION__ << " Disabling AntiAliasing...");
				SHARED.DeviceMultiSampleFlag = false;
				SHARED.DeviceMultiSampleType = D3DMULTISAMPLE_NONE;
				SHARED.DeviceMultiSampleQuality = 0;
				SHARED.SetSSAA = false;
				SHARED.SetATOC = false;
			}

		} while (false);
	}
	else
	{
		// Reset device
		hr = ResetT(func, &d3dpp, pFullscreenDisplayMode);
	}

	if (SUCCEEDED(hr))
	{
		m_IDirect3D9Ex::GetFinalPresentParameter(&d3dpp, SHARED);

		if (WndDataStruct && WndDataStruct->IsExclusiveMode)
		{
			d3dpp.Windowed = FALSE;
		}

		SHARED.IsWindowMode = IsSettingWindowMode;

		CopyMemory(pPresentationParameters, &d3dpp, sizeof(D3DPRESENT_PARAMETERS));

		ClearVars(pPresentationParameters);

		ReInitInterface();
	}

	if (WndDataStruct)
	{
		WndDataStruct->IsCreatingDevice = false;
	}

	return hr;
}

HRESULT m_IDirect3DDevice9Ex::Reset(D3DPRESENT_PARAMETERS *pPresentationParameters)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!pPresentationParameters)
	{
		return D3DERR_INVALIDCALL;
	}

	if (Config.D3d9to9Ex && ProxyInterfaceEx)
	{
		D3DDISPLAYMODEEX FullscreenDisplayMode = {};

		m_IDirect3D9Ex::GetFullscreenDisplayMode(*pPresentationParameters, FullscreenDisplayMode);

		return ResetEx(pPresentationParameters, &FullscreenDisplayMode);
	}

	return ResetT<fReset>(nullptr, pPresentationParameters, false, nullptr);
}

HRESULT m_IDirect3DDevice9Ex::CallEndScene()
{
	// clear Begin/End Scene flags
	IsInScene = false;

#ifdef ENABLE_DEBUGOVERLAY
	if (Config.EnableImgui && DOverlay.Getd3d9Device() == ProxyInterface)
	{
		DOverlay.EndScene();
	}
#endif

	return ProxyInterface->EndScene();
}

HRESULT m_IDirect3DDevice9Ex::EndScene()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.ForceSingleBeginEndScene)
	{
		if (IsInScene)
		{
			IsInScene = false;

			return D3D_OK;
		}

		return D3DERR_INVALIDCALL;
	}

	return CallEndScene();
}

void m_IDirect3DDevice9Ex::SetCursorPosition(int X, int Y, DWORD Flags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->SetCursorPosition(X, Y, Flags);
}

HRESULT m_IDirect3DDevice9Ex::SetCursorProperties(UINT XHotSpot, UINT YHotSpot, IDirect3DSurface9 *pCursorBitmap)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (pCursorBitmap)
	{
		pCursorBitmap = static_cast<m_IDirect3DSurface9 *>(pCursorBitmap)->GetProxyInterface();
	}

	return ProxyInterface->SetCursorProperties(XHotSpot, YHotSpot, pCursorBitmap);
}

BOOL m_IDirect3DDevice9Ex::ShowCursor(BOOL bShow)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->ShowCursor(bShow);
}

HRESULT m_IDirect3DDevice9Ex::CreateAdditionalSwapChain(D3DPRESENT_PARAMETERS *pPresentationParameters, IDirect3DSwapChain9 **ppSwapChain)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!pPresentationParameters || !ppSwapChain)
	{
		return D3DERR_INVALIDCALL;
	}

	HRESULT hr = D3DERR_INVALIDCALL;

	// Check fullscreen
	bool ForceFullscreen = false;
	if (m_pD3DEx)
	{
		ForceFullscreen = m_pD3DEx->TestResolution(SHARED.Adapter, pPresentationParameters->BackBufferWidth, pPresentationParameters->BackBufferHeight);
	}

	// Setup presentation parameters
	D3DPRESENT_PARAMETERS d3dpp;
	CopyMemory(&d3dpp, pPresentationParameters, sizeof(D3DPRESENT_PARAMETERS));
	m_IDirect3D9Ex::UpdatePresentParameter(&d3dpp, SHARED.DeviceWindow, SHARED, false, ForceFullscreen, false);

	// Test for Multisample
	if (SHARED.DeviceMultiSampleFlag)
	{
		// Update Present Parameter for Multisample
		m_IDirect3D9Ex::UpdatePresentParameterForMultisample(&d3dpp, SHARED.DeviceMultiSampleType, SHARED.DeviceMultiSampleQuality);

		// Create CwapChain
		hr = ProxyInterface->CreateAdditionalSwapChain(&d3dpp, ppSwapChain);
	}
	
	if (FAILED(hr))
	{
		CopyMemory(&d3dpp, pPresentationParameters, sizeof(D3DPRESENT_PARAMETERS));
		m_IDirect3D9Ex::UpdatePresentParameter(&d3dpp, SHARED.DeviceWindow, SHARED, false, ForceFullscreen, false);

		// Create CwapChain
		hr = ProxyInterface->CreateAdditionalSwapChain(&d3dpp, ppSwapChain);

		if (SUCCEEDED(hr) && SHARED.DeviceMultiSampleFlag)
		{
			LOG_LIMIT(3, __FUNCTION__ <<" Disabling AntiAliasing for Swap Chain...");
		}
	}

	if (SUCCEEDED(hr))
	{
		CopyMemory(pPresentationParameters, &d3dpp, sizeof(D3DPRESENT_PARAMETERS));

		IDirect3DSwapChain9* pSwapChainQuery = nullptr;

		if (WrapperID == IID_IDirect3DDevice9Ex && SUCCEEDED((*ppSwapChain)->QueryInterface(IID_IDirect3DSwapChain9Ex, (LPVOID*)&pSwapChainQuery)))
		{
			(*ppSwapChain)->Release();

			*ppSwapChain = SHARED.ProxyAddressLookupTable9.FindCreateAddress<m_IDirect3DSwapChain9Ex, m_IDirect3DDevice9Ex, LPVOID>((IDirect3DSwapChain9Ex*)pSwapChainQuery, this, IID_IDirect3DSwapChain9Ex, nullptr);
		}
		else
		{
			*ppSwapChain = SHARED.ProxyAddressLookupTable9.FindCreateAddress<m_IDirect3DSwapChain9Ex, m_IDirect3DDevice9Ex, LPVOID>((IDirect3DSwapChain9Ex*)*ppSwapChain, this, IID_IDirect3DSwapChain9, nullptr);
		}

		return D3D_OK;
	}

	Logging::LogDebug() << __FUNCTION__ << " FAILED! " << (D3DERR)hr << " " << *pPresentationParameters;
	return hr;
}

HRESULT m_IDirect3DDevice9Ex::CreateCubeTexture(THIS_ UINT EdgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DCubeTexture9** ppCubeTexture, HANDLE* pSharedHandle)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ppCubeTexture)
	{
		return D3DERR_INVALIDCALL;
	}

	if (Config.D3d9to9Ex)
	{
		if (Pool == D3DPOOL_MANAGED)
		{
			Pool = D3DPOOL_DEFAULT;
			Usage |= D3DUSAGE_DYNAMIC;
		}
	}

	// Override stencil format
	if (Config.OverrideStencilFormat && Usage == D3DUSAGE_DEPTHSTENCIL)
	{
		Format = (D3DFORMAT)Config.OverrideStencilFormat;
	}

	HRESULT hr = ProxyInterface->CreateCubeTexture(EdgeLength, Levels, Usage, Format, Pool, ppCubeTexture, pSharedHandle);

	if (SUCCEEDED(hr))
	{
		*ppCubeTexture = SHARED.ProxyAddressLookupTable9.FindCreateAddress<m_IDirect3DCubeTexture9, m_IDirect3DDevice9Ex, LPVOID>(*ppCubeTexture, this, IID_IDirect3DCubeTexture9, nullptr);
		return D3D_OK;
	}

	Logging::LogDebug() << __FUNCTION__ << " FAILED! " << (D3DERR)hr << " " << EdgeLength << " " << Levels << " " << Usage << " " << Format << " " << Pool << " " << pSharedHandle;
	return hr;
}

HRESULT m_IDirect3DDevice9Ex::CreateDepthStencilSurface(THIS_ UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Discard, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ppSurface)
	{
		return D3DERR_INVALIDCALL;
	}

	if (Config.D3d9to9Ex && ProxyInterfaceEx)
	{
		return CreateDepthStencilSurfaceEx(Width, Height, Format, MultiSample, MultisampleQuality, Discard, ppSurface, pSharedHandle, 0);
	}

	// Override stencil format
	if (Config.OverrideStencilFormat)
	{
		Format = (D3DFORMAT)Config.OverrideStencilFormat;
		LOG_LIMIT(100, __FUNCTION__ << " Setting Stencil format: " << Format);
	}

	HRESULT hr = D3DERR_INVALIDCALL;

	// Test for Multisample
	if (SHARED.DeviceMultiSampleFlag)
	{
		hr = ProxyInterface->CreateDepthStencilSurface(Width, Height, Format, SHARED.DeviceMultiSampleType, SHARED.DeviceMultiSampleQuality, TRUE, ppSurface, pSharedHandle);
	}

	if (FAILED(hr))
	{
		hr = ProxyInterface->CreateDepthStencilSurface(Width, Height, Format, MultiSample, MultisampleQuality, Discard, ppSurface, pSharedHandle);

		if (SUCCEEDED(hr) && SHARED.DeviceMultiSampleFlag)
		{
			LOG_LIMIT(3, __FUNCTION__ << " Disabling AntiAliasing for Depth Stencil...");
		}
	}

	if (SUCCEEDED(hr))
	{
		*ppSurface = SHARED.ProxyAddressLookupTable9.FindCreateAddress<m_IDirect3DSurface9, m_IDirect3DDevice9Ex, LPVOID>(*ppSurface, this, IID_IDirect3DSurface9, nullptr);
		return D3D_OK;
	}

	Logging::LogDebug() << __FUNCTION__ << " FAILED! " << (D3DERR)hr << " " << Width << " " << Height << " " << Format << " " << MultiSample << " " << MultisampleQuality << " " << Discard << " " << pSharedHandle;
	return hr;
}

HRESULT m_IDirect3DDevice9Ex::CreateIndexBuffer(THIS_ UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DIndexBuffer9** ppIndexBuffer, HANDLE* pSharedHandle)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ppIndexBuffer)
	{
		return D3DERR_INVALIDCALL;
	}

	if (Config.D3d9to9Ex)
	{
		if (Pool == D3DPOOL_MANAGED)
		{
			Pool = D3DPOOL_DEFAULT;
			Usage |= D3DUSAGE_DYNAMIC;
		}
	}

	HRESULT hr = ProxyInterface->CreateIndexBuffer(Length, Usage, Format, Pool, ppIndexBuffer, pSharedHandle);

	if (SUCCEEDED(hr))
	{
		*ppIndexBuffer = SHARED.ProxyAddressLookupTable9.FindCreateAddress<m_IDirect3DIndexBuffer9, m_IDirect3DDevice9Ex, LPVOID>(*ppIndexBuffer, this, IID_IDirect3DIndexBuffer9, nullptr);
		return D3D_OK;
	}

	Logging::LogDebug() << __FUNCTION__ << " FAILED! " << (D3DERR)hr << " " << Length << " " << Usage << " " << Format << " " << Pool << " " << pSharedHandle;
	return hr;
}

HRESULT m_IDirect3DDevice9Ex::CreateRenderTarget(THIS_ UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ppSurface)
	{
		return D3DERR_INVALIDCALL;
	}

	if (Config.D3d9to9Ex && ProxyInterfaceEx)
	{
		return CreateRenderTargetEx(Width, Height, Format, MultiSample, MultisampleQuality, Lockable, ppSurface, pSharedHandle, 0);
	}

	HRESULT hr = D3DERR_INVALIDCALL;

	// Test for Multisample
	if (SHARED.DeviceMultiSampleFlag)
	{
		hr = ProxyInterface->CreateRenderTarget(Width, Height, Format, SHARED.DeviceMultiSampleType, SHARED.DeviceMultiSampleQuality, FALSE, ppSurface, pSharedHandle);
	}

	if (FAILED(hr))
	{
		hr = ProxyInterface->CreateRenderTarget(Width, Height, Format, MultiSample, MultisampleQuality, Lockable, ppSurface, pSharedHandle);

		if (SUCCEEDED(hr) && SHARED.DeviceMultiSampleFlag)
		{
			LOG_LIMIT(3, __FUNCTION__ << " Disabling AntiAliasing for Render Target...");
		}
	}

	if (SUCCEEDED(hr))
	{
		*ppSurface = SHARED.ProxyAddressLookupTable9.FindCreateAddress<m_IDirect3DSurface9, m_IDirect3DDevice9Ex, LPVOID>(*ppSurface, this, IID_IDirect3DSurface9, nullptr);
		return D3D_OK;
	}

	Logging::LogDebug() << __FUNCTION__ << " FAILED! " << (D3DERR)hr << " " << Width << " " << Height << " " << Format << " " << MultiSample << " " << MultisampleQuality << " " << Lockable << " " << pSharedHandle;
	return hr;
}

HRESULT m_IDirect3DDevice9Ex::CreateTexture(THIS_ UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture9** ppTexture, HANDLE* pSharedHandle)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ppTexture)
	{
		return D3DERR_INVALIDCALL;
	}

	if (Config.D3d9to9Ex)
	{
		if (Pool == D3DPOOL_MANAGED)
		{
			Pool = D3DPOOL_DEFAULT;
			Usage |= D3DUSAGE_DYNAMIC;
		}
	}

	// Override stencil format
	if (Config.OverrideStencilFormat && Usage == D3DUSAGE_DEPTHSTENCIL)
	{
		Format = (D3DFORMAT)Config.OverrideStencilFormat;
	}

	HRESULT hr = ProxyInterface->CreateTexture(Width, Height, Levels, Usage, Format, Pool, ppTexture, pSharedHandle);

	if (SUCCEEDED(hr))
	{
		*ppTexture = SHARED.ProxyAddressLookupTable9.FindCreateAddress<m_IDirect3DTexture9, m_IDirect3DDevice9Ex, LPVOID>(*ppTexture, this, IID_IDirect3DTexture9, nullptr);
		return D3D_OK;
	}

	Logging::LogDebug() << __FUNCTION__ << " FAILED! " << (D3DERR)hr << " " << Width << " " << Height << " " << Levels << " " << Usage << " " << Format << " " << Pool << " " << pSharedHandle;
	return hr;
}

HRESULT m_IDirect3DDevice9Ex::CreateVertexBuffer(THIS_ UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool, IDirect3DVertexBuffer9** ppVertexBuffer, HANDLE* pSharedHandle)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ppVertexBuffer)
	{
		return D3DERR_INVALIDCALL;
	}

	if (Config.ForceSystemMemVertexCache && Pool == D3DPOOL_MANAGED)
	{
		Pool = D3DPOOL_SYSTEMMEM;
		Usage = D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY;
	}

	if (Config.D3d9to9Ex)
	{
		if (Pool == D3DPOOL_MANAGED)
		{
			Pool = D3DPOOL_DEFAULT;
			Usage |= D3DUSAGE_DYNAMIC;
		}
	}

	HRESULT hr = ProxyInterface->CreateVertexBuffer(Length, Usage, FVF, Pool, ppVertexBuffer, pSharedHandle);

	if (SUCCEEDED(hr))
	{
		*ppVertexBuffer = SHARED.ProxyAddressLookupTable9.FindCreateAddress<m_IDirect3DVertexBuffer9, m_IDirect3DDevice9Ex, LPVOID>(*ppVertexBuffer, this, IID_IDirect3DVertexBuffer9, nullptr);
		return D3D_OK;
	}

	Logging::LogDebug() << __FUNCTION__ << " FAILED! " << (D3DERR)hr << " " << Length << " " << Usage << " " << FVF << " " << Pool << " " << pSharedHandle;
	return hr;
}

HRESULT m_IDirect3DDevice9Ex::CreateVolumeTexture(THIS_ UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DVolumeTexture9** ppVolumeTexture, HANDLE* pSharedHandle)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ppVolumeTexture)
	{
		return D3DERR_INVALIDCALL;
	}

	if (Config.D3d9to9Ex)
	{
		if (Pool == D3DPOOL_MANAGED)
		{
			Pool = D3DPOOL_DEFAULT;
			Usage |= D3DUSAGE_DYNAMIC;
		}
	}

	HRESULT hr = ProxyInterface->CreateVolumeTexture(Width, Height, Depth, Levels, Usage, Format, Pool, ppVolumeTexture, pSharedHandle);

	if (SUCCEEDED(hr))
	{
		*ppVolumeTexture = SHARED.ProxyAddressLookupTable9.FindCreateAddress<m_IDirect3DVolumeTexture9, m_IDirect3DDevice9Ex, LPVOID>(*ppVolumeTexture, this, IID_IDirect3DVolumeTexture9, nullptr);
		return D3D_OK;
	}

	Logging::LogDebug() << __FUNCTION__ << " FAILED! " << (D3DERR)hr << " " << Width << " " << Height << " " << Depth << " " << Levels << " " << Usage << " " << Format << " " << Pool << " " << pSharedHandle;
	return hr;
}

m_IDirect3DStateBlock9* m_IDirect3DDevice9Ex::GetCreateStateBlock(IDirect3DStateBlock9* pSB)
{
	m_IDirect3DStateBlock9* StateBlockX = nullptr;

	if (SHARED.DeletedStateBlocks.size())
	{
		StateBlockX = SHARED.DeletedStateBlocks.back();
		SHARED.DeletedStateBlocks.RemoveStateBlock(StateBlockX);

		StateBlockX->SetProxyAddress(pSB);
		StateBlockX->InitInterface(this, IID_IDirect3DStateBlock9, nullptr);
	}
	else
	{
		StateBlockX = SHARED.ProxyAddressLookupTable9.FindCreateAddress<m_IDirect3DStateBlock9, m_IDirect3DDevice9Ex, LPVOID>(pSB, this, IID_IDirect3DStateBlock9, nullptr);
	}

	return StateBlockX;
}

HRESULT m_IDirect3DDevice9Ex::BeginStateBlock()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->BeginStateBlock();
}

HRESULT m_IDirect3DDevice9Ex::CreateStateBlock(THIS_ D3DSTATEBLOCKTYPE Type, IDirect3DStateBlock9** ppSB)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ppSB)
	{
		return D3DERR_INVALIDCALL;
	}

	HRESULT hr = ProxyInterface->CreateStateBlock(Type, ppSB);

	if (SUCCEEDED(hr))
	{
		m_IDirect3DStateBlock9* StateBlockX = GetCreateStateBlock(*ppSB);

		if (Config.LimitStateBlocks)
		{
			SHARED.StateBlockTable.AddStateBlock(StateBlockX);
		}

		*ppSB = StateBlockX;

		return D3D_OK;
	}

	Logging::LogDebug() << __FUNCTION__ << " FAILED! " << (D3DERR)hr << " " << Type;

	return hr;
}

HRESULT m_IDirect3DDevice9Ex::EndStateBlock(THIS_ IDirect3DStateBlock9** ppSB)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ppSB)
	{
		return D3DERR_INVALIDCALL;
	}

	HRESULT hr = ProxyInterface->EndStateBlock(ppSB);

	if (SUCCEEDED(hr))
	{
		m_IDirect3DStateBlock9* StateBlockX = GetCreateStateBlock(*ppSB);

		if (Config.LimitStateBlocks)
		{
			SHARED.StateBlockTable.AddStateBlock(StateBlockX);
		}

		*ppSB = StateBlockX;
	}

	return hr;
}

HRESULT m_IDirect3DDevice9Ex::GetClipStatus(D3DCLIPSTATUS9 *pClipStatus)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->GetClipStatus(pClipStatus);
}

HRESULT m_IDirect3DDevice9Ex::GetDisplayMode(THIS_ UINT iSwapChain, D3DDISPLAYMODE* pMode)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.D3d9to9Ex && ProxyInterfaceEx)
	{
		if (!pMode)
		{
			return D3DERR_INVALIDCALL;
		}

		D3DDISPLAYMODEEX ModeEx = {};
		ModeEx.Size = sizeof(D3DDISPLAYMODEEX);
		D3DDISPLAYROTATION Rotation = D3DDISPLAYROTATION_IDENTITY;

		HRESULT hr = GetDisplayModeEx(iSwapChain, &ModeEx, &Rotation);

		if (SUCCEEDED(hr))
		{
			ModeExToMode(ModeEx, *pMode);
		}

		return hr;
	}

	return ProxyInterface->GetDisplayMode(iSwapChain, pMode);
}

HRESULT m_IDirect3DDevice9Ex::GetRenderState(D3DRENDERSTATETYPE State, DWORD *pValue)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->GetRenderState(State, pValue);
}

HRESULT m_IDirect3DDevice9Ex::GetRenderTarget(THIS_ DWORD RenderTargetIndex, IDirect3DSurface9** ppRenderTarget)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ppRenderTarget)
	{
		return D3DERR_INVALIDCALL;
	}

	ScopedCriticalSection ThreadLock(&SHARED.d9cs, RequirePresentHandling());

	HRESULT hr = ProxyInterface->GetRenderTarget(RenderTargetIndex, ppRenderTarget);

	if (ShadowBackbuffer.Count())
	{
		auto it = std::find(BackBufferList.begin(), BackBufferList.end(), *ppRenderTarget);
		if (it != BackBufferList.end())
		{
			Logging::Log() << __FUNCTION__ << " Warning: GetRenderTarget is returning the real render target!";

			(*ppRenderTarget)->Release();

			*ppRenderTarget = ShadowBackbuffer.GetCurrentBackBuffer();
			if (!*ppRenderTarget)
			{
				return D3DERR_INVALIDCALL;
			}
		}
	}

	if (SUCCEEDED(hr) && ppRenderTarget)
	{
		*ppRenderTarget = SHARED.ProxyAddressLookupTable9.FindAddress<m_IDirect3DSurface9, m_IDirect3DDevice9Ex, LPVOID>(*ppRenderTarget, this, IID_IDirect3DSurface9, nullptr);
	}

	return hr;
}

HRESULT m_IDirect3DDevice9Ex::GetTransform(D3DTRANSFORMSTATETYPE State, D3DMATRIX *pMatrix)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->GetTransform(State, pMatrix);
}

HRESULT m_IDirect3DDevice9Ex::SetClipStatus(CONST D3DCLIPSTATUS9 *pClipStatus)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->SetClipStatus(pClipStatus);
}

HRESULT m_IDirect3DDevice9Ex::SetRenderState(D3DRENDERSTATETYPE State, DWORD Value)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	// Set for Multisample
	if (SHARED.DeviceMultiSampleFlag && State == D3DRS_MULTISAMPLEANTIALIAS)
	{
		Value = TRUE;
	}
	if (SHARED.SetATOC && State == D3DRS_ALPHATESTENABLE && Config.EnableMultisamplingATOC == 2)
	{
		Value = TRUE;
	}

	HRESULT hr = ProxyInterface->SetRenderState(State, Value);

	// CacheClipPlane
	if (SUCCEEDED(hr) && State == D3DRS_CLIPPLANEENABLE)
	{
		ClipPlaneRenderState = Value;
	}

	return hr;
}

HRESULT m_IDirect3DDevice9Ex::SetRenderTarget(THIS_ DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	ScopedCriticalSection ThreadLock(&SHARED.d9cs, RequirePresentHandling());

	if (pRenderTarget)
	{
		pRenderTarget = static_cast<m_IDirect3DSurface9 *>(pRenderTarget)->GetProxyInterface();
	}

	if (ShadowBackbuffer.Count())
	{
		if (std::find(BackBufferList.begin(), BackBufferList.end(), pRenderTarget) != BackBufferList.end())
		{
			Logging::Log() << __FUNCTION__ << " Warning: application is sending the real render target!";
		}
	}

	return ProxyInterface->SetRenderTarget(RenderTargetIndex, pRenderTarget);
}

HRESULT m_IDirect3DDevice9Ex::SetTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX *pMatrix)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ") State: " << State;

	HRESULT hr = ProxyInterface->SetTransform(State, pMatrix);

	if (SUCCEEDED(hr))
	{
		// Check if this is a texture stage transform
		if (Config.EnvironmentCubeMapFix)
		{
			CheckTransformForCubeMap(State, pMatrix);
		}
	}

	return hr;
}

HRESULT m_IDirect3DDevice9Ex::SetBrightnessLevel(D3DGAMMARAMP& Ramp)
{
	Logging::LogDebug() << __FUNCTION__;

	// Create or update the gamma LUT texture
	if (!GammaLUTTexture)
	{
		DWORD Usage = (Config.D3d9to9Ex ? D3DUSAGE_DYNAMIC : 0);
		D3DPOOL Pool = (Config.D3d9to9Ex ? D3DPOOL_DEFAULT : D3DPOOL_MANAGED);

		if (SUCCEEDED(ProxyInterface->CreateTexture(256, 1, 1, Usage, D3DFMT_A32B32G32R32F, Pool, &GammaLUTTexture, nullptr)))
		{
			UsingShader32f = true;
		}
		else
		{
			UsingShader32f = false;
			HRESULT hr = ProxyInterface->CreateTexture(256, 1, 1, Usage, D3DFMT_A8R8G8B8, Pool, &GammaLUTTexture, nullptr);
			if (FAILED(hr))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: Failed to create gamma LUD texture!");
				return hr;
			}
		}
	}

	D3DLOCKED_RECT lockedRect;
	if (SUCCEEDED(GammaLUTTexture->LockRect(0, &lockedRect, nullptr, D3DLOCK_DISCARD)))
	{
		if (UsingShader32f)
		{
			float* texData = static_cast<float*>(lockedRect.pBits);
			for (int i = 0; i < 256; ++i)
			{
				texData[i * 4 + 0] = Ramp.red[i] / 65535.0f;
				texData[i * 4 + 1] = Ramp.green[i] / 65535.0f;
				texData[i * 4 + 2] = Ramp.blue[i] / 65535.0f;
				texData[i * 4 + 3] = 1.0f;
			}
		}
		else
		{
			DWORD* texData = static_cast<DWORD*>(lockedRect.pBits);
			for (int i = 0; i < 256; ++i)
			{
				BYTE r = static_cast<BYTE>(Ramp.red[i] / 256);
				BYTE g = static_cast<BYTE>(Ramp.green[i] / 256);
				BYTE b = static_cast<BYTE>(Ramp.blue[i] / 256);

				texData[i] = D3DCOLOR_ARGB(255, r, g, b);
			}
		}
		GammaLUTTexture->UnlockRect(0);
	}

	return D3D_OK;
}

LPDIRECT3DPIXELSHADER9 m_IDirect3DDevice9Ex::GetGammaPixelShader()
{
	// Create pixel shaders
	if (!gammaPixelShader)
	{
		ProxyInterface->CreatePixelShader((DWORD*)GammaPixelShaderSrc, &gammaPixelShader);
	}
	return gammaPixelShader;
}

void m_IDirect3DDevice9Ex::ApplyBrightnessLevel()
{
	if (IsGammaSet && !GammaLUTTexture)
	{
		SetBrightnessLevel(RampData);
	}
	bool UsingGamma = IsGammaSet && GammaLUTTexture;

	// Set shader
	IDirect3DPixelShader9* pShader = UsingGamma ? GetGammaPixelShader() : nullptr;
	if (UsingGamma && !pShader)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Failed to retrieve gamma pixel shader!");
		return;
	}

	// Get current backbuffer (make sure to get it from wrapper not proxy)
	IDirect3DSurface9* pBackBuffer = nullptr;
	{
		ComPtr<IDirect3DSurface9> tmpBackBuffer;
		if (FAILED(GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, tmpBackBuffer.GetAddressOf())))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Failed to get back buffer!");
			return;
		}
		pBackBuffer = tmpBackBuffer.Get();

		void* pVoid = nullptr;
		if (SUCCEEDED(pBackBuffer->QueryInterface(IID_GetInterfaceX, &pVoid)))
		{
			pBackBuffer = static_cast<m_IDirect3DSurface9*>(pBackBuffer)->GetProxyInterface();
		}
	}

	// Create intermediate texture for shader input
	D3DSURFACE_DESC desc;
	pBackBuffer->GetDesc(&desc);
	if (!ScreenCopyTexture)
	{
		if (FAILED(ProxyInterface->CreateTexture(desc.Width, desc.Height, 1, D3DUSAGE_RENDERTARGET, desc.Format, D3DPOOL_DEFAULT, &ScreenCopyTexture, nullptr)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Failed to create screen copy texture!");
			return;
		}
	}

	ComPtr<IDirect3DSurface9> pCopySurface;
	if (FAILED(ScreenCopyTexture->GetSurfaceLevel(0, pCopySurface.GetAddressOf())))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Failed to get surface level from screen copy texture!");
		return;
	}
	if (FAILED(ProxyInterface->StretchRect(pBackBuffer, nullptr, pCopySurface.Get(), nullptr, D3DTEXF_NONE)))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Failed to copy render target!");
		return;
	}

	// Set render states
	ProxyInterface->SetRenderState(D3DRS_LIGHTING, FALSE);
	ProxyInterface->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	ProxyInterface->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	ProxyInterface->SetRenderState(D3DRS_FOGENABLE, FALSE);
	ProxyInterface->SetRenderState(D3DRS_ZENABLE, FALSE);
	ProxyInterface->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	ProxyInterface->SetRenderState(D3DRS_STENCILENABLE, FALSE);
	ProxyInterface->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	ProxyInterface->SetRenderState(D3DRS_CLIPPING, FALSE);
	ProxyInterface->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);

	// Set texture states
	ProxyInterface->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	ProxyInterface->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	ProxyInterface->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CURRENT);
	ProxyInterface->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	// Set sampler states
	for (UINT x = 0; x < 2; x++)
	{
		ProxyInterface->SetSamplerState(x, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
		ProxyInterface->SetSamplerState(x, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
	}

	// Set viewport
	D3DVIEWPORT9 Viewport = { 0, 0, static_cast<DWORD>(SHARED.BufferWidth), static_cast<DWORD>(SHARED.BufferHeight), 0.0f, 1.0f };
	ProxyInterface->SetViewport(&Viewport);

	// Set trasform
	D3DMATRIX identityMatrix = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};
	ProxyInterface->SetTransform(D3DTS_WORLD, &identityMatrix);
	ProxyInterface->SetTransform(D3DTS_VIEW, &identityMatrix);
	ProxyInterface->SetTransform(D3DTS_PROJECTION, &identityMatrix);

	// Clear render target
	ProxyInterface->Clear(0, nullptr, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

	// Set texture
	ProxyInterface->SetTexture(0, ScreenCopyTexture);
	ProxyInterface->SetTexture(1, GammaLUTTexture);

	// Clear textures
	for (int x = 2; x < MAX_TEXTURE_STAGES; x++)
	{
		ProxyInterface->SetTexture(x, nullptr);
	}

	// Set shader
	ProxyInterface->SetPixelShader(pShader);

	const DWORD TLVERTEXFVF = (D3DFVF_XYZRHW | D3DFVF_TEX1);
	struct TLVERTEX
	{
		float x, y, z, rhw;
		float u, v;
	};

	// Define fullscreen quad vertices
	TLVERTEX FullScreenQuadVertices[4] = {
		{ -0.5f,  -0.5f, 0.0f, 1.0f, 0.0f, 0.0f },                          // Top-left
		{ desc.Width - 0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 0.0f },               // Top-right
		{ -0.5f,  desc.Height - 0.5f, 0.0f, 1.0f, 0.0f, 1.0f },             // Bottom-left
		{ desc.Width - 0.5f, desc.Height - 0.5f, 0.0f, 1.0f, 1.0f, 1.0f }   // Bottom-right
	};

	// Set FVF and render
	ProxyInterface->SetFVF(TLVERTEXFVF);
	if (FAILED(ProxyInterface->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, FullScreenQuadVertices, sizeof(TLVERTEX))))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Failed to draw primitive!");
	}

	// Clear shader
	ProxyInterface->SetPixelShader(nullptr);

	// Clear texture
	ProxyInterface->SetTexture(0, nullptr);
	ProxyInterface->SetTexture(1, nullptr);
}

DWORD m_IDirect3DDevice9Ex::GetResourceRefCount()
{
	return
		(GammaLUTTexture ? 1 : 0) +
		(ScreenCopyTexture ? 1 : 0) +
		(gammaPixelShader ? 1 : 0) +
		(BlankTexture ? 1 : 0) +
		(pFont ? 1 : 0) +
		(pSprite ? 2 : 0) +
		(pStateBlock ? 1 : 0) +
		ShadowBackbuffer.Count();
}

void m_IDirect3DDevice9Ex::ReleaseResources(bool isReset)
{
	ScopedCriticalSection ThreadLock(&SHARED.d9cs, RequirePresentHandling());

	if (GammaLUTTexture)
	{
		ULONG ref = GammaLUTTexture->Release();
		if (ref)
		{
			Logging::Log() << __FUNCTION__ << " Error: there is still a reference to 'GammaLUTTexture' " << ref;
		}
		GammaLUTTexture = nullptr;
	}

	if (ScreenCopyTexture)
	{
		ULONG ref = ScreenCopyTexture->Release();
		if (ref)
		{
			Logging::Log() << __FUNCTION__ << " Error: there is still a reference to 'ScreenCopyTexture' " << ref;
		}
		ScreenCopyTexture = nullptr;
	}

	if (gammaPixelShader)
	{
		ULONG ref = gammaPixelShader->Release();
		if (ref)
		{
			Logging::Log() << __FUNCTION__ << " Error: there is still a reference to 'gammaPixelShader' " << ref;
		}
		gammaPixelShader = nullptr;
	}

	if (BlankTexture)
	{
		if (isBlankTextureUsed)
		{
			isBlankTextureUsed = false;
			ProxyInterface->SetTexture(0, nullptr);
		}
		ULONG ref = BlankTexture->Release();
		if (ref)
		{
			Logging::Log() << __FUNCTION__ << " Error: there is still a reference to 'BlankTexture' " << ref;
		}
		BlankTexture = nullptr;
	}

	if (pFont)
	{
		ULONG ref = pFont->Release();
		if (ref)
		{
			Logging::Log() << __FUNCTION__ << " Error: there is still a reference to 'pFont' " << ref;
		}
		pFont = nullptr;
	}

	if (pSprite)
	{
		ULONG ref = pSprite->Release();
		if (ref)
		{
			Logging::Log() << __FUNCTION__ << " Error: there is still a reference to 'pSprite' " << ref;
		}
		pSprite = nullptr;
	}

	if (pStateBlock)
	{
		ULONG ref = pStateBlock->Release();
		if (ref)
		{
			Logging::Log() << __FUNCTION__ << " Error: there is still a reference to 'pStateBlock' " << ref;
		}
		pStateBlock = nullptr;
	}

	// Release shadow backbuffer
	ReleaseShadowBackbuffer();

	if (isReset)
	{
		// Anisotropic Filtering
		isAnisotropySet = false;
		AnisotropyDisabledFlag = false;

		// clear Begin/End Scene flags
		IsInScene = false;
		BeginSceneCalled = false;

		// For environment map cube
		std::fill(std::begin(isTextureCubeMap), std::end(isTextureCubeMap), false);
		std::fill(std::begin(isTransformCubeMap), std::end(isTransformCubeMap), false);
		std::fill(std::begin(texCoordIndex), std::end(texCoordIndex), 0);
		std::fill(std::begin(texTransformFlags), std::end(texTransformFlags), 0);
		isBlankTextureUsed = false;
		pCurrentTexture = nullptr;

		// For CacheClipPlane
		isClipPlaneSet = false;
		ClipPlaneRenderState = 0;
		for (int i = 0; i < MAX_CLIP_PLANES; ++i)
		{
			std::fill(std::begin(StoredClipPlanes[i]), std::end(StoredClipPlanes[i]), 0.0f);
		}

		// For gamma
		IsGammaSet = false;
		UsingShader32f = true;
	}
}

void m_IDirect3DDevice9Ex::GetGammaRamp(THIS_ UINT iSwapChain, D3DGAMMARAMP* pRamp)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (pRamp && (Config.WindowModeGammaShader == 2 || (Config.WindowModeGammaShader && Config.EnableWindowMode)))
	{
		if (iSwapChain)
		{
			LOG_LIMIT(3, __FUNCTION__ << " Warning: Gamma support for swapchains not implemented: " << iSwapChain);
			return ProxyInterface->GetGammaRamp(iSwapChain, pRamp);
		}

		memcpy(pRamp, &RampData, sizeof(D3DGAMMARAMP));
		return;
	}

	return ProxyInterface->GetGammaRamp(iSwapChain, pRamp);
}

void m_IDirect3DDevice9Ex::SetGammaRamp(THIS_ UINT iSwapChain, DWORD Flags, CONST D3DGAMMARAMP* pRamp)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (pRamp && (Config.WindowModeGammaShader == 2 || (Config.WindowModeGammaShader && Config.EnableWindowMode)))
	{
		if (iSwapChain)
		{
			LOG_LIMIT(3, __FUNCTION__ << " Warning: Gamma support for swapchains not implemented: " << iSwapChain);
			return ProxyInterface->SetGammaRamp(iSwapChain, Flags, pRamp);
		}

		IsGammaSet = false;
		memcpy(&RampData, pRamp, sizeof(D3DGAMMARAMP));

		if (memcmp(&DefaultRampData, &RampData, sizeof(D3DGAMMARAMP)) != S_OK)
		{
			IsGammaSet = true;
			SetBrightnessLevel(RampData);
		}

		return;
	}

	return ProxyInterface->SetGammaRamp(iSwapChain, Flags, pRamp);
}

HRESULT m_IDirect3DDevice9Ex::DeletePatch(UINT Handle)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->DeletePatch(Handle);
}

HRESULT m_IDirect3DDevice9Ex::DrawRectPatch(UINT Handle, CONST float *pNumSegs, CONST D3DRECTPATCH_INFO *pRectPatchInfo)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->DrawRectPatch(Handle, pNumSegs, pRectPatchInfo);
}

HRESULT m_IDirect3DDevice9Ex::DrawTriPatch(UINT Handle, CONST float *pNumSegs, CONST D3DTRIPATCH_INFO *pTriPatchInfo)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->DrawTriPatch(Handle, pNumSegs, pTriPatchInfo);
}

HRESULT m_IDirect3DDevice9Ex::GetIndices(THIS_ IDirect3DIndexBuffer9** ppIndexData)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	HRESULT hr = ProxyInterface->GetIndices(ppIndexData);

	if (SUCCEEDED(hr) && ppIndexData)
	{
		*ppIndexData = SHARED.ProxyAddressLookupTable9.FindAddress<m_IDirect3DIndexBuffer9, m_IDirect3DDevice9Ex, LPVOID>(*ppIndexData, this, IID_IDirect3DIndexBuffer9, nullptr);
	}

	return hr;
}

HRESULT m_IDirect3DDevice9Ex::SetIndices(THIS_ IDirect3DIndexBuffer9* pIndexData)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (pIndexData)
	{
		pIndexData = static_cast<m_IDirect3DIndexBuffer9 *>(pIndexData)->GetProxyInterface();
	}

	return ProxyInterface->SetIndices(pIndexData);
}

UINT m_IDirect3DDevice9Ex::GetAvailableTextureMem()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->GetAvailableTextureMem();
}

HRESULT m_IDirect3DDevice9Ex::GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS *pParameters)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->GetCreationParameters(pParameters);
}

HRESULT m_IDirect3DDevice9Ex::GetDeviceCaps(D3DCAPS9 *pCaps)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->GetDeviceCaps(pCaps);
}

HRESULT m_IDirect3DDevice9Ex::GetDirect3D(IDirect3D9 **ppD3D9)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ppD3D9)
	{
		return D3DERR_INVALIDCALL;
	}

	return m_pD3DEx->QueryInterface(WrapperID == IID_IDirect3DDevice9Ex ? IID_IDirect3D9Ex : IID_IDirect3D9, (LPVOID*)ppD3D9);
}

HRESULT m_IDirect3DDevice9Ex::GetRasterStatus(THIS_ UINT iSwapChain, D3DRASTER_STATUS* pRasterStatus)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->GetRasterStatus(iSwapChain, pRasterStatus);
}

HRESULT m_IDirect3DDevice9Ex::GetLight(DWORD Index, D3DLIGHT9 *pLight)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->GetLight(Index, pLight);
}

HRESULT m_IDirect3DDevice9Ex::GetLightEnable(DWORD Index, BOOL *pEnable)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->GetLightEnable(Index, pEnable);
}

HRESULT m_IDirect3DDevice9Ex::GetMaterial(D3DMATERIAL9 *pMaterial)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->GetMaterial(pMaterial);
}

HRESULT m_IDirect3DDevice9Ex::LightEnable(DWORD LightIndex, BOOL bEnable)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->LightEnable(LightIndex, bEnable);
}

HRESULT m_IDirect3DDevice9Ex::SetLight(DWORD Index, CONST D3DLIGHT9 *pLight)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->SetLight(Index, pLight);
}

HRESULT m_IDirect3DDevice9Ex::SetMaterial(CONST D3DMATERIAL9 *pMaterial)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->SetMaterial(pMaterial);
}

HRESULT m_IDirect3DDevice9Ex::MultiplyTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX *pMatrix)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->MultiplyTransform(State, pMatrix);
}

HRESULT m_IDirect3DDevice9Ex::ProcessVertices(THIS_ UINT SrcStartIndex, UINT DestIndex, UINT VertexCount, IDirect3DVertexBuffer9* pDestBuffer, IDirect3DVertexDeclaration9* pVertexDecl, DWORD Flags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (pDestBuffer)
	{
		pDestBuffer = static_cast<m_IDirect3DVertexBuffer9 *>(pDestBuffer)->GetProxyInterface();
	}

	if (pVertexDecl)
	{
		pVertexDecl = static_cast<m_IDirect3DVertexDeclaration9 *>(pVertexDecl)->GetProxyInterface();
	}

	return ProxyInterface->ProcessVertices(SrcStartIndex, DestIndex, VertexCount, pDestBuffer, pVertexDecl, Flags);
}

HRESULT m_IDirect3DDevice9Ex::TestCooperativeLevel()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->TestCooperativeLevel();
}

HRESULT m_IDirect3DDevice9Ex::GetCurrentTexturePalette(UINT *pPaletteNumber)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->GetCurrentTexturePalette(pPaletteNumber);
}

HRESULT m_IDirect3DDevice9Ex::GetPaletteEntries(UINT PaletteNumber, PALETTEENTRY *pEntries)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->GetPaletteEntries(PaletteNumber, pEntries);
}

HRESULT m_IDirect3DDevice9Ex::SetCurrentTexturePalette(UINT PaletteNumber)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->SetCurrentTexturePalette(PaletteNumber);
}

HRESULT m_IDirect3DDevice9Ex::SetPaletteEntries(UINT PaletteNumber, CONST PALETTEENTRY *pEntries)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->SetPaletteEntries(PaletteNumber, pEntries);
}

HRESULT m_IDirect3DDevice9Ex::CreatePixelShader(THIS_ CONST DWORD* pFunction, IDirect3DPixelShader9** ppShader)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ppShader)
	{
		return D3DERR_INVALIDCALL;
	}

	HRESULT hr = ProxyInterface->CreatePixelShader(pFunction, ppShader);

	if (SUCCEEDED(hr))
	{
		*ppShader = SHARED.ProxyAddressLookupTable9.FindCreateAddress<m_IDirect3DPixelShader9, m_IDirect3DDevice9Ex, LPVOID>(*ppShader, this, IID_IDirect3DPixelShader9, nullptr);
		return D3D_OK;
	}

	Logging::LogDebug() << __FUNCTION__ << " FAILED! " << (D3DERR)hr << " " << pFunction;
	return hr;
}

HRESULT m_IDirect3DDevice9Ex::GetPixelShader(THIS_ IDirect3DPixelShader9** ppShader)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	HRESULT hr = ProxyInterface->GetPixelShader(ppShader);

	if (SUCCEEDED(hr) && ppShader)
	{
		*ppShader = SHARED.ProxyAddressLookupTable9.FindAddress<m_IDirect3DPixelShader9, m_IDirect3DDevice9Ex, LPVOID>(*ppShader, this, IID_IDirect3DPixelShader9, nullptr);
	}

	return hr;
}

HRESULT m_IDirect3DDevice9Ex::SetPixelShader(THIS_ IDirect3DPixelShader9* pShader)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (pShader)
	{
		pShader = static_cast<m_IDirect3DPixelShader9 *>(pShader)->GetProxyInterface();
	}

	return ProxyInterface->SetPixelShader(pShader);
}

void m_IDirect3DDevice9Ex::ApplyPrePresentFixes()
{
	bool CalledBeginScene = false;

	if (RequirePresentHandling())
	{
		ScopedCriticalSection ThreadLock(&SHARED.d9cs);

		// Create state block
		if (pStateBlock || SUCCEEDED(ProxyInterface->CreateStateBlock(D3DSBT_ALL, &pStateBlock)))
		{
			// Begin scene
			if (!Config.ForceSingleBeginEndScene || !BeginSceneCalled)
			{
				CalledBeginScene = true;
				CallBeginScene();
			}

			// Capture modified state
			pStateBlock->Capture();

			{
				// Backup depth stencil
				ComPtr<IDirect3DSurface9> pOldDepthStencil;
				ProxyInterface->GetDepthStencilSurface(pOldDepthStencil.GetAddressOf());

				// Set back buffer as render target
				ComPtr<IDirect3DSurface9> pOldRenderTarget, pBackBuffer;
				if (SUCCEEDED(ProxyInterface->GetRenderTarget(0, pOldRenderTarget.GetAddressOf())) &&
					SUCCEEDED(ProxyInterface->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, pBackBuffer.GetAddressOf())))
				{
					ProxyInterface->SetDepthStencilSurface(nullptr);
					ProxyInterface->SetRenderTarget(0, pBackBuffer.Get());
				}

				// Apply brightness level
				if (ShadowBackbuffer.Count() || IsGammaSet)
				{
					ApplyBrightnessLevel();
				}

				// Draw FPS counter to screen
				if (Config.ShowFPSCounter)
				{
					RECT rect = { 0, 0, SHARED.BufferWidth, SHARED.BufferHeight };
					if (SHARED.IsDirectDrawDevice && SHARED.IsWindowMode)
					{
						GetClientRect(SHARED.DeviceWindow, &rect);
					}
					DrawFPS(static_cast<float>(SHARED.AverageFPSCounter), rect, Config.ShowFPSCounter);
				}

				// Restore render target
				if (pOldRenderTarget.Get())
				{
					ProxyInterface->SetRenderTarget(0, pOldRenderTarget.Get());
				}

				// Restore depth stencil
				if (pOldDepthStencil.Get())
				{
					ProxyInterface->SetDepthStencilSurface(pOldDepthStencil.Get());
				}
			}

			// Apply state block
			pStateBlock->Apply();
		}
	}

	if (CalledBeginScene || (Config.ForceSingleBeginEndScene && BeginSceneCalled))
	{
		CallEndScene();
	}
	BeginSceneCalled = false;

	// Check FPU state before presenting
	Utils::ResetInvalidFPUState();
}

void m_IDirect3DDevice9Ex::ApplyPostPresentFixes()
{
	if (ShadowBackbuffer.Count())
	{
		if (SHARED.BackBufferCount == 1)
		{
			if (FAILED(ProxyInterface->StretchRect(ShadowBackbuffer.GetCurrentBackBuffer()->GetProxyInterface(), nullptr, ShadowBackbuffer.GetCurrentFrontBuffer()->GetProxyInterface(), nullptr, D3DTEXF_NONE)))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Warning: Failed to copy shadow backbuffer into shadow front buffer!");
			}
		}
		else
		{
			ShadowBackbuffer.Rotate();

			ComPtr<IDirect3DSurface9> pSurface;
			if (SUCCEEDED(ProxyInterface->GetRenderTarget(0, pSurface.GetAddressOf())))
			{
				if (pSurface.Get() == ShadowBackbuffer.GetCurrentFrontBuffer()->GetProxyInterface() ||
					std::find(BackBufferList.begin(), BackBufferList.end(), pSurface.Get()) != BackBufferList.end())
				{
					ProxyInterface->SetRenderTarget(0, ShadowBackbuffer.GetCurrentBackBuffer()->GetProxyInterface());
				}
			}
		}
	}

	if (Config.LimitPerFrameFPS)
	{
		LimitFrameRate();
	}

	if (Config.ShowFPSCounter || Config.EnableImgui)
	{
		CalculateFPS();
	}
}

HRESULT m_IDirect3DDevice9Ex::Present(CONST RECT *pSourceRect, CONST RECT *pDestRect, HWND hDestWindowOverride, CONST RGNDATA *pDirtyRegion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.D3d9to9Ex && ProxyInterfaceEx)
	{
		return PresentEx(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion, 0);
	}

	ApplyPrePresentFixes();

	HRESULT hr = ProxyInterface->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);

	if (SUCCEEDED(hr))
	{
		ApplyPostPresentFixes();
	}

	return hr;
}

void m_IDirect3DDevice9Ex::ApplyDrawFixes()
{
	// CacheClipPlane
	if (Config.CacheClipPlane && isClipPlaneSet)
	{
		ApplyClipPlanes();
	}

	// Reenable Anisotropic Filtering
	if (MaxAnisotropy)
	{
		ReeableAnisotropicSamplerState();
	}

	// Fix environment map cubes
	if (Config.EnvironmentCubeMapFix)
	{
		SetEnvironmentCubeMapTexture();
	}
}

HRESULT m_IDirect3DDevice9Ex::DrawIndexedPrimitive(THIS_ D3DPRIMITIVETYPE Type, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	ApplyDrawFixes();

	return ProxyInterface->DrawIndexedPrimitive(Type, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
}

HRESULT m_IDirect3DDevice9Ex::DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT MinIndex, UINT NumVertices, UINT PrimitiveCount, CONST void *pIndexData, D3DFORMAT IndexDataFormat, CONST void *pVertexStreamZeroData, UINT VertexStreamZeroStride)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	ApplyDrawFixes();

	return ProxyInterface->DrawIndexedPrimitiveUP(PrimitiveType, MinIndex, NumVertices, PrimitiveCount, pIndexData, IndexDataFormat, pVertexStreamZeroData, VertexStreamZeroStride);
}

HRESULT m_IDirect3DDevice9Ex::DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	ApplyDrawFixes();

	return ProxyInterface->DrawPrimitive(PrimitiveType, StartVertex, PrimitiveCount);
}

HRESULT m_IDirect3DDevice9Ex::DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, CONST void *pVertexStreamZeroData, UINT VertexStreamZeroStride)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	ApplyDrawFixes();

	return ProxyInterface->DrawPrimitiveUP(PrimitiveType, PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride);
}

HRESULT m_IDirect3DDevice9Ex::CallBeginScene()
{
	HRESULT hr = ProxyInterface->BeginScene();

	if (SUCCEEDED(hr))
	{
		IsInScene = true;
		BeginSceneCalled = true;

#ifdef ENABLE_DEBUGOVERLAY
		if (Config.EnableImgui)
		{
			DOverlay.BeginScene();
		}
#endif
	}

	return hr;
}

HRESULT m_IDirect3DDevice9Ex::BeginScene()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

#ifdef ENABLE_DEBUGOVERLAY
	// Setup overlay before BeginScene if not already setup on this device
	if (Config.EnableImgui && IsWindow(SHARED.DeviceWindow) && (!DOverlay.IsSetup() || DOverlay.Getd3d9Device() != ProxyInterface))
	{
		DOverlay.Setup(SHARED.DeviceWindow, ProxyInterface);
	}
#endif

	if (Config.ForceSingleBeginEndScene && (IsInScene || BeginSceneCalled))
	{
		if (IsInScene)
		{
			return D3DERR_INVALIDCALL;
		}

		IsInScene = true;

		return D3D_OK;
	}

	HRESULT hr = CallBeginScene();

	if (SUCCEEDED(hr))
	{
		// Get DeviceCaps
		if (Caps.DeviceType == NULL)
		{
			if (SUCCEEDED(ProxyInterface->GetDeviceCaps(&Caps)))
			{
				// Set for Anisotropic Filtering
				MaxAnisotropy = (Config.AnisotropicFiltering == 1) ? Caps.MaxAnisotropy : min((DWORD)Config.AnisotropicFiltering, Caps.MaxAnisotropy);
			}
			else
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: Falied to get DeviceCaps (" << this << ")");
				ZeroMemory(&Caps, sizeof(D3DCAPS9));
			}
		}

		// Set for Multisample
		if (SHARED.DeviceMultiSampleFlag)
		{
			ProxyInterface->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, TRUE);
			if (SHARED.SetSSAA)
			{
				ProxyInterface->SetRenderState(D3DRS_ADAPTIVETESS_Y, MAKEFOURCC('S', 'S', 'A', 'A'));
			}
			if (SHARED.SetATOC)
			{
				ProxyInterface->SetRenderState(D3DRS_ADAPTIVETESS_Y, MAKEFOURCC('A', 'T', 'O', 'C'));
				if (Config.EnableMultisamplingATOC == 2)
				{
					ProxyInterface->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
				}
			}
		}
	}

	return hr;
}

HRESULT m_IDirect3DDevice9Ex::GetStreamSource(THIS_ UINT StreamNumber, IDirect3DVertexBuffer9** ppStreamData, UINT* OffsetInBytes, UINT* pStride)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	HRESULT hr = ProxyInterface->GetStreamSource(StreamNumber, ppStreamData, OffsetInBytes, pStride);

	if (SUCCEEDED(hr) && ppStreamData)
	{
		*ppStreamData = SHARED.ProxyAddressLookupTable9.FindAddress<m_IDirect3DVertexBuffer9, m_IDirect3DDevice9Ex, LPVOID>(*ppStreamData, this, IID_IDirect3DVertexBuffer9, nullptr);
	}

	return hr;
}

HRESULT m_IDirect3DDevice9Ex::SetStreamSource(THIS_ UINT StreamNumber, IDirect3DVertexBuffer9* pStreamData, UINT OffsetInBytes, UINT Stride)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (pStreamData)
	{
		pStreamData = static_cast<m_IDirect3DVertexBuffer9 *>(pStreamData)->GetProxyInterface();
	}

	return ProxyInterface->SetStreamSource(StreamNumber, pStreamData, OffsetInBytes, Stride);
}

HRESULT m_IDirect3DDevice9Ex::GetBackBuffer(THIS_ UINT iSwapChain, UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface9** ppBackBuffer)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (iSwapChain == 0 && ShadowBackbuffer.Count())
	{
		if (!ppBackBuffer)
		{
			return D3DERR_INVALIDCALL;
		}
		*ppBackBuffer = nullptr;

		if (iBackBuffer >= SHARED.BackBufferCount)
		{
			return D3DERR_INVALIDCALL;
		}

		if (Type != D3DBACKBUFFER_TYPE_MONO)
		{
			Logging::Log() << __FUNCTION__ << " Warning: unsupported backbuffer type requested: " << Type;
		}

		// For stereo buffers, we just return the mono shadow for simplicity
		IDirect3DSurface9* pSurface = ShadowBackbuffer.GetSurface(iBackBuffer);
		if (!pSurface)
		{
			return D3DERR_INVALIDCALL;
		}

		// Caller expects a refcount increment
		pSurface->AddRef();

		*ppBackBuffer = pSurface;

		return D3D_OK;
	}

	HRESULT hr = ProxyInterface->GetBackBuffer(iSwapChain, iBackBuffer, Type, ppBackBuffer);

	if (SUCCEEDED(hr) && ppBackBuffer)
	{
		*ppBackBuffer = SHARED.ProxyAddressLookupTable9.FindAddress<m_IDirect3DSurface9, m_IDirect3DDevice9Ex, LPVOID>(*ppBackBuffer, this, IID_IDirect3DSurface9, nullptr);
	}

	return hr;
}

HRESULT m_IDirect3DDevice9Ex::GetDepthStencilSurface(IDirect3DSurface9 **ppZStencilSurface)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	HRESULT hr = ProxyInterface->GetDepthStencilSurface(ppZStencilSurface);

	if (SUCCEEDED(hr) && ppZStencilSurface)
	{
		*ppZStencilSurface = SHARED.ProxyAddressLookupTable9.FindAddress<m_IDirect3DSurface9, m_IDirect3DDevice9Ex, LPVOID>(*ppZStencilSurface, this, IID_IDirect3DSurface9, nullptr);
	}

	return hr;
}

HRESULT m_IDirect3DDevice9Ex::GetTexture(DWORD Stage, IDirect3DBaseTexture9 **ppTexture)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Stage == 0 && isBlankTextureUsed)
	{
		if (ppTexture)
		{
			*ppTexture = nullptr;
		}
		return D3D_OK;
	}

	HRESULT hr = ProxyInterface->GetTexture(Stage, ppTexture);

	if (SUCCEEDED(hr) && ppTexture && *ppTexture)
	{
		switch ((*ppTexture)->GetType())
		{
		case D3DRTYPE_TEXTURE:
			*ppTexture = SHARED.ProxyAddressLookupTable9.FindAddress<m_IDirect3DTexture9, m_IDirect3DDevice9Ex, LPVOID>(*ppTexture, this, IID_IDirect3DTexture9, nullptr);
			break;
		case D3DRTYPE_VOLUMETEXTURE:
			*ppTexture = SHARED.ProxyAddressLookupTable9.FindAddress<m_IDirect3DVolumeTexture9, m_IDirect3DDevice9Ex, LPVOID>(*ppTexture, this, IID_IDirect3DVolumeTexture9, nullptr);
			break;
		case D3DRTYPE_CUBETEXTURE:
			*ppTexture = SHARED.ProxyAddressLookupTable9.FindAddress<m_IDirect3DCubeTexture9, m_IDirect3DDevice9Ex, LPVOID>(*ppTexture, this, IID_IDirect3DCubeTexture9, nullptr);
			break;
		default:
			return D3DERR_INVALIDCALL;
		}
	}

	return hr;
}

HRESULT m_IDirect3DDevice9Ex::GetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD *pValue)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->GetTextureStageState(Stage, Type, pValue);
}

// Check if this is a texture stage transform for cube mapping
void m_IDirect3DDevice9Ex::CheckTransformForCubeMap(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix)
{
	if (State >= D3DTS_TEXTURE0 && State <= D3DTS_TEXTURE7)
	{
		DWORD stage = State - D3DTS_TEXTURE0;

		if (pMatrix)
		{
			const D3DMATRIX& m = *pMatrix;
			bool isCubeMap = false;

			// Cube maps typically do not have translation
			if (m._41 == 0.0f && m._42 == 0.0f && m._43 == 0.0f && m._44 == 1.0f)
			{
				// Ensure no perspective projection
				if (m._14 == 0.0f && m._24 == 0.0f && m._34 == 0.0f)
				{
					// Check for a rotation matrix (orthonormal basis vectors)
					float dotX = m._11 * m._11 + m._21 * m._21 + m._31 * m._31;
					float dotY = m._12 * m._12 + m._22 * m._22 + m._32 * m._32;
					float dotZ = m._13 * m._13 + m._23 * m._23 + m._33 * m._33;

					float dotXY = m._11 * m._12 + m._21 * m._22 + m._31 * m._32;
					float dotXZ = m._11 * m._13 + m._21 * m._23 + m._31 * m._33;
					float dotYZ = m._12 * m._13 + m._22 * m._23 + m._32 * m._33;

					// A proper rotation matrix should have:
					// - Each column close to unit length (1.0)
					// - Orthogonality (dot product of different columns near 0)
					const float epsilon = 0.01f; // Allow small floating-point error
					if (fabs(dotX - 1.0f) < epsilon && fabs(dotY - 1.0f) < epsilon && fabs(dotZ - 1.0f) < epsilon &&
						fabs(dotXY) < epsilon && fabs(dotXZ) < epsilon && fabs(dotYZ) < epsilon)
					{
						isCubeMap = true;
					}
				}
			}

			// Store cube map detection result
			isTransformCubeMap[stage] = isCubeMap;
		}
	}
}

// Check if an environment cube map is being used
bool m_IDirect3DDevice9Ex::CheckTextureStageForCubeMap() const
{
	for (DWORD i = 0; i < MAX_TEXTURE_STAGES; i++)
	{
		if ((texCoordIndex[i] == D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR || texCoordIndex[i] == D3DTSS_TCI_CAMERASPACENORMAL) &&
			((texTransformFlags[i] & D3DTTFF_COUNT3) || (texTransformFlags[i] & D3DTTFF_COUNT4)))
		{
			return true;
		}
	}
	return false;
}

void m_IDirect3DDevice9Ex::SetEnvironmentCubeMapTexture()
{
	const bool isCubeMap = CheckTextureStageForCubeMap() ||
		[&]() {
		for (int i = 0; i < MAX_TEXTURE_STAGES; ++i)
		{
			if (isTextureCubeMap[i] || isTransformCubeMap[i])
			{
				return true;
			}
		}
		return false;
		}();

	if (isCubeMap && pCurrentTexture == nullptr)
	{
		if (!BlankTexture)
		{
			const UINT CubeSize = 64;
			const DWORD Usage = (Config.D3d9to9Ex ? D3DUSAGE_DYNAMIC : 0);
			const D3DPOOL Pool = (Config.D3d9to9Ex ? D3DPOOL_DEFAULT : D3DPOOL_MANAGED);

			HRESULT hr = ProxyInterface->CreateCubeTexture(CubeSize, 1, Usage, D3DFMT_A8R8G8B8, Pool, &BlankTexture, nullptr);
			if (FAILED(hr))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: failed to create BlankCubeTexture for environment map!");
				return;
			}

			D3DLOCKED_RECT lockedRect;
			for (UINT face = 0; face < 6; ++face)
			{
				if (SUCCEEDED(BlankTexture->LockRect((D3DCUBEMAP_FACES)face, 0, &lockedRect, nullptr, 0)))
				{
					DWORD* pixels = static_cast<DWORD*>(lockedRect.pBits);
					for (UINT y = 0; y < CubeSize; ++y)
					{
						for (UINT x = 0; x < CubeSize; ++x)
						{
							pixels[x] = D3DCOLOR_ARGB(255, 255, 255, 255); // White with full alpha
						}
						pixels += lockedRect.Pitch / sizeof(DWORD);
					}
					BlankTexture->UnlockRect((D3DCUBEMAP_FACES)face, 0);
				}
				else
				{
					LOG_LIMIT(100, __FUNCTION__ << " Error: failed to lock BlankCubeTexture face: " << face);
				}
			}
		}

		isBlankTextureUsed = true;
		ProxyInterface->SetTexture(0, BlankTexture);
	}
	else if (!isCubeMap && isBlankTextureUsed)
	{
		isBlankTextureUsed = false;
		ProxyInterface->SetTexture(0, nullptr);
	}
}

HRESULT m_IDirect3DDevice9Ex::SetTexture(DWORD Stage, IDirect3DBaseTexture9 *pTexture)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	bool isTexCube = false;
	if (pTexture)
	{
		switch (pTexture->GetType())
		{
		case D3DRTYPE_TEXTURE:
			pTexture = static_cast<m_IDirect3DTexture9 *>(pTexture)->GetProxyInterface();
			if (MaxAnisotropy && Stage > 0)
			{
				DisableAnisotropicSamplerState((Caps.TextureFilterCaps & D3DPTFILTERCAPS_MINFANISOTROPIC), (Caps.TextureFilterCaps & D3DPTFILTERCAPS_MAGFANISOTROPIC));
			}
			break;
		case D3DRTYPE_VOLUMETEXTURE:
			pTexture = static_cast<m_IDirect3DVolumeTexture9 *>(pTexture)->GetProxyInterface();
			if (MaxAnisotropy && Stage > 0)
			{
				DisableAnisotropicSamplerState((Caps.VolumeTextureFilterCaps & D3DPTFILTERCAPS_MINFANISOTROPIC), (Caps.VolumeTextureFilterCaps & D3DPTFILTERCAPS_MAGFANISOTROPIC));
			}
			break;
		case D3DRTYPE_CUBETEXTURE:
			pTexture = static_cast<m_IDirect3DCubeTexture9 *>(pTexture)->GetProxyInterface();
			isTexCube = true;
			if (MaxAnisotropy && Stage > 0)
			{
				DisableAnisotropicSamplerState((Caps.CubeTextureFilterCaps & D3DPTFILTERCAPS_MINFANISOTROPIC), (Caps.CubeTextureFilterCaps & D3DPTFILTERCAPS_MAGFANISOTROPIC));
			}
			break;
		default:
			return D3DERR_INVALIDCALL;
		}
	}

	HRESULT hr = ProxyInterface->SetTexture(Stage, pTexture);

	if (SUCCEEDED(hr))
	{
		if (Stage < MAX_TEXTURE_STAGES)
		{
			isTextureCubeMap[Stage] = isTexCube;
		}
		if (Stage == 0)
		{
			isBlankTextureUsed = false;
			pCurrentTexture = pTexture;
		}
	}

	return hr;
}

HRESULT m_IDirect3DDevice9Ex::SetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ") Stage: " << Stage << " Type: " << Type << " Value: " << Value;

	HRESULT hr = ProxyInterface->SetTextureStageState(Stage, Type, Value);

	if (SUCCEEDED(hr))
	{
		if (Stage < MAX_TEXTURE_STAGES)
		{
			if (Type == D3DTSS_TEXCOORDINDEX)
			{
				texCoordIndex[Stage] = Value;
			}
			else if (Type == D3DTSS_TEXTURETRANSFORMFLAGS)
			{
				texTransformFlags[Stage] = Value;
			}
		}
	}

	return hr;
}

HRESULT m_IDirect3DDevice9Ex::UpdateTexture(IDirect3DBaseTexture9 *pSourceTexture, IDirect3DBaseTexture9 *pDestinationTexture)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (pSourceTexture)
	{
		switch (pSourceTexture->GetType())
		{
		case D3DRTYPE_TEXTURE:
			pSourceTexture = static_cast<m_IDirect3DTexture9 *>(pSourceTexture)->GetProxyInterface();
			break;
		case D3DRTYPE_VOLUMETEXTURE:
			pSourceTexture = static_cast<m_IDirect3DVolumeTexture9 *>(pSourceTexture)->GetProxyInterface();
			break;
		case D3DRTYPE_CUBETEXTURE:
			pSourceTexture = static_cast<m_IDirect3DCubeTexture9 *>(pSourceTexture)->GetProxyInterface();
			break;
		default:
			return D3DERR_INVALIDCALL;
		}
	}
	if (pDestinationTexture)
	{
		switch (pDestinationTexture->GetType())
		{
		case D3DRTYPE_TEXTURE:
			pDestinationTexture = static_cast<m_IDirect3DTexture9 *>(pDestinationTexture)->GetProxyInterface();
			break;
		case D3DRTYPE_VOLUMETEXTURE:
			pDestinationTexture = static_cast<m_IDirect3DVolumeTexture9 *>(pDestinationTexture)->GetProxyInterface();
			break;
		case D3DRTYPE_CUBETEXTURE:
			pDestinationTexture = static_cast<m_IDirect3DCubeTexture9 *>(pDestinationTexture)->GetProxyInterface();
			break;
		default:
			return D3DERR_INVALIDCALL;
		}
	}

	return ProxyInterface->UpdateTexture(pSourceTexture, pDestinationTexture);
}

HRESULT m_IDirect3DDevice9Ex::ValidateDevice(DWORD *pNumPasses)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->ValidateDevice(pNumPasses);
}

HRESULT m_IDirect3DDevice9Ex::GetClipPlane(DWORD Index, float *pPlane)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	// CacheClipPlane if not using d3d8to9 (it already exists there)
	if (Config.CacheClipPlane && SHARED.ClientDirectXVersion != 8)
	{
		if (!pPlane || Index >= MAX_CLIP_PLANES)
		{
			return D3DERR_INVALIDCALL;
		}

		memcpy(pPlane, StoredClipPlanes[Index], sizeof(StoredClipPlanes[0]));

		return D3D_OK;
	}

	return ProxyInterface->GetClipPlane(Index, pPlane);
}

HRESULT m_IDirect3DDevice9Ex::SetClipPlane(DWORD Index, CONST float *pPlane)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	// CacheClipPlane if not using d3d8to9 (it already exists there)
	if (Config.CacheClipPlane && SHARED.ClientDirectXVersion != 8)
	{
		if (!pPlane || Index >= MAX_CLIP_PLANES)
		{
			return D3DERR_INVALIDCALL;
		}

		isClipPlaneSet = true;

		memcpy(StoredClipPlanes[Index], pPlane, sizeof(StoredClipPlanes[0]));

		return D3D_OK;
	}

	return ProxyInterface->SetClipPlane(Index, pPlane);
}

// CacheClipPlane
void m_IDirect3DDevice9Ex::ApplyClipPlanes()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	DWORD index = 0;
	for (const auto plane : StoredClipPlanes)
	{
		if ((ClipPlaneRenderState & (1 << index)) != 0)
		{
			ProxyInterface->SetClipPlane(index, plane);
		}
		index++;
	}
}

HRESULT m_IDirect3DDevice9Ex::Clear(DWORD Count, CONST D3DRECT *pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->Clear(Count, pRects, Flags, Color, Z, Stencil);
}

HRESULT m_IDirect3DDevice9Ex::GetViewport(D3DVIEWPORT9 *pViewport)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->GetViewport(pViewport);
}

HRESULT m_IDirect3DDevice9Ex::SetViewport(CONST D3DVIEWPORT9 *pViewport)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->SetViewport(pViewport);
}

HRESULT m_IDirect3DDevice9Ex::CreateVertexShader(THIS_ CONST DWORD* pFunction, IDirect3DVertexShader9** ppShader)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ppShader)
	{
		return D3DERR_INVALIDCALL;
	}

	HRESULT hr = ProxyInterface->CreateVertexShader(pFunction, ppShader);

	if (SUCCEEDED(hr))
	{
		*ppShader = SHARED.ProxyAddressLookupTable9.FindCreateAddress<m_IDirect3DVertexShader9, m_IDirect3DDevice9Ex, LPVOID>(*ppShader, this, IID_IDirect3DVertexShader9, nullptr);
		return D3D_OK;
	}

	Logging::LogDebug() << __FUNCTION__ << " FAILED! " << (D3DERR)hr << " " << pFunction;
	return hr;
}

HRESULT m_IDirect3DDevice9Ex::GetVertexShader(THIS_ IDirect3DVertexShader9** ppShader)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	HRESULT hr = ProxyInterface->GetVertexShader(ppShader);

	if (SUCCEEDED(hr) && ppShader)
	{
		*ppShader = SHARED.ProxyAddressLookupTable9.FindAddress<m_IDirect3DVertexShader9, m_IDirect3DDevice9Ex, LPVOID>(*ppShader, this, IID_IDirect3DVertexShader9, nullptr);
	}

	return hr;
}

HRESULT m_IDirect3DDevice9Ex::SetVertexShader(THIS_ IDirect3DVertexShader9* pShader)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (pShader)
	{
		pShader = static_cast<m_IDirect3DVertexShader9 *>(pShader)->GetProxyInterface();
	}

	return ProxyInterface->SetVertexShader(pShader);
}

HRESULT m_IDirect3DDevice9Ex::CreateQuery(THIS_ D3DQUERYTYPE Type, IDirect3DQuery9** ppQuery)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ppQuery)
	{
		return D3DERR_INVALIDCALL;
	}

	HRESULT hr = ProxyInterface->CreateQuery(Type, ppQuery);

	if (SUCCEEDED(hr))
	{
		*ppQuery = SHARED.ProxyAddressLookupTable9.FindCreateAddress<m_IDirect3DQuery9, m_IDirect3DDevice9Ex, LPVOID>(*ppQuery, this, IID_IDirect3DQuery9, nullptr);
		return D3D_OK;
	}

	Logging::LogDebug() << __FUNCTION__ << " FAILED! " << (D3DERR)hr << " " << Type;
	return hr;
}

HRESULT m_IDirect3DDevice9Ex::SetPixelShaderConstantB(THIS_ UINT StartRegister, CONST BOOL* pConstantData, UINT  BoolCount)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->SetPixelShaderConstantB(StartRegister, pConstantData, BoolCount);
}

HRESULT m_IDirect3DDevice9Ex::GetPixelShaderConstantB(THIS_ UINT StartRegister, BOOL* pConstantData, UINT BoolCount)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->GetPixelShaderConstantB(StartRegister, pConstantData, BoolCount);
}

HRESULT m_IDirect3DDevice9Ex::SetPixelShaderConstantI(THIS_ UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->SetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount);
}

HRESULT m_IDirect3DDevice9Ex::GetPixelShaderConstantI(THIS_ UINT StartRegister, int* pConstantData, UINT Vector4iCount)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->GetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount);
}

HRESULT m_IDirect3DDevice9Ex::SetPixelShaderConstantF(THIS_ UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->SetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount);
}

HRESULT m_IDirect3DDevice9Ex::GetPixelShaderConstantF(THIS_ UINT StartRegister, float* pConstantData, UINT Vector4fCount)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->GetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount);
}

HRESULT m_IDirect3DDevice9Ex::SetStreamSourceFreq(THIS_ UINT StreamNumber, UINT Divider)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->SetStreamSourceFreq(StreamNumber, Divider);
}

HRESULT m_IDirect3DDevice9Ex::GetStreamSourceFreq(THIS_ UINT StreamNumber, UINT* Divider)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->GetStreamSourceFreq(StreamNumber, Divider);
}

HRESULT m_IDirect3DDevice9Ex::SetVertexShaderConstantB(THIS_ UINT StartRegister, CONST BOOL* pConstantData, UINT  BoolCount)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->SetVertexShaderConstantB(StartRegister, pConstantData, BoolCount);
}

HRESULT m_IDirect3DDevice9Ex::GetVertexShaderConstantB(THIS_ UINT StartRegister, BOOL* pConstantData, UINT BoolCount)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->GetVertexShaderConstantB(StartRegister, pConstantData, BoolCount);
}

HRESULT m_IDirect3DDevice9Ex::SetVertexShaderConstantF(THIS_ UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->SetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount);
}

HRESULT m_IDirect3DDevice9Ex::GetVertexShaderConstantF(THIS_ UINT StartRegister, float* pConstantData, UINT Vector4fCount)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->GetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount);
}

HRESULT m_IDirect3DDevice9Ex::SetVertexShaderConstantI(THIS_ UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->SetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount);
}

HRESULT m_IDirect3DDevice9Ex::GetVertexShaderConstantI(THIS_ UINT StartRegister, int* pConstantData, UINT Vector4iCount)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->GetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount);
}

HRESULT m_IDirect3DDevice9Ex::SetFVF(THIS_ DWORD FVF)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->SetFVF(FVF);
}

HRESULT m_IDirect3DDevice9Ex::GetFVF(THIS_ DWORD* pFVF)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->GetFVF(pFVF);
}

HRESULT m_IDirect3DDevice9Ex::CreateVertexDeclaration(THIS_ CONST D3DVERTEXELEMENT9* pVertexElements, IDirect3DVertexDeclaration9** ppDecl)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ppDecl)
	{
		return D3DERR_INVALIDCALL;
	}

	HRESULT hr = ProxyInterface->CreateVertexDeclaration(pVertexElements, ppDecl);

	if (SUCCEEDED(hr))
	{
		*ppDecl = SHARED.ProxyAddressLookupTable9.FindCreateAddress<m_IDirect3DVertexDeclaration9, m_IDirect3DDevice9Ex, LPVOID>(*ppDecl, this, IID_IDirect3DVertexDeclaration9, nullptr);
		return D3D_OK;
	}

	Logging::LogDebug() << __FUNCTION__ << " FAILED! " << (D3DERR)hr << " " << pVertexElements;
	return hr;
}

HRESULT m_IDirect3DDevice9Ex::SetVertexDeclaration(THIS_ IDirect3DVertexDeclaration9* pDecl)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (pDecl)
	{
		pDecl = static_cast<m_IDirect3DVertexDeclaration9 *>(pDecl)->GetProxyInterface();
	}

	return ProxyInterface->SetVertexDeclaration(pDecl);
}

HRESULT m_IDirect3DDevice9Ex::GetVertexDeclaration(THIS_ IDirect3DVertexDeclaration9** ppDecl)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	HRESULT hr = ProxyInterface->GetVertexDeclaration(ppDecl);

	if (SUCCEEDED(hr) && ppDecl)
	{
		*ppDecl = SHARED.ProxyAddressLookupTable9.FindAddress<m_IDirect3DVertexDeclaration9, m_IDirect3DDevice9Ex, LPVOID>(*ppDecl, this, IID_IDirect3DVertexDeclaration9, nullptr);
	}

	return hr;
}

HRESULT m_IDirect3DDevice9Ex::SetNPatchMode(THIS_ float nSegments)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->SetNPatchMode(nSegments);
}

float m_IDirect3DDevice9Ex::GetNPatchMode(THIS)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->GetNPatchMode();
}

int m_IDirect3DDevice9Ex::GetSoftwareVertexProcessing(THIS)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->GetSoftwareVertexProcessing();
}

unsigned int m_IDirect3DDevice9Ex::GetNumberOfSwapChains(THIS)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->GetNumberOfSwapChains();
}

HRESULT m_IDirect3DDevice9Ex::EvictManagedResources(THIS)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->EvictManagedResources();
}

HRESULT m_IDirect3DDevice9Ex::SetSoftwareVertexProcessing(THIS_ BOOL bSoftware)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->SetSoftwareVertexProcessing(bSoftware);
}

HRESULT m_IDirect3DDevice9Ex::SetScissorRect(THIS_ CONST RECT* pRect)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->SetScissorRect(pRect);
}

HRESULT m_IDirect3DDevice9Ex::GetScissorRect(THIS_ RECT* pRect)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->GetScissorRect(pRect);
}

HRESULT m_IDirect3DDevice9Ex::GetSamplerState(THIS_ DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD* pValue)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->GetSamplerState(Sampler, Type, pValue);
}

HRESULT m_IDirect3DDevice9Ex::SetSamplerState(THIS_ DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	// Disable AntiAliasing when using point filtering
	if (Config.AntiAliasing)
	{
		if (Type == D3DSAMP_MINFILTER || Type == D3DSAMP_MAGFILTER)
		{
			if (Value == D3DTEXF_NONE || Value == D3DTEXF_POINT)
			{
				ProxyInterface->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, FALSE);
			}
			else
			{
				ProxyInterface->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, TRUE);
			}
		}
	}

	// Enable Anisotropic Filtering
	if (MaxAnisotropy)
	{
		if (Type == D3DSAMP_MAXANISOTROPY)
		{
			if (SUCCEEDED(ProxyInterface->SetSamplerState(Sampler, D3DSAMP_MAXANISOTROPY, MaxAnisotropy)))
			{
				return D3D_OK;
			}
		}
		else if ((Value == D3DTEXF_LINEAR || Value == D3DTEXF_ANISOTROPIC) && (Type == D3DSAMP_MINFILTER || Type == D3DSAMP_MAGFILTER))
		{
			if (SUCCEEDED(ProxyInterface->SetSamplerState(Sampler, D3DSAMP_MAXANISOTROPY, MaxAnisotropy)) &&
				SUCCEEDED(ProxyInterface->SetSamplerState(Sampler, Type, D3DTEXF_ANISOTROPIC)))
			{
				if (!isAnisotropySet)
				{
					isAnisotropySet = true;
					Logging::Log() << "Setting Anisotropic Filtering at " << MaxAnisotropy << "x";
				}
				return D3D_OK;
			}
		}
	}

	return ProxyInterface->SetSamplerState(Sampler, Type, Value);
}

void m_IDirect3DDevice9Ex::DisableAnisotropicSamplerState(bool AnisotropyMin, bool AnisotropyMag)
{
	DWORD Value = 0;
	for (int x = 0; x < 4; x++)
	{
		if (!AnisotropyMin)	// Anisotropic Min Filter is not supported for multi-stage textures
		{
			if (SUCCEEDED(ProxyInterface->GetSamplerState(x, D3DSAMP_MINFILTER, &Value)) && Value == D3DTEXF_ANISOTROPIC &&
				SUCCEEDED(ProxyInterface->SetSamplerState(x, D3DSAMP_MINFILTER, D3DTEXF_LINEAR)))
			{
				AnisotropyDisabledFlag = true;
			}
		}
		if (!AnisotropyMag)	// Anisotropic Mag Filter is not supported for multi-stage textures
		{
			if (SUCCEEDED(ProxyInterface->GetSamplerState(x, D3DSAMP_MAGFILTER, &Value)) && Value == D3DTEXF_ANISOTROPIC &&
				SUCCEEDED(ProxyInterface->SetSamplerState(x, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR)))
			{
				AnisotropyDisabledFlag = true;
			}
		}
	}
}

void m_IDirect3DDevice9Ex::ReeableAnisotropicSamplerState()
{
	if (AnisotropyDisabledFlag)
	{
		bool Flag = false;
		DWORD Value = 0;
		for (int x = 0; x < 4; x++)
		{
			if (!(SUCCEEDED(ProxyInterface->GetSamplerState(x, D3DSAMP_MINFILTER, &Value)) && Value == D3DTEXF_LINEAR &&
				SUCCEEDED(ProxyInterface->SetSamplerState(x, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC))))
			{
				Flag = true;	// Unable to re-eanble Anisotropic filtering
			}
			if (!(SUCCEEDED(ProxyInterface->GetSamplerState(x, D3DSAMP_MAGFILTER, &Value)) && Value == D3DTEXF_LINEAR &&
				SUCCEEDED(ProxyInterface->SetSamplerState(x, D3DSAMP_MAGFILTER, D3DTEXF_ANISOTROPIC))))
			{
				Flag = true;	// Unable to re-eanble Anisotropic filtering
			}
		}
		AnisotropyDisabledFlag = Flag;
	}
}

HRESULT m_IDirect3DDevice9Ex::SetDepthStencilSurface(THIS_ IDirect3DSurface9* pNewZStencil)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (pNewZStencil)
	{
		pNewZStencil = static_cast<m_IDirect3DSurface9 *>(pNewZStencil)->GetProxyInterface();
	}

	return ProxyInterface->SetDepthStencilSurface(pNewZStencil);
}

HRESULT m_IDirect3DDevice9Ex::CreateOffscreenPlainSurface(THIS_ UINT Width, UINT Height, D3DFORMAT Format, D3DPOOL Pool, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ppSurface)
	{
		return D3DERR_INVALIDCALL;
	}

	if (Config.D3d9to9Ex && ProxyInterfaceEx)
	{
		return CreateOffscreenPlainSurfaceEx(Width, Height, Format, Pool, ppSurface, pSharedHandle, 0);
	}

	HRESULT hr = ProxyInterface->CreateOffscreenPlainSurface(Width, Height, Format, Pool, ppSurface, pSharedHandle);

	if (SUCCEEDED(hr))
	{
		*ppSurface = SHARED.ProxyAddressLookupTable9.FindCreateAddress<m_IDirect3DSurface9, m_IDirect3DDevice9Ex, LPVOID>(*ppSurface, this, IID_IDirect3DSurface9, nullptr);
		return D3D_OK;
	}

	Logging::LogDebug() << __FUNCTION__ << " FAILED! " << (D3DERR)hr << " " << Width << " " << Height << " " << Format << " " << Pool << " " << pSharedHandle;
	return hr;
}

HRESULT m_IDirect3DDevice9Ex::ColorFill(THIS_ IDirect3DSurface9* pSurface, CONST RECT* pRect, D3DCOLOR color)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (pSurface)
	{
		pSurface = static_cast<m_IDirect3DSurface9 *>(pSurface)->GetProxyInterface();
	}

	return ProxyInterface->ColorFill(pSurface, pRect, color);
}

// Copy surface rect to destination rect
HRESULT m_IDirect3DDevice9Ex::CopyRects(THIS_ IDirect3DSurface9 *pSourceSurface, const RECT *pSourceRectsArray, UINT cRects, IDirect3DSurface9 *pDestinationSurface, const POINT *pDestPointsArray)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!pSourceSurface || !pDestinationSurface || pSourceSurface == pDestinationSurface)
	{
		return D3DERR_INVALIDCALL;
	}

	void* pVoid = nullptr;
	if (SUCCEEDED(pSourceSurface->QueryInterface(IID_GetInterfaceX, &pVoid)))
	{
		pSourceSurface = static_cast<m_IDirect3DSurface9*>(pSourceSurface)->GetProxyInterface();
	}
	if (SUCCEEDED(pDestinationSurface->QueryInterface(IID_GetInterfaceX, &pVoid)))
	{
		pDestinationSurface = static_cast<m_IDirect3DSurface9*>(pDestinationSurface)->GetProxyInterface();
	}

	D3DSURFACE_DESC SrcDesc = {}, DestDesc = {};

	if (SUCCEEDED(pSourceSurface->GetDesc(&SrcDesc)) && SUCCEEDED(pDestinationSurface->GetDesc(&DestDesc)) && SrcDesc.Format != DestDesc.Format)
	{
		return D3DERR_INVALIDCALL;
	}

	HRESULT hr = D3DERR_INVALIDCALL;

	if (cRects == 0)
	{
		cRects = 1;
	}

	for (UINT i = 0; i < cRects; i++)
	{
		RECT SourceRect = (pSourceRectsArray) ? pSourceRectsArray[i] : RECT { 0, 0, (LONG)SrcDesc.Width, (LONG)SrcDesc.Height };

		RECT DestinationRect = (pDestPointsArray) ? RECT {
			pDestPointsArray[i].x,
			pDestPointsArray[i].y,
			pDestPointsArray[i].x + (SourceRect.right - SourceRect.left),
			pDestPointsArray[i].y + (SourceRect.bottom - SourceRect.top)
		} : SourceRect;

		hr = D3DERR_INVALIDCALL;

		if (SrcDesc.Pool == D3DPOOL_DEFAULT && DestDesc.Pool == D3DPOOL_DEFAULT)
		{
			hr = ProxyInterface->StretchRect(pSourceSurface, &SourceRect, pDestinationSurface, &DestinationRect, D3DTEXF_NONE);
		}
		else if (SrcDesc.Pool == D3DPOOL_SYSTEMMEM && DestDesc.Pool == D3DPOOL_DEFAULT && !DestDesc.MultiSampleType)
		{
			hr = ProxyInterface->UpdateSurface(pSourceSurface, &SourceRect, pDestinationSurface, (LPPOINT)&DestinationRect);
		}
		if (FAILED(hr))
		{
			if (SUCCEEDED(D3DXLoadSurfaceFromSurface(pDestinationSurface, nullptr, &DestinationRect, pSourceSurface, nullptr, &SourceRect, D3DX_FILTER_NONE, 0)))
			{
				hr = D3D_OK;
			}
		}

		if (FAILED(hr))
		{
			break;
		}
	}

	return hr;
}

HRESULT m_IDirect3DDevice9Ex::StretchRect(THIS_ IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestSurface, CONST RECT* pDestRect, D3DTEXTUREFILTERTYPE Filter)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (pSourceSurface)
	{
		pSourceSurface = static_cast<m_IDirect3DSurface9 *>(pSourceSurface)->GetProxyInterface();
	}

	if (pDestSurface)
	{
		pDestSurface = static_cast<m_IDirect3DSurface9 *>(pDestSurface)->GetProxyInterface();
	}

	return ProxyInterface->StretchRect(pSourceSurface, pSourceRect, pDestSurface, pDestRect, Filter);
}

HRESULT m_IDirect3DDevice9Ex::GetFrontBufferData(THIS_ UINT iSwapChain, IDirect3DSurface9* pDestSurface)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!pDestSurface)
	{
		return D3DERR_INVALIDCALL;
	}

	if (Config.EnableWindowMode && (SHARED.BufferWidth != SHARED.screenWidth || SHARED.BufferHeight != SHARED.screenHeight))
	{
		return FakeGetFrontBufferData(iSwapChain, pDestSurface);
	}
	else
	{
		if (iSwapChain == 0 && ShadowBackbuffer.Count())
		{
			return GetFrontBufferShadowData(iSwapChain, pDestSurface);
		}

		pDestSurface = static_cast<m_IDirect3DSurface9*>(pDestSurface)->GetProxyInterface();

		return ProxyInterface->GetFrontBufferData(iSwapChain, pDestSurface);
	}
}

HRESULT m_IDirect3DDevice9Ex::GetFrontBufferShadowData(THIS_ UINT iSwapChain, IDirect3DSurface9* pDestSurface)
{
	// Get surface desc
	D3DSURFACE_DESC Desc;
	if (!pDestSurface || FAILED(pDestSurface->GetDesc(&Desc)))
	{
		return D3DERR_INVALIDCALL;
	}

	void* pVoid = nullptr;
	if (SUCCEEDED(pDestSurface->QueryInterface(IID_GetInterfaceX, &pVoid)))
	{
		pDestSurface = static_cast<m_IDirect3DSurface9*>(pDestSurface)->GetProxyInterface();
	}

	if (iSwapChain != 0)
	{
		return ProxyInterface->GetFrontBufferData(iSwapChain, pDestSurface);
	}

	if ((LONG)Desc.Width != SHARED.BufferWidth || (LONG)Desc.Height != SHARED.BufferHeight || Desc.Format != D3DFMT_A8R8G8B8)
	{
		Logging::Log() << __FUNCTION__ << " Error: incorrect size or format: " << Desc.Width << "x" << Desc.Height <<
			" " << SHARED.BufferWidth << "x" << SHARED.BufferHeight << " " << Desc.Format;
		return D3DERR_INVALIDCALL;
	}

	IDirect3DSurface9* pShadowSurface = ShadowBackbuffer.GetCurrentFrontBuffer()->GetProxyInterface();

	// Copy shadow buffer into destination
	return ProxyInterface->StretchRect(pShadowSurface, nullptr, pDestSurface, nullptr, D3DTEXF_NONE);
}

HRESULT m_IDirect3DDevice9Ex::FakeGetFrontBufferData(THIS_ UINT iSwapChain, IDirect3DSurface9* pDestSurface)
{
	// Get surface desc
	D3DSURFACE_DESC Desc;
	if (!pDestSurface || FAILED(pDestSurface->GetDesc(&Desc)))
	{
		return D3DERR_INVALIDCALL;
	}

	void* pVoid = nullptr;
	if (SUCCEEDED(pDestSurface->QueryInterface(IID_GetInterfaceX, &pVoid)))
	{
		pDestSurface = static_cast<m_IDirect3DSurface9 *>(pDestSurface)->GetProxyInterface();
	}

	// Get location of client window
	RECT RectSrc = { 0, 0, SHARED.BufferWidth , SHARED.BufferHeight };
	RECT rcClient = { 0, 0, SHARED.BufferWidth , SHARED.BufferHeight };
	if (Config.EnableWindowMode && IsWindow(SHARED.DeviceWindow) && (IsIconic(SHARED.DeviceWindow) || !GetWindowRect(SHARED.DeviceWindow, &RectSrc) || !GetClientRect(SHARED.DeviceWindow, &rcClient)))
	{
		return D3DERR_INVALIDCALL;
	}
	int border_thickness = ((RectSrc.right - RectSrc.left) - rcClient.right) / 2;
	int top_border = (RectSrc.bottom - RectSrc.top) - rcClient.bottom - border_thickness;
	RectSrc.left += border_thickness;
	RectSrc.top += top_border;
	RectSrc.right = RectSrc.left + rcClient.right;
	RectSrc.bottom = RectSrc.top + rcClient.bottom;

	// Create new surface to hold data
	ComPtr<IDirect3DSurface9> pSrcSurface;
	if (FAILED(ProxyInterface->CreateOffscreenPlainSurface(max(SHARED.screenWidth, RectSrc.right), max(SHARED.screenHeight, RectSrc.bottom), Desc.Format, Desc.Pool, pSrcSurface.GetAddressOf(), nullptr)))
	{
		return D3DERR_INVALIDCALL;
	}

	// Get FrontBuffer data to new surface
	HRESULT hr = ShadowBackbuffer.Count() ?
		GetFrontBufferShadowData(iSwapChain, pSrcSurface.Get()) :
		ProxyInterface->GetFrontBufferData(iSwapChain, pSrcSurface.Get());
	if (FAILED(hr))
	{
		return hr;
	}

	// Copy data to DestSurface
	hr = D3DERR_INVALIDCALL;
	if (rcClient.left == 0 && rcClient.top == 0 && (LONG)Desc.Width == rcClient.right && (LONG)Desc.Height == rcClient.bottom)
	{
		POINT PointDest = { 0, 0 };
		hr = CopyRects(pSrcSurface.Get(), &RectSrc, 1, pDestSurface, &PointDest);
	}

	// Try using StretchRect
	if (FAILED(hr))
	{
		ComPtr<IDirect3DSurface9> pTmpSurface;
		if (SUCCEEDED(ProxyInterface->CreateOffscreenPlainSurface(Desc.Width, Desc.Height, Desc.Format, Desc.Pool, pTmpSurface.GetAddressOf(), nullptr)))
		{
			if (SUCCEEDED(ProxyInterface->StretchRect(pSrcSurface.Get(), &RectSrc, pTmpSurface.Get(), nullptr, D3DTEXF_NONE)))
			{
				POINT PointDest = { 0, 0 };
				RECT Rect = { 0, 0, (LONG)Desc.Width, (LONG)Desc.Height };
				hr = CopyRects(pTmpSurface.Get(), &Rect, 1, pDestSurface, &PointDest);
			}
		}
	}

	// Release surface
	return hr;
}

HRESULT m_IDirect3DDevice9Ex::GetRenderTargetData(THIS_ IDirect3DSurface9* pRenderTarget, IDirect3DSurface9* pDestSurface)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	ScopedCriticalSection ThreadLock(&SHARED.d9cs, RequirePresentHandling());

	if (pRenderTarget)
	{
		pRenderTarget = static_cast<m_IDirect3DSurface9 *>(pRenderTarget)->GetNonMultiSampledSurface(nullptr, 0);
	}

	if (pDestSurface)
	{
		pDestSurface = static_cast<m_IDirect3DSurface9 *>(pDestSurface)->GetProxyInterface();
	}

	return ProxyInterface->GetRenderTargetData(pRenderTarget, pDestSurface);
}

HRESULT m_IDirect3DDevice9Ex::UpdateSurface(THIS_ IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestinationSurface, CONST POINT* pDestPoint)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (pSourceSurface)
	{
		pSourceSurface = static_cast<m_IDirect3DSurface9 *>(pSourceSurface)->GetProxyInterface();
	}

	m_IDirect3DSurface9* m_pDestinationSurface = static_cast<m_IDirect3DSurface9 *>(pDestinationSurface);
	if (pDestinationSurface)
	{
		RECT Rect = (pSourceRect && pDestPoint) ?
			RECT{ pDestPoint->x, pDestPoint->y, pDestPoint->x + (pSourceRect->right - pSourceRect->left), pDestPoint->y + (pSourceRect->bottom - pSourceRect->top) } : RECT{};
		pDestinationSurface = m_pDestinationSurface->GetNonMultiSampledSurface((pSourceRect && pDestPoint) ? &Rect : nullptr, 0);
	}

	HRESULT hr = ProxyInterface->UpdateSurface(pSourceSurface, pSourceRect, pDestinationSurface, pDestPoint);

	if (SUCCEEDED(hr))
	{
		m_pDestinationSurface->RestoreMultiSampleData();
	}

	return hr;
}

HRESULT m_IDirect3DDevice9Ex::SetDialogBoxMode(THIS_ BOOL bEnableDialogs)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->SetDialogBoxMode(bEnableDialogs);
}

HRESULT m_IDirect3DDevice9Ex::GetSwapChain(THIS_ UINT iSwapChain, IDirect3DSwapChain9** ppSwapChain)
{
	// Add 16 bytes for Steam Overlay Fix
	my_nop();
	my_nop();
	my_nop();
	my_nop();
	my_nop();
	my_nop();
	my_nop();
	my_nop();
	my_nop();
	my_nop();
	my_nop();
	my_nop();
	my_nop();
	my_nop();
	my_nop();
	my_nop();

	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ppSwapChain)
	{
		return D3DERR_INVALIDCALL;
	}

	HRESULT hr = ProxyInterface->GetSwapChain(iSwapChain, ppSwapChain);

	if (SUCCEEDED(hr))
	{
		IDirect3DSwapChain9* pSwapChainQuery = nullptr;

		if (WrapperID == IID_IDirect3DDevice9Ex && SUCCEEDED((*ppSwapChain)->QueryInterface(IID_IDirect3DSwapChain9Ex, (LPVOID*)&pSwapChainQuery)))
		{
			(*ppSwapChain)->Release();

			*ppSwapChain = SHARED.ProxyAddressLookupTable9.FindAddress<m_IDirect3DSwapChain9Ex, m_IDirect3DDevice9Ex, LPVOID>(pSwapChainQuery, this, IID_IDirect3DSwapChain9Ex, nullptr);
		}
		else
		{
			*ppSwapChain = SHARED.ProxyAddressLookupTable9.FindAddress<m_IDirect3DSwapChain9Ex, m_IDirect3DDevice9Ex, LPVOID>(*ppSwapChain, this, IID_IDirect3DSwapChain9, nullptr);
		}
	}

	return hr;
}

HRESULT m_IDirect3DDevice9Ex::SetConvolutionMonoKernel(THIS_ UINT width, UINT height, float* rows, float* columns)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterfaceEx)
	{
		Logging::Log() << __FUNCTION__ << " Error: Calling extension function from a non-extension device!";
		return D3DERR_INVALIDCALL;
	}

	return ProxyInterfaceEx->SetConvolutionMonoKernel(width, height, rows, columns);
}

HRESULT m_IDirect3DDevice9Ex::ComposeRects(THIS_ IDirect3DSurface9* pSrc, IDirect3DSurface9* pDst, IDirect3DVertexBuffer9* pSrcRectDescs, UINT NumRects, IDirect3DVertexBuffer9* pDstRectDescs, D3DCOMPOSERECTSOP Operation, int Xoffset, int Yoffset)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterfaceEx)
	{
		Logging::Log() << __FUNCTION__ << " Error: Calling extension function from a non-extension device!";
		return D3DERR_INVALIDCALL;
	}

	if (pSrc)
	{
		pSrc = static_cast<m_IDirect3DSurface9 *>(pSrc)->GetProxyInterface();
	}

	if (pDst)
	{
		pDst = static_cast<m_IDirect3DSurface9 *>(pDst)->GetProxyInterface();
	}

	if (pSrcRectDescs)
	{
		pSrcRectDescs = static_cast<m_IDirect3DVertexBuffer9 *>(pSrcRectDescs)->GetProxyInterface();
	}

	if (pDstRectDescs)
	{
		pDstRectDescs = static_cast<m_IDirect3DVertexBuffer9 *>(pDstRectDescs)->GetProxyInterface();
	}

	return ProxyInterfaceEx->ComposeRects(pSrc, pDst, pSrcRectDescs, NumRects, pDstRectDescs, Operation, Xoffset, Yoffset);
}

HRESULT m_IDirect3DDevice9Ex::PresentEx(THIS_ CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	ApplyPrePresentFixes();

	HRESULT hr = ProxyInterfaceEx->PresentEx(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion, dwFlags);

	if (SUCCEEDED(hr))
	{
		ApplyPostPresentFixes();
	}

	return hr;
}

HRESULT m_IDirect3DDevice9Ex::GetGPUThreadPriority(THIS_ INT* pPriority)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterfaceEx)
	{
		Logging::Log() << __FUNCTION__ << " Error: Calling extension function from a non-extension device!";
		return D3DERR_INVALIDCALL;
	}

	return ProxyInterfaceEx->GetGPUThreadPriority(pPriority);
}

HRESULT m_IDirect3DDevice9Ex::SetGPUThreadPriority(THIS_ INT Priority)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterfaceEx)
	{
		Logging::Log() << __FUNCTION__ << " Error: Calling extension function from a non-extension device!";
		return D3DERR_INVALIDCALL;
	}

	return ProxyInterfaceEx->SetGPUThreadPriority(Priority);
}

HRESULT m_IDirect3DDevice9Ex::WaitForVBlank(THIS_ UINT iSwapChain)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterfaceEx)
	{
		Logging::Log() << __FUNCTION__ << " Error: Calling extension function from a non-extension device!";
		return D3DERR_INVALIDCALL;
	}

	return ProxyInterfaceEx->WaitForVBlank(iSwapChain);
}

HRESULT m_IDirect3DDevice9Ex::CheckResourceResidency(THIS_ IDirect3DResource9** pResourceArray, UINT32 NumResources)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterfaceEx)
	{
		Logging::Log() << __FUNCTION__ << " Error: Calling extension function from a non-extension device!";
		return D3DERR_INVALIDCALL;
	}

	if (pResourceArray)
	{
		for (UINT32 i = 0; i < NumResources; i++)
		{
			if (pResourceArray[i])
			{
				switch (pResourceArray[i]->GetType())
				{
				case D3DRTYPE_SURFACE:
					pResourceArray[i] = static_cast<m_IDirect3DSurface9 *>(pResourceArray[i])->GetProxyInterface();
					break;
				case D3DRTYPE_VOLUME:
					pResourceArray[i] = (IDirect3DResource9*)reinterpret_cast<m_IDirect3DVolume9 *>(pResourceArray[i])->GetProxyInterface();
					break;
				case D3DRTYPE_TEXTURE:
					pResourceArray[i] = static_cast<m_IDirect3DTexture9 *>(pResourceArray[i])->GetProxyInterface();
					break;
				case D3DRTYPE_VOLUMETEXTURE:
					pResourceArray[i] = static_cast<m_IDirect3DVolumeTexture9 *>(pResourceArray[i])->GetProxyInterface();
					break;
				case D3DRTYPE_CUBETEXTURE:
					pResourceArray[i] = static_cast<m_IDirect3DCubeTexture9 *>(pResourceArray[i])->GetProxyInterface();
					break;
				case D3DRTYPE_VERTEXBUFFER:
					pResourceArray[i] = static_cast<m_IDirect3DVertexBuffer9 *>(pResourceArray[i])->GetProxyInterface();
					break;
				case D3DRTYPE_INDEXBUFFER:
					pResourceArray[i] = static_cast<m_IDirect3DIndexBuffer9 *>(pResourceArray[i])->GetProxyInterface();
					break;
				default:
					return D3DERR_INVALIDCALL;
				}
			}
		}
	}

	return ProxyInterfaceEx->CheckResourceResidency(pResourceArray, NumResources);
}

HRESULT m_IDirect3DDevice9Ex::SetMaximumFrameLatency(THIS_ UINT MaxLatency)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterfaceEx)
	{
		Logging::Log() << __FUNCTION__ << " Error: Calling extension function from a non-extension device!";
		return D3DERR_INVALIDCALL;
	}

	return ProxyInterfaceEx->SetMaximumFrameLatency(MaxLatency);
}

HRESULT m_IDirect3DDevice9Ex::GetMaximumFrameLatency(THIS_ UINT* pMaxLatency)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterfaceEx)
	{
		Logging::Log() << __FUNCTION__ << " Error: Calling extension function from a non-extension device!";
		return D3DERR_INVALIDCALL;
	}

	return ProxyInterfaceEx->GetMaximumFrameLatency(pMaxLatency);
}

HRESULT m_IDirect3DDevice9Ex::CheckDeviceState(THIS_ HWND hDestinationWindow)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterfaceEx)
	{
		Logging::Log() << __FUNCTION__ << " Error: Calling extension function from a non-extension device!";
		return D3DERR_INVALIDCALL;
	}

	return ProxyInterfaceEx->CheckDeviceState(hDestinationWindow);
}

HRESULT m_IDirect3DDevice9Ex::CreateRenderTargetEx(THIS_ UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle, DWORD Usage)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterfaceEx)
	{
		Logging::Log() << __FUNCTION__ << " Error: Calling extension function from a non-extension device!";
		return D3DERR_INVALIDCALL;
	}

	if (!ppSurface)
	{
		return D3DERR_INVALIDCALL;
	}

	HRESULT hr = D3DERR_INVALIDCALL;

	// Test for Multisample
	if (SHARED.DeviceMultiSampleFlag)
	{
		hr = ProxyInterfaceEx->CreateRenderTargetEx(Width, Height, Format, SHARED.DeviceMultiSampleType, SHARED.DeviceMultiSampleQuality, FALSE, ppSurface, pSharedHandle, Usage);
	}

	if (FAILED(hr))
	{
		hr = ProxyInterfaceEx->CreateRenderTargetEx(Width, Height, Format, MultiSample, MultisampleQuality, Lockable, ppSurface, pSharedHandle, Usage);

		if (SUCCEEDED(hr) && SHARED.DeviceMultiSampleFlag)
		{
			LOG_LIMIT(3, __FUNCTION__ << " Disabling AntiAliasing for Render Target...");
		}
	}

	if (SUCCEEDED(hr))
	{
		*ppSurface = SHARED.ProxyAddressLookupTable9.FindCreateAddress<m_IDirect3DSurface9, m_IDirect3DDevice9Ex, LPVOID>(*ppSurface, this, IID_IDirect3DSurface9, nullptr);
		return D3D_OK;
	}

	Logging::LogDebug() << __FUNCTION__ << " FAILED! " << (D3DERR)hr << " " << Width << " " << Height << " " << Format << " " << MultiSample << " " << MultisampleQuality << " " << Lockable << " " << pSharedHandle << " " << Usage;
	return hr;
}

HRESULT m_IDirect3DDevice9Ex::CreateOffscreenPlainSurfaceEx(THIS_ UINT Width, UINT Height, D3DFORMAT Format, D3DPOOL Pool, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle, DWORD Usage)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterfaceEx)
	{
		Logging::Log() << __FUNCTION__ << " Error: Calling extension function from a non-extension device!";
		return D3DERR_INVALIDCALL;
	}

	if (!ppSurface)
	{
		return D3DERR_INVALIDCALL;
	}

	if (Config.D3d9to9Ex)
	{
		if (Pool == D3DPOOL_DEFAULT && Usage == 0)
		{
			Usage = D3DUSAGE_DYNAMIC;
		}
	}

	// Override stencil format
	if (Config.OverrideStencilFormat && Usage == D3DUSAGE_DEPTHSTENCIL)
	{
		Format = (D3DFORMAT)Config.OverrideStencilFormat;
	}

	HRESULT hr = ProxyInterfaceEx->CreateOffscreenPlainSurfaceEx(Width, Height, Format, Pool, ppSurface, pSharedHandle, Usage);

	if (SUCCEEDED(hr))
	{
		*ppSurface = SHARED.ProxyAddressLookupTable9.FindCreateAddress<m_IDirect3DSurface9, m_IDirect3DDevice9Ex, LPVOID>(*ppSurface, this, IID_IDirect3DSurface9, nullptr);
		return D3D_OK;
	}

	Logging::LogDebug() << __FUNCTION__ << " FAILED! " << (D3DERR)hr << " " << Width << " " << Height << " " << Format << " " << Pool << " " << pSharedHandle;
	return hr;
}

HRESULT m_IDirect3DDevice9Ex::CreateDepthStencilSurfaceEx(THIS_ UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Discard, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle, DWORD Usage)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterfaceEx)
	{
		Logging::Log() << __FUNCTION__ << " Error: Calling extension function from a non-extension device!";
		return D3DERR_INVALIDCALL;
	}

	if (!ppSurface)
	{
		return D3DERR_INVALIDCALL;
	}

	// Override stencil format
	if (Config.OverrideStencilFormat)
	{
		Format = (D3DFORMAT)Config.OverrideStencilFormat;
		LOG_LIMIT(100, __FUNCTION__ << " Setting Stencil format: " << Format);
	}

	HRESULT hr = D3DERR_INVALIDCALL;

	// Test for Multisample
	if (SHARED.DeviceMultiSampleFlag)
	{
		hr = ProxyInterfaceEx->CreateDepthStencilSurfaceEx(Width, Height, Format, SHARED.DeviceMultiSampleType, SHARED.DeviceMultiSampleQuality, TRUE, ppSurface, pSharedHandle, Usage);
	}

	if (FAILED(hr))
	{
		hr = ProxyInterfaceEx->CreateDepthStencilSurfaceEx(Width, Height, Format, MultiSample, MultisampleQuality, Discard, ppSurface, pSharedHandle, Usage);

		if (SUCCEEDED(hr) && SHARED.DeviceMultiSampleFlag)
		{
			LOG_LIMIT(3, __FUNCTION__ << " Disabling AntiAliasing for Depth Stencil...");
		}
	}

	if (SUCCEEDED(hr))
	{
		*ppSurface = SHARED.ProxyAddressLookupTable9.FindCreateAddress<m_IDirect3DSurface9, m_IDirect3DDevice9Ex, LPVOID>(*ppSurface, this, IID_IDirect3DSurface9, nullptr);
		return D3D_OK;
	}

	Logging::LogDebug() << __FUNCTION__ << " FAILED! " << (D3DERR)hr << " " << Width << " " << Height << " " << Format << " " << MultiSample << " " << MultisampleQuality << " " << Discard << " " << pSharedHandle << " " << Usage;
	return hr;
}

HRESULT m_IDirect3DDevice9Ex::ResetEx(THIS_ D3DPRESENT_PARAMETERS* pPresentationParameters, D3DDISPLAYMODEEX *pFullscreenDisplayMode)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!pPresentationParameters)
	{
		return D3DERR_INVALIDCALL;
	}

	return ResetT<fResetEx>(nullptr, pPresentationParameters, true, pFullscreenDisplayMode);
}

HRESULT m_IDirect3DDevice9Ex::GetDisplayModeEx(THIS_ UINT iSwapChain, D3DDISPLAYMODEEX* pMode, D3DDISPLAYROTATION* pRotation)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterfaceEx)
	{
		Logging::Log() << __FUNCTION__ << " Error: Calling extension function from a non-extension device!";
		return D3DERR_INVALIDCALL;
	}

	return ProxyInterfaceEx->GetDisplayModeEx(iSwapChain, pMode, pRotation);
}

// Runs when device is created and on every successful Reset()
void m_IDirect3DDevice9Ex::ReInitInterface()
{
	Utils::GetScreenSize(SHARED.hMonitor, SHARED.screenWidth, SHARED.screenHeight);

	IsGammaSet = false;
	for (int i = 0; i < 256; ++i)
	{
		WORD value = static_cast<WORD>(i * 65535 / 255); // Linear interpolation from 0 to 65535
		RampData.red[i] = value;
		RampData.green[i] = value;
		RampData.blue[i] = value;
		DefaultRampData.red[i] = value;
		DefaultRampData.green[i] = value;
		DefaultRampData.blue[i] = value;
	}

	ShadowBackbuffer.ReleaseAll();

	if (!SHARED.IsDirectDrawDevice && Config.UseShadowBackbuffer)
	{
		CreateShadowBackbuffer();
	}
}

void m_IDirect3DDevice9Ex::CreateShadowBackbuffer()
{
	if (!SHARED.BackBufferCount)
	{
		Logging::Log() << __FUNCTION__ << " Error: too small BackBufferCount: " << SHARED.BackBufferCount;
		return;
	}

	DWORD BackBufferCount = max(2, SHARED.BackBufferCount);

	BackBufferList.clear();

	D3DSURFACE_DESC Desc = {};
	{
		ComPtr<IDirect3DSurface9> pBackbuffer;
		if (FAILED(ProxyInterface->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, pBackbuffer.GetAddressOf())))
		{
			Logging::Log() << __FUNCTION__ << " Error: failed to get Back Buffer!";
			return;
		}
		if (FAILED(pBackbuffer.Get()->GetDesc(&Desc)))
		{
			Logging::Log() << __FUNCTION__ << " Error: failed to get surface Desc!";
			return;
		}
	}

	ShadowBackbuffer.Initialize(BackBufferCount);

	for (size_t i = 0; i < BackBufferCount; ++i)
	{
		m_IDirect3DSurface9* surf = nullptr;
		if (FAILED(CreateRenderTarget(Desc.Width, Desc.Height, Desc.Format, Desc.MultiSampleType, Desc.MultiSampleQuality, FALSE, reinterpret_cast<IDirect3DSurface9**>(&surf), nullptr)))
		{
			Logging::Log() << __FUNCTION__ << " Error: failed to create render target!";
			ShadowBackbuffer.ReleaseAll();
			return;
		}
		ShadowBackbuffer.SetSurface(i, surf);

		ComPtr<IDirect3DSurface9> pBackbuffer;
		if (SUCCEEDED(ProxyInterface->GetBackBuffer(0, i, D3DBACKBUFFER_TYPE_MONO, pBackbuffer.GetAddressOf())))
		{
			BackBufferList.push_back(pBackbuffer.Get());
		}
	}

	ProxyInterface->SetRenderTarget(0, ShadowBackbuffer.GetCurrentBackBuffer()->GetProxyInterface());
}

void m_IDirect3DDevice9Ex::ReleaseShadowBackbuffer()
{
	if (ShadowBackbuffer.Count())
	{
		ComPtr<IDirect3DSurface9> pBackbuffer;
		if (SUCCEEDED(ProxyInterface->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, pBackbuffer.GetAddressOf())))
		{
			ProxyInterface->SetRenderTarget(0, pBackbuffer.Get());
		}

		ShadowBackbuffer.ReleaseAll();
	}
}

void m_IDirect3DDevice9Ex::ModeExToMode(D3DDISPLAYMODEEX& ModeEx, D3DDISPLAYMODE& Mode)
{
	Mode.Width = ModeEx.Width;
	Mode.Height = ModeEx.Height;
	Mode.RefreshRate = ModeEx.RefreshRate;
	Mode.Format = ModeEx.Format;
}

void m_IDirect3DDevice9Ex::LimitFrameRate() const
{
	// Count the number of frames
	SHARED.Counter.FrameCounter++;

	// Get performance frequency once
	static const LARGE_INTEGER Frequency = [] {
		LARGE_INTEGER freq = {};
		QueryPerformanceFrequency(&freq);
		return freq;
		}();
	static const LONGLONG TicksPerMS = Frequency.QuadPart / 1000;

	// Calculate time per frame in ticks
	static long double PerFrameFPS = Config.LimitPerFrameFPS;
	static LONGLONG PerFrameTicks = static_cast<LONGLONG>(static_cast<long double>(Frequency.QuadPart) / PerFrameFPS);

	// Get current time
	LARGE_INTEGER ClickTime = {};
	QueryPerformanceCounter(&ClickTime);

	LONGLONG TargetEndTicks = SHARED.Counter.LastPresentTime.QuadPart + PerFrameTicks;

	// First frame or if we fell behind, reset base time
	if (SHARED.Counter.LastPresentTime.QuadPart == 0 || ClickTime.QuadPart >= TargetEndTicks)
	{
		SHARED.Counter.LastPresentTime.QuadPart = ClickTime.QuadPart;
		return;
	}

	// Wait until target time
	while (true)
	{
		QueryPerformanceCounter(&ClickTime);
		LONGLONG RemainingTicks = TargetEndTicks - ClickTime.QuadPart;

		if (RemainingTicks <= 0) break;

		// Busy wait until we reach the target time
		Utils::BusyWaitYield(static_cast<DWORD>(RemainingTicks / TicksPerMS));
	}

	// Store target time for next frame
	SHARED.Counter.LastPresentTime.QuadPart = TargetEndTicks;
}

void m_IDirect3DDevice9Ex::CalculateFPS() const
{
	// Calculate frame time
	auto endTime = std::chrono::steady_clock::now();
	auto newstart = std::chrono::steady_clock::now();
	std::chrono::duration<double> frameTime = endTime - SHARED.startTime;
	SHARED.startTime = newstart;

	// Store the frame time along with the time it occurred
	SHARED.frameTimes.emplace_back(endTime, frameTime);

	// Remove frame times older than FPS_CALCULATION_WINDOW
	while (!SHARED.frameTimes.empty() && (endTime - SHARED.frameTimes.front().first) > FPS_CALCULATION_WINDOW)
	{
		SHARED.frameTimes.pop_front();
	}

	if (SHARED.frameTimes.empty())
	{
		// No frame times available
		return;
	}

	double totalTime = 0.0;
	for (const auto& entry : SHARED.frameTimes)
	{
		totalTime += entry.second.count();
	}

	// Calculate average frame time
	double averageFrameTime = totalTime / SHARED.frameTimes.size();

	// Calculate FPS
	if (averageFrameTime > 0.0)
	{
		SHARED.AverageFPSCounter = 1.0 / averageFrameTime;
	}

#ifdef ENABLE_DEBUGOVERLAY
	DOverlay.SetFPSCount(SHARED.AverageFPSCounter);
#endif

	// Output FPS
	Logging::LogDebug() << "Frames: " << SHARED.frameTimes.size() << " Average time: " << averageFrameTime << " FPS: " << SHARED.AverageFPSCounter;
}

void m_IDirect3DDevice9Ex::DrawFPS(float fps, const RECT& presentRect, DWORD position)
{
	// Scale the font size based on the rect height (adjustable factor)
	int fontSize = SHARED.BufferHeight / 40;
	if (fontSize < 4) fontSize = 4;		// Minimum font size
	if (fontSize > 128) fontSize = 128;	// Maximum font size

	// Create the font if not created
	if (!pFont &&
		FAILED(D3DXCreateFontW(ProxyInterface, fontSize, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET,
			OUT_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
			L"Arial", &pFont)))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: failed to create font!");
		return;
	}

	// Create the sprite if not created
	if (!pSprite && FAILED(D3DXCreateSprite(ProxyInterface, &pSprite)))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: failed to create sprite!");
		return;
	}

	// Format FPS text
	wchar_t fpsText[16];
	swprintf(fpsText, 16, L"FPS: %.1f", fps);

	// Determine text position based on 'position'
	int padding = fontSize / 2;
	RECT textRect = presentRect;

	switch (position)
	{
	case 1: // Top-left
		textRect.left = presentRect.left + padding;
		textRect.top = presentRect.top + padding;
		break;
	case 2: // Top-right
		textRect.right = presentRect.right - padding;
		textRect.top = presentRect.top + padding;
		break;
	case 3: // Bottom-right
		textRect.right = presentRect.right - padding;
		textRect.bottom = presentRect.bottom - padding;
		break;
	case 4: // Bottom-left
		textRect.left = presentRect.left + padding;
		textRect.bottom = presentRect.bottom - padding;
		break;
	default: // Default to top-left if an invalid position is given
		textRect.left = presentRect.left + padding;
		textRect.top = presentRect.top + padding;
		break;
	}

	// Set alignment flags based on position
	DWORD alignment = 0;
	if (position == 2 || position == 3)
		alignment |= DT_RIGHT;
	else
		alignment |= DT_LEFT;

	if (position == 3 || position == 4)
		alignment |= DT_BOTTOM;
	else
		alignment |= DT_TOP;

	// Start drawing
	pSprite->Begin(D3DXSPRITE_ALPHABLEND | D3DXSPRITE_SORT_TEXTURE | D3DXSPRITE_DONOTSAVESTATE);

	// Draw the text
	INT ret = pFont->DrawTextW(pSprite, fpsText, -1, &textRect, alignment, D3DCOLOR_XRGB(247, 247, 0));

	// End drawing
	pSprite->End();

	if (ret == 0)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: could not DrawText!");
	}
}
