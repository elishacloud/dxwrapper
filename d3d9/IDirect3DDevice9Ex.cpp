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

HRESULT m_IDirect3DDevice9Ex::QueryInterface(REFIID riid, void** ppvObj)
{
	if ((riid == IID_IDirect3DDevice9Ex || riid == IID_IDirect3DDevice9 || riid == IID_IUnknown) && ppvObj)
	{
		AddRef();

		*ppvObj = this;

		return S_OK;
	}

	return ProxyInterface->QueryInterface(riid, ppvObj);
}

ULONG m_IDirect3DDevice9Ex::AddRef()
{
	return ProxyInterface->AddRef();
}

ULONG m_IDirect3DDevice9Ex::Release()
{
	ULONG count = ProxyInterface->Release();

	if (count == 0)
	{
		delete this;
	}

	return count;
}
HRESULT m_IDirect3DDevice9Ex::Reset(D3DPRESENT_PARAMETERS *pPresentationParameters)
{
	if (DeviceMultiSampleType != D3DMULTISAMPLE_NONE)
	{
		pPresentationParameters->MultiSampleType = DeviceMultiSampleType;
		pPresentationParameters->MultiSampleQuality = DeviceMultiSampleQuality;
	}

	return ProxyInterface->Reset(pPresentationParameters);
}

HRESULT m_IDirect3DDevice9Ex::EndScene()
{
	return ProxyInterface->EndScene();
}

void m_IDirect3DDevice9Ex::SetCursorPosition(int X, int Y, DWORD Flags)
{
	return ProxyInterface->SetCursorPosition(X, Y, Flags);
}

HRESULT m_IDirect3DDevice9Ex::SetCursorProperties(UINT XHotSpot, UINT YHotSpot, IDirect3DSurface9 *pCursorBitmap)
{
	if (pCursorBitmap)
	{
		pCursorBitmap = static_cast<m_IDirect3DSurface9 *>(pCursorBitmap)->GetProxyInterface();
	}

	return ProxyInterface->SetCursorProperties(XHotSpot, YHotSpot, pCursorBitmap);
}

BOOL m_IDirect3DDevice9Ex::ShowCursor(BOOL bShow)
{
	return ProxyInterface->ShowCursor(bShow);
}

HRESULT m_IDirect3DDevice9Ex::CreateAdditionalSwapChain(D3DPRESENT_PARAMETERS *pPresentationParameters, IDirect3DSwapChain9 **ppSwapChain)
{
	if (DeviceMultiSampleType != D3DMULTISAMPLE_NONE)
	{
		pPresentationParameters->MultiSampleType = DeviceMultiSampleType;
		pPresentationParameters->MultiSampleQuality = DeviceMultiSampleQuality;
	}

	HRESULT hr = ProxyInterface->CreateAdditionalSwapChain(pPresentationParameters, ppSwapChain);

	if (SUCCEEDED(hr))
	{
		*ppSwapChain = ProxyAddressLookupTable->FindAddress<m_IDirect3DSwapChain9>(*ppSwapChain);
	}

	return hr;
}

HRESULT m_IDirect3DDevice9Ex::CreateCubeTexture(THIS_ UINT EdgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DCubeTexture9** ppCubeTexture, HANDLE* pSharedHandle)
{
	HRESULT hr = ProxyInterface->CreateCubeTexture(EdgeLength, Levels, Usage, Format, Pool, ppCubeTexture, pSharedHandle);

	if (SUCCEEDED(hr))
	{
		*ppCubeTexture = ProxyAddressLookupTable->FindAddress<m_IDirect3DCubeTexture9>(*ppCubeTexture);
	}

	return hr;
}

HRESULT m_IDirect3DDevice9Ex::CreateDepthStencilSurface(THIS_ UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Discard, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle)
{
	if (DeviceMultiSampleType != D3DMULTISAMPLE_NONE)
	{
		MultiSample = DeviceMultiSampleType;
		MultisampleQuality = DeviceMultiSampleQuality;
	}

	HRESULT hr = ProxyInterface->CreateDepthStencilSurface(Width, Height, Format, MultiSample, MultisampleQuality, Discard, ppSurface, pSharedHandle);

	if (SUCCEEDED(hr))
	{
		*ppSurface = ProxyAddressLookupTable->FindAddress<m_IDirect3DSurface9>(*ppSurface);
	}

	return hr;
}

HRESULT m_IDirect3DDevice9Ex::CreateIndexBuffer(THIS_ UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DIndexBuffer9** ppIndexBuffer, HANDLE* pSharedHandle)
{
	HRESULT hr = ProxyInterface->CreateIndexBuffer(Length, Usage, Format, Pool, ppIndexBuffer, pSharedHandle);

	if (SUCCEEDED(hr))
	{
		*ppIndexBuffer = ProxyAddressLookupTable->FindAddress<m_IDirect3DIndexBuffer9>(*ppIndexBuffer);
	}

	return hr;
}

