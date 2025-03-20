/**
* Copyright (C) 2024 Elisha Riedlinger
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
#include "d3d9\d3d9External.h"
#include "ddraw\Shaders\GammaPixelShader.h"
#include "GDI\WndProc.h"
#include "Utils\Utils.h"
#include <intrin.h>

#ifdef ENABLE_DEBUGOVERLAY
DebugOverlay DOverlay;
#endif

#define SHARED DeviceDetailsMap[DDKey]

std::unordered_map<UINT, DEVICEDETAILS> DeviceDetailsMap;

HRESULT m_IDirect3DDevice9Ex::QueryInterface(REFIID riid, void** ppvObj)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ") " << riid;

	if (riid == IID_IUnknown || riid == WrapperID)
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

		if (ppvObj)
		{
			*ppvObj = nullptr;
		}

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

	ReleaseResources(false);

	ULONG ref = ProxyInterface->Release();

#ifdef ENABLE_DEBUGOVERLAY
	// Teardown debug overlay before destroying device
	if (Config.EnableImgui && ref == 1 && DOverlay.IsSetup() && DOverlay.Getd3d9Device() == ProxyInterface)
	{
		ProxyInterface->AddRef();
		DOverlay.Shutdown();
		ref = ProxyInterface->Release();
	}
#endif

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
			for (auto it = DeviceDetailsMap.begin(); it != DeviceDetailsMap.end(); ++it)
			{
				if (it->first == DDKey)
				{
					DeviceDetailsMap.erase(it);
					break;
				}
			}
		}
	}

	return ref;
}

inline void m_IDirect3DDevice9Ex::ClearVars(D3DPRESENT_PARAMETERS* pPresentationParameters) const
{
	UNREFERENCED_PARAMETER(pPresentationParameters);

	// Clear variables
	ZeroMemory(&SHARED.Caps, sizeof(D3DCAPS9));
	SHARED.MaxAnisotropy = 0;
	SHARED.isAnisotropySet = false;
	SHARED.AnisotropyDisabledFlag = false;
	SHARED.isClipPlaneSet = false;
	SHARED.m_clipPlaneRenderState = 0;
}

template <typename T>
inline HRESULT m_IDirect3DDevice9Ex::ResetT(T func, D3DPRESENT_PARAMETERS* pPresentationParameters, D3DDISPLAYMODEEX* pFullscreenDisplayMode)
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
		WndDataStruct->IsExclusiveMode = !pPresentationParameters->Windowed;
	}

	HRESULT hr;

	// Check fullscreen
	bool ForceFullscreen = false;
	if (m_pD3DEx)
	{
		ForceFullscreen = m_pD3DEx->TestResolution(D3DADAPTER_DEFAULT, pPresentationParameters->BackBufferWidth, pPresentationParameters->BackBufferHeight);
	}

	// Setup presentation parameters
	D3DPRESENT_PARAMETERS d3dpp;
	CopyMemory(&d3dpp, pPresentationParameters, sizeof(D3DPRESENT_PARAMETERS));
	UpdatePresentParameter(&d3dpp, nullptr, SHARED, ForceFullscreen, true);

	bool IsWindowMode = d3dpp.Windowed != FALSE;

	// Test for Multisample
	if (SHARED.DeviceMultiSampleFlag)
	{
		do {

			// Update Present Parameter for Multisample
			UpdatePresentParameterForMultisample(&d3dpp, SHARED.DeviceMultiSampleType, SHARED.DeviceMultiSampleQuality);

			// Reset device
			hr = ResetT(func, &d3dpp, pFullscreenDisplayMode);

			// Check if device was reset successfully
			if (SUCCEEDED(hr))
			{
				break;
			}

			// Reset presentation parameters
			CopyMemory(&d3dpp, pPresentationParameters, sizeof(D3DPRESENT_PARAMETERS));
			UpdatePresentParameter(&d3dpp, nullptr, SHARED, ForceFullscreen, false);

			// Reset device
			hr = ResetT(func, &d3dpp, pFullscreenDisplayMode);

			if (SUCCEEDED(hr))
			{
				LOG_LIMIT(3, __FUNCTION__ << " Disabling AntiAliasing...");
				SHARED.DeviceMultiSampleFlag = false;
				SHARED.DeviceMultiSampleType = D3DMULTISAMPLE_NONE;
				SHARED.DeviceMultiSampleQuality = 0;
				SHARED.SetSSAA = false;
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
		GetFinalPresentParameter(&d3dpp, SHARED);

		if (WndDataStruct && WndDataStruct->IsExclusiveMode)
		{
			d3dpp.Windowed = FALSE;
		}

		SHARED.IsWindowMode = IsWindowMode;

		CopyMemory(pPresentationParameters, &d3dpp, sizeof(D3DPRESENT_PARAMETERS));

		ClearVars(pPresentationParameters);

		ReInitInterface();
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

	return ResetT<fReset>(nullptr, pPresentationParameters);
}

HRESULT m_IDirect3DDevice9Ex::CallEndScene()
{
	// clear Begin/End Scene flags
	SHARED.IsInScene = false;
	SHARED.BeginSceneCalled = false;

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
		if (SHARED.IsInScene)
		{
			SHARED.IsInScene = false;

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
		ForceFullscreen = m_pD3DEx->TestResolution(D3DADAPTER_DEFAULT, pPresentationParameters->BackBufferWidth, pPresentationParameters->BackBufferHeight);
	}

	// Setup presentation parameters
	D3DPRESENT_PARAMETERS d3dpp;
	CopyMemory(&d3dpp, pPresentationParameters, sizeof(D3DPRESENT_PARAMETERS));
	UpdatePresentParameter(&d3dpp, nullptr, SHARED, ForceFullscreen, false);

	// Test for Multisample
	if (SHARED.DeviceMultiSampleFlag)
	{
		// Update Present Parameter for Multisample
		UpdatePresentParameterForMultisample(&d3dpp, SHARED.DeviceMultiSampleType, SHARED.DeviceMultiSampleQuality);

		// Create CwapChain
		hr = ProxyInterface->CreateAdditionalSwapChain(&d3dpp, ppSwapChain);
	}
	
	if (FAILED(hr))
	{
		CopyMemory(&d3dpp, pPresentationParameters, sizeof(D3DPRESENT_PARAMETERS));
		UpdatePresentParameter(&d3dpp, nullptr, SHARED, ForceFullscreen, false);

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

			*ppSwapChain = new m_IDirect3DSwapChain9Ex((IDirect3DSwapChain9Ex*)pSwapChainQuery, this, IID_IDirect3DSwapChain9Ex);
		}
		else
		{
			*ppSwapChain = new m_IDirect3DSwapChain9Ex((IDirect3DSwapChain9Ex*)*ppSwapChain, this, IID_IDirect3DSwapChain9);
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

	HRESULT hr = ProxyInterface->CreateCubeTexture(EdgeLength, Levels, Usage, Format, Pool, ppCubeTexture, pSharedHandle);

	if (SUCCEEDED(hr))
	{
		*ppCubeTexture = new m_IDirect3DCubeTexture9(*ppCubeTexture, this);
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
		*ppSurface = new m_IDirect3DSurface9(*ppSurface, this);
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

	HRESULT hr = ProxyInterface->CreateIndexBuffer(Length, Usage, Format, Pool, ppIndexBuffer, pSharedHandle);

	if (SUCCEEDED(hr))
	{
		*ppIndexBuffer = new m_IDirect3DIndexBuffer9(*ppIndexBuffer, this);
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
		*ppSurface = new m_IDirect3DSurface9(*ppSurface, this);
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

	HRESULT hr = ProxyInterface->CreateTexture(Width, Height, Levels, Usage, Format, Pool, ppTexture, pSharedHandle);

	if (SUCCEEDED(hr))
	{
		*ppTexture = new m_IDirect3DTexture9(*ppTexture, this);
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

	HRESULT hr = ProxyInterface->CreateVertexBuffer(Length, Usage, FVF, Pool, ppVertexBuffer, pSharedHandle);

	if (SUCCEEDED(hr))
	{
		*ppVertexBuffer = new m_IDirect3DVertexBuffer9(*ppVertexBuffer, this);
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

	HRESULT hr = ProxyInterface->CreateVolumeTexture(Width, Height, Depth, Levels, Usage, Format, Pool, ppVolumeTexture, pSharedHandle);

	if (SUCCEEDED(hr))
	{
		*ppVolumeTexture = new m_IDirect3DVolumeTexture9(*ppVolumeTexture, this);
		return D3D_OK;
	}

	Logging::LogDebug() << __FUNCTION__ << " FAILED! " << (D3DERR)hr << " " << Width << " " << Height << " " << Depth << " " << Levels << " " << Usage << " " << Format << " " << Pool << " " << pSharedHandle;
	return hr;
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
		m_IDirect3DStateBlock9* StateBlockX = new m_IDirect3DStateBlock9(*ppSB, this);

		if (Config.LimitStateBlocks)
		{
			SHARED.StateBlockTable.AddStateBlock(StateBlockX);

			StateBlockX->SetDDKey(DDKey);
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
		m_IDirect3DStateBlock9* StateBlockX = SHARED.ProxyAddressLookupTable9.FindAddress<m_IDirect3DStateBlock9, m_IDirect3DDevice9Ex, LPVOID>(*ppSB, this, IID_IDirect3DStateBlock9, nullptr);

		if (Config.LimitStateBlocks)
		{
			SHARED.StateBlockTable.AddStateBlock(StateBlockX);

			StateBlockX->SetDDKey(DDKey);
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

	HRESULT hr = ProxyInterface->GetRenderTarget(RenderTargetIndex, ppRenderTarget);

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

	HRESULT hr = ProxyInterface->SetRenderState(State, Value);

	// CacheClipPlane
	if (SUCCEEDED(hr) && State == D3DRS_CLIPPLANEENABLE)
	{
		SHARED.m_clipPlaneRenderState = Value;
	}

	return hr;
}

HRESULT m_IDirect3DDevice9Ex::SetRenderTarget(THIS_ DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (pRenderTarget)
	{
		pRenderTarget = static_cast<m_IDirect3DSurface9 *>(pRenderTarget)->GetProxyInterface();
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
		if (Config.EnvironmentMapCubeFix)
		{
			CheckTransformForCubeMap(State, pMatrix);
		}
	}

	return hr;
}

inline HRESULT m_IDirect3DDevice9Ex::SetBrightnessLevel(D3DGAMMARAMP& Ramp)
{
	Logging::LogDebug() << __FUNCTION__;

	// Create or update the gamma LUT texture
	if (!SHARED.GammaLUTTexture)
	{
		if (SUCCEEDED(ProxyInterface->CreateTexture(256, 1, 1, 0, D3DFMT_A32B32G32R32F, D3DPOOL_MANAGED, &SHARED.GammaLUTTexture, nullptr)))
		{
			SHARED.UsingShader32f = true;
		}
		else
		{
			ProxyInterface->CreateTexture(256, 1, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &SHARED.GammaLUTTexture, nullptr);
			SHARED.UsingShader32f = false;
		}
	}

	D3DLOCKED_RECT lockedRect;
	if (SUCCEEDED(SHARED.GammaLUTTexture->LockRect(0, &lockedRect, nullptr, D3DLOCK_DISCARD)))
	{
		if (SHARED.UsingShader32f)
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
		SHARED.GammaLUTTexture->UnlockRect(0);
	}

	return D3D_OK;
}

inline LPDIRECT3DPIXELSHADER9 m_IDirect3DDevice9Ex::GetGammaPixelShader() const
{
	// Create pixel shaders
	if (!SHARED.gammaPixelShader)
	{
		ProxyInterface->CreatePixelShader((DWORD*)GammaPixelShaderSrc, &SHARED.gammaPixelShader);
	}
	return SHARED.gammaPixelShader;
}

inline void m_IDirect3DDevice9Ex::ApplyBrightnessLevel()
{
	if (!SHARED.GammaLUTTexture)
	{
		SetBrightnessLevel(SHARED.RampData);
	}

	// Set shader
	IDirect3DPixelShader9* pShader = GetGammaPixelShader();
	if (!pShader)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Failed to retrieve gamma pixel shader!");
		return;
	}

	// Get current backbuffer
	IDirect3DSurface9* pBackBuffer = nullptr;
	if (FAILED(ProxyInterface->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer)))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Failed to get back buffer!");
		return;
	}

	// Create intermediate texture for shader input
	D3DSURFACE_DESC desc;
	pBackBuffer->GetDesc(&desc);
	if (!SHARED.ScreenCopyTexture)
	{
		if (FAILED(ProxyInterface->CreateTexture(desc.Width, desc.Height, 1, D3DUSAGE_RENDERTARGET, desc.Format, D3DPOOL_DEFAULT, &SHARED.ScreenCopyTexture, nullptr)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Failed to create screen copy texture!");
			pBackBuffer->Release();
			return;
		}
	}

	IDirect3DSurface9* pCopySurface = nullptr;
	SHARED.ScreenCopyTexture->GetSurfaceLevel(0, &pCopySurface);
	if (FAILED(ProxyInterface->StretchRect(pBackBuffer, nullptr, pCopySurface, nullptr, D3DTEXF_NONE)))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Failed to copy render target!");
	}
	pCopySurface->Release();

	// Set texture
	ProxyInterface->SetTexture(0, SHARED.ScreenCopyTexture);
	ProxyInterface->SetTexture(1, SHARED.GammaLUTTexture);

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

	// Cleanup
	pBackBuffer->Release();
}

inline void m_IDirect3DDevice9Ex::ReleaseResources(bool isReset)
{
	if (SHARED.GammaLUTTexture)
	{
		ULONG ref = SHARED.GammaLUTTexture->Release();
		if (ref)
		{
			Logging::Log() << __FUNCTION__ << " Error: there is still a reference to 'GammaLUTTexture' " << ref;
		}
		SHARED.GammaLUTTexture = nullptr;
	}

	if (SHARED.ScreenCopyTexture)
	{
		ULONG ref = SHARED.ScreenCopyTexture->Release();
		if (ref)
		{
			Logging::Log() << __FUNCTION__ << " Error: there is still a reference to 'ScreenCopyTexture' " << ref;
		}
		SHARED.ScreenCopyTexture = nullptr;
	}

	if (SHARED.gammaPixelShader)
	{
		ULONG ref = SHARED.gammaPixelShader->Release();
		if (ref)
		{
			Logging::Log() << __FUNCTION__ << " Error: there is still a reference to 'gammaPixelShader' " << ref;
		}
		SHARED.gammaPixelShader = nullptr;
	}

	if (SHARED.BlankTexture)
	{
		if (SHARED.isBlankTextureUsed)
		{
			SHARED.isBlankTextureUsed = false;
			ProxyInterface->SetTexture(0, nullptr);
		}
		ULONG ref = SHARED.BlankTexture->Release();
		if (ref)
		{
			Logging::Log() << __FUNCTION__ << " Error: there is still a reference to 'BlankTexture' " << ref;
		}
		SHARED.BlankTexture = nullptr;
	}

	if (SHARED.pFont)
	{
		ULONG ref = SHARED.pFont->Release();
		if (ref)
		{
			Logging::Log() << __FUNCTION__ << " Error: there is still a reference to 'gammaPixelShader' " << ref;
		}
		SHARED.pFont = nullptr;
	}

	if (isReset)
	{
		// Clear all state blocks on reset
		SHARED.StateBlockTable.ReleaseAllStateBlocks();

		// Anisotropic Filtering
		SHARED.isAnisotropySet = false;
		SHARED.AnisotropyDisabledFlag = false;

		// clear Begin/End Scene flags
		SHARED.IsInScene = false;
		SHARED.BeginSceneCalled = false;

		// For environment map cube
		std::fill(std::begin(SHARED.isTextureMapCube), std::end(SHARED.isTextureMapCube), false);
		std::fill(std::begin(SHARED.isTransformMapCube), std::end(SHARED.isTransformMapCube), false);
		std::fill(std::begin(SHARED.texCoordIndex), std::end(SHARED.texCoordIndex), 0);
		std::fill(std::begin(SHARED.texTransformFlags), std::end(SHARED.texTransformFlags), 0);
		SHARED.isBlankTextureUsed = false;
		SHARED.pCurrentTexture = nullptr;

		// For CacheClipPlane
		SHARED.isClipPlaneSet = false;
		SHARED.m_clipPlaneRenderState = 0;
		for (int i = 0; i < MAX_CLIP_PLANES; ++i)
		{
			std::fill(std::begin(SHARED.m_storedClipPlanes[i]), std::end(SHARED.m_storedClipPlanes[i]), 0.0f);
		}

		// For gamma
		SHARED.IsGammaSet = false;
		SHARED.UsingShader32f = true;
	}
}

void m_IDirect3DDevice9Ex::GetGammaRamp(THIS_ UINT iSwapChain, D3DGAMMARAMP* pRamp)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (pRamp && Config.WindowModeGammaShader && SHARED.IsWindowMode)
	{
		if (iSwapChain)
		{
			LOG_LIMIT(3, __FUNCTION__ << " Warning: Gamma support for swapchains not implemented: " << iSwapChain);
		}

		memcpy(pRamp, &SHARED.RampData, sizeof(D3DGAMMARAMP));
		return;
	}

	return ProxyInterface->GetGammaRamp(iSwapChain, pRamp);
}

void m_IDirect3DDevice9Ex::SetGammaRamp(THIS_ UINT iSwapChain, DWORD Flags, CONST D3DGAMMARAMP* pRamp)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (pRamp && Config.WindowModeGammaShader && SHARED.IsWindowMode)
	{
		if (iSwapChain)
		{
			LOG_LIMIT(3, __FUNCTION__ << " Warning: Gamma support for swapchains not implemented: " << iSwapChain);
		}

		SHARED.IsGammaSet = false;
		memcpy(&SHARED.RampData, pRamp, sizeof(D3DGAMMARAMP));

		if (memcmp(&SHARED.DefaultRampData, &SHARED.RampData, sizeof(D3DGAMMARAMP)) != S_OK)
		{
			SHARED.IsGammaSet = true;
			SetBrightnessLevel(SHARED.RampData);
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
		*ppShader = new m_IDirect3DPixelShader9(*ppShader, this);
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

void m_IDirect3DDevice9Ex::BackupDeviceState()
{
	// Set render states
	ProxyInterface->GetRenderState(D3DRS_LIGHTING, &ds.rsLighting);
	ProxyInterface->GetRenderState(D3DRS_ALPHATESTENABLE, &ds.rsAlphaTestEnable);
	ProxyInterface->GetRenderState(D3DRS_ALPHABLENDENABLE, &ds.rsAlphaBlendEnable);
	ProxyInterface->GetRenderState(D3DRS_FOGENABLE, &ds.rsFogEnable);
	ProxyInterface->GetRenderState(D3DRS_ZENABLE, &ds.rsZEnable);
	ProxyInterface->GetRenderState(D3DRS_ZWRITEENABLE, &ds.rsZWriteEnable);
	ProxyInterface->GetRenderState(D3DRS_STENCILENABLE, &ds.rsStencilEnable);
	ProxyInterface->GetRenderState(D3DRS_CULLMODE, &ds.rsCullMode);
	ProxyInterface->GetRenderState(D3DRS_CLIPPING, &ds.rsClipping);
	ProxyInterface->SetRenderState(D3DRS_LIGHTING, FALSE);
	ProxyInterface->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	ProxyInterface->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	ProxyInterface->SetRenderState(D3DRS_FOGENABLE, FALSE);
	ProxyInterface->SetRenderState(D3DRS_ZENABLE, FALSE);
	ProxyInterface->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	ProxyInterface->SetRenderState(D3DRS_STENCILENABLE, FALSE);
	ProxyInterface->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	ProxyInterface->SetRenderState(D3DRS_CLIPPING, FALSE);

	// Set texture states
	ProxyInterface->GetTextureStageState(0, D3DTSS_COLOROP, &ds.tsColorOP);
	ProxyInterface->GetTextureStageState(0, D3DTSS_COLORARG1, &ds.tsColorArg1);
	ProxyInterface->GetTextureStageState(0, D3DTSS_COLORARG2, &ds.tsColorArg2);
	ProxyInterface->GetTextureStageState(0, D3DTSS_ALPHAOP, &ds.tsAlphaOP);
	ProxyInterface->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	ProxyInterface->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	ProxyInterface->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CURRENT);
	ProxyInterface->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	// Set sampler states
	for (UINT x = 0; x < 2; x++)
	{
		ProxyInterface->GetSamplerState(x, D3DSAMP_ADDRESSU, &ds.ssaddressU[x]);
		ProxyInterface->GetSamplerState(x, D3DSAMP_ADDRESSV, &ds.ssaddressV[x]);
		ProxyInterface->GetSamplerState(x, D3DSAMP_ADDRESSW, &ds.ssaddressW[x]);
		ProxyInterface->SetSamplerState(x, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
		ProxyInterface->SetSamplerState(x, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
		ProxyInterface->SetSamplerState(x, D3DSAMP_ADDRESSW, D3DTADDRESS_WRAP);
	}

	// Set viewport
	ProxyInterface->GetViewport(&ds.oldViewport);

	ds.newViewport.X = 0;
	ds.newViewport.Y = 0;
	ds.newViewport.Width = SHARED.BufferWidth;
	ds.newViewport.Height = SHARED.BufferHeight;
	ds.newViewport.MinZ = 0.0f;
	ds.newViewport.MaxZ = 1.0f;

	ProxyInterface->SetViewport(&ds.newViewport);

	// Set texture
	for (int x = 0; x < 8; x++)
	{
		ProxyInterface->GetTexture(x, &ds.pTexture[x]);
	}
	for (int x = 0; x < 8; x++)
	{
		ProxyInterface->SetTexture(x, nullptr);
	}

	// Set shader
	ProxyInterface->GetPixelShader(&ds.pPixelShader);
	ProxyInterface->GetVertexShader(&ds.pVertexShader);
	ProxyInterface->SetPixelShader(nullptr);
	ProxyInterface->SetVertexShader(nullptr);

	// Get current render target
	ProxyInterface->GetRenderTarget(0, &ds.pRenderTarget);

	// Set backbuffer as render target
	IDirect3DSurface9* pBackBuffer = nullptr;
	if (SUCCEEDED(ProxyInterface->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer)))
	{
		ProxyInterface->SetRenderTarget(0, pBackBuffer);
		pBackBuffer->Release();
	}
}

void m_IDirect3DDevice9Ex::RestoreDeviceState()
{
	// Ensure we are rendering to the backbuffer
	if (ds.pRenderTarget)
	{
		ProxyInterface->SetRenderTarget(0, ds.pRenderTarget);
		ds.pRenderTarget->Release();
		ds.pRenderTarget = nullptr;
	}

	// Reset textures
	for (int x = 0; x < 8; x++)
	{
		ProxyInterface->SetTexture(x, ds.pTexture[x]);
		if (ds.pTexture[x])
		{
			ds.pTexture[x]->Release();
			ds.pTexture[x] = nullptr;
		}
	}

	// Reset shaders
	ProxyInterface->SetPixelShader(ds.pPixelShader);
	ds.pPixelShader = nullptr;
	ProxyInterface->SetVertexShader(ds.pVertexShader);
	ds.pVertexShader = nullptr;

	// Restore game viewport
	ProxyInterface->SetViewport(&ds.oldViewport);

	// Restore sampler states
	for (UINT x = 0; x < 2; x++)
	{
		ProxyInterface->SetSamplerState(x, D3DSAMP_ADDRESSU, ds.ssaddressU[x]);
		ProxyInterface->SetSamplerState(x, D3DSAMP_ADDRESSV, ds.ssaddressV[x]);
		ProxyInterface->SetSamplerState(x, D3DSAMP_ADDRESSW, ds.ssaddressW[x]);
	}

	// Restore texture states
	ProxyInterface->SetTextureStageState(0, D3DTSS_COLOROP, ds.tsColorOP);
	ProxyInterface->SetTextureStageState(0, D3DTSS_COLORARG1, ds.tsColorArg1);
	ProxyInterface->SetTextureStageState(0, D3DTSS_COLORARG2, ds.tsColorArg2);
	ProxyInterface->SetTextureStageState(0, D3DTSS_ALPHAOP, ds.tsAlphaOP);

	// Restore render states
	ProxyInterface->SetRenderState(D3DRS_LIGHTING, ds.rsLighting);
	ProxyInterface->SetRenderState(D3DRS_ALPHATESTENABLE, ds.rsAlphaTestEnable);
	ProxyInterface->SetRenderState(D3DRS_ALPHABLENDENABLE, ds.rsAlphaBlendEnable);
	ProxyInterface->SetRenderState(D3DRS_FOGENABLE, ds.rsFogEnable);
	ProxyInterface->SetRenderState(D3DRS_ZENABLE, ds.rsZEnable);
	ProxyInterface->SetRenderState(D3DRS_ZWRITEENABLE, ds.rsZWriteEnable);
	ProxyInterface->SetRenderState(D3DRS_STENCILENABLE, ds.rsStencilEnable);
	ProxyInterface->SetRenderState(D3DRS_CULLMODE, ds.rsCullMode);
	ProxyInterface->SetRenderState(D3DRS_CLIPPING, ds.rsClipping);
}

inline void m_IDirect3DDevice9Ex::ApplyPresentFixes()
{
	bool CalledBeginScene = false;
	if (!Config.ForceSingleBeginEndScene || !SHARED.BeginSceneCalled)
	{
		CalledBeginScene = true;
		BeginScene();
	}

	if (SHARED.IsGammaSet || Config.ShowFPSCounter)
	{
		BackupDeviceState();

		if (SHARED.IsGammaSet)
		{
			ApplyBrightnessLevel();
		}

		if (Config.ShowFPSCounter)
		{
			RECT rect = { 0, 0, SHARED.BufferWidth, SHARED.BufferHeight };
			if (SHARED.IsDirectDrawDevice && SHARED.IsWindowMode)
			{
				GetClientRect(SHARED.DeviceWindow, &rect);
			}
			DrawFPS(static_cast<float>(SHARED.AverageFPSCounter), rect, Config.ShowFPSCounter);
		}

		RestoreDeviceState();
	}

	if (CalledBeginScene || (Config.ForceSingleBeginEndScene && SHARED.BeginSceneCalled))
	{
		CallEndScene();
	}

	if (Config.LimitPerFrameFPS)
	{
		LimitFrameRate();
	}

	// Check FPU state before presenting
	Utils::ResetInvalidFPUState();
}

HRESULT m_IDirect3DDevice9Ex::Present(CONST RECT *pSourceRect, CONST RECT *pDestRect, HWND hDestWindowOverride, CONST RGNDATA *pDirtyRegion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	ApplyPresentFixes();

	HRESULT hr = ProxyInterface->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);

	if (SUCCEEDED(hr))
	{
		if (Config.ShowFPSCounter || Config.EnableImgui)
		{
			CalculateFPS();
		}
	}

	return hr;
}

inline void m_IDirect3DDevice9Ex::ApplyDrawFixes()
{
	// CacheClipPlane
	if (Config.CacheClipPlane && SHARED.isClipPlaneSet)
	{
		ApplyClipPlanes();
	}

	// Reenable Anisotropic Filtering
	if (SHARED.MaxAnisotropy)
	{
		ReeableAnisotropicSamplerState();
	}

	// Fix environment map cubes
	if (Config.EnvironmentMapCubeFix)
	{
		SetEnvironmentMapCubeTexture();
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

	HRESULT hr;
	
	if (Config.ForceSingleBeginEndScene && (SHARED.IsInScene || SHARED.BeginSceneCalled))
	{
		if (SHARED.IsInScene)
		{
			hr = D3DERR_INVALIDCALL;
		}
		else
		{
			SHARED.IsInScene = true;

			hr = D3D_OK;
		}
	}
	else
	{
		hr = ProxyInterface->BeginScene();

		if (SUCCEEDED(hr))
		{
			SHARED.IsInScene = true;
			SHARED.BeginSceneCalled = true;
		}
	}

#ifdef ENABLE_DEBUGOVERLAY
	if (Config.EnableImgui)
	{
		DOverlay.BeginScene();
	}
#endif

	// Get DeviceCaps
	if (SHARED.Caps.DeviceType == NULL)
	{
		if (SUCCEEDED(ProxyInterface->GetDeviceCaps(&SHARED.Caps)))
		{
			// Set for Anisotropic Filtering
			SHARED.MaxAnisotropy = (Config.AnisotropicFiltering == 1) ? SHARED.Caps.MaxAnisotropy : min((DWORD)Config.AnisotropicFiltering, SHARED.Caps.MaxAnisotropy);
		}
		else
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Falied to get DeviceCaps (" << this << ")");
			ZeroMemory(&SHARED.Caps, sizeof(D3DCAPS9));
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
inline void m_IDirect3DDevice9Ex::CheckTransformForCubeMap(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix) const
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
			SHARED.isTransformMapCube[stage] = isCubeMap;
		}
	}
}

// Check if an environment cube map is being used
inline bool m_IDirect3DDevice9Ex::CheckTextureStageForCubeMap() const
{
	for (DWORD i = 0; i < MAX_TEXTURE_STAGES; i++)
	{
		if ((SHARED.texCoordIndex[i] == D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR || SHARED.texCoordIndex[i] == D3DTSS_TCI_CAMERASPACENORMAL) &&
			((SHARED.texTransformFlags[i] & D3DTTFF_COUNT3) || (SHARED.texTransformFlags[i] & D3DTTFF_COUNT4)))
		{
			return true;
		}
	}
	return false;
}

inline void m_IDirect3DDevice9Ex::SetEnvironmentMapCubeTexture()
{
	const bool isCubeMap = CheckTextureStageForCubeMap() ||
		[&]() {
		for (int i = 0; i < MAX_TEXTURE_STAGES; ++i)
		{
			if (SHARED.isTextureMapCube[i] || SHARED.isTransformMapCube[i])
			{
				return true;
			}
		}
		return false;
		}();

	if (isCubeMap && SHARED.pCurrentTexture == nullptr)
	{
		if (!SHARED.BlankTexture)
		{
			const UINT CubeSize = 64;
			HRESULT hr = ProxyInterface->CreateCubeTexture(CubeSize, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &SHARED.BlankTexture, nullptr);
			if (FAILED(hr))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: failed to create BlankCubeTexture for environment map!");
				return;
			}

			D3DLOCKED_RECT lockedRect;
			for (UINT face = 0; face < 6; ++face)
			{
				if (SUCCEEDED(SHARED.BlankTexture->LockRect((D3DCUBEMAP_FACES)face, 0, &lockedRect, nullptr, 0)))
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
					SHARED.BlankTexture->UnlockRect((D3DCUBEMAP_FACES)face, 0);
				}
				else
				{
					LOG_LIMIT(100, __FUNCTION__ << " Error: failed to lock BlankCubeTexture face: " << face);
				}
			}
		}

		SHARED.isBlankTextureUsed = true;
		ProxyInterface->SetTexture(0, SHARED.BlankTexture);
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
			if (SHARED.MaxAnisotropy && Stage > 0)
			{
				DisableAnisotropicSamplerState((SHARED.Caps.TextureFilterCaps & D3DPTFILTERCAPS_MINFANISOTROPIC), (SHARED.Caps.TextureFilterCaps & D3DPTFILTERCAPS_MAGFANISOTROPIC));
			}
			break;
		case D3DRTYPE_VOLUMETEXTURE:
			pTexture = static_cast<m_IDirect3DVolumeTexture9 *>(pTexture)->GetProxyInterface();
			if (SHARED.MaxAnisotropy && Stage > 0)
			{
				DisableAnisotropicSamplerState((SHARED.Caps.VolumeTextureFilterCaps & D3DPTFILTERCAPS_MINFANISOTROPIC), (SHARED.Caps.VolumeTextureFilterCaps & D3DPTFILTERCAPS_MAGFANISOTROPIC));
			}
			break;
		case D3DRTYPE_CUBETEXTURE:
			pTexture = static_cast<m_IDirect3DCubeTexture9 *>(pTexture)->GetProxyInterface();
			isTexCube = true;
			if (SHARED.MaxAnisotropy && Stage > 0)
			{
				DisableAnisotropicSamplerState((SHARED.Caps.CubeTextureFilterCaps & D3DPTFILTERCAPS_MINFANISOTROPIC), (SHARED.Caps.CubeTextureFilterCaps & D3DPTFILTERCAPS_MAGFANISOTROPIC));
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
			SHARED.isTextureMapCube[Stage] = isTexCube;
		}
		if (Stage == 0)
		{
			SHARED.isBlankTextureUsed = false;
			SHARED.pCurrentTexture = pTexture;
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
				SHARED.texCoordIndex[Stage] = Value;
			}
			else if (Type == D3DTSS_TEXTURETRANSFORMFLAGS)
			{
				SHARED.texTransformFlags[Stage] = Value;
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

	// CacheClipPlane
	if (Config.CacheClipPlane)
	{
		if (!pPlane || Index >= MAX_CLIP_PLANES)
		{
			return D3DERR_INVALIDCALL;
		}

		memcpy(pPlane, SHARED.m_storedClipPlanes[Index], sizeof(SHARED.m_storedClipPlanes[0]));

		return D3D_OK;
	}

	return ProxyInterface->GetClipPlane(Index, pPlane);
}

HRESULT m_IDirect3DDevice9Ex::SetClipPlane(DWORD Index, CONST float *pPlane)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	// CacheClipPlane
	if (Config.CacheClipPlane)
	{
		if (!pPlane || Index >= MAX_CLIP_PLANES)
		{
			return D3DERR_INVALIDCALL;
		}

		SHARED.isClipPlaneSet = true;

		memcpy(SHARED.m_storedClipPlanes[Index], pPlane, sizeof(SHARED.m_storedClipPlanes[0]));

		return D3D_OK;
	}

	return ProxyInterface->SetClipPlane(Index, pPlane);
}

// CacheClipPlane
inline void m_IDirect3DDevice9Ex::ApplyClipPlanes()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	DWORD index = 0;
	for (const auto clipPlane : SHARED.m_storedClipPlanes)
	{
		if ((SHARED.m_clipPlaneRenderState & (1 << index)) != 0)
		{
			ProxyInterface->SetClipPlane(index, clipPlane);
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
		*ppShader = new m_IDirect3DVertexShader9(*ppShader, this);
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
		*ppQuery = new m_IDirect3DQuery9(*ppQuery, this);
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
		*ppDecl = new m_IDirect3DVertexDeclaration9(*ppDecl, this);
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
	if (SHARED.MaxAnisotropy)
	{
		if (Type == D3DSAMP_MAXANISOTROPY)
		{
			if (SUCCEEDED(ProxyInterface->SetSamplerState(Sampler, D3DSAMP_MAXANISOTROPY, SHARED.MaxAnisotropy)))
			{
				return D3D_OK;
			}
		}
		else if ((Value == D3DTEXF_LINEAR || Value == D3DTEXF_ANISOTROPIC) && (Type == D3DSAMP_MINFILTER || Type == D3DSAMP_MAGFILTER))
		{
			if (SUCCEEDED(ProxyInterface->SetSamplerState(Sampler, D3DSAMP_MAXANISOTROPY, SHARED.MaxAnisotropy)) &&
				SUCCEEDED(ProxyInterface->SetSamplerState(Sampler, Type, D3DTEXF_ANISOTROPIC)))
			{
				if (!SHARED.isAnisotropySet)
				{
					SHARED.isAnisotropySet = true;
					Logging::Log() << "Setting Anisotropic Filtering at " << SHARED.MaxAnisotropy << "x";
				}
				return D3D_OK;
			}
		}
	}

	return ProxyInterface->SetSamplerState(Sampler, Type, Value);
}

inline void m_IDirect3DDevice9Ex::DisableAnisotropicSamplerState(bool AnisotropyMin, bool AnisotropyMag)
{
	DWORD Value = 0;
	for (int x = 0; x < 4; x++)
	{
		if (!AnisotropyMin)	// Anisotropic Min Filter is not supported for multi-stage textures
		{
			if (SUCCEEDED(ProxyInterface->GetSamplerState(x, D3DSAMP_MINFILTER, &Value)) && Value == D3DTEXF_ANISOTROPIC &&
				SUCCEEDED(ProxyInterface->SetSamplerState(x, D3DSAMP_MINFILTER, D3DTEXF_LINEAR)))
			{
				SHARED.AnisotropyDisabledFlag = true;
			}
		}
		if (!AnisotropyMag)	// Anisotropic Mag Filter is not supported for multi-stage textures
		{
			if (SUCCEEDED(ProxyInterface->GetSamplerState(x, D3DSAMP_MAGFILTER, &Value)) && Value == D3DTEXF_ANISOTROPIC &&
				SUCCEEDED(ProxyInterface->SetSamplerState(x, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR)))
			{
				SHARED.AnisotropyDisabledFlag = true;
			}
		}
	}
}

inline void m_IDirect3DDevice9Ex::ReeableAnisotropicSamplerState()
{
	if (SHARED.AnisotropyDisabledFlag)
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
		SHARED.AnisotropyDisabledFlag = Flag;
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

	HRESULT hr = ProxyInterface->CreateOffscreenPlainSurface(Width, Height, Format, Pool, ppSurface, pSharedHandle);

	if (SUCCEEDED(hr))
	{
		*ppSurface = new m_IDirect3DSurface9(*ppSurface, this);
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

	if (Config.EnableWindowMode && (SHARED.BufferWidth != SHARED.screenWidth || SHARED.BufferHeight != SHARED.screenHeight))
	{
		return FakeGetFrontBufferData(iSwapChain, pDestSurface);
	}
	else
	{
		if (pDestSurface)
		{
			pDestSurface = static_cast<m_IDirect3DSurface9 *>(pDestSurface)->GetProxyInterface();
		}

		return ProxyInterface->GetFrontBufferData(iSwapChain, pDestSurface);
	}
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
	IDirect3DSurface9 *pSrcSurface = nullptr;
	if (FAILED(ProxyInterface->CreateOffscreenPlainSurface(max(SHARED.screenWidth, RectSrc.right), max(SHARED.screenHeight, RectSrc.bottom), Desc.Format, Desc.Pool, &pSrcSurface, nullptr)))
	{
		return D3DERR_INVALIDCALL;
	}

	// Get FrontBuffer data to new surface
	HRESULT hr = ProxyInterface->GetFrontBufferData(iSwapChain, pSrcSurface);
	if (FAILED(hr))
	{
		pSrcSurface->Release();
		return hr;
	}

	// Copy data to DestSurface
	hr = D3DERR_INVALIDCALL;
	if (rcClient.left == 0 && rcClient.top == 0 && (LONG)Desc.Width == rcClient.right && (LONG)Desc.Height == rcClient.bottom)
	{
		POINT PointDest = { 0, 0 };
		hr = CopyRects(pSrcSurface, &RectSrc, 1, pDestSurface, &PointDest);
	}

	// Try using StretchRect
	if (FAILED(hr))
	{
		IDirect3DSurface9 *pTmpSurface = nullptr;
		if (SUCCEEDED(ProxyInterface->CreateOffscreenPlainSurface(Desc.Width, Desc.Height, Desc.Format, Desc.Pool, &pTmpSurface, nullptr)))
		{
			if (SUCCEEDED(ProxyInterface->StretchRect(pSrcSurface, &RectSrc, pTmpSurface, nullptr, D3DTEXF_NONE)))
			{
				POINT PointDest = { 0, 0 };
				RECT Rect = { 0, 0, (LONG)Desc.Width, (LONG)Desc.Height };
				hr = CopyRects(pTmpSurface, &Rect, 1, pDestSurface, &PointDest);
			}
			pTmpSurface->Release();
		}
	}

	// Release surface
	pSrcSurface->Release();
	return hr;
}

HRESULT m_IDirect3DDevice9Ex::GetRenderTargetData(THIS_ IDirect3DSurface9* pRenderTarget, IDirect3DSurface9* pDestSurface)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

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
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();

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

	ApplyPresentFixes();

	HRESULT hr = ProxyInterfaceEx->PresentEx(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion, dwFlags);

	if (SUCCEEDED(hr))
	{
		if (Config.ShowFPSCounter || Config.EnableImgui)
		{
			CalculateFPS();
		}
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
		*ppSurface = new m_IDirect3DSurface9(*ppSurface, this);
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

	HRESULT hr = ProxyInterfaceEx->CreateOffscreenPlainSurfaceEx(Width, Height, Format, Pool, ppSurface, pSharedHandle, Usage);

	if (SUCCEEDED(hr))
	{
		*ppSurface = new m_IDirect3DSurface9(*ppSurface, this);
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
		*ppSurface = new m_IDirect3DSurface9(*ppSurface, this);
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

	return ResetT<fResetEx>(nullptr, pPresentationParameters, pFullscreenDisplayMode);
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
inline void m_IDirect3DDevice9Ex::ReInitInterface() const
{
	Utils::GetScreenSize(SHARED.DeviceWindow, SHARED.screenWidth, SHARED.screenHeight);

	SHARED.IsGammaSet = false;
	for (int i = 0; i < 256; ++i)
	{
		WORD value = static_cast<WORD>(i * 65535 / 255); // Linear interpolation from 0 to 65535
		SHARED.RampData.red[i] = value;
		SHARED.RampData.green[i] = value;
		SHARED.RampData.blue[i] = value;
		SHARED.DefaultRampData.red[i] = value;
		SHARED.DefaultRampData.green[i] = value;
		SHARED.DefaultRampData.blue[i] = value;
	}
}

inline void m_IDirect3DDevice9Ex::DrawFPS(float fps, const RECT& presentRect, DWORD position) const
{
	// Scale the font size based on the rect height (adjustable factor)
	int fontSize = SHARED.BufferHeight / 40;
	if (fontSize < 4) fontSize = 4;		// Minimum font size
	if (fontSize > 128) fontSize = 128;	// Maximum font size

	// Recreate the font if size changes
	if (!SHARED.pFont || SHARED.lastFontSize != fontSize)
	{
		if (SHARED.pFont)
		{
			ULONG ref = SHARED.pFont->Release();
			if (ref)
			{
				Logging::Log() << __FUNCTION__ << " Error: there is still a reference to 'gammaPixelShader' " << ref;
			}
			SHARED.pFont = nullptr;
		}

		D3DXCreateFontW(ProxyInterface, fontSize, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET,
			OUT_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
			L"Arial", &SHARED.pFont);
		SHARED.lastFontSize = fontSize;
	}

	if (!SHARED.pFont)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: failed to create font!");
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

	// Draw the text
	INT ret = SHARED.pFont->DrawTextW(nullptr, fpsText, -1, &textRect, alignment, D3DCOLOR_XRGB(247, 247, 0));
	if (ret == 0)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: could not DrawText!");
	}
}

inline void m_IDirect3DDevice9Ex::LimitFrameRate() const
{
	// Count the number of frames
	SHARED.Counter.FrameCounter++;

	// Get performance frequency if not already cached
	static LARGE_INTEGER Frequency = {};
	if (!Frequency.QuadPart)
	{
		QueryPerformanceFrequency(&Frequency);
	}
	static LONGLONG TicksPerMS = Frequency.QuadPart / 1000;

	// Calculate the delay time in ticks
	static long double PerFrameFPS = Config.LimitPerFrameFPS;
	static LONGLONG PreFrameTicks = static_cast<LONGLONG>(static_cast<long double>(Frequency.QuadPart) / PerFrameFPS);

	// Get next tick time
	LARGE_INTEGER ClickTime = {};
	QueryPerformanceCounter(&ClickTime);
	LONGLONG TargetEndTicks = SHARED.Counter.LastPresentTime.QuadPart;
	LONGLONG FramesSinceLastCall = ((ClickTime.QuadPart - SHARED.Counter.LastPresentTime.QuadPart - 1) / PreFrameTicks) + 1;
	if (SHARED.Counter.LastPresentTime.QuadPart == 0 || FramesSinceLastCall > 2)
	{
		QueryPerformanceCounter(&SHARED.Counter.LastPresentTime);
		TargetEndTicks = SHARED.Counter.LastPresentTime.QuadPart;
	}
	else
	{
		TargetEndTicks += FramesSinceLastCall * PreFrameTicks;
	}

	// Wait for time to expire
	bool DoLoop;
	do {
		QueryPerformanceCounter(&ClickTime);
		LONGLONG RemainingTicks = TargetEndTicks - ClickTime.QuadPart;

		// Check if we still need to wait
		DoLoop = RemainingTicks > 0;

		if (DoLoop)
		{
			// Busy wait until we reach the target time
			Utils::BusyWaitYield(static_cast<DWORD>(RemainingTicks / TicksPerMS));
		}
	} while (DoLoop);

	// Update the last present time
	SHARED.Counter.LastPresentTime.QuadPart = TargetEndTicks;
}

inline void m_IDirect3DDevice9Ex::CalculateFPS() const
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
