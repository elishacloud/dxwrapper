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
#include "d3dx9.h"
#include "Utils\Utils.h"
#include <intrin.h>

HRESULT m_IDirect3DDevice9Ex::QueryInterface(REFIID riid, void** ppvObj)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if ((riid == IID_IUnknown || riid == WrapperID) && ppvObj)
	{
		AddRef();

		*ppvObj = this;

		return D3D_OK;
	}

	HRESULT hr = ProxyInterface->QueryInterface(riid, ppvObj);

	if (SUCCEEDED(hr))
	{
		D3d9Wrapper::genericQueryInterface(riid, ppvObj, this);
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

	if (ref == 0)
	{
		delete this;
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
	m_clipPlaneRenderState = 0;
}

template <typename T>
HRESULT m_IDirect3DDevice9Ex::ResetT(T func, D3DPRESENT_PARAMETERS &d3dpp, D3DPRESENT_PARAMETERS* pPresentationParameters, D3DDISPLAYMODEEX* pFullscreenDisplayMode)
{
	if (!pPresentationParameters)
	{
		return D3DERR_INVALIDCALL;
	}

	HRESULT hr;

	// Check fullscreen
	bool ForceFullscreen = false;
	if (m_pD3DEx)
	{
		ForceFullscreen = m_pD3DEx->TestResolution(D3DADAPTER_DEFAULT, pPresentationParameters->BackBufferWidth, pPresentationParameters->BackBufferHeight);
	}

	// Setup presentation parameters
	CopyMemory(&d3dpp, pPresentationParameters, sizeof(D3DPRESENT_PARAMETERS));
	UpdatePresentParameter(&d3dpp, DeviceWindow, ForceFullscreen, true);

	// Test for Multisample
	if (DeviceMultiSampleFlag)
	{
		// Update Present Parameter for Multisample
		UpdatePresentParameterForMultisample(&d3dpp, DeviceMultiSampleType, DeviceMultiSampleQuality);

		// Reset device
		hr = ResetT(func, &d3dpp, pFullscreenDisplayMode);

		// Check if device was reset successfully
		if (SUCCEEDED(hr))
		{
			return D3D_OK;
		}

		// Reset presentation parameters
		CopyMemory(&d3dpp, pPresentationParameters, sizeof(D3DPRESENT_PARAMETERS));
		UpdatePresentParameter(&d3dpp, DeviceWindow, ForceFullscreen, false);

		// Reset device
		hr = ResetT(func, &d3dpp, pFullscreenDisplayMode);

		if (SUCCEEDED(hr))
		{
			LOG_LIMIT(3, __FUNCTION__ << " Disabling AntiAliasing...");
			DeviceMultiSampleFlag = false;
			DeviceMultiSampleType = D3DMULTISAMPLE_NONE;
			DeviceMultiSampleQuality = 0;
			SetSSAA = false;
		}

		return hr;
	}

	// Reset device
	return ResetT(func, &d3dpp, pFullscreenDisplayMode);
}

HRESULT m_IDirect3DDevice9Ex::Reset(D3DPRESENT_PARAMETERS *pPresentationParameters)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!pPresentationParameters)
	{
		return D3DERR_INVALIDCALL;
	}

	D3DPRESENT_PARAMETERS d3dpp;

	HRESULT hr = ResetT<fReset>(nullptr, d3dpp, pPresentationParameters);

	if (SUCCEEDED(hr))
	{
		CopyMemory(pPresentationParameters, &d3dpp, sizeof(D3DPRESENT_PARAMETERS));

		ClearVars(pPresentationParameters);

		// Get screen size
		Utils::GetScreenSize(DeviceWindow, screenWidth, screenHeight);
	}

	return hr;
}