HRESULT m_IDirect3DDevice9Ex::CreateRenderTarget(THIS_ UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle)
{
	if (DeviceMultiSampleType != D3DMULTISAMPLE_NONE)
	{
		MultiSample = DeviceMultiSampleType;
		MultisampleQuality = DeviceMultiSampleQuality;
	}

	HRESULT hr = ProxyInterface->CreateRenderTarget(Width, Height, Format, MultiSample, MultisampleQuality, Lockable, ppSurface, pSharedHandle);

	if (SUCCEEDED(hr))
	{
		*ppSurface = ProxyAddressLookupTable->FindAddress<m_IDirect3DSurface9>(*ppSurface);
	}

	return hr;
}

HRESULT m_IDirect3DDevice9Ex::CreateTexture(THIS_ UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture9** ppTexture, HANDLE* pSharedHandle)
{
	HRESULT hr = ProxyInterface->CreateTexture(Width, Height, Levels, Usage, Format, Pool, ppTexture, pSharedHandle);

	if (SUCCEEDED(hr))
	{
		*ppTexture = ProxyAddressLookupTable->FindAddress<m_IDirect3DTexture9>(*ppTexture);
	}

	return hr;
}

HRESULT m_IDirect3DDevice9Ex::CreateVertexBuffer(THIS_ UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool, IDirect3DVertexBuffer9** ppVertexBuffer, HANDLE* pSharedHandle)
{
	HRESULT hr = ProxyInterface->CreateVertexBuffer(Length, Usage, FVF, Pool, ppVertexBuffer, pSharedHandle);

	if (SUCCEEDED(hr))
	{
		*ppVertexBuffer = ProxyAddressLookupTable->FindAddress<m_IDirect3DVertexBuffer9>(*ppVertexBuffer);
	}

	return hr;
}

HRESULT m_IDirect3DDevice9Ex::CreateVolumeTexture(THIS_ UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DVolumeTexture9** ppVolumeTexture, HANDLE* pSharedHandle)
{
	HRESULT hr = ProxyInterface->CreateVolumeTexture(Width, Height, Depth, Levels, Usage, Format, Pool, ppVolumeTexture, pSharedHandle);

	if (SUCCEEDED(hr))
	{
		*ppVolumeTexture = ProxyAddressLookupTable->FindAddress<m_IDirect3DVolumeTexture9>(*ppVolumeTexture);
	}

	return hr;
}

HRESULT m_IDirect3DDevice9Ex::BeginStateBlock()
{
	return ProxyInterface->BeginStateBlock();
}

HRESULT m_IDirect3DDevice9Ex::CreateStateBlock(THIS_ D3DSTATEBLOCKTYPE Type, IDirect3DStateBlock9** ppSB)
{
	HRESULT hr = ProxyInterface->CreateStateBlock(Type, ppSB);

	if (SUCCEEDED(hr))
	{
		*ppSB = ProxyAddressLookupTable->FindAddress<m_IDirect3DStateBlock9>(*ppSB);
	}

	return hr;
}

HRESULT m_IDirect3DDevice9Ex::EndStateBlock(THIS_ IDirect3DStateBlock9** ppSB)
{
	HRESULT hr = ProxyInterface->EndStateBlock(ppSB);

	if (SUCCEEDED(hr))
	{
		*ppSB = ProxyAddressLookupTable->FindAddress<m_IDirect3DStateBlock9>(*ppSB);
	}

	return hr;
}

HRESULT m_IDirect3DDevice9Ex::GetClipStatus(D3DCLIPSTATUS9 *pClipStatus)
{
	return ProxyInterface->GetClipStatus(pClipStatus);
}

HRESULT m_IDirect3DDevice9Ex::GetDisplayMode(THIS_ UINT iSwapChain, D3DDISPLAYMODE* pMode)
{
	return ProxyInterface->GetDisplayMode(iSwapChain, pMode);
}

HRESULT m_IDirect3DDevice9Ex::GetRenderState(D3DRENDERSTATETYPE State, DWORD *pValue)
{
	return ProxyInterface->GetRenderState(State, pValue);
}

HRESULT m_IDirect3DDevice9Ex::GetRenderTarget(THIS_ DWORD RenderTargetIndex, IDirect3DSurface9** ppRenderTarget)
{
	HRESULT hr = ProxyInterface->GetRenderTarget(RenderTargetIndex, ppRenderTarget);

	if (SUCCEEDED(hr))
	{
		*ppRenderTarget = ProxyAddressLookupTable->FindAddress<m_IDirect3DSurface9>(*ppRenderTarget);
	}

	return hr;
}

HRESULT m_IDirect3DDevice9Ex::GetTransform(D3DTRANSFORMSTATETYPE State, D3DMATRIX *pMatrix)
{
	return ProxyInterface->GetTransform(State, pMatrix);
}

HRESULT m_IDirect3DDevice9Ex::SetClipStatus(CONST D3DCLIPSTATUS9 *pClipStatus)
{
	return ProxyInterface->SetClipStatus(pClipStatus);
}

HRESULT m_IDirect3DDevice9Ex::SetRenderState(D3DRENDERSTATETYPE State, DWORD Value)
{
	return ProxyInterface->SetRenderState(State, Value);
}

