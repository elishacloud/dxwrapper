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
*
* Code taken from code found here: https://gist.github.com/shaunlebron/3854bf4eec5bec297907
*/

#include "Settings\Settings.h"
#include "Wrappers\Wrapper.h"
#include "d3d9.h"
#include "d3dx9.h"
#include "Logging\Logging.h"

IDirect3D9 *WINAPI _Direct3DCreate9_Wrapper(UINT SDKVersion)
{
	Logging::Log() << "Direct3DCreate9 function (" << SDKVersion << ")";
	orig_Direct3DCreate9 = (D3DC9)Wrapper::d3d9_Wrapper::_Direct3DCreate9_RealProc;
	return new f_iD3D9(orig_Direct3DCreate9(SDKVersion));
}

namespace Wrapper
{
	namespace d3d9_Wrapper
	{
		FARPROC _Direct3DCreate9_RealProc;
		FARPROC _Direct3DCreate9_WrapperProc = (FARPROC)*_Direct3DCreate9_Wrapper;
	}
}

HRESULT f_iD3D9::CreateDevice(UINT Adapter, D3DDEVTYPE DeviceType,
	HWND hFocusWindow, DWORD BehaviorFlags,
	D3DPRESENT_PARAMETERS *pPresentationParameters,
	IDirect3DDevice9 **ppReturnedDeviceInterface)
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
			f_pD3D->CheckDeviceMultiSampleType(Adapter,
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
			// Full-scene antialiasing is supported. Enable it here.
			(*pPresentationParameters).Flags = (*pPresentationParameters).Flags & ~D3DPRESENTFLAG_LOCKABLE_BACKBUFFER & ~D3DPRESENTFLAG_VIDEO;
			(*pPresentationParameters).MultiSampleType = (D3DMULTISAMPLE_TYPE)SampleType;
			(*pPresentationParameters).MultiSampleQuality = QualityLevels;
			(*pPresentationParameters).SwapEffect = D3DSWAPEFFECT_DISCARD;
			BehaviorFlags = D3DCREATE_HARDWARE_VERTEXPROCESSING | (BehaviorFlags & ~D3DCREATE_SOFTWARE_VERTEXPROCESSING);
		}
	}

	// Create Device
	HRESULT hr = f_pD3D->CreateDevice(Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);

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
			BehaviorFlags = bFlags;
			CopyMemory(pPresentationParameters, &d3dpp, sizeof(d3dpp));
			hr = f_pD3D->CreateDevice(Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);
		}
	}

	if (SUCCEEDED(hr))
	{
		*ppReturnedDeviceInterface = new f_IDirect3DDevice9(*ppReturnedDeviceInterface, f_pD3D);
	}

	return hr;
}

ULONG f_iD3D9::AddRef()
{
	return f_pD3D->AddRef();
}

HRESULT f_iD3D9::QueryInterface(REFIID riid, void** ppvObj)
{
	return f_pD3D->QueryInterface(riid, ppvObj);
}

ULONG f_iD3D9::Release()
{
	return f_pD3D->Release();
}

HRESULT f_iD3D9::EnumAdapterModes(THIS_ UINT Adapter, D3DFORMAT Format, UINT Mode, D3DDISPLAYMODE* pMode)
{
	return f_pD3D->EnumAdapterModes(Adapter, Format, Mode, pMode);
}

UINT f_iD3D9::GetAdapterCount()
{
	UINT Count = f_pD3D->GetAdapterCount();
#ifdef D3D9LOGGING
	Logging::Log() << "AdapterCount " << Count;
#endif // D3D9LOGGING
	return  Count;
}

HRESULT f_iD3D9::GetAdapterDisplayMode(UINT Adapter, D3DDISPLAYMODE *pMode)
{
	return f_pD3D->GetAdapterDisplayMode(Adapter, pMode);
}

HRESULT f_iD3D9::GetAdapterIdentifier(UINT Adapter, DWORD Flags, D3DADAPTER_IDENTIFIER9 *pIdentifier)
{
	return f_pD3D->GetAdapterIdentifier(Adapter, Flags, pIdentifier);
}

UINT f_iD3D9::GetAdapterModeCount(THIS_ UINT Adapter, D3DFORMAT Format)
{
	UINT Count = f_pD3D->GetAdapterModeCount(Adapter, Format);
#ifdef D3D9LOGGING
	Logging::Log() << "AdapterModeCount Adapter " << Adapter << " Format " << Format << " Count " << Count;
#endif // D3D9LOGGING
	return Count;
}

HMONITOR f_iD3D9::GetAdapterMonitor(UINT Adapter)
{
	return f_pD3D->GetAdapterMonitor(Adapter);
}