HRESULT m_IDirect3DDevice9Ex::EndScene()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->EndScene();
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
	UpdatePresentParameter(&d3dpp, DeviceWindow, ForceFullscreen, false);

	// Test for Multisample
	if (DeviceMultiSampleFlag)
	{
		// Update Present Parameter for Multisample
		UpdatePresentParameterForMultisample(&d3dpp, DeviceMultiSampleType, DeviceMultiSampleQuality);

		// Create CwapChain
		hr = ProxyInterface->CreateAdditionalSwapChain(&d3dpp, ppSwapChain);
	}
	
	if (FAILED(hr))
	{
		CopyMemory(&d3dpp, pPresentationParameters, sizeof(D3DPRESENT_PARAMETERS));
		UpdatePresentParameter(&d3dpp, DeviceWindow, ForceFullscreen, false);

		// Create CwapChain
		hr = ProxyInterface->CreateAdditionalSwapChain(&d3dpp, ppSwapChain);

		if (SUCCEEDED(hr) && DeviceMultiSampleFlag)
		{
			LOG_LIMIT(3, __FUNCTION__ <<" Disabling AntiAliasing for SwapChain...");
		}
	}

	if (SUCCEEDED(hr))
	{
		CopyMemory(pPresentationParameters, &d3dpp, sizeof(D3DPRESENT_PARAMETERS));

		*ppSwapChain = new m_IDirect3DSwapChain9Ex((IDirect3DSwapChain9Ex*)*ppSwapChain, this);
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

	HRESULT hr = ProxyInterface->CreateDepthStencilSurface(Width, Height, Format, MultiSample, MultisampleQuality, Discard, ppSurface, pSharedHandle);

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

	HRESULT hr = ProxyInterface->CreateRenderTarget(Width, Height, Format, MultiSample, MultisampleQuality, Lockable, ppSurface, pSharedHandle);

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
		*ppSB = new m_IDirect3DStateBlock9(*ppSB, this);
		return D3D_OK;
	}

	Logging::LogDebug() << __FUNCTION__ << " FAILED! " << (D3DERR)hr << " " << Type;
	return hr;
}

HRESULT m_IDirect3DDevice9Ex::EndStateBlock(THIS_ IDirect3DStateBlock9** ppSB)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	HRESULT hr = ProxyInterface->EndStateBlock(ppSB);

	if (SUCCEEDED(hr) && ppSB)
	{
		*ppSB = ProxyAddressLookupTable->FindAddress<m_IDirect3DStateBlock9>(*ppSB);
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
		*ppRenderTarget = ProxyAddressLookupTable->FindAddress<m_IDirect3DSurface9>(*ppRenderTarget);
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
	if (DeviceMultiSampleFlag && State == D3DRS_MULTISAMPLEANTIALIAS)
	{
		Value = TRUE;
	}

	HRESULT hr = ProxyInterface->SetRenderState(State, Value);

	// CacheClipPlane
	if (SUCCEEDED(hr) && State == D3DRS_CLIPPLANEENABLE)
	{
		m_clipPlaneRenderState = Value;
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
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->SetTransform(State, pMatrix);
}

void m_IDirect3DDevice9Ex::GetGammaRamp(THIS_ UINT iSwapChain, D3DGAMMARAMP* pRamp)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->GetGammaRamp(iSwapChain, pRamp);
}

void m_IDirect3DDevice9Ex::SetGammaRamp(THIS_ UINT iSwapChain, DWORD Flags, CONST D3DGAMMARAMP* pRamp)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

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
		*ppIndexData = ProxyAddressLookupTable->FindAddress<m_IDirect3DIndexBuffer9>(*ppIndexData);
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

	m_pD3DEx->AddRef();

	*ppD3D9 = m_pD3DEx;

	return D3D_OK;
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
		*ppShader = ProxyAddressLookupTable->FindAddress<m_IDirect3DPixelShader9>(*ppShader);
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

HRESULT m_IDirect3DDevice9Ex::Present(CONST RECT *pSourceRect, CONST RECT *pDestRect, HWND hDestWindowOverride, CONST RGNDATA *pDirtyRegion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
}