HRESULT m_IDirect3DDevice9Ex::SetRenderTarget(THIS_ DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget)
{
	if (pRenderTarget)
	{
		pRenderTarget = static_cast<m_IDirect3DSurface9 *>(pRenderTarget)->GetProxyInterface();
	}

	return ProxyInterface->SetRenderTarget(RenderTargetIndex, pRenderTarget);
}

HRESULT m_IDirect3DDevice9Ex::SetTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX *pMatrix)
{
	return ProxyInterface->SetTransform(State, pMatrix);
}

void m_IDirect3DDevice9Ex::GetGammaRamp(THIS_ UINT iSwapChain, D3DGAMMARAMP* pRamp)
{
	return ProxyInterface->GetGammaRamp(iSwapChain, pRamp);
}

void m_IDirect3DDevice9Ex::SetGammaRamp(THIS_ UINT iSwapChain, DWORD Flags, CONST D3DGAMMARAMP* pRamp)
{
	return ProxyInterface->SetGammaRamp(iSwapChain, Flags, pRamp);
}

HRESULT m_IDirect3DDevice9Ex::DeletePatch(UINT Handle)
{
	return ProxyInterface->DeletePatch(Handle);
}

HRESULT m_IDirect3DDevice9Ex::DrawRectPatch(UINT Handle, CONST float *pNumSegs, CONST D3DRECTPATCH_INFO *pRectPatchInfo)
{
	return ProxyInterface->DrawRectPatch(Handle, pNumSegs, pRectPatchInfo);
}

HRESULT m_IDirect3DDevice9Ex::DrawTriPatch(UINT Handle, CONST float *pNumSegs, CONST D3DTRIPATCH_INFO *pTriPatchInfo)
{
	return ProxyInterface->DrawTriPatch(Handle, pNumSegs, pTriPatchInfo);
}

HRESULT m_IDirect3DDevice9Ex::GetIndices(THIS_ IDirect3DIndexBuffer9** ppIndexData)
{
	HRESULT hr = ProxyInterface->GetIndices(ppIndexData);

	if (SUCCEEDED(hr))
	{
		*ppIndexData = ProxyAddressLookupTable->FindAddress<m_IDirect3DIndexBuffer9>(*ppIndexData);
	}

	return hr;
}

HRESULT m_IDirect3DDevice9Ex::SetIndices(THIS_ IDirect3DIndexBuffer9* pIndexData)
{
	if (pIndexData)
	{
		pIndexData = static_cast<m_IDirect3DIndexBuffer9 *>(pIndexData)->GetProxyInterface();
	}

	return ProxyInterface->SetIndices(pIndexData);
}

UINT m_IDirect3DDevice9Ex::GetAvailableTextureMem()
{
	return ProxyInterface->GetAvailableTextureMem();
}

HRESULT m_IDirect3DDevice9Ex::GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS *pParameters)
{
	return ProxyInterface->GetCreationParameters(pParameters);
}

HRESULT m_IDirect3DDevice9Ex::GetDeviceCaps(D3DCAPS9 *pCaps)
{
	return ProxyInterface->GetDeviceCaps(pCaps);
}

HRESULT m_IDirect3DDevice9Ex::GetDirect3D(IDirect3D9 **ppD3D9)
{
	if (ppD3D9)
	{
		m_pD3DEx->AddRef();

		*ppD3D9 = m_pD3DEx;

		return D3D_OK;
	}
	return D3DERR_INVALIDCALL;
}

HRESULT m_IDirect3DDevice9Ex::GetRasterStatus(THIS_ UINT iSwapChain, D3DRASTER_STATUS* pRasterStatus)
{
	return ProxyInterface->GetRasterStatus(iSwapChain, pRasterStatus);
}

HRESULT m_IDirect3DDevice9Ex::GetLight(DWORD Index, D3DLIGHT9 *pLight)
{
	return ProxyInterface->GetLight(Index, pLight);
}

HRESULT m_IDirect3DDevice9Ex::GetLightEnable(DWORD Index, BOOL *pEnable)
{
	return ProxyInterface->GetLightEnable(Index, pEnable);
}

HRESULT m_IDirect3DDevice9Ex::GetMaterial(D3DMATERIAL9 *pMaterial)
{
	return ProxyInterface->GetMaterial(pMaterial);
}

HRESULT m_IDirect3DDevice9Ex::LightEnable(DWORD LightIndex, BOOL bEnable)
{
	return ProxyInterface->LightEnable(LightIndex, bEnable);
}

HRESULT m_IDirect3DDevice9Ex::SetLight(DWORD Index, CONST D3DLIGHT9 *pLight)
{

	return ProxyInterface->SetLight(Index, pLight);
}

HRESULT m_IDirect3DDevice9Ex::SetMaterial(CONST D3DMATERIAL9 *pMaterial)
{
	return ProxyInterface->SetMaterial(pMaterial);
}

HRESULT m_IDirect3DDevice9Ex::MultiplyTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX *pMatrix)
{
	return ProxyInterface->MultiplyTransform(State, pMatrix);
}