HRESULT f_iD3D9::GetDeviceCaps(UINT Adapter, D3DDEVTYPE DeviceType, D3DCAPS9 *pCaps)
{
	return f_pD3D->GetDeviceCaps(Adapter, DeviceType, pCaps);
}

HRESULT f_iD3D9::RegisterSoftwareDevice(void *pInitializeFunction)
{
	return f_pD3D->RegisterSoftwareDevice(pInitializeFunction);
}

HRESULT f_iD3D9::CheckDepthStencilMatch(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, D3DFORMAT RenderTargetFormat, D3DFORMAT DepthStencilFormat)
{
#ifdef D3D9LOGGING
	Logging::Log() << "CheckDepthStencilMatch " << Adapter << " " << DeviceType << " " << AdapterFormat << " " << RenderTargetFormat << " " << DepthStencilFormat;
#endif // D3D9LOGGING
	return f_pD3D->CheckDepthStencilMatch(Adapter, DeviceType, AdapterFormat, RenderTargetFormat, DepthStencilFormat);
}

HRESULT f_iD3D9::CheckDeviceFormat(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, DWORD Usage, D3DRESOURCETYPE RType, D3DFORMAT CheckFormat)
{
	return f_pD3D->CheckDeviceFormat(Adapter, DeviceType, AdapterFormat, Usage, RType, CheckFormat);
}

HRESULT f_iD3D9::CheckDeviceMultiSampleType(THIS_ UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SurfaceFormat, BOOL Windowed, D3DMULTISAMPLE_TYPE MultiSampleType, DWORD* pQualityLevels)
{
	return f_pD3D->CheckDeviceMultiSampleType(Adapter, DeviceType, SurfaceFormat, Windowed, MultiSampleType, pQualityLevels);
}

HRESULT f_iD3D9::CheckDeviceType(UINT Adapter, D3DDEVTYPE CheckType, D3DFORMAT DisplayFormat, D3DFORMAT BackBufferFormat, BOOL Windowed)
{
	return f_pD3D->CheckDeviceType(Adapter, CheckType, DisplayFormat, BackBufferFormat, Windowed);
}


HRESULT f_iD3D9::CheckDeviceFormatConversion(THIS_ UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SourceFormat, D3DFORMAT TargetFormat)
{
	return f_pD3D->CheckDeviceFormatConversion(Adapter, DeviceType, SourceFormat, TargetFormat);
}

ULONG f_IDirect3DDevice9::AddRef()
{
	return f_pD3DDevice->AddRef();
}

HRESULT f_IDirect3DDevice9::QueryInterface(REFIID riid, void** ppvObj)
{
	return f_pD3DDevice->QueryInterface(riid, ppvObj);
}

ULONG f_IDirect3DDevice9::Release()
{
	return f_pD3DDevice->Release();
}

HRESULT f_IDirect3DDevice9::Reset(D3DPRESENT_PARAMETERS *pPresentationParameters)
{
	HRESULT hr = f_pD3DDevice->Reset(pPresentationParameters);

	return hr;
}

HRESULT f_IDirect3DDevice9::EndScene()
{
	return f_pD3DDevice->EndScene();
}

void f_IDirect3DDevice9::SetCursorPosition(int X, int Y, DWORD Flags)
{
	f_pD3DDevice->SetCursorPosition(X, Y, Flags);
}

HRESULT f_IDirect3DDevice9::SetCursorProperties(UINT XHotSpot, UINT YHotSpot, IDirect3DSurface9 *pCursorBitmap)
{
	return f_pD3DDevice->SetCursorProperties(XHotSpot, YHotSpot, pCursorBitmap);
}

BOOL f_IDirect3DDevice9::ShowCursor(BOOL bShow)
{
	return f_pD3DDevice->ShowCursor(bShow);
}

HRESULT f_IDirect3DDevice9::CreateAdditionalSwapChain(D3DPRESENT_PARAMETERS *pPresentationParameters, IDirect3DSwapChain9 **ppSwapChain)
{
	return f_pD3DDevice->CreateAdditionalSwapChain(pPresentationParameters, ppSwapChain);
}

HRESULT f_IDirect3DDevice9::CreateCubeTexture(THIS_ UINT EdgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DCubeTexture9** ppCubeTexture, HANDLE* pSharedHandle)
{
	return f_pD3DDevice->CreateCubeTexture(EdgeLength, Levels, Usage, Format, Pool, ppCubeTexture, pSharedHandle);
}

HRESULT f_IDirect3DDevice9::CreateDepthStencilSurface(THIS_ UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Discard, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle)
{
	return f_pD3DDevice->CreateDepthStencilSurface(Width, Height, Format, MultiSample, MultisampleQuality, Discard, ppSurface, pSharedHandle);
}

