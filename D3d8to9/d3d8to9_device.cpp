/**
 * Copyright (C) 2015 Patrick Mours. All rights reserved.
 * License: https://github.com/crosire/d3d8to9#license
 */

#include "d3dx9.hpp"
#include "d3d8to9.hpp"
#include <regex>
#include <assert.h>

struct VertexShaderInfo
{
	IDirect3DVertexShader9 *Shader;
	IDirect3DVertexDeclaration9 *Declaration;
};

// IDirect3DDevice8
Direct3DDevice8::Direct3DDevice8(Direct3D8 *d3d, IDirect3DDevice9 *ProxyInterface, BOOL EnableZBufferDiscarding) :
	D3D(d3d), ProxyInterface(ProxyInterface), ZBufferDiscarding(EnableZBufferDiscarding)
{
	ProxyAddressLookupTable = new AddressLookupTable(this);
	PaletteFlag = SupportsPalettes();
}
Direct3DDevice8::~Direct3DDevice8()
{
	delete ProxyAddressLookupTable;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::QueryInterface(REFIID riid, void **ppvObj)
{
	if (ppvObj == nullptr)
	{
		return E_POINTER;
	}

	if (riid == __uuidof(this) ||
		riid == __uuidof(IUnknown))
	{
		AddRef();

		*ppvObj = this;

		return S_OK;
	}

	return ProxyInterface->QueryInterface(riid, ppvObj);
}
ULONG STDMETHODCALLTYPE Direct3DDevice8::AddRef()
{
	return ProxyInterface->AddRef();
}
ULONG STDMETHODCALLTYPE Direct3DDevice8::Release()
{
	ULONG LastRefCount = ProxyInterface->Release();

	if (LastRefCount == 0)
	{
		delete this;
	}

	return LastRefCount;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::TestCooperativeLevel()
{
	return ProxyInterface->TestCooperativeLevel();
}
UINT STDMETHODCALLTYPE Direct3DDevice8::GetAvailableTextureMem()
{
	return ProxyInterface->GetAvailableTextureMem();
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::ResourceManagerDiscardBytes(DWORD Bytes)
{
	UNREFERENCED_PARAMETER(Bytes);

	return ProxyInterface->EvictManagedResources();
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetDirect3D(Direct3D8 **ppD3D8)
{
	if (ppD3D8 == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	D3D->AddRef();

	*ppD3D8 = D3D;

	return D3D_OK;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetDeviceCaps(D3DCAPS8 *pCaps)
{
	if (pCaps == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	D3DCAPS9 DeviceCaps;

	const HRESULT hr = ProxyInterface->GetDeviceCaps(&DeviceCaps);

	if (FAILED(hr))
	{
		return hr;
	}

	ConvertCaps(DeviceCaps, *pCaps);

	return D3D_OK;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetDisplayMode(D3DDISPLAYMODE *pMode)
{
	return ProxyInterface->GetDisplayMode(0, pMode);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS *pParameters)
{
	return ProxyInterface->GetCreationParameters(pParameters);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::SetCursorProperties(UINT XHotSpot, UINT YHotSpot, Direct3DSurface8 *pCursorBitmap)
{
	if (pCursorBitmap == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	return ProxyInterface->SetCursorProperties(XHotSpot, YHotSpot, pCursorBitmap->GetProxyInterface());
}
void STDMETHODCALLTYPE Direct3DDevice8::SetCursorPosition(UINT XScreenSpace, UINT YScreenSpace, DWORD Flags)
{
	ProxyInterface->SetCursorPosition(XScreenSpace, YScreenSpace, Flags);
}
BOOL STDMETHODCALLTYPE Direct3DDevice8::ShowCursor(BOOL bShow)
{
	return ProxyInterface->ShowCursor(bShow);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::CreateAdditionalSwapChain(D3DPRESENT_PARAMETERS8 *pPresentationParameters, Direct3DSwapChain8 **ppSwapChain)
{
#ifndef D3D8TO9NOLOG
	LOG << "Redirecting '" << "IDirect3DDevice8::CreateAdditionalSwapChain" << "(" << this << ", " << pPresentationParameters << ", " << ppSwapChain << ")' ..." << std::endl;
#endif

	if (pPresentationParameters == nullptr || ppSwapChain == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	*ppSwapChain = nullptr;

	D3DPRESENT_PARAMETERS PresentParams;
	ConvertPresentParameters(*pPresentationParameters, PresentParams);

	IDirect3DSwapChain9 *SwapChainInterface = nullptr;

	const HRESULT hr = ProxyInterface->CreateAdditionalSwapChain(&PresentParams, &SwapChainInterface);

	if (FAILED(hr))
	{
		return hr;
	}

	*ppSwapChain = new Direct3DSwapChain8(this, SwapChainInterface);

	return D3D_OK;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::Reset(D3DPRESENT_PARAMETERS8 *pPresentationParameters)
{
#ifndef D3D8TO9NOLOG
	LOG << "Redirecting '" << "IDirect3DDevice8::Reset" << "(" << this << ", " << pPresentationParameters << ")' ..." << std::endl;
#endif

	if (pPresentationParameters == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	D3DPRESENT_PARAMETERS PresentParams;
	ConvertPresentParameters(*pPresentationParameters, PresentParams);

	return ProxyInterface->Reset(&PresentParams);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::Present(const RECT *pSourceRect, const RECT *pDestRect, HWND hDestWindowOverride, const RGNDATA *pDirtyRegion)
{
	UNREFERENCED_PARAMETER(pDirtyRegion);

	return ProxyInterface->Present(pSourceRect, pDestRect, hDestWindowOverride, nullptr);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetBackBuffer(UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, Direct3DSurface8 **ppBackBuffer)
{
	if (ppBackBuffer == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	*ppBackBuffer = nullptr;

	IDirect3DSurface9 *SurfaceInterface = nullptr;

	const HRESULT hr = ProxyInterface->GetBackBuffer(0, iBackBuffer, Type, &SurfaceInterface);

	if (FAILED(hr))
	{
		return hr;
	}

	*ppBackBuffer = ProxyAddressLookupTable->FindAddress<Direct3DSurface8>(SurfaceInterface);

	return D3D_OK;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetRasterStatus(D3DRASTER_STATUS *pRasterStatus)
{
	return ProxyInterface->GetRasterStatus(0, pRasterStatus);
}
void STDMETHODCALLTYPE Direct3DDevice8::SetGammaRamp(DWORD Flags, const D3DGAMMARAMP *pRamp)
{
	ProxyInterface->SetGammaRamp(0, Flags, pRamp);
}
void STDMETHODCALLTYPE Direct3DDevice8::GetGammaRamp(D3DGAMMARAMP *pRamp)
{
	ProxyInterface->GetGammaRamp(0, pRamp);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::CreateTexture(UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, Direct3DTexture8 **ppTexture)
{
	if (ppTexture == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	*ppTexture = nullptr;

	if (Pool == D3DPOOL_DEFAULT)
	{
		D3DDEVICE_CREATION_PARAMETERS CreationParams;
		ProxyInterface->GetCreationParameters(&CreationParams);

		if ((Usage & D3DUSAGE_DYNAMIC) == 0 &&
			SUCCEEDED(D3D->GetProxyInterface()->CheckDeviceFormat(CreationParams.AdapterOrdinal, CreationParams.DeviceType, D3DFMT_X8R8G8B8, D3DUSAGE_RENDERTARGET, D3DRTYPE_TEXTURE, Format)))
		{
			Usage |= D3DUSAGE_RENDERTARGET;
		}
		else
		{
			Usage |= D3DUSAGE_DYNAMIC;
		}
	}

	IDirect3DTexture9 *TextureInterface = nullptr;

	const HRESULT hr = ProxyInterface->CreateTexture(Width, Height, Levels, Usage, Format, Pool, &TextureInterface, nullptr);

	if (FAILED(hr))
	{
		return hr;
	}

	*ppTexture = new Direct3DTexture8(this, TextureInterface);

	return D3D_OK;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::CreateVolumeTexture(UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, Direct3DVolumeTexture8 **ppVolumeTexture)
{
	if (ppVolumeTexture == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	*ppVolumeTexture = nullptr;

	IDirect3DVolumeTexture9 *TextureInterface = nullptr;

	const HRESULT hr = ProxyInterface->CreateVolumeTexture(Width, Height, Depth, Levels, Usage, Format, Pool, &TextureInterface, nullptr);

	if (FAILED(hr))
	{
		return hr;
	}

	*ppVolumeTexture = new Direct3DVolumeTexture8(this, TextureInterface);

	return D3D_OK;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::CreateCubeTexture(UINT EdgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, Direct3DCubeTexture8 **ppCubeTexture)
{
	if (ppCubeTexture == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	*ppCubeTexture = nullptr;

	IDirect3DCubeTexture9 *TextureInterface = nullptr;

	const HRESULT hr = ProxyInterface->CreateCubeTexture(EdgeLength, Levels, Usage, Format, Pool, &TextureInterface, nullptr);

	if (FAILED(hr))
	{
		return hr;
	}

	*ppCubeTexture = new Direct3DCubeTexture8(this, TextureInterface);

	return D3D_OK;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::CreateVertexBuffer(UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool, Direct3DVertexBuffer8 **ppVertexBuffer)
{
	if (ppVertexBuffer == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	*ppVertexBuffer = nullptr;

	IDirect3DVertexBuffer9 *BufferInterface = nullptr;

	const HRESULT hr = ProxyInterface->CreateVertexBuffer(Length, Usage, FVF, Pool, &BufferInterface, nullptr);

	if (FAILED(hr))
	{
		return hr;
	}

	*ppVertexBuffer = new Direct3DVertexBuffer8(this, BufferInterface);

	return D3D_OK;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::CreateIndexBuffer(UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, Direct3DIndexBuffer8 **ppIndexBuffer)
{
	if (ppIndexBuffer == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	*ppIndexBuffer = nullptr;

	IDirect3DIndexBuffer9 *BufferInterface = nullptr;

	const HRESULT hr = ProxyInterface->CreateIndexBuffer(Length, Usage, Format, Pool, &BufferInterface, nullptr);

	if (FAILED(hr))
	{
		return hr;
	}

	*ppIndexBuffer = new Direct3DIndexBuffer8(this, BufferInterface);

	return D3D_OK;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::CreateRenderTarget(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, BOOL Lockable, Direct3DSurface8 **ppSurface)
{
	if (ppSurface == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	*ppSurface = nullptr;

	DWORD QualityLevels = 1;
	D3DDEVICE_CREATION_PARAMETERS CreationParams;
	ProxyInterface->GetCreationParameters(&CreationParams);

	HRESULT hr = D3D->GetProxyInterface()->CheckDeviceMultiSampleType(CreationParams.AdapterOrdinal, CreationParams.DeviceType, Format, FALSE, MultiSample, &QualityLevels);

	if (FAILED(hr))
	{
		return D3DERR_INVALIDCALL;
	}

	IDirect3DSurface9 *SurfaceInterface = nullptr;

	hr = ProxyInterface->CreateRenderTarget(Width, Height, Format, MultiSample, QualityLevels - 1, Lockable, &SurfaceInterface, nullptr);

	if (FAILED(hr))
	{
		return hr;
	}

	*ppSurface = new Direct3DSurface8(this, SurfaceInterface);

	return D3D_OK;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::CreateDepthStencilSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, Direct3DSurface8 **ppSurface)
{
	if (ppSurface == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	*ppSurface = nullptr;

	DWORD QualityLevels = 1;
	D3DDEVICE_CREATION_PARAMETERS CreationParams;
	ProxyInterface->GetCreationParameters(&CreationParams);

	HRESULT hr = D3D->GetProxyInterface()->CheckDeviceMultiSampleType(CreationParams.AdapterOrdinal, CreationParams.DeviceType, Format, FALSE, MultiSample, &QualityLevels);

	if (FAILED(hr))
	{
		return D3DERR_INVALIDCALL;
	}

	IDirect3DSurface9 *SurfaceInterface = nullptr;

	hr = ProxyInterface->CreateDepthStencilSurface(Width, Height, Format, MultiSample, QualityLevels - 1, ZBufferDiscarding, &SurfaceInterface, nullptr);

	if (FAILED(hr))
	{
		return hr;
	}

	*ppSurface = new Direct3DSurface8(this, SurfaceInterface);

	return D3D_OK;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::CreateImageSurface(UINT Width, UINT Height, D3DFORMAT Format, Direct3DSurface8 **ppSurface)
{
#ifndef D3D8TO9NOLOG
	LOG << "Redirecting '" << "IDirect3DDevice8::CreateImageSurface" << "(" << this << ", " << Width << ", " << Height << ", " << Format << ", " << ppSurface << ")' ..." << std::endl;
#endif

	if (ppSurface == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	*ppSurface = nullptr;

	if (Format == D3DFMT_R8G8B8)
	{
#ifndef D3D8TO9NOLOG
		LOG << "> Replacing format 'D3DFMT_R8G8B8' with 'D3DFMT_X8R8G8B8' ..." << std::endl;
#endif

		Format = D3DFMT_X8R8G8B8;
	}

	IDirect3DSurface9 *SurfaceInterface = nullptr;

	const HRESULT hr = ProxyInterface->CreateOffscreenPlainSurface(Width, Height, Format, D3DPOOL_SYSTEMMEM, &SurfaceInterface, nullptr);

	if (FAILED(hr))
	{
#ifndef D3D8TO9NOLOG
		LOG << "> 'IDirect3DDevice9::CreateOffscreenPlainSurface' failed with error code " << std::hex << hr << std::dec << "!" << std::endl;
#endif

		return hr;
	}

	*ppSurface = new Direct3DSurface8(this, SurfaceInterface);

	return D3D_OK;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::CopyRects(Direct3DSurface8 *pSourceSurface, const RECT *pSourceRectsArray, UINT cRects, Direct3DSurface8 *pDestinationSurface, const POINT *pDestPointsArray)
{
	if (pSourceSurface == nullptr || pDestinationSurface == nullptr || pSourceSurface == pDestinationSurface)
	{
		return D3DERR_INVALIDCALL;
	}

	D3DSURFACE_DESC SourceDesc, DestinationDesc;
	pSourceSurface->GetProxyInterface()->GetDesc(&SourceDesc);
	pDestinationSurface->GetProxyInterface()->GetDesc(&DestinationDesc);

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

		if (SourceDesc.Pool == D3DPOOL_MANAGED || DestinationDesc.Pool != D3DPOOL_DEFAULT)
		{
			if (D3DXLoadSurfaceFromSurface != nullptr)
			{
				hr = D3DXLoadSurfaceFromSurface(pDestinationSurface->GetProxyInterface(), nullptr, &DestinationRect, pSourceSurface->GetProxyInterface(), nullptr, &SourceRect, D3DX_FILTER_NONE, 0);
			}
			else
			{
				hr = D3DERR_INVALIDCALL;
			}
		}
		else if (SourceDesc.Pool == D3DPOOL_DEFAULT)
		{
			hr = ProxyInterface->StretchRect(pSourceSurface->GetProxyInterface(), &SourceRect, pDestinationSurface->GetProxyInterface(), &DestinationRect, D3DTEXF_NONE);
		}
		else if (SourceDesc.Pool == D3DPOOL_SYSTEMMEM)
		{
			const POINT pt = { DestinationRect.left, DestinationRect.top };

			hr = ProxyInterface->UpdateSurface(pSourceSurface->GetProxyInterface(), &SourceRect, pDestinationSurface->GetProxyInterface(), &pt);
		}

		if (FAILED(hr))
		{
#ifndef D3D8TO9NOLOG
			LOG << "Failed to translate 'IDirect3DDevice8::CopyRects' call from '[" << SourceDesc.Width << "x" << SourceDesc.Height << ", " << SourceDesc.Format << ", " << SourceDesc.MultiSampleType << ", " << SourceDesc.Usage << ", " << SourceDesc.Pool << "]' to '[" << DestinationDesc.Width << "x" << DestinationDesc.Height << ", " << DestinationDesc.Format << ", " << DestinationDesc.MultiSampleType << ", " << DestinationDesc.Usage << ", " << DestinationDesc.Pool << "]'!" << std::endl;
#endif
			break;
		}
	}

	return hr;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::UpdateTexture(Direct3DBaseTexture8 *pSourceTexture, Direct3DBaseTexture8 *pDestinationTexture)
{
	if (pSourceTexture == nullptr || pDestinationTexture == nullptr || pSourceTexture->GetType() != pDestinationTexture->GetType())
	{
		return D3DERR_INVALIDCALL;
	}

	IDirect3DBaseTexture9 *SourceBaseTextureInterface, *DestinationBaseTextureInterface;

	switch (pSourceTexture->GetType())
	{
		case D3DRTYPE_TEXTURE:
			SourceBaseTextureInterface = static_cast<Direct3DTexture8 *>(pSourceTexture)->GetProxyInterface();
			DestinationBaseTextureInterface = static_cast<Direct3DTexture8 *>(pDestinationTexture)->GetProxyInterface();
			break;
		case D3DRTYPE_VOLUMETEXTURE:
			SourceBaseTextureInterface = static_cast<Direct3DVolumeTexture8 *>(pSourceTexture)->GetProxyInterface();
			DestinationBaseTextureInterface = static_cast<Direct3DVolumeTexture8 *>(pDestinationTexture)->GetProxyInterface();
			break;
		case D3DRTYPE_CUBETEXTURE:
			SourceBaseTextureInterface = static_cast<Direct3DCubeTexture8 *>(pSourceTexture)->GetProxyInterface();
			DestinationBaseTextureInterface = static_cast<Direct3DCubeTexture8 *>(pDestinationTexture)->GetProxyInterface();
			break;
		default:
			return D3DERR_INVALIDCALL;
	}

	return ProxyInterface->UpdateTexture(SourceBaseTextureInterface, DestinationBaseTextureInterface);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetFrontBuffer(Direct3DSurface8 *pDestSurface)
{
	if (pDestSurface == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	return ProxyInterface->GetFrontBufferData(0, pDestSurface->GetProxyInterface());
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::SetRenderTarget(Direct3DSurface8 *pRenderTarget, Direct3DSurface8 *pNewZStencil)
{
	HRESULT hr;

	if (pRenderTarget != nullptr)
	{
		hr = ProxyInterface->SetRenderTarget(0, pRenderTarget->GetProxyInterface());

		if (FAILED(hr))
		{
			return hr;
		}
	}

	if (pNewZStencil != nullptr)
	{
		hr = ProxyInterface->SetDepthStencilSurface(pNewZStencil->GetProxyInterface());

		if (FAILED(hr))
		{
			return hr;
		}
	}
	else
	{
		ProxyInterface->SetDepthStencilSurface(nullptr);
	}

	return D3D_OK;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetRenderTarget(Direct3DSurface8 **ppRenderTarget)
{
	if (ppRenderTarget == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	IDirect3DSurface9 *SurfaceInterface = nullptr;

	const HRESULT hr = ProxyInterface->GetRenderTarget(0, &SurfaceInterface);

	if (FAILED(hr))
	{
		return hr;
	}

	*ppRenderTarget = ProxyAddressLookupTable->FindAddress<Direct3DSurface8>(SurfaceInterface);

	return D3D_OK;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetDepthStencilSurface(Direct3DSurface8 **ppZStencilSurface)
{
	if (ppZStencilSurface == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	IDirect3DSurface9 *SurfaceInterface = nullptr;

	const HRESULT hr = ProxyInterface->GetDepthStencilSurface(&SurfaceInterface);

	if (FAILED(hr))
	{
		return hr;
	}

	*ppZStencilSurface = ProxyAddressLookupTable->FindAddress<Direct3DSurface8>(SurfaceInterface);

	return D3D_OK;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::BeginScene()
{
	return ProxyInterface->BeginScene();
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::EndScene()
{
	return ProxyInterface->EndScene();
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::Clear(DWORD Count, const D3DRECT *pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil)
{
	return ProxyInterface->Clear(Count, pRects, Flags, Color, Z, Stencil);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::SetTransform(D3DTRANSFORMSTATETYPE State, const D3DMATRIX *pMatrix)
{
	return ProxyInterface->SetTransform(State, pMatrix);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetTransform(D3DTRANSFORMSTATETYPE State, D3DMATRIX *pMatrix)
{
	return ProxyInterface->GetTransform(State, pMatrix);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::MultiplyTransform(D3DTRANSFORMSTATETYPE State, const D3DMATRIX *pMatrix)
{
	return ProxyInterface->MultiplyTransform(State, pMatrix);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::SetViewport(const D3DVIEWPORT8 *pViewport)
{
	return ProxyInterface->SetViewport(pViewport);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetViewport(D3DVIEWPORT8 *pViewport)
{
	return ProxyInterface->GetViewport(pViewport);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::SetMaterial(const D3DMATERIAL8 *pMaterial)
{
	return ProxyInterface->SetMaterial(pMaterial);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetMaterial(D3DMATERIAL8 *pMaterial)
{
	return ProxyInterface->GetMaterial(pMaterial);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::SetLight(DWORD Index, const D3DLIGHT8 *pLight)
{
	return ProxyInterface->SetLight(Index, pLight);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetLight(DWORD Index, D3DLIGHT8 *pLight)
{
	return ProxyInterface->GetLight(Index, pLight);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::LightEnable(DWORD Index, BOOL Enable)
{
	return ProxyInterface->LightEnable(Index, Enable);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetLightEnable(DWORD Index, BOOL *pEnable)
{
	return ProxyInterface->GetLightEnable(Index, pEnable);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::SetClipPlane(DWORD Index, const float *pPlane)
{
	return ProxyInterface->SetClipPlane(Index, pPlane);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetClipPlane(DWORD Index, float *pPlane)
{
	return ProxyInterface->GetClipPlane(Index, pPlane);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::SetRenderState(D3DRENDERSTATETYPE State, DWORD Value)
{
	FLOAT Biased;

	switch (static_cast<DWORD>(State))
	{
		case D3DRS_LINEPATTERN:
		case D3DRS_ZVISIBLE:
		case D3DRS_EDGEANTIALIAS:
		case D3DRS_PATCHSEGMENTS:
			return D3DERR_INVALIDCALL;
		case D3DRS_SOFTWAREVERTEXPROCESSING:
			return D3D_OK;
		case D3DRS_ZBIAS:
			Biased = static_cast<FLOAT>(Value) * -0.000005f;
			Value = *reinterpret_cast<const DWORD *>(&Biased);
			State = D3DRS_DEPTHBIAS;
		default:
			return ProxyInterface->SetRenderState(State, Value);
	}
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetRenderState(D3DRENDERSTATETYPE State, DWORD *pValue)
{
	if (pValue == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	HRESULT hr;
	*pValue = 0;

	switch (static_cast<DWORD>(State))
	{
		case D3DRS_LINEPATTERN:
		case D3DRS_ZVISIBLE:
		case D3DRS_EDGEANTIALIAS:
			return D3DERR_INVALIDCALL;
		case D3DRS_ZBIAS:
			hr = ProxyInterface->GetRenderState(D3DRS_DEPTHBIAS, pValue);
			*pValue = static_cast<DWORD>(*reinterpret_cast<const FLOAT *>(pValue) * -500000.0f);
			return hr;
		case D3DRS_SOFTWAREVERTEXPROCESSING:
			*pValue = ProxyInterface->GetSoftwareVertexProcessing();
			return D3D_OK;
		case D3DRS_PATCHSEGMENTS:
			*pValue = 1;
			return D3D_OK;
		default:
			return ProxyInterface->GetRenderState(State, pValue);
	}
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::BeginStateBlock()
{
	return ProxyInterface->BeginStateBlock();
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::EndStateBlock(DWORD *pToken)
{
	if (pToken == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	return ProxyInterface->EndStateBlock(reinterpret_cast<IDirect3DStateBlock9 **>(pToken));
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::ApplyStateBlock(DWORD Token)
{
	if (Token == 0)
	{
		return D3DERR_INVALIDCALL;
	}

	return reinterpret_cast<IDirect3DStateBlock9 *>(Token)->Apply();
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::CaptureStateBlock(DWORD Token)
{
	if (Token == 0)
	{
		return D3DERR_INVALIDCALL;
	}

	return reinterpret_cast<IDirect3DStateBlock9 *>(Token)->Capture();
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::DeleteStateBlock(DWORD Token)
{
	if (Token == 0)
	{
		return D3DERR_INVALIDCALL;
	}

	reinterpret_cast<IDirect3DStateBlock9 *>(Token)->Release();

	return D3D_OK;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::CreateStateBlock(D3DSTATEBLOCKTYPE Type, DWORD *pToken)
{
#ifndef D3D8TO9NOLOG
	LOG << "Redirecting '" << "IDirect3DDevice8::CreateStateBlock" << "(" << Type << ", " << pToken << ")' ..." << std::endl;
#endif

	if (pToken == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	return ProxyInterface->CreateStateBlock(Type, reinterpret_cast<IDirect3DStateBlock9 **>(pToken));
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::SetClipStatus(const D3DCLIPSTATUS8 *pClipStatus)
{
	return ProxyInterface->SetClipStatus(pClipStatus);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetClipStatus(D3DCLIPSTATUS8 *pClipStatus)
{
	return ProxyInterface->GetClipStatus(pClipStatus);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetTexture(DWORD Stage, Direct3DBaseTexture8 **ppTexture)
{
	if (ppTexture == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	*ppTexture = nullptr;

	IDirect3DBaseTexture9 *BaseTextureInterface = nullptr;

	const HRESULT hr = ProxyInterface->GetTexture(Stage, &BaseTextureInterface);

	if (FAILED(hr))
	{
		return hr;
	}

	if (BaseTextureInterface != nullptr)
	{
		IDirect3DTexture9 *TextureInterface = nullptr;
		IDirect3DCubeTexture9 *CubeTextureInterface = nullptr;
		IDirect3DVolumeTexture9 *VolumeTextureInterface = nullptr;

		switch (BaseTextureInterface->GetType())
		{
			case D3DRTYPE_TEXTURE:
				BaseTextureInterface->QueryInterface(IID_PPV_ARGS(&TextureInterface));
				*ppTexture = ProxyAddressLookupTable->FindAddress<Direct3DTexture8>(TextureInterface);
				break;
			case D3DRTYPE_VOLUMETEXTURE:
				BaseTextureInterface->QueryInterface(IID_PPV_ARGS(&VolumeTextureInterface));
				*ppTexture = ProxyAddressLookupTable->FindAddress<Direct3DVolumeTexture8>(VolumeTextureInterface);
				break;
			case D3DRTYPE_CUBETEXTURE:
				BaseTextureInterface->QueryInterface(IID_PPV_ARGS(&CubeTextureInterface));
				*ppTexture = ProxyAddressLookupTable->FindAddress<Direct3DCubeTexture8>(CubeTextureInterface);
				break;
			default:
				return D3DERR_INVALIDCALL;
		}
	}

	return D3D_OK;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::SetTexture(DWORD Stage, Direct3DBaseTexture8 *pTexture)
{
	if (pTexture == nullptr)
	{
		return ProxyInterface->SetTexture(Stage, nullptr);
	}

	IDirect3DBaseTexture9 *BaseTextureInterface;

	switch (pTexture->GetType())
	{
		case D3DRTYPE_TEXTURE:
			BaseTextureInterface = static_cast<Direct3DTexture8 *>(pTexture)->GetProxyInterface();
			break;
		case D3DRTYPE_VOLUMETEXTURE:
			BaseTextureInterface = static_cast<Direct3DVolumeTexture8 *>(pTexture)->GetProxyInterface();
			break;
		case D3DRTYPE_CUBETEXTURE:
			BaseTextureInterface = static_cast<Direct3DCubeTexture8 *>(pTexture)->GetProxyInterface();
			break;
		default:
			return D3DERR_INVALIDCALL;
	}

	return ProxyInterface->SetTexture(Stage, BaseTextureInterface);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD *pValue)
{
	switch (static_cast<DWORD>(Type))
	{
		case D3DTSS_ADDRESSU:
			return ProxyInterface->GetSamplerState(Stage, D3DSAMP_ADDRESSU, pValue);
		case D3DTSS_ADDRESSV:
			return ProxyInterface->GetSamplerState(Stage, D3DSAMP_ADDRESSV, pValue);
		case D3DTSS_ADDRESSW:
			return ProxyInterface->GetSamplerState(Stage, D3DSAMP_ADDRESSW, pValue);
		case D3DTSS_BORDERCOLOR:
			return ProxyInterface->GetSamplerState(Stage, D3DSAMP_BORDERCOLOR, pValue);
		case D3DTSS_MAGFILTER:
			return ProxyInterface->GetSamplerState(Stage, D3DSAMP_MAGFILTER, pValue);
		case D3DTSS_MINFILTER:
			return ProxyInterface->GetSamplerState(Stage, D3DSAMP_MINFILTER, pValue);
		case D3DTSS_MIPFILTER:
			return ProxyInterface->GetSamplerState(Stage, D3DSAMP_MIPFILTER, pValue);
		case D3DTSS_MIPMAPLODBIAS:
			return ProxyInterface->GetSamplerState(Stage, D3DSAMP_MIPMAPLODBIAS, pValue);
		case D3DTSS_MAXMIPLEVEL:
			return ProxyInterface->GetSamplerState(Stage, D3DSAMP_MAXMIPLEVEL, pValue);
		case D3DTSS_MAXANISOTROPY:
			return ProxyInterface->GetSamplerState(Stage, D3DSAMP_MAXANISOTROPY, pValue);
		default:
			return ProxyInterface->GetTextureStageState(Stage, Type, pValue);
	}
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::SetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value)
{
	switch (static_cast<DWORD>(Type))
	{
		case D3DTSS_ADDRESSU:
			return ProxyInterface->SetSamplerState(Stage, D3DSAMP_ADDRESSU, Value);
		case D3DTSS_ADDRESSV:
			return ProxyInterface->SetSamplerState(Stage, D3DSAMP_ADDRESSV, Value);
		case D3DTSS_ADDRESSW:
			return ProxyInterface->SetSamplerState(Stage, D3DSAMP_ADDRESSW, Value);
		case D3DTSS_BORDERCOLOR:
			return ProxyInterface->SetSamplerState(Stage, D3DSAMP_BORDERCOLOR, Value);
		case D3DTSS_MAGFILTER:
			return ProxyInterface->SetSamplerState(Stage, D3DSAMP_MAGFILTER, Value);
		case D3DTSS_MINFILTER:
			return ProxyInterface->SetSamplerState(Stage, D3DSAMP_MINFILTER, Value);
		case D3DTSS_MIPFILTER:
			return ProxyInterface->SetSamplerState(Stage, D3DSAMP_MIPFILTER, Value);
		case D3DTSS_MIPMAPLODBIAS:
			return ProxyInterface->SetSamplerState(Stage, D3DSAMP_MIPMAPLODBIAS, Value);
		case D3DTSS_MAXMIPLEVEL:
			return ProxyInterface->SetSamplerState(Stage, D3DSAMP_MAXMIPLEVEL, Value);
		case D3DTSS_MAXANISOTROPY:
			return ProxyInterface->SetSamplerState(Stage, D3DSAMP_MAXANISOTROPY, Value);
		default:
			return ProxyInterface->SetTextureStageState(Stage, Type, Value);
	}
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::ValidateDevice(DWORD *pNumPasses)
{
	return ProxyInterface->ValidateDevice(pNumPasses);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetInfo(DWORD DevInfoID, void *pDevInfoStruct, DWORD DevInfoStructSize)
{
	UNREFERENCED_PARAMETER(DevInfoID);
	UNREFERENCED_PARAMETER(pDevInfoStruct);
	UNREFERENCED_PARAMETER(DevInfoStructSize);

#ifndef D3D8TO9NOLOG
	LOG << "Redirecting '" << "IDirect3DDevice8::GetInfo" << "(" << this << ", " << DevInfoID << ", " << pDevInfoStruct << ", " << DevInfoStructSize << ")' ..." << std::endl;
#endif

	return S_FALSE;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::SetPaletteEntries(UINT PaletteNumber, const PALETTEENTRY *pEntries)
{
	if (pEntries == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}
	return ProxyInterface->SetPaletteEntries(PaletteNumber, pEntries);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetPaletteEntries(UINT PaletteNumber, PALETTEENTRY *pEntries)
{
	if (pEntries == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}
	return ProxyInterface->GetPaletteEntries(PaletteNumber, pEntries);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::SetCurrentTexturePalette(UINT PaletteNumber)
{
	if (!PaletteFlag)
	{
		return D3DERR_INVALIDCALL;
	}
	return ProxyInterface->SetCurrentTexturePalette(PaletteNumber);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetCurrentTexturePalette(UINT *pPaletteNumber)
{
	if (!PaletteFlag)
	{
		return D3DERR_INVALIDCALL;
	}
	return ProxyInterface->GetCurrentTexturePalette(pPaletteNumber);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount)
{
	return ProxyInterface->DrawPrimitive(PrimitiveType, StartVertex, PrimitiveCount);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::DrawIndexedPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT MinIndex, UINT NumVertices, UINT StartIndex, UINT PrimitiveCount)
{
	return ProxyInterface->DrawIndexedPrimitive(PrimitiveType, CurrentBaseVertexIndex, MinIndex, NumVertices, StartIndex, PrimitiveCount);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, const void *pVertexStreamZeroData, UINT VertexStreamZeroStride)
{
	return ProxyInterface->DrawPrimitiveUP(PrimitiveType, PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex, UINT NumVertexIndices, UINT PrimitiveCount, const void *pIndexData, D3DFORMAT IndexDataFormat, const void *pVertexStreamZeroData, UINT VertexStreamZeroStride)
{
	return ProxyInterface->DrawIndexedPrimitiveUP(PrimitiveType, MinVertexIndex, NumVertexIndices, PrimitiveCount, pIndexData, IndexDataFormat, pVertexStreamZeroData, VertexStreamZeroStride);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::ProcessVertices(UINT SrcStartIndex, UINT DestIndex, UINT VertexCount, Direct3DVertexBuffer8 *pDestBuffer, DWORD Flags)
{
	if (pDestBuffer == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	return ProxyInterface->ProcessVertices(SrcStartIndex, DestIndex, VertexCount, pDestBuffer->GetProxyInterface(), nullptr, Flags);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::CreateVertexShader(const DWORD *pDeclaration, const DWORD *pFunction, DWORD *pHandle, DWORD Usage)
{
	UNREFERENCED_PARAMETER(Usage);

#ifndef D3D8TO9NOLOG
	LOG << "Redirecting '" << "IDirect3DDevice8::CreateVertexShader" << "(" << this << ", " << pDeclaration << ", " << pFunction << ", " << pHandle << ", " << Usage << ")' ..." << std::endl;
#endif

	if (pDeclaration == nullptr || pHandle == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	*pHandle = 0;

	UINT ElementIndex = 0;
	const UINT ElementLimit = 32;
	std::string ConstantsCode;
	WORD Stream = 0, Offset = 0;
	DWORD VertexShaderInputs[ElementLimit];
	D3DVERTEXELEMENT9 VertexElements[ElementLimit];

#ifndef D3D8TO9NOLOG
	LOG << "> Translating vertex declaration ..." << std::endl;
#endif

	static const BYTE DeclTypes[][2] =
	{
		{ D3DDECLTYPE_FLOAT1, 4 },
		{ D3DDECLTYPE_FLOAT2, 8 },
		{ D3DDECLTYPE_FLOAT3, 12 },
		{ D3DDECLTYPE_FLOAT4, 16 },
		{ D3DDECLTYPE_D3DCOLOR, 4 },
		{ D3DDECLTYPE_UBYTE4, 4 },
		{ D3DDECLTYPE_SHORT2, 4 },
		{ D3DDECLTYPE_SHORT4, 8 },
		{ D3DDECLTYPE_UBYTE4N, 4 },
		{ D3DDECLTYPE_SHORT2N, 4 },
		{ D3DDECLTYPE_SHORT4N, 8 },
		{ D3DDECLTYPE_USHORT2N, 4 },
		{ D3DDECLTYPE_USHORT4N, 8 },
		{ D3DDECLTYPE_UDEC3, 6 },
		{ D3DDECLTYPE_DEC3N, 6 },
		{ D3DDECLTYPE_FLOAT16_2, 8 },
		{ D3DDECLTYPE_FLOAT16_4, 16 }
	};
	static const BYTE DeclAddressUsages[][2] =
	{
		{ D3DDECLUSAGE_POSITION, 0 },
		{ D3DDECLUSAGE_BLENDWEIGHT, 0 },
		{ D3DDECLUSAGE_BLENDINDICES, 0 },
		{ D3DDECLUSAGE_NORMAL, 0 },
		{ D3DDECLUSAGE_PSIZE, 0 },
		{ D3DDECLUSAGE_COLOR, 0 },
		{ D3DDECLUSAGE_COLOR, 1 },
		{ D3DDECLUSAGE_TEXCOORD, 0 },
		{ D3DDECLUSAGE_TEXCOORD, 1 },
		{ D3DDECLUSAGE_TEXCOORD, 2 },
		{ D3DDECLUSAGE_TEXCOORD, 3 },
		{ D3DDECLUSAGE_TEXCOORD, 4 },
		{ D3DDECLUSAGE_TEXCOORD, 5 },
		{ D3DDECLUSAGE_TEXCOORD, 6 },
		{ D3DDECLUSAGE_TEXCOORD, 7 },
		{ D3DDECLUSAGE_POSITION, 1 },
		{ D3DDECLUSAGE_NORMAL, 1 }
	};

	while (ElementIndex < ElementLimit)
	{
		const DWORD Token = *pDeclaration;
		const DWORD TokenType = (Token & D3DVSD_TOKENTYPEMASK) >> D3DVSD_TOKENTYPESHIFT;

		if (Token == D3DVSD_END())
		{
			break;
		}
		else if (TokenType == D3DVSD_TOKEN_STREAM)
		{
			Stream = static_cast<WORD>((Token & D3DVSD_STREAMNUMBERMASK) >> D3DVSD_STREAMNUMBERSHIFT);
			Offset = 0;
		}
		else if (TokenType == D3DVSD_TOKEN_STREAMDATA && !(Token & 0x10000000))
		{
			VertexElements[ElementIndex].Stream = Stream;
			VertexElements[ElementIndex].Offset = Offset;
			const DWORD type = (Token & D3DVSD_DATATYPEMASK) >> D3DVSD_DATATYPESHIFT;
			VertexElements[ElementIndex].Type = DeclTypes[type][0];
			Offset += DeclTypes[type][1];
			VertexElements[ElementIndex].Method = D3DDECLMETHOD_DEFAULT;
			const DWORD Address = (Token & D3DVSD_VERTEXREGMASK) >> D3DVSD_VERTEXREGSHIFT;
			VertexElements[ElementIndex].Usage = DeclAddressUsages[Address][0];
			VertexElements[ElementIndex].UsageIndex = DeclAddressUsages[Address][1];

			VertexShaderInputs[ElementIndex++] = Address;
		}
		else if (TokenType == D3DVSD_TOKEN_STREAMDATA && (Token & 0x10000000))
		{
			Offset += ((Token & D3DVSD_SKIPCOUNTMASK) >> D3DVSD_SKIPCOUNTSHIFT) * sizeof(DWORD);
		}
		else if (TokenType == D3DVSD_TOKEN_TESSELLATOR && !(Token & 0x10000000))
		{
			VertexElements[ElementIndex].Stream = Stream;
			VertexElements[ElementIndex].Offset = Offset;

			const DWORD UsageType = (Token & D3DVSD_VERTEXREGINMASK) >> D3DVSD_VERTEXREGINSHIFT;

			for (UINT r = 0; r < ElementIndex; ++r)
			{
				if (VertexElements[r].Usage == DeclAddressUsages[UsageType][0] && VertexElements[r].UsageIndex == DeclAddressUsages[UsageType][1])
				{
					VertexElements[ElementIndex].Stream = VertexElements[r].Stream;
					VertexElements[ElementIndex].Offset = VertexElements[r].Offset;
					break;
				}
			}

			VertexElements[ElementIndex].Type = D3DDECLTYPE_FLOAT3;
			VertexElements[ElementIndex].Method = D3DDECLMETHOD_CROSSUV;
			const DWORD Address = (Token & 0xF);
			VertexElements[ElementIndex].Usage = DeclAddressUsages[Address][0];
			VertexElements[ElementIndex].UsageIndex = DeclAddressUsages[Address][1];

			if (VertexElements[ElementIndex].Usage == D3DDECLUSAGE_BLENDINDICES)
			{
				VertexElements[ElementIndex].Method = D3DDECLMETHOD_DEFAULT;
			}

			VertexShaderInputs[ElementIndex++] = Address;
		}
		else if (TokenType == D3DVSD_TOKEN_TESSELLATOR && (Token & 0x10000000))
		{
			VertexElements[ElementIndex].Stream = 0;
			VertexElements[ElementIndex].Offset = 0;
			VertexElements[ElementIndex].Type = D3DDECLTYPE_UNUSED;
			VertexElements[ElementIndex].Method = D3DDECLMETHOD_UV;
			const DWORD Address = (Token & 0xF);
			VertexElements[ElementIndex].Usage = DeclAddressUsages[Address][0];
			VertexElements[ElementIndex].UsageIndex = DeclAddressUsages[Address][1];

			if (VertexElements[ElementIndex].Usage == D3DDECLUSAGE_BLENDINDICES)
			{
				VertexElements[ElementIndex].Method = D3DDECLMETHOD_DEFAULT;
			}

			VertexShaderInputs[ElementIndex++] = Address;
		}
		else if (TokenType == D3DVSD_TOKEN_CONSTMEM)
		{
			const DWORD RegisterCount = 4 * ((Token & D3DVSD_CONSTCOUNTMASK) >> D3DVSD_CONSTCOUNTSHIFT);
			DWORD Address = (Token & D3DVSD_CONSTADDRESSMASK) >> D3DVSD_CONSTADDRESSSHIFT;

			for (DWORD RegisterIndex = 0; RegisterIndex < RegisterCount; RegisterIndex += 4, ++Address)
			{
				ConstantsCode += "    def c" + std::to_string(Address) + ", " +
					std::to_string(*reinterpret_cast<const float *>(&pDeclaration[RegisterIndex + 1])) + ", " +
					std::to_string(*reinterpret_cast<const float *>(&pDeclaration[RegisterIndex + 2])) + ", " +
					std::to_string(*reinterpret_cast<const float *>(&pDeclaration[RegisterIndex + 3])) + ", " +
					std::to_string(*reinterpret_cast<const float *>(&pDeclaration[RegisterIndex + 4])) + " /* vertex declaration constant */\n";
			}

			pDeclaration += RegisterCount;
		}
		else
		{
#ifndef D3D8TO9NOLOG
			LOG << "> Failed because token type '" << TokenType << "' is not supported!" << std::endl;
#endif

			return D3DERR_INVALIDCALL;
		}

		++pDeclaration;
	}

	const D3DVERTEXELEMENT9 Terminator = D3DDECL_END();
	VertexElements[ElementIndex] = Terminator;

	HRESULT hr;
	VertexShaderInfo *ShaderInfo;

	if (pFunction != nullptr)
	{
#ifndef D3D8TO9NOLOG
		LOG << "> Disassembling shader and translating assembly to Direct3D 9 compatible code ..." << std::endl;
#endif

		if (*pFunction < D3DVS_VERSION(1, 0) || *pFunction > D3DVS_VERSION(1, 1))
		{
#ifndef D3D8TO9NOLOG
			LOG << "> Failed because of version mismatch ('" << std::showbase << std::hex << *pFunction << std::dec << std::noshowbase << "')! Only 'vs_1_x' shaders are supported." << std::endl;
#endif

			return D3DERR_INVALIDCALL;
		}

		ID3DXBuffer *Disassembly = nullptr, *Assembly = nullptr, *ErrorBuffer = nullptr;

		if (D3DXDisassembleShader != nullptr)
		{
			hr = D3DXDisassembleShader(pFunction, FALSE, nullptr, &Disassembly);
		}
		else
		{
			hr = D3DERR_INVALIDCALL;
		}

		if (FAILED(hr))
		{
#ifndef D3D8TO9NOLOG
			LOG << "> Failed to disassemble shader with error code " << std::hex << hr << std::dec << "!" << std::endl;
#endif

			return hr;
		}

		std::string SourceCode(static_cast<const char *>(Disassembly->GetBufferPointer()), Disassembly->GetBufferSize() - 1);
		const size_t VersionPosition = SourceCode.find("vs_1_");

		assert(VersionPosition != std::string::npos);

		if (SourceCode.at(VersionPosition + 5) == '0')
		{
#ifndef D3D8TO9NOLOG
			LOG << "> Replacing version 'vs_1_0' with 'vs_1_1' ..." << std::endl;
#endif

			SourceCode.replace(VersionPosition, 6, "vs_1_1");
		}

		size_t DeclPosition = VersionPosition + 7;

		for (UINT k = 0; k < ElementIndex; k++)
		{
			std::string DeclCode = "    ";

			switch (VertexElements[k].Usage)
			{
				case D3DDECLUSAGE_POSITION:
					DeclCode += "dcl_position";
					break;
				case D3DDECLUSAGE_BLENDWEIGHT:
					DeclCode += "dcl_blendweight";
					break;
				case D3DDECLUSAGE_BLENDINDICES:
					DeclCode += "dcl_blendindices";
					break;
				case D3DDECLUSAGE_NORMAL:
					DeclCode += "dcl_normal";
					break;
				case D3DDECLUSAGE_PSIZE:
					DeclCode += "dcl_psize";
					break;
				case D3DDECLUSAGE_COLOR:
					DeclCode += "dcl_color";
					break;
				case D3DDECLUSAGE_TEXCOORD:
					DeclCode += "dcl_texcoord";
					break;
			}

			if (VertexElements[k].UsageIndex > 0)
			{
				DeclCode += std::to_string(VertexElements[k].UsageIndex);
			}

			DeclCode += " v" + std::to_string(VertexShaderInputs[k]) + '\n';

			SourceCode.insert(DeclPosition, DeclCode);
			DeclPosition += DeclCode.length();
		}

		#pragma region Fill registers with default value
		SourceCode.insert(DeclPosition, ConstantsCode);

		// Get number of arithmetic instructions used
		const size_t InstructionPosition = SourceCode.find("instruction");
		size_t InstructionCount = InstructionPosition > 2 && InstructionPosition < SourceCode.size() ? strtoul(SourceCode.substr(InstructionPosition - 4, 4).c_str(), nullptr, 10) : 0;

		for (size_t j = 0; j < 8; j++)
		{
			const std::string reg = "oT" + std::to_string(j);

			if (SourceCode.find(reg) != std::string::npos && InstructionCount < 128)
			{
				++InstructionCount;
				SourceCode.insert(DeclPosition + ConstantsCode.size(), "    mov " + reg + ", c0 /* initialize output register " + reg + " */\n");
			}
		}
		for (size_t j = 0; j < 2; j++)
		{
			const std::string reg = "oD" + std::to_string(j);

			if (SourceCode.find(reg) != std::string::npos && InstructionCount < 128)
			{
				++InstructionCount;
				SourceCode.insert(DeclPosition + ConstantsCode.size(), "    mov " + reg + ", c0 /* initialize output register " + reg + " */\n");
			}
		}
		for (size_t j = 0; j < 12; j++)
		{
			const std::string reg = "r" + std::to_string(j);

			if (SourceCode.find(reg) != std::string::npos && InstructionCount < 128)
			{
				++InstructionCount;
				SourceCode.insert(DeclPosition + ConstantsCode.size(), "    mov " + reg + ", c0 /* initialize register " + reg + " */\n");
			}
		}
		#pragma endregion

		SourceCode = std::regex_replace(SourceCode, std::regex("    \\/\\/ vs\\.1\\.1\\n((?! ).+\\n)+"), "");
		SourceCode = std::regex_replace(SourceCode, std::regex("(oFog|oPts)\\.x"), "$1 /* removed swizzle */");
		SourceCode = std::regex_replace(SourceCode, std::regex("(add|sub|mul|min|max) (oFog|oPts), ([cr][0-9]+), (.+)\\n"), "$1 $2, $3.x /* added swizzle */, $4\n");
		SourceCode = std::regex_replace(SourceCode, std::regex("(add|sub|mul|min|max) (oFog|oPts), (.+), ([cr][0-9]+)\\n"), "$1 $2, $3, $4.x /* added swizzle */\n");
		SourceCode = std::regex_replace(SourceCode, std::regex("mov (oFog|oPts)(.*), (-?)([crv][0-9]+(?![\\.0-9]))"), "mov $1$2, $3$4.x /* select single component */");

		// Dest register cannot be the same as first source register for m*x* instructions.
		if (std::regex_search(SourceCode, std::regex("m.x.")))
		{
			// Check for unused register
			size_t r;
			for (r = 0; r < 12; r++)
			{
				if (SourceCode.find("r" + std::to_string(r)) == std::string::npos) break;
			}

			// Check if first source register is the same as the destination register
			for (size_t j = 0; j < 12; j++)
			{
				const std::string reg = "(m.x.) (r" + std::to_string(j) + "), ((-?)r" + std::to_string(j) + "([\\.xyzw]*))(?![0-9])";

				while (std::regex_search(SourceCode, std::regex(reg)))
				{
					// If there is enough remaining instructions and an unused register then update to use a temp register
					if (r < 12 && InstructionCount < 128)
					{
						++InstructionCount;
						SourceCode = std::regex_replace(SourceCode, std::regex(reg),
							"mov r" + std::to_string(r) + ", $2 /* added line */\n    $1 $2, $4r" + std::to_string(r) + "$5 /* changed $3 to r" + std::to_string(r) + " */",
							std::regex_constants::format_first_only);
					}
					// Disable line to prevent assembly error
					else
					{
						SourceCode = std::regex_replace(SourceCode, std::regex("(.*" + reg + ".*)"), "/*$1*/ /* disabled this line */");
						break;
					}
				}
			}
		}

#ifndef D3D8TO9NOLOG
		LOG << "> Dumping translated shader assembly:" << std::endl << std::endl << SourceCode << std::endl;
#endif

		if (D3DXAssembleShader != nullptr)
		{
			hr = D3DXAssembleShader(SourceCode.data(), static_cast<UINT>(SourceCode.size()), nullptr, nullptr, 0, &Assembly, &ErrorBuffer);
		}
		else
		{
			hr = D3DERR_INVALIDCALL;
		}

		Disassembly->Release();

		if (FAILED(hr))
		{
			if (ErrorBuffer != nullptr)
			{
#ifndef D3D8TO9NOLOG
				LOG << "> Failed to reassemble shader:" << std::endl << std::endl << static_cast<const char *>(ErrorBuffer->GetBufferPointer()) << std::endl;
#endif

				ErrorBuffer->Release();
			}
			else
			{
#ifndef D3D8TO9NOLOG
				LOG << "> Failed to reassemble shader with error code " << std::hex << hr << std::dec << "!" << std::endl;
#endif
			}

			return hr;
		}

		ShaderInfo = new VertexShaderInfo();

		hr = ProxyInterface->CreateVertexShader(static_cast<const DWORD *>(Assembly->GetBufferPointer()), &ShaderInfo->Shader);

		Assembly->Release();
	}
	else
	{
		ShaderInfo = new VertexShaderInfo();
		ShaderInfo->Shader = nullptr;

		hr = D3D_OK;
	}

	if (SUCCEEDED(hr))
	{
		hr = ProxyInterface->CreateVertexDeclaration(VertexElements, &ShaderInfo->Declaration);

		if (SUCCEEDED(hr))
		{
			// Since 'Shader' is at least 8 byte aligned, we can safely shift it to right and end up not overwriting the top bit
			assert((reinterpret_cast<DWORD>(ShaderInfo) & 1) == 0);
			const DWORD ShaderMagic = reinterpret_cast<DWORD>(ShaderInfo) >> 1;

			*pHandle = ShaderMagic | 0x80000000;
		}
		else
		{
#ifndef D3D8TO9NOLOG
			LOG << "> 'IDirect3DDevice9::CreateVertexDeclaration' failed with error code " << std::hex << hr << std::dec << "!" << std::endl;
#endif

			if (ShaderInfo->Shader != nullptr)
			{
				ShaderInfo->Shader->Release();
			}
		}
	}
	else
	{
#ifndef D3D8TO9NOLOG
		LOG << "> 'IDirect3DDevice9::CreateVertexShader' failed with error code " << std::hex << hr << std::dec << "!" << std::endl;
#endif
	}

	if (FAILED(hr))
	{
		delete ShaderInfo;
	}

	return hr;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::SetVertexShader(DWORD Handle)
{
	HRESULT hr;

	if ((Handle & 0x80000000) == 0)
	{
		ProxyInterface->SetVertexShader(nullptr);
		hr = ProxyInterface->SetFVF(Handle);

		CurrentVertexShaderHandle = 0;
	}
	else
	{
		const DWORD handleMagic = Handle << 1;
		VertexShaderInfo *const ShaderInfo = reinterpret_cast<VertexShaderInfo *>(handleMagic);

		hr = ProxyInterface->SetVertexShader(ShaderInfo->Shader);
		ProxyInterface->SetVertexDeclaration(ShaderInfo->Declaration);

		CurrentVertexShaderHandle = Handle;
	}

	return hr;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetVertexShader(DWORD *pHandle)
{
	if (pHandle == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	if (CurrentVertexShaderHandle == 0)
	{
		return ProxyInterface->GetFVF(pHandle);
	}
	else
	{
		*pHandle = CurrentVertexShaderHandle;

		return D3D_OK;
	}
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::DeleteVertexShader(DWORD Handle)
{
	if ((Handle & 0x80000000) == 0)
	{
		return D3DERR_INVALIDCALL;
	}

	if (CurrentVertexShaderHandle == Handle)
	{
		SetVertexShader(0);
	}

	const DWORD HandleMagic = Handle << 1;
	VertexShaderInfo *const ShaderInfo = reinterpret_cast<VertexShaderInfo *>(HandleMagic);

	if (ShaderInfo->Shader != nullptr)
	{
		ShaderInfo->Shader->Release();
	}
	if (ShaderInfo->Declaration != nullptr)
	{
		ShaderInfo->Declaration->Release();
	}

	delete ShaderInfo;

	return D3D_OK;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::SetVertexShaderConstant(DWORD Register, const void *pConstantData, DWORD ConstantCount)
{
	return ProxyInterface->SetVertexShaderConstantF(Register, static_cast<const float *>(pConstantData), ConstantCount);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetVertexShaderConstant(DWORD Register, void *pConstantData, DWORD ConstantCount)
{
	return ProxyInterface->GetVertexShaderConstantF(Register, static_cast<float *>(pConstantData), ConstantCount);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetVertexShaderDeclaration(DWORD Handle, void *pData, DWORD *pSizeOfData)
{
	UNREFERENCED_PARAMETER(Handle);
	UNREFERENCED_PARAMETER(pData);
	UNREFERENCED_PARAMETER(pSizeOfData);

#ifndef D3D8TO9NOLOG
	LOG << "Redirecting '" << "IDirect3DDevice8::GetVertexShaderDeclaration" << "(" << this << ", " << Handle << ", " << pData << ", " << pSizeOfData << ")' ..." << std::endl;
	LOG << "> 'IDirect3DDevice8::GetVertexShaderDeclaration' is not implemented!" << std::endl;
#endif

	return D3DERR_INVALIDCALL;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetVertexShaderFunction(DWORD Handle, void *pData, DWORD *pSizeOfData)
{
#ifndef D3D8TO9NOLOG
	LOG << "Redirecting '" << "IDirect3DDevice8::GetVertexShaderFunction" << "(" << this << ", " << Handle << ", " << pData << ", " << pSizeOfData << ")' ..." << std::endl;
#endif

	if ((Handle & 0x80000000) == 0)
	{
		return D3DERR_INVALIDCALL;
	}

	const DWORD HandleMagic = Handle << 1;
	IDirect3DVertexShader9 *VertexShaderInterface = reinterpret_cast<VertexShaderInfo *>(HandleMagic)->Shader;

	if (VertexShaderInterface == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

#ifndef D3D8TO9NOLOG
	LOG << "> Returning translated shader byte code." << std::endl;
#endif

	return VertexShaderInterface->GetFunction(pData, reinterpret_cast<UINT *>(pSizeOfData));
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::SetStreamSource(UINT StreamNumber, Direct3DVertexBuffer8 *pStreamData, UINT Stride)
{
	if (pStreamData == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	return ProxyInterface->SetStreamSource(StreamNumber, pStreamData->GetProxyInterface(), 0, Stride);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetStreamSource(UINT StreamNumber, Direct3DVertexBuffer8 **ppStreamData, UINT *pStride)
{
	if (ppStreamData == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}
	else
	{
		*ppStreamData = nullptr;
	}

	UINT StreamOffset = 0;
	IDirect3DVertexBuffer9 *VertexBufferInterface = nullptr;

	const HRESULT hr = ProxyInterface->GetStreamSource(StreamNumber, &VertexBufferInterface, &StreamOffset, pStride);

	if (FAILED(hr))
	{
		return hr;
	}

	if (VertexBufferInterface != nullptr)
	{
		*ppStreamData = ProxyAddressLookupTable->FindAddress<Direct3DVertexBuffer8>(VertexBufferInterface);
	}

	return D3D_OK;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::SetIndices(Direct3DIndexBuffer8 *pIndexData, UINT BaseVertexIndex)
{
	if (pIndexData == nullptr || BaseVertexIndex > 0x7FFFFFFF)
	{
		return D3DERR_INVALIDCALL;
	}

	CurrentBaseVertexIndex = static_cast<INT>(BaseVertexIndex);

	return ProxyInterface->SetIndices(pIndexData->GetProxyInterface());
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetIndices(Direct3DIndexBuffer8 **ppIndexData, UINT *pBaseVertexIndex)
{
	if (ppIndexData == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	*ppIndexData = nullptr;

	if (pBaseVertexIndex != nullptr)
	{
		*pBaseVertexIndex = static_cast<UINT>(CurrentBaseVertexIndex);
	}

	IDirect3DIndexBuffer9 *IntexBufferInterface = nullptr;

	const HRESULT hr = ProxyInterface->GetIndices(&IntexBufferInterface);

	if (FAILED(hr))
	{
		return hr;
	}

	if (IntexBufferInterface != nullptr)
	{
		*ppIndexData = ProxyAddressLookupTable->FindAddress<Direct3DIndexBuffer8>(IntexBufferInterface);
	}

	return D3D_OK;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::CreatePixelShader(const DWORD *pFunction, DWORD *pHandle)
{
#ifndef D3D8TO9NOLOG
	LOG << "Redirecting '" << "IDirect3DDevice8::CreatePixelShader" << "(" << this << ", " << pFunction << ", " << pHandle << ")' ..." << std::endl;
#endif

	if (pFunction == nullptr || pHandle == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	*pHandle = 0;

#ifndef D3D8TO9NOLOG
	LOG << "> Disassembling shader and translating assembly to Direct3D 9 compatible code ..." << std::endl;
#endif

	if (*pFunction < D3DPS_VERSION(1, 0) || *pFunction > D3DPS_VERSION(1, 4))
	{
#ifndef D3D8TO9NOLOG
		LOG << "> Failed because of version mismatch ('" << std::showbase << std::hex << *pFunction << std::dec << std::noshowbase << "')! Only 'ps_1_x' shaders are supported." << std::endl;
#endif

		return D3DERR_INVALIDCALL;
	}

	ID3DXBuffer *Disassembly = nullptr, *Assembly = nullptr, *ErrorBuffer = nullptr;

	HRESULT hr = D3DERR_INVALIDCALL;

	if (D3DXDisassembleShader != nullptr)
	{
		hr = D3DXDisassembleShader(pFunction, FALSE, nullptr, &Disassembly);
	}

	if (FAILED(hr))
	{
#ifndef D3D8TO9NOLOG
		LOG << "> Failed to disassemble shader with error code " << std::hex << hr << std::dec << "!" << std::endl;
#endif

		return hr;
	}

	std::string SourceCode(static_cast<const char *>(Disassembly->GetBufferPointer()), Disassembly->GetBufferSize() - 1);
	const size_t VersionPosition = SourceCode.find("ps_1_");

	assert(VersionPosition != std::string::npos);

	if (SourceCode.at(VersionPosition + 5) == '0')
	{
#ifndef D3D8TO9NOLOG
		LOG << "> Replacing version 'ps_1_0' with 'ps_1_1' ..." << std::endl;
#endif

		SourceCode.replace(VersionPosition, 6, "ps_1_1");
	}

	// Get number of arithmetic instructions used
	const size_t ArithmeticPosition = SourceCode.find("arithmetic");
	size_t ArithmeticCount = ArithmeticPosition > 2 && ArithmeticPosition < SourceCode.size() ? strtoul(SourceCode.substr(ArithmeticPosition - 2, 2).c_str(), nullptr, 10) : 0;
	ArithmeticCount = (ArithmeticCount != 0) ? ArithmeticCount : 10;	// Default to 10

	// Remove lines when "    // ps.1.1" string is found and the next line does not start with a space
	SourceCode = std::regex_replace(SourceCode,
		std::regex("    \\/\\/ ps\\.1\\.[1-4]\\n((?! ).+\\n)+"),
		"");

	// Fix '-' modifier for constant values when using 'add' arithmetic by changing it to use 'sub'
	SourceCode = std::regex_replace(SourceCode,
		std::regex("(add)([_satxd248]*) (r[0-9][\\.wxyz]*), ((1-|)[crtv][0-9][\\.wxyz_abdis2]*), (-)(c[0-9][\\.wxyz]*)(_bx2|_bias|_x2|_d[zbwa]|)(?![_\\.wxyz])"),
		"sub$2 $3, $4, $7$8 /* changed 'add' to 'sub' removed modifier $6 */");

	// Create temporary varables for ps_1_4
	std::string SourceCode14 = SourceCode;
	int ArithmeticCount14 = ArithmeticCount;

	// Fix modifiers for constant values by using any remaining arithmetic places to add an instruction to move the constant value to a temporary register
	while (std::regex_search(SourceCode, std::regex("-c[0-9]|c[0-9][\\.wxyz]*_")) && ArithmeticCount < 8)
	{
		// Make sure that the dest register is not already being used
		std::string tmpLine = "\n" + std::regex_replace(SourceCode, std::regex("1?-(c[0-9])[\\._a-z0-9]*|(c[0-9])[\\.wxyz]*_[a-z0-9]*"), "-$1$2") + "\n";
		size_t start = tmpLine.substr(0, tmpLine.find("-c")).rfind("\n") + 1;
		tmpLine = tmpLine.substr(start, tmpLine.find("\n", start) - start);
		const std::string destReg = std::regex_replace(tmpLine, std::regex("[ \\+]+[a-z_\\.0-9]+ (r[0-9]).*-c[0-9].*"),"$1");
		const std::string sourceReg = std::regex_replace(tmpLine, std::regex("[ \\+]+[a-z_\\.0-9]+ r[0-9][\\._a-z0-9]*, (.*)-c[0-9](.*)"), "$1$2");
		if (sourceReg.find(destReg) != std::string::npos)
		{
			break;
		}

		// Replace one constant modifier using the dest register as a temporary register
		SourceCode = std::regex_replace(SourceCode,
			std::regex("    (...)(_[_satxd248]*|) (r[0-9][\\.wxyz]*), (1?-?[crtv][0-9][\\.wxyz_abdis2]*, )?(1?-?[crtv][0-9][\\.wxyz_abdis2]*, )?(1?-?[crtv][0-9][\\.wxyz_abdis2]*, )?((1?-)(c[0-9])([\\.wxyz]*)(_bx2|_bias|_x2|_d[zbwa]|)|(1?-?)(c[0-9])([\\.wxyz]*)(_bx2|_bias|_x2|_d[zbwa]))(?![_\\.wxyz])"),
			"    mov $3, $9$10$13$14 /* added line */\n    $1$2 $3, $4$5$8$12$3$11$15 /* changed $9$10$13$14 to $3 */", std::regex_constants::format_first_only);
		ArithmeticCount++;
	}

	// Check if this should be converted to ps_1_4
	if (std::regex_search(SourceCode, std::regex("-c[0-9]|c[0-9][\\.wxyz]*_")) &&	// Check for modifiers on constants
		!std::regex_search(SourceCode, std::regex("tex[bcdmr]")) &&					// Verify unsupported instructions are not used
		std::regex_search(SourceCode, std::regex("ps_1_[0-3]")))					// Verify PixelShader is using version 1.0 to 1.3
	{
		bool ConvertError = false;
		bool RegisterUsed[7] = { false };
		RegisterUsed[6] = true;
		struct MyStrings
		{
			std::string dest;
			std::string source;
		};
		std::vector<MyStrings> ReplaceReg;
		std::string NewSourceCode = "    ps_1_4 /* converted */\n";

		// Ensure at least one command will be above the phase marker
		bool PhaseMarkerSet = (ArithmeticCount14 >= 8);
		if (SourceCode14.find("def c") == std::string::npos && !PhaseMarkerSet)
		{
			for (size_t j = 0; j < 8; j++)
			{
				const std::string reg = "c" + std::to_string(j);

				if (SourceCode14.find(reg) == std::string::npos)
				{
					PhaseMarkerSet = true;
					NewSourceCode.append("    def " + reg + ", 0, 0, 0, 0 /* added line */\n");
					break;
				}
			}
		}

		// Update registers to use different numbers from textures
		size_t FirstReg = 0;
		for (size_t j = 0; j < 2; j++)
		{
			const std::string reg = "r" + std::to_string(j);

			if (SourceCode14.find(reg) != std::string::npos)
			{
				while (SourceCode14.find("t" + std::to_string(FirstReg)) != std::string::npos ||
					(SourceCode14.find("r" + std::to_string(FirstReg)) != std::string::npos && j != FirstReg))
				{
					FirstReg++;
				}
				SourceCode14 = std::regex_replace(SourceCode14, std::regex(reg), "r" + std::to_string(FirstReg));
				FirstReg++;
			}
		}

		// Set phase location
		size_t PhasePosition = NewSourceCode.length();
		size_t TexturePosition = 0;

		// Loop through each line
		size_t LinePosition = 1;
		std::string NewLine = SourceCode14;
		while (true)
		{
			// Get next line
			size_t tmpLinePos = SourceCode14.find("\n", LinePosition) + 1;
			if (tmpLinePos == std::string::npos || tmpLinePos < LinePosition)
			{
				break;
			}
			LinePosition = tmpLinePos;
			NewLine = SourceCode14.substr(LinePosition, SourceCode14.length());
			tmpLinePos = NewLine.find("\n");
			if (tmpLinePos != std::string::npos)
			{
				NewLine.resize(tmpLinePos);
			}

			// Skip 'ps_x_x' lines
			if (std::regex_search(NewLine, std::regex("ps_._.")))
			{
				// Do nothing
			}

			// Check for 'def' and add before 'phase' statement
			else if (NewLine.find("def c") != std::string::npos)
			{
				PhaseMarkerSet = true;
				const std::string tmpLine = NewLine + "\n";
				NewSourceCode.insert(PhasePosition, tmpLine);
				PhasePosition += tmpLine.length();
			}

			// Check for 'tex' and update to 'texld'
			else if (NewLine.find("tex t") != std::string::npos)
			{
				const std::string regNum = std::regex_replace(NewLine, std::regex(".*tex t([0-9]).*"), "$1");
				const std::string tmpLine = "    texld r" + regNum + ", t" + regNum + "\n";

				// Mark as a texture register and add 'texld' statement before or after the 'phase' statement
				const unsigned long Num = strtoul(regNum.c_str(), nullptr, 10);
				RegisterUsed[(Num < 6) ? Num : 6] = true;
				NewSourceCode.insert(PhasePosition, tmpLine);
				if (PhaseMarkerSet)
				{
					TexturePosition += tmpLine.length();
				}
				else
				{
					PhaseMarkerSet = true;
					PhasePosition += tmpLine.length();
				}
			}

			// Other instructions
			else
			{
				// Check for constant modifiers and update them to use unused temp register
				if (std::regex_search(NewLine, std::regex("-c[0-9]|c[0-9][\\.wxyz]*_")))
				{
					for (size_t j = 0; j < 6; j++)
					{
						std::string reg = "r" + std::to_string(j);

						if (NewSourceCode.find(reg) == std::string::npos)
						{
							const std::string constReg = std::regex_replace(NewLine, std::regex(".*-(c[0-9]).*|.*(c[0-9])[\\.wxyz]*_.*"), "$1$2");

							// Check if this constant has modifiers in more than one line
							if (std::regex_search(SourceCode14.substr(LinePosition + NewLine.length(), SourceCode14.length()), std::regex("-" + constReg + "|" + constReg + "[\\.wxyz]*_")))
							{
								// Find an unused register
								while (j < 6 && 
									(NewSourceCode.find("r" + std::to_string(j)) != std::string::npos ||
									SourceCode14.find("r" + std::to_string(j)) != std::string::npos))
								{
									j++;
								}
								// Replace all constants with the unused register
								if (j < 6)
								{
									reg = "r" + std::to_string(j);
									SourceCode14 = std::regex_replace(SourceCode14, std::regex(constReg), reg);
								}
							}

							const std::string tmpLine = "    mov " + reg + ", " + constReg + "\n";

							// Update the constant in this line and add 'mov' statement before or after the 'phase' statement
							NewLine = std::regex_replace(NewLine, std::regex(constReg), reg);
							if (ArithmeticCount14 < 8)
							{
								NewSourceCode.insert(PhasePosition + TexturePosition, tmpLine);
								ArithmeticCount14++;
							}
							else
							{
								PhaseMarkerSet = true;
								NewSourceCode.insert(PhasePosition, tmpLine);
								PhasePosition += tmpLine.length();
							}
							break;
						}
					}
				}

				// Update register from vector once it is used for the last time
				if (ReplaceReg.size() > 0)
				{
					for (size_t x = 0; x < ReplaceReg.size(); x++)
					{
						// Check if register is used in this line
						if (NewLine.find(ReplaceReg[x].dest) != std::string::npos)
						{
							// Get position of all lines after this line
							size_t start = LinePosition + NewLine.length();
							// Move position to next line if the first line is a co-issed command
							start = (SourceCode14.substr(start, 4).find("+") == std::string::npos) ? start : SourceCode14.find("\n", start + 1);

							// Check if register is used in the code after this position
							if (SourceCode14.find(ReplaceReg[x].dest, start) == std::string::npos)
							{
								// Update dest register using source register from the vector
								NewLine = std::regex_replace(NewLine, std::regex("([ \\+]+[a-z_\\.0-9]+ )r[0-9](.*)"), "$1" + ReplaceReg[x].source + "$2");
								ReplaceReg.erase(ReplaceReg.begin() + x);
								break;
							}
						}
					}
				}

				// Check if texture is no longer being used and update the dest register
				if (std::regex_search(NewLine, std::regex("t[0-9]")))
				{
					const std::string texNum = std::regex_replace(NewLine, std::regex(".*t([0-9]).*"), "$1");

					// Get position of all lines after this line
					size_t start = LinePosition + NewLine.length();
					// Move position to next line if the first line is a co-issed command
					start = (SourceCode14.substr(start, 4).find("+") == std::string::npos) ? start : SourceCode14.find("\n", start + 1);

					// Check if texture is used in the code after this position
					if (SourceCode14.find("t" + texNum, start) == std::string::npos)
					{
						const std::string destRegNum = std::regex_replace(NewLine, std::regex("[ \\+]+[a-z_\\.0-9]+ r([0-9]).*"), "$1");

						// Check if destination register is already being used by a texture register
						const unsigned long Num = strtoul(destRegNum.c_str(), nullptr, 10);
						if (!RegisterUsed[(Num < 6) ? Num : 6])
						{
							// Check if line is using more than one texture and error out
							if (std::regex_search(std::regex_replace(NewLine, std::regex("t" + texNum), "r" + texNum), std::regex("t[0-9]")))
							{
								ConvertError = true;
								break;
							}
							// Check if this is the first or last time the register is used
							if (NewSourceCode.find("r" + destRegNum) == std::string::npos ||
								SourceCode14.find("r" + destRegNum, start) == std::string::npos)
							{
								// Update dest register using texture register
								NewLine = std::regex_replace(NewLine, std::regex("([ \\+]+[a-z_\\.0-9]+ )r[0-9](.*)"), "$1r" + texNum + "$2");
								// Update code replacing all regsiters after the marked position with the texture register
								const std::string tempSourceCode = std::regex_replace(SourceCode14.substr(start, SourceCode14.length()), std::regex("r" + destRegNum), "r" + texNum);
								SourceCode14.resize(start);
								SourceCode14.append(tempSourceCode);
							}
							else
							{
								// If register is still being used then add registers to vector to be replaced later
								RegisterUsed[(Num < 6) ? Num : 6] = true;
								MyStrings tempReplaceReg;
								tempReplaceReg.dest = "r" + destRegNum;
								tempReplaceReg.source = "r" + texNum;
								ReplaceReg.push_back(tempReplaceReg);
							}
						}
					}
				}

				// Add line to SourceCode
				NewLine = std::regex_replace(NewLine, std::regex("t([0-9])"), "r$1") + "\n";
				NewSourceCode.append(NewLine);
			}
		}

		// Add 'phase' instruction
		NewSourceCode.insert(PhasePosition, "    phase\n");

		// If no errors were encountered then check if code assembles
		if (!ConvertError)
		{
			// Test if ps_1_4 assembles
			if (SUCCEEDED(D3DXAssembleShader(NewSourceCode.data(), static_cast<UINT>(NewSourceCode.size()), nullptr, nullptr, 0, &Assembly, &ErrorBuffer)))
			{
				SourceCode = NewSourceCode;
			}
			else
			{
#ifndef D3D8TO9NOLOG
				LOG << "> Failed to convert shader to ps_1_4" << std::endl;
				LOG << "> Dumping translated shader assembly:" << std::endl << std::endl << NewSourceCode << std::endl;
				LOG << "> Failed to reassemble shader:" << std::endl << std::endl << static_cast<const char *>(ErrorBuffer->GetBufferPointer()) << std::endl;
#endif
			}
		}
	}

	// Change '-' modifier for constant values when using 'mad' arithmetic by changing it to use 'sub'
	SourceCode = std::regex_replace(SourceCode,
		std::regex("(mad)([_satxd248]*) (r[0-9][\\.wxyz]*), (1?-?[crtv][0-9][\\.wxyz_abdis2]*), (1?-?[crtv][0-9][\\.wxyz_abdis2]*), (-)(c[0-9][\\.wxyz]*)(_bx2|_bias|_x2|_d[zbwa]|)(?![_\\.wxyz])"),
		"sub$2 $3, $4, $7$8 /* changed 'mad' to 'sub' removed $5 removed modifier $6 */");

	// Remove trailing modifiers for constant values
	SourceCode = std::regex_replace(SourceCode,
		std::regex("(c[0-9][\\.wxyz]*)(_bx2|_bias|_x2|_d[zbwa])"),
		"$1 /* removed modifier $2 */");

	// Remove remaining modifiers for constant values
	SourceCode = std::regex_replace(SourceCode,
		std::regex("(1?-)(c[0-9][\\.wxyz]*(?![\\.wxyz]))"),
		"$2 /* removed modifier $1 */");

#ifndef D3D8TO9NOLOG
	LOG << "> Dumping translated shader assembly:" << std::endl << std::endl << SourceCode << std::endl;
#endif

	if (D3DXAssembleShader != nullptr)
	{
		hr = D3DXAssembleShader(SourceCode.data(), static_cast<UINT>(SourceCode.size()), nullptr, nullptr, 0, &Assembly, &ErrorBuffer);
	}
	else
	{
		hr = D3DERR_INVALIDCALL;
	}

	Disassembly->Release();

	if (FAILED(hr))
	{
		if (ErrorBuffer != nullptr)
		{
#ifndef D3D8TO9NOLOG
			LOG << "> Failed to reassemble shader:" << std::endl << std::endl << static_cast<const char *>(ErrorBuffer->GetBufferPointer()) << std::endl;
#endif

			ErrorBuffer->Release();
		}
		else
		{
#ifndef D3D8TO9NOLOG
			LOG << "> Failed to reassemble shader with error code " << std::hex << hr << std::dec << "!" << std::endl;
#endif
		}

		return hr;
	}

	hr = ProxyInterface->CreatePixelShader(static_cast<const DWORD *>(Assembly->GetBufferPointer()), reinterpret_cast<IDirect3DPixelShader9 **>(pHandle));

	if (FAILED(hr))
	{
#ifndef D3D8TO9NOLOG
		LOG << "> 'IDirect3DDevice9::CreatePixelShader' failed with error code " << std::hex << hr << std::dec << "!" << std::endl;
#endif
	}

	return hr;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::SetPixelShader(DWORD Handle)
{
	CurrentPixelShaderHandle = Handle;

	return ProxyInterface->SetPixelShader(reinterpret_cast<IDirect3DPixelShader9 *>(Handle));
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetPixelShader(DWORD *pHandle)
{
	if (pHandle == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	*pHandle = CurrentPixelShaderHandle;

	return D3D_OK;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::DeletePixelShader(DWORD Handle)
{
	if (Handle == 0)
	{
		return D3DERR_INVALIDCALL;
	}

	if (CurrentPixelShaderHandle == Handle)
	{
		SetPixelShader(0);
	}

	reinterpret_cast<IDirect3DPixelShader9 *>(Handle)->Release();

	return D3D_OK;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::SetPixelShaderConstant(DWORD Register, const void *pConstantData, DWORD ConstantCount)
{
	return ProxyInterface->SetPixelShaderConstantF(Register, static_cast<const float *>(pConstantData), ConstantCount);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetPixelShaderConstant(DWORD Register, void *pConstantData, DWORD ConstantCount)
{
	return ProxyInterface->GetPixelShaderConstantF(Register, static_cast<float *>(pConstantData), ConstantCount);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetPixelShaderFunction(DWORD Handle, void *pData, DWORD *pSizeOfData)
{
#ifndef D3D8TO9NOLOG
	LOG << "Redirecting '" << "IDirect3DDevice8::GetPixelShaderFunction" << "(" << this << ", " << Handle << ", " << pData << ", " << pSizeOfData << ")' ..." << std::endl;
#endif

	if (Handle == 0)
	{
		return D3DERR_INVALIDCALL;
	}

	IDirect3DPixelShader9 *const PixelShaderInterface = reinterpret_cast<IDirect3DPixelShader9 *>(Handle);

#ifndef D3D8TO9NOLOG
	LOG << "> Returning translated shader byte code." << std::endl;
#endif

	return PixelShaderInterface->GetFunction(pData, reinterpret_cast<UINT *>(pSizeOfData));
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::DrawRectPatch(UINT Handle, const float *pNumSegs, const D3DRECTPATCH_INFO *pRectPatchInfo)
{
	return ProxyInterface->DrawRectPatch(Handle, pNumSegs, pRectPatchInfo);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::DrawTriPatch(UINT Handle, const float *pNumSegs, const D3DTRIPATCH_INFO *pTriPatchInfo)
{
	return ProxyInterface->DrawTriPatch(Handle, pNumSegs, pTriPatchInfo);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::DeletePatch(UINT Handle)
{
	return ProxyInterface->DeletePatch(Handle);
}