HRESULT m_IDirect3DDevice9Ex::ProcessVertices(THIS_ UINT SrcStartIndex, UINT DestIndex, UINT VertexCount, IDirect3DVertexBuffer9* pDestBuffer, IDirect3DVertexDeclaration9* pVertexDecl, DWORD Flags)
{
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
	return ProxyInterface->TestCooperativeLevel();
}

HRESULT m_IDirect3DDevice9Ex::GetCurrentTexturePalette(UINT *pPaletteNumber)
{
	return ProxyInterface->GetCurrentTexturePalette(pPaletteNumber);
}

HRESULT m_IDirect3DDevice9Ex::GetPaletteEntries(UINT PaletteNumber, PALETTEENTRY *pEntries)
{
	return ProxyInterface->GetPaletteEntries(PaletteNumber, pEntries);
}

HRESULT m_IDirect3DDevice9Ex::SetCurrentTexturePalette(UINT PaletteNumber)
{
	return ProxyInterface->SetCurrentTexturePalette(PaletteNumber);
}

HRESULT m_IDirect3DDevice9Ex::SetPaletteEntries(UINT PaletteNumber, CONST PALETTEENTRY *pEntries)
{
	return ProxyInterface->SetPaletteEntries(PaletteNumber, pEntries);
}

HRESULT m_IDirect3DDevice9Ex::CreatePixelShader(THIS_ CONST DWORD* pFunction, IDirect3DPixelShader9** ppShader)
{
	HRESULT hr = ProxyInterface->CreatePixelShader(pFunction, ppShader);

	if (SUCCEEDED(hr))
	{
		*ppShader = ProxyAddressLookupTable->FindAddress<m_IDirect3DPixelShader9>(*ppShader);
	}

	return hr;
}

HRESULT m_IDirect3DDevice9Ex::GetPixelShader(THIS_ IDirect3DPixelShader9** ppShader)
{
	HRESULT hr = ProxyInterface->GetPixelShader(ppShader);

	if (SUCCEEDED(hr))
	{
		*ppShader = ProxyAddressLookupTable->FindAddress<m_IDirect3DPixelShader9>(*ppShader);
	}

	return hr;
}

HRESULT m_IDirect3DDevice9Ex::SetPixelShader(THIS_ IDirect3DPixelShader9* pShader)
{
	if (pShader)
	{
		pShader = static_cast<m_IDirect3DPixelShader9 *>(pShader)->GetProxyInterface();
	}

	return ProxyInterface->SetPixelShader(pShader);
}

HRESULT m_IDirect3DDevice9Ex::Present(CONST RECT *pSourceRect, CONST RECT *pDestRect, HWND hDestWindowOverride, CONST RGNDATA *pDirtyRegion)
{
	return ProxyInterface->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
}

HRESULT m_IDirect3DDevice9Ex::DrawIndexedPrimitive(THIS_ D3DPRIMITIVETYPE Type, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount)
{
	return ProxyInterface->DrawIndexedPrimitive(Type, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
}

HRESULT m_IDirect3DDevice9Ex::DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT MinIndex, UINT NumVertices, UINT PrimitiveCount, CONST void *pIndexData, D3DFORMAT IndexDataFormat, CONST void *pVertexStreamZeroData, UINT VertexStreamZeroStride)
{
	return ProxyInterface->DrawIndexedPrimitiveUP(PrimitiveType, MinIndex, NumVertices, PrimitiveCount, pIndexData, IndexDataFormat, pVertexStreamZeroData, VertexStreamZeroStride);
}

HRESULT m_IDirect3DDevice9Ex::DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount)
{
	return ProxyInterface->DrawPrimitive(PrimitiveType, StartVertex, PrimitiveCount);
}

HRESULT m_IDirect3DDevice9Ex::DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, CONST void *pVertexStreamZeroData, UINT VertexStreamZeroStride)
{
	return ProxyInterface->DrawPrimitiveUP(PrimitiveType, PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride);
}

HRESULT m_IDirect3DDevice9Ex::BeginScene()
{
	return ProxyInterface->BeginScene();
}

HRESULT m_IDirect3DDevice9Ex::GetStreamSource(THIS_ UINT StreamNumber, IDirect3DVertexBuffer9** ppStreamData, UINT* OffsetInBytes, UINT* pStride)
{
	HRESULT hr = ProxyInterface->GetStreamSource(StreamNumber, ppStreamData, OffsetInBytes, pStride);

	if (SUCCEEDED(hr))
	{
		*ppStreamData = ProxyAddressLookupTable->FindAddress<m_IDirect3DVertexBuffer9>(*ppStreamData);
	}

	return hr;
}

HRESULT m_IDirect3DDevice9Ex::SetStreamSource(THIS_ UINT StreamNumber, IDirect3DVertexBuffer9* pStreamData, UINT OffsetInBytes, UINT Stride)
{
	if (pStreamData)
	{
		pStreamData = static_cast<m_IDirect3DVertexBuffer9 *>(pStreamData)->GetProxyInterface();
	}

	return ProxyInterface->SetStreamSource(StreamNumber, pStreamData, OffsetInBytes, Stride);
}