HRESULT f_IDirect3DDevice9::CreateIndexBuffer(THIS_ UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DIndexBuffer9** ppIndexBuffer, HANDLE* pSharedHandle)
{
	return f_pD3DDevice->CreateIndexBuffer(Length, Usage, Format, Pool, ppIndexBuffer, pSharedHandle);
}

HRESULT f_IDirect3DDevice9::CreateRenderTarget(THIS_ UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle)
{
	return f_pD3DDevice->CreateRenderTarget(Width, Height, Format, MultiSample, MultisampleQuality, Lockable, ppSurface, pSharedHandle);
}

HRESULT f_IDirect3DDevice9::CreateTexture(THIS_ UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture9** ppTexture, HANDLE* pSharedHandle)
{
	return f_pD3DDevice->CreateTexture(Width, Height, Levels, Usage, Format, Pool, ppTexture, pSharedHandle);
}

HRESULT f_IDirect3DDevice9::CreateVertexBuffer(THIS_ UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool, IDirect3DVertexBuffer9** ppVertexBuffer, HANDLE* pSharedHandle)
{
	return f_pD3DDevice->CreateVertexBuffer(Length, Usage, FVF, Pool, ppVertexBuffer, pSharedHandle);
}

HRESULT f_IDirect3DDevice9::CreateVolumeTexture(THIS_ UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DVolumeTexture9** ppVolumeTexture, HANDLE* pSharedHandle)
{
	return f_pD3DDevice->CreateVolumeTexture(Width, Height, Depth, Levels, Usage, Format, Pool, ppVolumeTexture, pSharedHandle);
}

HRESULT f_IDirect3DDevice9::BeginStateBlock()
{
	return f_pD3DDevice->BeginStateBlock();
}

HRESULT f_IDirect3DDevice9::CreateStateBlock(THIS_ D3DSTATEBLOCKTYPE Type, IDirect3DStateBlock9** ppSB)
{
	return f_pD3DDevice->CreateStateBlock(Type, ppSB);
}

HRESULT f_IDirect3DDevice9::EndStateBlock(THIS_ IDirect3DStateBlock9** ppSB)
{
	return f_pD3DDevice->EndStateBlock(ppSB);
}

HRESULT f_IDirect3DDevice9::GetClipStatus(D3DCLIPSTATUS9 *pClipStatus)
{
	return f_pD3DDevice->GetClipStatus(pClipStatus);
}

HRESULT f_IDirect3DDevice9::GetDisplayMode(THIS_ UINT iSwapChain, D3DDISPLAYMODE* pMode)
{
	return f_pD3DDevice->GetDisplayMode(iSwapChain, pMode);
}

HRESULT f_IDirect3DDevice9::GetRenderState(D3DRENDERSTATETYPE State, DWORD *pValue)
{
	return f_pD3DDevice->GetRenderState(State, pValue);
}

HRESULT f_IDirect3DDevice9::GetRenderTarget(THIS_ DWORD RenderTargetIndex, IDirect3DSurface9** ppRenderTarget)
{
	return f_pD3DDevice->GetRenderTarget(RenderTargetIndex, ppRenderTarget);
}

HRESULT f_IDirect3DDevice9::GetTransform(D3DTRANSFORMSTATETYPE State, D3DMATRIX *pMatrix)
{
	return f_pD3DDevice->GetTransform(State, pMatrix);
}

HRESULT f_IDirect3DDevice9::SetClipStatus(CONST D3DCLIPSTATUS9 *pClipStatus)
{
	return f_pD3DDevice->SetClipStatus(pClipStatus);
}

HRESULT f_IDirect3DDevice9::SetRenderState(D3DRENDERSTATETYPE State, DWORD Value)
{
#ifdef D3D9LOGGING
	Logging::Log() << "SetRenderState " << State << " " << Value;
#endif // D3D9LOGGING
	return f_pD3DDevice->SetRenderState(State, Value);
}

HRESULT f_IDirect3DDevice9::SetRenderTarget(THIS_ DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget)
{
	return f_pD3DDevice->SetRenderTarget(RenderTargetIndex, pRenderTarget);
}

HRESULT f_IDirect3DDevice9::SetTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX *pMatrix)
{
	return f_pD3DDevice->SetTransform(State, pMatrix);
}

void f_IDirect3DDevice9::GetGammaRamp(THIS_ UINT iSwapChain, D3DGAMMARAMP* pRamp)
{
	f_pD3DDevice->GetGammaRamp(iSwapChain, pRamp);
}