HRESULT m_IDirect3DDevice9Ex::DrawIndexedPrimitive(THIS_ D3DPRIMITIVETYPE Type, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

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

	return ProxyInterface->DrawIndexedPrimitive(Type, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
}

HRESULT m_IDirect3DDevice9Ex::DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT MinIndex, UINT NumVertices, UINT PrimitiveCount, CONST void *pIndexData, D3DFORMAT IndexDataFormat, CONST void *pVertexStreamZeroData, UINT VertexStreamZeroStride)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

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

	return ProxyInterface->DrawIndexedPrimitiveUP(PrimitiveType, MinIndex, NumVertices, PrimitiveCount, pIndexData, IndexDataFormat, pVertexStreamZeroData, VertexStreamZeroStride);
}

HRESULT m_IDirect3DDevice9Ex::DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

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

	return ProxyInterface->DrawPrimitive(PrimitiveType, StartVertex, PrimitiveCount);
}

HRESULT m_IDirect3DDevice9Ex::DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, CONST void *pVertexStreamZeroData, UINT VertexStreamZeroStride)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

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

	return ProxyInterface->DrawPrimitiveUP(PrimitiveType, PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride);
}

HRESULT m_IDirect3DDevice9Ex::BeginScene()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	HRESULT hr = ProxyInterface->BeginScene();

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
	if (DeviceMultiSampleFlag)
	{
		ProxyInterface->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, TRUE);
		if (SetSSAA)
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
		*ppStreamData = ProxyAddressLookupTable->FindAddress<m_IDirect3DVertexBuffer9>(*ppStreamData);
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
		*ppBackBuffer = ProxyAddressLookupTable->FindAddress<m_IDirect3DSurface9>(*ppBackBuffer);
	}

	return hr;
}

HRESULT m_IDirect3DDevice9Ex::GetDepthStencilSurface(IDirect3DSurface9 **ppZStencilSurface)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	HRESULT hr = ProxyInterface->GetDepthStencilSurface(ppZStencilSurface);

	if (SUCCEEDED(hr) && ppZStencilSurface)
	{
		*ppZStencilSurface = ProxyAddressLookupTable->FindAddress<m_IDirect3DSurface9>(*ppZStencilSurface);
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
			*ppTexture = ProxyAddressLookupTable->FindAddress<m_IDirect3DTexture9>(*ppTexture);
			break;
		case D3DRTYPE_VOLUMETEXTURE:
			*ppTexture = ProxyAddressLookupTable->FindAddress<m_IDirect3DVolumeTexture9>(*ppTexture);
			break;
		case D3DRTYPE_CUBETEXTURE:
			*ppTexture = ProxyAddressLookupTable->FindAddress<m_IDirect3DCubeTexture9>(*ppTexture);
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

HRESULT m_IDirect3DDevice9Ex::SetTexture(DWORD Stage, IDirect3DBaseTexture9 *pTexture)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

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
			if (MaxAnisotropy && Stage > 0)
			{
				DisableAnisotropicSamplerState((Caps.CubeTextureFilterCaps & D3DPTFILTERCAPS_MINFANISOTROPIC), (Caps.CubeTextureFilterCaps & D3DPTFILTERCAPS_MAGFANISOTROPIC));
			}
			break;
		default:
			return D3DERR_INVALIDCALL;
		}
	}

	return ProxyInterface->SetTexture(Stage, pTexture);
}

HRESULT m_IDirect3DDevice9Ex::SetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->SetTextureStageState(Stage, Type, Value);
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

		memcpy(pPlane, m_storedClipPlanes[Index], sizeof(m_storedClipPlanes[0]));

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

		isClipPlaneSet = true;

		memcpy(m_storedClipPlanes[Index], pPlane, sizeof(m_storedClipPlanes[0]));

		return D3D_OK;
	}

	return ProxyInterface->SetClipPlane(Index, pPlane);
}