HRESULT m_IDirect3DDevice9Ex::GetBackBuffer(THIS_ UINT iSwapChain, UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface9** ppBackBuffer)
{
	HRESULT hr = ProxyInterface->GetBackBuffer(iSwapChain, iBackBuffer, Type, ppBackBuffer);

	if (SUCCEEDED(hr))
	{
		*ppBackBuffer = ProxyAddressLookupTable->FindAddress<m_IDirect3DSurface9>(*ppBackBuffer);
	}

	return hr;
}

HRESULT m_IDirect3DDevice9Ex::GetDepthStencilSurface(IDirect3DSurface9 **ppZStencilSurface)
{
	HRESULT hr = ProxyInterface->GetDepthStencilSurface(ppZStencilSurface);

	if (SUCCEEDED(hr))
	{
		*ppZStencilSurface = ProxyAddressLookupTable->FindAddress<m_IDirect3DSurface9>(*ppZStencilSurface);
	}

	return hr;
}

HRESULT m_IDirect3DDevice9Ex::GetTexture(DWORD Stage, IDirect3DBaseTexture9 **ppTexture)
{
	HRESULT hr = ProxyInterface->GetTexture(Stage, ppTexture);

	if (SUCCEEDED(hr) && (*ppTexture))
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
	return ProxyInterface->GetTextureStageState(Stage, Type, pValue);
}

HRESULT m_IDirect3DDevice9Ex::SetTexture(DWORD Stage, IDirect3DBaseTexture9 *pTexture)
{
	if (pTexture)
	{
		switch (pTexture->GetType())
		{
		case D3DRTYPE_TEXTURE:
			pTexture = static_cast<m_IDirect3DTexture9 *>(pTexture)->GetProxyInterface();
			break;
		case D3DRTYPE_VOLUMETEXTURE:
			pTexture = static_cast<m_IDirect3DVolumeTexture9 *>(pTexture)->GetProxyInterface();
			break;
		case D3DRTYPE_CUBETEXTURE:
			pTexture = static_cast<m_IDirect3DCubeTexture9 *>(pTexture)->GetProxyInterface();
			break;
		default:
			return D3DERR_INVALIDCALL;
		}
	}

	return ProxyInterface->SetTexture(Stage, pTexture);
}

HRESULT m_IDirect3DDevice9Ex::SetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value)
{
	return ProxyInterface->SetTextureStageState(Stage, Type, Value);
}

HRESULT m_IDirect3DDevice9Ex::UpdateTexture(IDirect3DBaseTexture9 *pSourceTexture, IDirect3DBaseTexture9 *pDestinationTexture)
{
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
	return ProxyInterface->ValidateDevice(pNumPasses);
}

HRESULT m_IDirect3DDevice9Ex::GetClipPlane(DWORD Index, float *pPlane)
{
	return ProxyInterface->GetClipPlane(Index, pPlane);
}

HRESULT m_IDirect3DDevice9Ex::SetClipPlane(DWORD Index, CONST float *pPlane)
{
	return ProxyInterface->SetClipPlane(Index, pPlane);
}

HRESULT m_IDirect3DDevice9Ex::Clear(DWORD Count, CONST D3DRECT *pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil)
{
	return ProxyInterface->Clear(Count, pRects, Flags, Color, Z, Stencil);
}

HRESULT m_IDirect3DDevice9Ex::GetViewport(D3DVIEWPORT9 *pViewport)
{
	return ProxyInterface->GetViewport(pViewport);
}

HRESULT m_IDirect3DDevice9Ex::SetViewport(CONST D3DVIEWPORT9 *pViewport)
{
	return ProxyInterface->SetViewport(pViewport);
}

HRESULT m_IDirect3DDevice9Ex::CreateVertexShader(THIS_ CONST DWORD* pFunction, IDirect3DVertexShader9** ppShader)
{
	HRESULT hr = ProxyInterface->CreateVertexShader(pFunction, ppShader);

	if (SUCCEEDED(hr))
	{
		*ppShader = ProxyAddressLookupTable->FindAddress<m_IDirect3DVertexShader9>(*ppShader);
	}

	return hr;
}

HRESULT m_IDirect3DDevice9Ex::GetVertexShader(THIS_ IDirect3DVertexShader9** ppShader)
{
	HRESULT hr = ProxyInterface->GetVertexShader(ppShader);

	if (SUCCEEDED(hr))
	{
		*ppShader = ProxyAddressLookupTable->FindAddress<m_IDirect3DVertexShader9>(*ppShader);
	}

	return hr;
}

HRESULT m_IDirect3DDevice9Ex::SetVertexShader(THIS_ IDirect3DVertexShader9* pShader)
{
	if (pShader)
	{
		pShader = static_cast<m_IDirect3DVertexShader9 *>(pShader)->GetProxyInterface();
	}

	return ProxyInterface->SetVertexShader(pShader);
}