void f_IDirect3DDevice9::SetGammaRamp(THIS_ UINT iSwapChain, DWORD Flags, CONST D3DGAMMARAMP* pRamp)
{
	f_pD3DDevice->SetGammaRamp(iSwapChain, Flags, pRamp);
}

HRESULT f_IDirect3DDevice9::DeletePatch(UINT Handle)
{
	return f_pD3DDevice->DeletePatch(Handle);
}

HRESULT f_IDirect3DDevice9::DrawRectPatch(UINT Handle, CONST float *pNumSegs, CONST D3DRECTPATCH_INFO *pRectPatchInfo)
{
	return f_pD3DDevice->DrawRectPatch(Handle, pNumSegs, pRectPatchInfo);
}

HRESULT f_IDirect3DDevice9::DrawTriPatch(UINT Handle, CONST float *pNumSegs, CONST D3DTRIPATCH_INFO *pTriPatchInfo)
{
	return f_pD3DDevice->DrawTriPatch(Handle, pNumSegs, pTriPatchInfo);
}

HRESULT f_IDirect3DDevice9::GetIndices(THIS_ IDirect3DIndexBuffer9** ppIndexData)
{
	return f_pD3DDevice->GetIndices(ppIndexData);
}

HRESULT f_IDirect3DDevice9::SetIndices(THIS_ IDirect3DIndexBuffer9* pIndexData)
{
	return f_pD3DDevice->SetIndices(pIndexData);
}

UINT f_IDirect3DDevice9::GetAvailableTextureMem()
{
	return f_pD3DDevice->GetAvailableTextureMem();
}

HRESULT f_IDirect3DDevice9::GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS *pParameters)
{
	return f_pD3DDevice->GetCreationParameters(pParameters);
}

HRESULT f_IDirect3DDevice9::GetDeviceCaps(D3DCAPS9 *pCaps)
{
	return f_pD3DDevice->GetDeviceCaps(pCaps);
}

HRESULT f_IDirect3DDevice9::GetDirect3D(IDirect3D9 **ppD3D9)
{
	*ppD3D9 = f_pD3D;
	return D3D_OK;
}

HRESULT f_IDirect3DDevice9::GetRasterStatus(THIS_ UINT iSwapChain, D3DRASTER_STATUS* pRasterStatus)
{
	return f_pD3DDevice->GetRasterStatus(iSwapChain, pRasterStatus);
}

HRESULT f_IDirect3DDevice9::GetLight(DWORD Index, D3DLIGHT9 *pLight)
{
	return f_pD3DDevice->GetLight(Index, pLight);
}

HRESULT f_IDirect3DDevice9::GetLightEnable(DWORD Index, BOOL *pEnable)
{
	return f_pD3DDevice->GetLightEnable(Index, pEnable);
}

HRESULT f_IDirect3DDevice9::GetMaterial(D3DMATERIAL9 *pMaterial)
{
	return f_pD3DDevice->GetMaterial(pMaterial);
}

HRESULT f_IDirect3DDevice9::LightEnable(DWORD LightIndex, BOOL bEnable)
{
	return f_pD3DDevice->LightEnable(LightIndex, bEnable);
}

HRESULT f_IDirect3DDevice9::SetLight(DWORD Index, CONST D3DLIGHT9 *pLight)
{

	return f_pD3DDevice->SetLight(Index, pLight);
}

HRESULT f_IDirect3DDevice9::SetMaterial(CONST D3DMATERIAL9 *pMaterial)
{
	return f_pD3DDevice->SetMaterial(pMaterial);
}

HRESULT f_IDirect3DDevice9::MultiplyTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX *pMatrix)
{
	return f_pD3DDevice->MultiplyTransform(State, pMatrix);
}

HRESULT f_IDirect3DDevice9::ProcessVertices(THIS_ UINT SrcStartIndex, UINT DestIndex, UINT VertexCount, IDirect3DVertexBuffer9* pDestBuffer, IDirect3DVertexDeclaration9* pVertexDecl, DWORD Flags)
{
	return f_pD3DDevice->ProcessVertices(SrcStartIndex, DestIndex, VertexCount, pDestBuffer, pVertexDecl, Flags);
}

HRESULT f_IDirect3DDevice9::TestCooperativeLevel()
{
	return f_pD3DDevice->TestCooperativeLevel();
}

HRESULT f_IDirect3DDevice9::GetCurrentTexturePalette(UINT *pPaletteNumber)
{
	return f_pD3DDevice->GetCurrentTexturePalette(pPaletteNumber);
}