// CacheClipPlane
void m_IDirect3DDevice9Ex::ApplyClipPlanes()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	DWORD index = 0;
	for (const auto clipPlane : m_storedClipPlanes)
	{
		if ((m_clipPlaneRenderState & (1 << index)) != 0)
		{
			ProxyInterface->SetClipPlane(index, clipPlane);
		}
		index++;
	}
}

HRESULT m_IDirect3DDevice9Ex::Clear(DWORD Count, CONST D3DRECT *pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (IsWindow(DeviceWindow) && (Config.FullscreenWindowMode || Config.EnableWindowMode))
	{
		// Attach thread input
		DWORD dwMyID = GetCurrentThreadId();
		DWORD dwCurID = GetWindowThreadProcessId(DeviceWindow, nullptr);
		AttachThreadInput(dwCurID, dwMyID, TRUE);

		// Peek messages to help prevent a "Not Responding" window
		MSG msg;
		PeekMessage(&msg, DeviceWindow, 0, 0, PM_NOREMOVE);

		// Detach thread input
		AttachThreadInput(dwCurID, dwMyID, FALSE);
	}

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
		*ppShader = ProxyAddressLookupTable->FindAddress<m_IDirect3DVertexShader9>(*ppShader);
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
		*ppDecl = ProxyAddressLookupTable->FindAddress<m_IDirect3DVertexDeclaration9>(*ppDecl);
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

	HRESULT hr;
	// handle depth texture
	if(Format >= D3DFMT_D16_LOCKABLE && Format <= D3DFMT_S8_LOCKABLE)
	{
		hr = ProxyInterface->CreateDepthStencilSurface(Width, Height, Format, D3DMULTISAMPLE_NONE, 0, TRUE, ppSurface, pSharedHandle);
	}
	else
	{
		hr = ProxyInterface->CreateOffscreenPlainSurface(Width, Height, Format, Pool, ppSurface, pSharedHandle);
	}

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

	pSourceSurface = static_cast<m_IDirect3DSurface9*>(pSourceSurface)->GetProxyInterface();
	pDestinationSurface = static_cast<m_IDirect3DSurface9*>(pDestinationSurface)->GetProxyInterface();

	D3DSURFACE_DESC SourceDesc, DestinationDesc;
	pSourceSurface->GetDesc(&SourceDesc);
	pDestinationSurface->GetDesc(&DestinationDesc);

	if (SourceDesc.Format != DestinationDesc.Format)
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
		RECT SourceRect, DestinationRect;

		if (pSourceRectsArray != nullptr)
		{
			SourceRect = pSourceRectsArray[i];
		}
		else
		{
			SourceRect.left = 0;
			SourceRect.right = SourceDesc.Width;
			SourceRect.top = 0;
			SourceRect.bottom = SourceDesc.Height;
		}

		if (pDestPointsArray != nullptr)
		{
			DestinationRect.left = pDestPointsArray[i].x;
			DestinationRect.right = DestinationRect.left + (SourceRect.right - SourceRect.left);
			DestinationRect.top = pDestPointsArray[i].y;
			DestinationRect.bottom = DestinationRect.top + (SourceRect.bottom - SourceRect.top);
		}
		else
		{
			DestinationRect = SourceRect;
		}

		if (SourceDesc.Pool == D3DPOOL_DEFAULT)
		{
			hr = ProxyInterface->StretchRect(pSourceSurface, &SourceRect, pDestinationSurface, &DestinationRect, D3DTEXF_NONE);
		}
		else if (SourceDesc.Pool == D3DPOOL_SYSTEMMEM)
		{
			const POINT pt = { DestinationRect.left, DestinationRect.top };

			hr = ProxyInterface->UpdateSurface(pSourceSurface, &SourceRect, pDestinationSurface, &pt);
		}
		if (FAILED(hr))
		{
			hr = D3DERR_INVALIDCALL;
			if (SUCCEEDED(D3DXLoadSurfaceFromSurface(pDestinationSurface, nullptr, &DestinationRect, pSourceSurface, nullptr, &SourceRect, D3DX_FILTER_NONE, 0)))
			{
				// Explicitly call AddDirtyRect on the surface
				void *pContainer = nullptr;
				if (SUCCEEDED(pDestinationSurface->GetContainer(IID_IDirect3DTexture9, &pContainer)) && pContainer)
				{
					IDirect3DTexture9 *pTexture = (IDirect3DTexture9*)pContainer;
					pTexture->AddDirtyRect(&DestinationRect);
					pTexture->Release();
				}
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

// Stretch source rect to destination rect
HRESULT m_IDirect3DDevice9Ex::StretchRectFake(THIS_ IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestSurface, CONST RECT* pDestRect, D3DTEXTUREFILTERTYPE Filter)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	UNREFERENCED_PARAMETER(Filter);

	// Check destination parameters
	if (!pSourceSurface || !pDestSurface)
	{
		return D3DERR_INVALIDCALL;
	}

	// Get surface desc
	D3DSURFACE_DESC SrcDesc, DestDesc;
	if (FAILED(pSourceSurface->GetDesc(&SrcDesc)))
	{
		return D3DERR_INVALIDCALL;
	}
	if (FAILED(pDestSurface->GetDesc(&DestDesc)))
	{
		return D3DERR_INVALIDCALL;
	}

	// Check rects
	RECT SrcRect, DestRect;
	if (!pSourceRect)
	{
		SrcRect.left = 0;
		SrcRect.top = 0;
		SrcRect.right = SrcDesc.Width;
		SrcRect.bottom = SrcDesc.Height;
	}
	else
	{
		memcpy(&SrcRect, pSourceRect, sizeof(RECT));
	}
	if (!pDestRect)
	{
		DestRect.left = 0;
		DestRect.top = 0;
		DestRect.right = DestDesc.Width;
		DestRect.bottom = DestDesc.Height;
	}
	else
	{
		memcpy(&DestRect, pDestRect, sizeof(RECT));
	}

	// Check if source and destination formats are the same
	if (SrcDesc.Format != DestDesc.Format)
	{
		return D3DERR_INVALIDCALL;
	}

	// Lock surface
	D3DLOCKED_RECT SrcLockRect, DestLockRect;
	if (FAILED(pSourceSurface->LockRect(&SrcLockRect, nullptr, D3DLOCK_NOSYSLOCK | D3DLOCK_READONLY)))
	{
		return D3DERR_INVALIDCALL;
	}
	if (FAILED(pDestSurface->LockRect(&DestLockRect, nullptr, D3DLOCK_NOSYSLOCK)))
	{
		pSourceSurface->UnlockRect();
		return D3DERR_INVALIDCALL;
	}

	// Get bit count
	DWORD ByteCount = SrcLockRect.Pitch / SrcDesc.Width;

	// Get width and height of rect
	LONG DestRectWidth = DestRect.right - DestRect.left;
	LONG DestRectHeight = DestRect.bottom - DestRect.top;
	LONG SrcRectWidth = SrcRect.right - SrcRect.left;
	LONG SrcRectHeight = SrcRect.bottom - SrcRect.top;

	// Get ratio
	float WidthRatio = (float)SrcRectWidth / (float)DestRectWidth;
	float HeightRatio = (float)SrcRectHeight / (float)DestRectHeight;

	// Copy memory using color key
	switch (ByteCount)
	{
	case 1: // 8-bit surfaces
	case 2: // 16-bit surfaces
	case 3: // 24-bit surfaces
	case 4: // 32-bit surfaces
	{
		for (LONG y = 0; y < DestRectHeight; y++)
		{
			DWORD StartDestLoc = ((y + DestRect.top) * DestLockRect.Pitch) + (DestRect.left * ByteCount);
			DWORD StartSrcLoc = ((((DWORD)((float)y * HeightRatio)) + SrcRect.top) * SrcLockRect.Pitch) + (SrcRect.left * ByteCount);

			for (LONG x = 0; x < DestRectWidth; x++)
			{
				memcpy((BYTE*)DestLockRect.pBits + StartDestLoc + x * ByteCount, (BYTE*)((BYTE*)SrcLockRect.pBits + StartSrcLoc + ((DWORD)((float)x * WidthRatio)) * ByteCount), ByteCount);
			}
		}
		break;
	}
	default: // Unsupported surface bit count
		pSourceSurface->UnlockRect();
		pDestSurface->UnlockRect();
		return D3DERR_INVALIDCALL;
	}

	// Unlock rect and return
	pSourceSurface->UnlockRect();
	pDestSurface->UnlockRect();
	return D3D_OK;
}

HRESULT m_IDirect3DDevice9Ex::GetFrontBufferData(THIS_ UINT iSwapChain, IDirect3DSurface9* pDestSurface)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.EnableWindowMode && (BufferWidth != screenWidth || BufferHeight != screenHeight))
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
	if (pDestSurface)
	{
		pDestSurface = static_cast<m_IDirect3DSurface9 *>(pDestSurface)->GetProxyInterface();
	}

	// Get surface desc
	D3DSURFACE_DESC Desc;
	if (!pDestSurface || FAILED(pDestSurface->GetDesc(&Desc)))
	{
		return D3DERR_INVALIDCALL;
	}

	// Get location of client window
	RECT RectSrc = { 0, 0, BufferWidth , BufferHeight };
	RECT rcClient = { 0, 0, BufferWidth , BufferHeight };
	if (Config.EnableWindowMode && DeviceWindow && (!GetWindowRect(DeviceWindow, &RectSrc) || !GetClientRect(DeviceWindow, &rcClient)))
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
	if (FAILED(ProxyInterface->CreateOffscreenPlainSurface(max(screenWidth, RectSrc.right), max(screenHeight, RectSrc.bottom), Desc.Format, Desc.Pool, &pSrcSurface, nullptr)))
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
			if (SUCCEEDED(StretchRect(pSrcSurface, &RectSrc, pTmpSurface, nullptr, D3DTEXF_NONE)))
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
		pRenderTarget = static_cast<m_IDirect3DSurface9 *>(pRenderTarget)->GetProxyInterface();
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

	if (pDestinationSurface)
	{
		pDestinationSurface = static_cast<m_IDirect3DSurface9 *>(pDestinationSurface)->GetProxyInterface();
	}

	return ProxyInterface->UpdateSurface(pSourceSurface, pSourceRect, pDestinationSurface, pDestPoint);
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
		*ppSwapChain = ProxyAddressLookupTable->FindAddress<m_IDirect3DSwapChain9Ex>(*ppSwapChain);
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

	if (!ProxyInterfaceEx)
	{
		Logging::Log() << __FUNCTION__ << " Error: Calling extension function from a non-extension device!";
		return D3DERR_INVALIDCALL;
	}

	return ProxyInterfaceEx->PresentEx(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion, dwFlags);
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

	HRESULT hr = ProxyInterfaceEx->CreateRenderTargetEx(Width, Height, Format, MultiSample, MultisampleQuality, Lockable, ppSurface, pSharedHandle, Usage);

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

	HRESULT hr = ProxyInterfaceEx->CreateDepthStencilSurfaceEx(Width, Height, Format, MultiSample, MultisampleQuality, Discard, ppSurface, pSharedHandle, Usage);

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

	D3DPRESENT_PARAMETERS d3dpp;

	HRESULT hr = ResetT<fResetEx>(nullptr, d3dpp, pPresentationParameters, pFullscreenDisplayMode);

	if (SUCCEEDED(hr))
	{
		CopyMemory(pPresentationParameters, &d3dpp, sizeof(D3DPRESENT_PARAMETERS));

		ClearVars(pPresentationParameters);

		// Get screen size
		Utils::GetScreenSize(DeviceWindow, screenWidth, screenHeight);
	}

	return hr;
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