HRESULT m_IDirect3DDevice9Ex::CreateQuery(THIS_ D3DQUERYTYPE Type, IDirect3DQuery9** ppQuery)
{
	HRESULT hr = ProxyInterface->CreateQuery(Type, ppQuery);

	if (SUCCEEDED(hr) && ppQuery)
	{
		*ppQuery = ProxyAddressLookupTable->FindAddress<m_IDirect3DQuery9>(*ppQuery);
	}

	return hr;
}

HRESULT m_IDirect3DDevice9Ex::SetPixelShaderConstantB(THIS_ UINT StartRegister, CONST BOOL* pConstantData, UINT  BoolCount)
{
	return ProxyInterface->SetPixelShaderConstantB(StartRegister, pConstantData, BoolCount);
}

HRESULT m_IDirect3DDevice9Ex::GetPixelShaderConstantB(THIS_ UINT StartRegister, BOOL* pConstantData, UINT BoolCount)
{
	return ProxyInterface->GetPixelShaderConstantB(StartRegister, pConstantData, BoolCount);
}

HRESULT m_IDirect3DDevice9Ex::SetPixelShaderConstantI(THIS_ UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount)
{
	return ProxyInterface->SetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount);
}

HRESULT m_IDirect3DDevice9Ex::GetPixelShaderConstantI(THIS_ UINT StartRegister, int* pConstantData, UINT Vector4iCount)
{
	return ProxyInterface->GetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount);
}

HRESULT m_IDirect3DDevice9Ex::SetPixelShaderConstantF(THIS_ UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount)
{
	return ProxyInterface->SetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount);
}

HRESULT m_IDirect3DDevice9Ex::GetPixelShaderConstantF(THIS_ UINT StartRegister, float* pConstantData, UINT Vector4fCount)
{
	return ProxyInterface->GetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount);
}

HRESULT m_IDirect3DDevice9Ex::SetStreamSourceFreq(THIS_ UINT StreamNumber, UINT Divider)
{
	return ProxyInterface->SetStreamSourceFreq(StreamNumber, Divider);
}

HRESULT m_IDirect3DDevice9Ex::GetStreamSourceFreq(THIS_ UINT StreamNumber, UINT* Divider)
{
	return ProxyInterface->GetStreamSourceFreq(StreamNumber, Divider);
}

HRESULT m_IDirect3DDevice9Ex::SetVertexShaderConstantB(THIS_ UINT StartRegister, CONST BOOL* pConstantData, UINT  BoolCount)
{
	return ProxyInterface->SetVertexShaderConstantB(StartRegister, pConstantData, BoolCount);
}

HRESULT m_IDirect3DDevice9Ex::GetVertexShaderConstantB(THIS_ UINT StartRegister, BOOL* pConstantData, UINT BoolCount)
{
	return ProxyInterface->GetVertexShaderConstantB(StartRegister, pConstantData, BoolCount);
}

HRESULT m_IDirect3DDevice9Ex::SetVertexShaderConstantF(THIS_ UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount)
{
	return ProxyInterface->SetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount);
}

HRESULT m_IDirect3DDevice9Ex::GetVertexShaderConstantF(THIS_ UINT StartRegister, float* pConstantData, UINT Vector4fCount)
{
	return ProxyInterface->GetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount);
}

HRESULT m_IDirect3DDevice9Ex::SetVertexShaderConstantI(THIS_ UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount)
{
	return ProxyInterface->SetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount);
}

HRESULT m_IDirect3DDevice9Ex::GetVertexShaderConstantI(THIS_ UINT StartRegister, int* pConstantData, UINT Vector4iCount)
{
	return ProxyInterface->GetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount);
}

HRESULT m_IDirect3DDevice9Ex::SetFVF(THIS_ DWORD FVF)
{
	return ProxyInterface->SetFVF(FVF);
}

HRESULT m_IDirect3DDevice9Ex::GetFVF(THIS_ DWORD* pFVF)
{
	return ProxyInterface->GetFVF(pFVF);
}

HRESULT m_IDirect3DDevice9Ex::CreateVertexDeclaration(THIS_ CONST D3DVERTEXELEMENT9* pVertexElements, IDirect3DVertexDeclaration9** ppDecl)
{
	HRESULT hr = ProxyInterface->CreateVertexDeclaration(pVertexElements, ppDecl);

	if (SUCCEEDED(hr))
	{
		*ppDecl = ProxyAddressLookupTable->FindAddress<m_IDirect3DVertexDeclaration9>(*ppDecl);
	}

	return hr;
}

HRESULT m_IDirect3DDevice9Ex::SetVertexDeclaration(THIS_ IDirect3DVertexDeclaration9* pDecl)
{
	if (pDecl)
	{
		pDecl = static_cast<m_IDirect3DVertexDeclaration9 *>(pDecl)->GetProxyInterface();
	}

	return ProxyInterface->SetVertexDeclaration(pDecl);
}