HRESULT f_IDirect3DDevice9::GetPaletteEntries(UINT PaletteNumber, PALETTEENTRY *pEntries)
{
	return f_pD3DDevice->GetPaletteEntries(PaletteNumber, pEntries);
}

HRESULT f_IDirect3DDevice9::SetCurrentTexturePalette(UINT PaletteNumber)
{
	return f_pD3DDevice->SetCurrentTexturePalette(PaletteNumber);
}

HRESULT f_IDirect3DDevice9::SetPaletteEntries(UINT PaletteNumber, CONST PALETTEENTRY *pEntries)
{
	return f_pD3DDevice->SetPaletteEntries(PaletteNumber, pEntries);
}

HRESULT f_IDirect3DDevice9::CreatePixelShader(THIS_ CONST DWORD* pFunction, IDirect3DPixelShader9** ppShader)
{
#ifdef D3D9LOGGING
	if (pFunction != nullptr)
	{
		Logging::Log() << "<CreatePixelShader> Disassembling shader and translating assembly to Direct3D 9 compatible code ...";

		/*if (*pFunction < D3DVS_VERSION(1, 0) || *pFunction > D3DVS_VERSION(1, 1))
		{
			Logging::Log() << "> Failed because of version mismatch ('" << std::showbase << std::hex << *pFunction << std::dec << std::noshowbase << "')! Only 'vs_1_x' shaders are supported.";

			return D3DERR_INVALIDCALL;
		}*/

		ID3DXBuffer *Disassembly = nullptr;

		HRESULT hr = D3DXDisassembleShader(pFunction, FALSE, nullptr, &Disassembly);

		if (FAILED(hr))
		{
			Logging::Log() << "> Failed to disassemble shader with error code " << std::hex << hr << std::dec << "!";

			return hr;
		}

		std::string SourceCode(static_cast<const char *>(Disassembly->GetBufferPointer()), Disassembly->GetBufferSize() - 1);
		const char *cstr = SourceCode.c_str();

		Logging::Log() << "> Dumping translated shader assembly:\n" << cstr;
	}
#endif // D3D9LOGGING

	return f_pD3DDevice->CreatePixelShader(pFunction, ppShader);
}

HRESULT f_IDirect3DDevice9::GetPixelShader(THIS_ IDirect3DPixelShader9** ppShader)
{
	return f_pD3DDevice->GetPixelShader(ppShader);
}

HRESULT f_IDirect3DDevice9::SetPixelShader(THIS_ IDirect3DPixelShader9* pShader)
{
	return f_pD3DDevice->SetPixelShader(pShader);
}

HRESULT f_IDirect3DDevice9::Present(CONST RECT *pSourceRect, CONST RECT *pDestRect, HWND hDestWindowOverride, CONST RGNDATA *pDirtyRegion)
{
	return f_pD3DDevice->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
}

HRESULT f_IDirect3DDevice9::DrawIndexedPrimitive(THIS_ D3DPRIMITIVETYPE Type, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount)
{
	return f_pD3DDevice->DrawIndexedPrimitive(Type, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
}

HRESULT f_IDirect3DDevice9::DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT MinIndex, UINT NumVertices, UINT PrimitiveCount, CONST void *pIndexData, D3DFORMAT IndexDataFormat, CONST void *pVertexStreamZeroData, UINT VertexStreamZeroStride)
{
	return f_pD3DDevice->DrawIndexedPrimitiveUP(PrimitiveType, MinIndex, NumVertices, PrimitiveCount, pIndexData, IndexDataFormat, pVertexStreamZeroData, VertexStreamZeroStride);
}

HRESULT f_IDirect3DDevice9::DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount)
{
	return f_pD3DDevice->DrawPrimitive(PrimitiveType, StartVertex, PrimitiveCount);
}

HRESULT f_IDirect3DDevice9::DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, CONST void *pVertexStreamZeroData, UINT VertexStreamZeroStride)
{
	return f_pD3DDevice->DrawPrimitiveUP(PrimitiveType, PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride);
}

HRESULT f_IDirect3DDevice9::BeginScene()
{
	return f_pD3DDevice->BeginScene();
}

HRESULT f_IDirect3DDevice9::GetStreamSource(THIS_ UINT StreamNumber, IDirect3DVertexBuffer9** ppStreamData, UINT* OffsetInBytes, UINT* pStride)
{
	return f_pD3DDevice->GetStreamSource(StreamNumber, ppStreamData, OffsetInBytes, pStride);
}

HRESULT f_IDirect3DDevice9::SetStreamSource(THIS_ UINT StreamNumber, IDirect3DVertexBuffer9* pStreamData, UINT OffsetInBytes, UINT Stride)
{
	return f_pD3DDevice->SetStreamSource(StreamNumber, pStreamData, OffsetInBytes, Stride);
}