HRESULT m_IDirect3DDevice9Ex::GetVertexDeclaration(THIS_ IDirect3DVertexDeclaration9** ppDecl)
{
	HRESULT hr = ProxyInterface->GetVertexDeclaration(ppDecl);

	if (SUCCEEDED(hr))
	{
		*ppDecl = ProxyAddressLookupTable->FindAddress<m_IDirect3DVertexDeclaration9>(*ppDecl);
	}

	return hr;
}

HRESULT m_IDirect3DDevice9Ex::SetNPatchMode(THIS_ float nSegments)
{
	return ProxyInterface->SetNPatchMode(nSegments);
}

float m_IDirect3DDevice9Ex::GetNPatchMode(THIS)
{
	return ProxyInterface->GetNPatchMode();
}

int m_IDirect3DDevice9Ex::GetSoftwareVertexProcessing(THIS)
{
	return ProxyInterface->GetSoftwareVertexProcessing();
}

unsigned int m_IDirect3DDevice9Ex::GetNumberOfSwapChains(THIS)
{
	return ProxyInterface->GetNumberOfSwapChains();
}

HRESULT m_IDirect3DDevice9Ex::EvictManagedResources(THIS)
{
	return ProxyInterface->EvictManagedResources();
}

HRESULT m_IDirect3DDevice9Ex::SetSoftwareVertexProcessing(THIS_ BOOL bSoftware)
{
	return ProxyInterface->SetSoftwareVertexProcessing(bSoftware);
}

HRESULT m_IDirect3DDevice9Ex::SetScissorRect(THIS_ CONST RECT* pRect)
{
	return ProxyInterface->SetScissorRect(pRect);
}

HRESULT m_IDirect3DDevice9Ex::GetScissorRect(THIS_ RECT* pRect)
{
	return ProxyInterface->GetScissorRect(pRect);
}

HRESULT m_IDirect3DDevice9Ex::GetSamplerState(THIS_ DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD* pValue)
{
	return ProxyInterface->GetSamplerState(Sampler, Type, pValue);
}

HRESULT m_IDirect3DDevice9Ex::SetSamplerState(THIS_ DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value)
{
	return ProxyInterface->SetSamplerState(Sampler, Type, Value);
}

HRESULT m_IDirect3DDevice9Ex::SetDepthStencilSurface(THIS_ IDirect3DSurface9* pNewZStencil)
{
	if (pNewZStencil)
	{
		pNewZStencil = static_cast<m_IDirect3DSurface9 *>(pNewZStencil)->GetProxyInterface();
	}

	return ProxyInterface->SetDepthStencilSurface(pNewZStencil);
}

HRESULT m_IDirect3DDevice9Ex::CreateOffscreenPlainSurface(THIS_ UINT Width, UINT Height, D3DFORMAT Format, D3DPOOL Pool, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle)
{
	HRESULT hr = ProxyInterface->CreateOffscreenPlainSurface(Width, Height, Format, Pool, ppSurface, pSharedHandle);

	if (SUCCEEDED(hr))
	{
		*ppSurface = ProxyAddressLookupTable->FindAddress<m_IDirect3DSurface9>(*ppSurface);
	}

	return hr;
}

HRESULT m_IDirect3DDevice9Ex::ColorFill(THIS_ IDirect3DSurface9* pSurface, CONST RECT* pRect, D3DCOLOR color)
{
	if (pSurface)
	{
		pSurface = static_cast<m_IDirect3DSurface9 *>(pSurface)->GetProxyInterface();
	}

	return ProxyInterface->ColorFill(pSurface, pRect, color);
}

HRESULT m_IDirect3DDevice9Ex::StretchRect(THIS_ IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestSurface, CONST RECT* pDestRect, D3DTEXTUREFILTERTYPE Filter)
{
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
	if (pDestSurface)
	{
		pDestSurface = static_cast<m_IDirect3DSurface9 *>(pDestSurface)->GetProxyInterface();
	}

	return ProxyInterface->GetFrontBufferData(iSwapChain, pDestSurface);
}

HRESULT m_IDirect3DDevice9Ex::GetRenderTargetData(THIS_ IDirect3DSurface9* pRenderTarget, IDirect3DSurface9* pDestSurface)
{
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
	return ProxyInterface->SetDialogBoxMode(bEnableDialogs);
}

HRESULT m_IDirect3DDevice9Ex::GetSwapChain(THIS_ UINT iSwapChain, IDirect3DSwapChain9** ppSwapChain)
{
	HRESULT hr = ProxyInterface->GetSwapChain(iSwapChain, ppSwapChain);

	if (SUCCEEDED(hr))
	{
		*ppSwapChain = ProxyAddressLookupTable->FindAddress<m_IDirect3DSwapChain9>(*ppSwapChain);
	}

	return hr;
}

HRESULT m_IDirect3DDevice9Ex::SetConvolutionMonoKernel(THIS_ UINT width, UINT height, float* rows, float* columns)
{
	return ProxyInterface->SetConvolutionMonoKernel(width, height, rows, columns);
}

HRESULT m_IDirect3DDevice9Ex::ComposeRects(THIS_ IDirect3DSurface9* pSrc, IDirect3DSurface9* pDst, IDirect3DVertexBuffer9* pSrcRectDescs, UINT NumRects, IDirect3DVertexBuffer9* pDstRectDescs, D3DCOMPOSERECTSOP Operation, int Xoffset, int Yoffset)
{
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

	return ProxyInterface->ComposeRects(pSrc, pDst, pSrcRectDescs, NumRects, pDstRectDescs, Operation, Xoffset, Yoffset);
}

HRESULT m_IDirect3DDevice9Ex::PresentEx(THIS_ CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion, DWORD dwFlags)
{
	return ProxyInterface->PresentEx(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion, dwFlags);
}

HRESULT m_IDirect3DDevice9Ex::GetGPUThreadPriority(THIS_ INT* pPriority)
{
	return ProxyInterface->GetGPUThreadPriority(pPriority);
}

HRESULT m_IDirect3DDevice9Ex::SetGPUThreadPriority(THIS_ INT Priority)
{
	return ProxyInterface->SetGPUThreadPriority(Priority);
}

HRESULT m_IDirect3DDevice9Ex::WaitForVBlank(THIS_ UINT iSwapChain)
{
	return ProxyInterface->WaitForVBlank(iSwapChain);
}

HRESULT m_IDirect3DDevice9Ex::CheckResourceResidency(THIS_ IDirect3DResource9** pResourceArray, UINT32 NumResources)
{
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

	return ProxyInterface->CheckResourceResidency(pResourceArray, NumResources);
}

HRESULT m_IDirect3DDevice9Ex::SetMaximumFrameLatency(THIS_ UINT MaxLatency)
{
	return ProxyInterface->SetMaximumFrameLatency(MaxLatency);
}

HRESULT m_IDirect3DDevice9Ex::GetMaximumFrameLatency(THIS_ UINT* pMaxLatency)
{
	return ProxyInterface->GetMaximumFrameLatency(pMaxLatency);
}

HRESULT m_IDirect3DDevice9Ex::CheckDeviceState(THIS_ HWND hDestinationWindow)
{
	return ProxyInterface->CheckDeviceState(hDestinationWindow);
}

HRESULT m_IDirect3DDevice9Ex::CreateRenderTargetEx(THIS_ UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle, DWORD Usage)
{
	if (DeviceMultiSampleType != D3DMULTISAMPLE_NONE)
	{
		MultiSample = DeviceMultiSampleType;
		MultisampleQuality = DeviceMultiSampleQuality;
	}

	HRESULT hr = ProxyInterface->CreateRenderTargetEx(Width, Height, Format, MultiSample, MultisampleQuality, Lockable, ppSurface, pSharedHandle, Usage);

	if (SUCCEEDED(hr))
	{
		*ppSurface = ProxyAddressLookupTable->FindAddress<m_IDirect3DSurface9>(*ppSurface);
	}

	return hr;
}

HRESULT m_IDirect3DDevice9Ex::CreateOffscreenPlainSurfaceEx(THIS_ UINT Width, UINT Height, D3DFORMAT Format, D3DPOOL Pool, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle, DWORD Usage)
{
	HRESULT hr = ProxyInterface->CreateOffscreenPlainSurfaceEx(Width, Height, Format, Pool, ppSurface, pSharedHandle, Usage);

	if (SUCCEEDED(hr))
	{
		*ppSurface = ProxyAddressLookupTable->FindAddress<m_IDirect3DSurface9>(*ppSurface);
	}

	return hr;
}

HRESULT m_IDirect3DDevice9Ex::CreateDepthStencilSurfaceEx(THIS_ UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Discard, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle, DWORD Usage)
{
	if (DeviceMultiSampleType != D3DMULTISAMPLE_NONE)
	{
		MultiSample = DeviceMultiSampleType;
		MultisampleQuality = DeviceMultiSampleQuality;
	}

	HRESULT hr = ProxyInterface->CreateDepthStencilSurfaceEx(Width, Height, Format, MultiSample, MultisampleQuality, Discard, ppSurface, pSharedHandle, Usage);

	if (SUCCEEDED(hr))
	{
		*ppSurface = ProxyAddressLookupTable->FindAddress<m_IDirect3DSurface9>(*ppSurface);
	}

	return hr;
}

HRESULT m_IDirect3DDevice9Ex::ResetEx(THIS_ D3DPRESENT_PARAMETERS* pPresentationParameters, D3DDISPLAYMODEEX *pFullscreenDisplayMode)
{
	if (DeviceMultiSampleType != D3DMULTISAMPLE_NONE)
	{
		pPresentationParameters->MultiSampleType = DeviceMultiSampleType;
		pPresentationParameters->MultiSampleQuality = DeviceMultiSampleQuality;
	}

	return ProxyInterface->ResetEx(pPresentationParameters, pFullscreenDisplayMode);
}

HRESULT m_IDirect3DDevice9Ex::GetDisplayModeEx(THIS_ UINT iSwapChain, D3DDISPLAYMODEEX* pMode, D3DDISPLAYROTATION* pRotation)
{
	return ProxyInterface->GetDisplayModeEx(iSwapChain, pMode, pRotation);
}