HRESULT f_IDirect3DDevice9::GetBackBuffer(THIS_ UINT iSwapChain, UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface9** ppBackBuffer)
{
	return f_pD3DDevice->GetBackBuffer(iSwapChain, iBackBuffer, Type, ppBackBuffer);
}

HRESULT f_IDirect3DDevice9::GetDepthStencilSurface(IDirect3DSurface9 **ppZStencilSurface)
{
	return f_pD3DDevice->GetDepthStencilSurface(ppZStencilSurface);
}

HRESULT f_IDirect3DDevice9::GetTexture(DWORD Stage, IDirect3DBaseTexture9 **ppTexture)
{
	return f_pD3DDevice->GetTexture(Stage, ppTexture);
}

HRESULT f_IDirect3DDevice9::GetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD *pValue)
{
	return f_pD3DDevice->GetTextureStageState(Stage, Type, pValue);
}

HRESULT f_IDirect3DDevice9::SetTexture(DWORD Stage, IDirect3DBaseTexture9 *pTexture)
{
	return f_pD3DDevice->SetTexture(Stage, pTexture);
}

HRESULT f_IDirect3DDevice9::SetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value)
{
#ifdef D3D9LOGGING
	Logging::Log() << "SetTextureStageState " << Stage << " " << Type << " " << Value;
#endif // D3D9LOGGING
	return f_pD3DDevice->SetTextureStageState(Stage, Type, Value);
}

HRESULT f_IDirect3DDevice9::UpdateTexture(IDirect3DBaseTexture9 *pSourceTexture, IDirect3DBaseTexture9 *pDestinationTexture)
{
	return f_pD3DDevice->UpdateTexture(pSourceTexture, pDestinationTexture);
}

HRESULT f_IDirect3DDevice9::ValidateDevice(DWORD *pNumPasses)
{
	HRESULT rt = f_pD3DDevice->ValidateDevice(pNumPasses);
#ifdef D3D9LOGGING
	Logging::Log() << "ValidateDevice " << rt << "\n";
#endif // D3D9LOGGING
	return rt;
}

HRESULT f_IDirect3DDevice9::GetClipPlane(DWORD Index, float *pPlane)
{
	return f_pD3DDevice->GetClipPlane(Index, pPlane);
}

HRESULT f_IDirect3DDevice9::SetClipPlane(DWORD Index, CONST float *pPlane)
{
	return f_pD3DDevice->SetClipPlane(Index, pPlane);
}

HRESULT f_IDirect3DDevice9::Clear(DWORD Count, CONST D3DRECT *pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil)
{
	return f_pD3DDevice->Clear(Count, pRects, Flags, Color, Z, Stencil);
}

HRESULT f_IDirect3DDevice9::GetViewport(D3DVIEWPORT9 *pViewport)
{
	return f_pD3DDevice->GetViewport(pViewport);
}

HRESULT f_IDirect3DDevice9::SetViewport(CONST D3DVIEWPORT9 *pViewport)
{
	return f_pD3DDevice->SetViewport(pViewport);
}

HRESULT f_IDirect3DDevice9::CreateVertexShader(THIS_ CONST DWORD* pFunction, IDirect3DVertexShader9** ppShader)
{
#ifdef D3D9LOGGING
	if (pFunction != nullptr)
	{
		Logging::Log() << "<CreateVertexShader> Disassembling shader and translating assembly to Direct3D 9 compatible code ...";

		if (*pFunction < D3DVS_VERSION(1, 0) || *pFunction > D3DVS_VERSION(1, 1))
		{
			Logging::Log() << "> Failed because of version mismatch ('" << std::showbase << std::hex << *pFunction << std::dec << std::noshowbase << "')! Only 'vs_1_x' shaders are supported.";

			return D3DERR_INVALIDCALL;
		}

		ID3DXBuffer *Disassembly = nullptr;

		HRESULT hr = D3DXDisassembleShader(pFunction, FALSE, nullptr, &Disassembly);

		if (FAILED(hr))
		{
			Logging::Log() << "> Failed to disassemble shader with error code " << std::hex << hr << std::dec << "!";

			return hr;
		}

		std::string SourceCode(static_cast<const char *>(Disassembly->GetBufferPointer()), Disassembly->GetBufferSize() - 1);
		const char *cstr = SourceCode.c_str();

		Logging::Log() << "> Dumping translated shader assembly:\n" << cstr;
	}
#endif // D3D9LOGGING

	return f_pD3DDevice->CreateVertexShader(pFunction, ppShader);
}

HRESULT f_IDirect3DDevice9::GetVertexShader(THIS_ IDirect3DVertexShader9** ppShader)
{
	return f_pD3DDevice->GetVertexShader(ppShader);
}

HRESULT f_IDirect3DDevice9::SetVertexShader(THIS_ IDirect3DVertexShader9* pShader)
{
	return f_pD3DDevice->SetVertexShader(pShader);
}

HRESULT f_IDirect3DDevice9::CreateQuery(THIS_ D3DQUERYTYPE Type, IDirect3DQuery9** ppQuery)
{
	return f_pD3DDevice->CreateQuery(Type, ppQuery);
}

HRESULT f_IDirect3DDevice9::SetPixelShaderConstantB(THIS_ UINT StartRegister, CONST BOOL* pConstantData, UINT  BoolCount)
{
	return f_pD3DDevice->SetPixelShaderConstantB(StartRegister, pConstantData, BoolCount);
}

HRESULT f_IDirect3DDevice9::GetPixelShaderConstantB(THIS_ UINT StartRegister, BOOL* pConstantData, UINT BoolCount)
{
	return f_pD3DDevice->GetPixelShaderConstantB(StartRegister, pConstantData, BoolCount);
}

HRESULT f_IDirect3DDevice9::SetPixelShaderConstantI(THIS_ UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount)
{
	return f_pD3DDevice->SetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount);
}

HRESULT f_IDirect3DDevice9::GetPixelShaderConstantI(THIS_ UINT StartRegister, int* pConstantData, UINT Vector4iCount)
{
	return f_pD3DDevice->GetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount);
}

HRESULT f_IDirect3DDevice9::SetPixelShaderConstantF(THIS_ UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount)
{
	return f_pD3DDevice->SetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount);
}

HRESULT f_IDirect3DDevice9::GetPixelShaderConstantF(THIS_ UINT StartRegister, float* pConstantData, UINT Vector4fCount)
{
	return f_pD3DDevice->GetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount);
}

HRESULT f_IDirect3DDevice9::SetStreamSourceFreq(THIS_ UINT StreamNumber, UINT Divider)
{
	return f_pD3DDevice->SetStreamSourceFreq(StreamNumber, Divider);
}

HRESULT f_IDirect3DDevice9::GetStreamSourceFreq(THIS_ UINT StreamNumber, UINT* Divider)
{
	return f_pD3DDevice->GetStreamSourceFreq(StreamNumber, Divider);
}

HRESULT f_IDirect3DDevice9::SetVertexShaderConstantB(THIS_ UINT StartRegister, CONST BOOL* pConstantData, UINT  BoolCount)
{
	return f_pD3DDevice->SetVertexShaderConstantB(StartRegister, pConstantData, BoolCount);
}

HRESULT f_IDirect3DDevice9::GetVertexShaderConstantB(THIS_ UINT StartRegister, BOOL* pConstantData, UINT BoolCount)
{
	return f_pD3DDevice->GetVertexShaderConstantB(StartRegister, pConstantData, BoolCount);
}

HRESULT f_IDirect3DDevice9::SetVertexShaderConstantF(THIS_ UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount)
{
	return f_pD3DDevice->SetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount);
}

HRESULT f_IDirect3DDevice9::GetVertexShaderConstantF(THIS_ UINT StartRegister, float* pConstantData, UINT Vector4fCount)
{
	return f_pD3DDevice->GetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount);
}

HRESULT f_IDirect3DDevice9::SetVertexShaderConstantI(THIS_ UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount)
{
	return f_pD3DDevice->SetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount);
}

HRESULT f_IDirect3DDevice9::GetVertexShaderConstantI(THIS_ UINT StartRegister, int* pConstantData, UINT Vector4iCount)
{
	return f_pD3DDevice->GetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount);
}

HRESULT f_IDirect3DDevice9::SetFVF(THIS_ DWORD FVF)
{
#ifdef D3D9LOGGING
	Logging::Log() << "SetFVF " << FVF;
#endif // D3D9LOGGING
	return f_pD3DDevice->SetFVF(FVF);
}

HRESULT f_IDirect3DDevice9::GetFVF(THIS_ DWORD* pFVF)
{
#ifdef D3D9LOGGING
	Logging::Log() << "GetFVF " << *pFVF;
#endif // D3D9LOGGING
	return f_pD3DDevice->GetFVF(pFVF);
}

HRESULT f_IDirect3DDevice9::CreateVertexDeclaration(THIS_ CONST D3DVERTEXELEMENT9* pVertexElements, IDirect3DVertexDeclaration9** ppDecl)
{
	return f_pD3DDevice->CreateVertexDeclaration(pVertexElements, ppDecl);
}

HRESULT f_IDirect3DDevice9::SetVertexDeclaration(THIS_ IDirect3DVertexDeclaration9* pDecl)
{
	return f_pD3DDevice->SetVertexDeclaration(pDecl);
}

HRESULT f_IDirect3DDevice9::GetVertexDeclaration(THIS_ IDirect3DVertexDeclaration9** ppDecl)
{
	return f_pD3DDevice->GetVertexDeclaration(ppDecl);
}

HRESULT f_IDirect3DDevice9::SetNPatchMode(THIS_ float nSegments)
{
	return f_pD3DDevice->SetNPatchMode(nSegments);
}

float f_IDirect3DDevice9::GetNPatchMode(THIS)
{
	return f_pD3DDevice->GetNPatchMode();
}

int f_IDirect3DDevice9::GetSoftwareVertexProcessing(THIS)
{
	return f_pD3DDevice->GetSoftwareVertexProcessing();
}

unsigned int f_IDirect3DDevice9::GetNumberOfSwapChains(THIS)
{
	return f_pD3DDevice->GetNumberOfSwapChains();
}

HRESULT f_IDirect3DDevice9::EvictManagedResources(THIS)
{
	return f_pD3DDevice->EvictManagedResources();
}

HRESULT f_IDirect3DDevice9::SetSoftwareVertexProcessing(THIS_ BOOL bSoftware)
{
	return f_pD3DDevice->SetSoftwareVertexProcessing(bSoftware);
}

HRESULT f_IDirect3DDevice9::SetScissorRect(THIS_ CONST RECT* pRect)
{
	return f_pD3DDevice->SetScissorRect(pRect);
}

HRESULT f_IDirect3DDevice9::GetScissorRect(THIS_ RECT* pRect)
{
	return f_pD3DDevice->GetScissorRect(pRect);
}

HRESULT f_IDirect3DDevice9::GetSamplerState(THIS_ DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD* pValue)
{
	return f_pD3DDevice->GetSamplerState(Sampler, Type, pValue);
}

HRESULT f_IDirect3DDevice9::SetSamplerState(THIS_ DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value)
{
	return f_pD3DDevice->SetSamplerState(Sampler, Type, Value);
}

HRESULT f_IDirect3DDevice9::SetDepthStencilSurface(THIS_ IDirect3DSurface9* pNewZStencil)
{
	return f_pD3DDevice->SetDepthStencilSurface(pNewZStencil);
}

HRESULT f_IDirect3DDevice9::CreateOffscreenPlainSurface(THIS_ UINT Width, UINT Height, D3DFORMAT Format, D3DPOOL Pool, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle)
{
	return f_pD3DDevice->CreateOffscreenPlainSurface(Width, Height, Format, Pool, ppSurface, pSharedHandle);
}

HRESULT f_IDirect3DDevice9::ColorFill(THIS_ IDirect3DSurface9* pSurface, CONST RECT* pRect, D3DCOLOR color)
{
	return f_pD3DDevice->ColorFill(pSurface, pRect, color);
}

HRESULT f_IDirect3DDevice9::StretchRect(THIS_ IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestSurface, CONST RECT* pDestRect, D3DTEXTUREFILTERTYPE Filter)
{
	return f_pD3DDevice->StretchRect(pSourceSurface, pSourceRect, pDestSurface, pDestRect, Filter);
}

HRESULT f_IDirect3DDevice9::GetFrontBufferData(THIS_ UINT iSwapChain, IDirect3DSurface9* pDestSurface)
{
	return f_pD3DDevice->GetFrontBufferData(iSwapChain, pDestSurface);
}

HRESULT f_IDirect3DDevice9::GetRenderTargetData(THIS_ IDirect3DSurface9* pRenderTarget, IDirect3DSurface9* pDestSurface)
{
	return f_pD3DDevice->GetRenderTargetData(pRenderTarget, pDestSurface);
}

HRESULT f_IDirect3DDevice9::UpdateSurface(THIS_ IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestinationSurface, CONST POINT* pDestPoint)
{
	return f_pD3DDevice->UpdateSurface(pSourceSurface, pSourceRect, pDestinationSurface, pDestPoint);
}

HRESULT f_IDirect3DDevice9::SetDialogBoxMode(THIS_ BOOL bEnableDialogs)
{
	return f_pD3DDevice->SetDialogBoxMode(bEnableDialogs);
}

HRESULT f_IDirect3DDevice9::GetSwapChain(THIS_ UINT iSwapChain, IDirect3DSwapChain9** pSwapChain)
{
	return f_pD3DDevice->GetSwapChain(iSwapChain, pSwapChain);
}
