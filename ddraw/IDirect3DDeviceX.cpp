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

#include "ddraw.h"

HRESULT m_IDirect3DDeviceX::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
	return ProxyQueryInterface(ProxyInterface, riid, ppvObj, WrapperID, WrapperInterface);
}

ULONG m_IDirect3DDeviceX::AddRef()
{
	return ProxyInterface->AddRef();
}

ULONG m_IDirect3DDeviceX::Release()
{
	ULONG x = ProxyInterface->Release();

	if (x == 0)
	{
		if (WrapperInterface)
		{
			WrapperInterface->DeleteMe();
		}
		else
		{
			delete this;
		}
	}

	return x;
}

HRESULT m_IDirect3DDeviceX::Initialize(LPDIRECT3D lpd3d, LPGUID lpGUID, LPD3DDEVICEDESC lpd3ddvdesc)
{
	if (ProxyDirectXVersion != 1)
	{
		return D3D_OK;
	}

	if (lpd3d)
	{
		lpd3d = static_cast<m_IDirect3D *>(lpd3d)->GetProxyInterface();
	}

	return ((IDirect3DDevice*)ProxyInterface)->Initialize(lpd3d, lpGUID, lpd3ddvdesc);
}

HRESULT m_IDirect3DDeviceX::CreateExecuteBuffer(LPD3DEXECUTEBUFFERDESC lpDesc, LPDIRECT3DEXECUTEBUFFER * lplpDirect3DExecuteBuffer, IUnknown * pUnkOuter)
{
	if (ProxyDirectXVersion != 1)
	{
		Logging::Log() << __FUNCTION__ << " Not Implimented";
		return E_NOTIMPL;
	}

	HRESULT hr = ((IDirect3DDevice*)ProxyInterface)->CreateExecuteBuffer(lpDesc, lplpDirect3DExecuteBuffer, pUnkOuter);

	if (SUCCEEDED(hr) && lplpDirect3DExecuteBuffer)
	{
		*lplpDirect3DExecuteBuffer = ProxyAddressLookupTable.FindAddress<m_IDirect3DExecuteBuffer>(*lplpDirect3DExecuteBuffer);
	}

	return hr;
}

HRESULT m_IDirect3DDeviceX::Execute(LPDIRECT3DEXECUTEBUFFER lpDirect3DExecuteBuffer, LPDIRECT3DVIEWPORT lpDirect3DViewport, DWORD dwFlags)
{
	if (ProxyDirectXVersion != 1)
	{
		Logging::Log() << __FUNCTION__ << " Not Implimented";
		return E_NOTIMPL;
	}

	if (lpDirect3DExecuteBuffer)
	{
		lpDirect3DExecuteBuffer = static_cast<m_IDirect3DExecuteBuffer *>(lpDirect3DExecuteBuffer)->GetProxyInterface();
	}
	if (lpDirect3DViewport)
	{
		lpDirect3DViewport = static_cast<m_IDirect3DViewport *>(lpDirect3DViewport)->GetProxyInterface();
	}

	return ((IDirect3DDevice*)ProxyInterface)->Execute(lpDirect3DExecuteBuffer, lpDirect3DViewport, dwFlags);
}

HRESULT m_IDirect3DDeviceX::Pick(LPDIRECT3DEXECUTEBUFFER lpDirect3DExecuteBuffer, LPDIRECT3DVIEWPORT lpDirect3DViewport, DWORD dwFlags, LPD3DRECT lpRect)
{
	if (ProxyDirectXVersion != 1)
	{
		Logging::Log() << __FUNCTION__ << " Not Implimented";
		return E_NOTIMPL;
	}

	if (lpDirect3DExecuteBuffer)
	{
		lpDirect3DExecuteBuffer = static_cast<m_IDirect3DExecuteBuffer *>(lpDirect3DExecuteBuffer)->GetProxyInterface();
	}
	if (lpDirect3DViewport)
	{
		lpDirect3DViewport = static_cast<m_IDirect3DViewport *>(lpDirect3DViewport)->GetProxyInterface();
	}

	return ((IDirect3DDevice*)ProxyInterface)->Pick(lpDirect3DExecuteBuffer, lpDirect3DViewport, dwFlags, lpRect);
}

HRESULT m_IDirect3DDeviceX::GetPickRecords(LPDWORD lpCount, LPD3DPICKRECORD lpD3DPickRec)
{
	if (ProxyDirectXVersion != 1)
	{
		Logging::Log() << __FUNCTION__ << " Not Implimented";
		return E_NOTIMPL;
	}

	return ((IDirect3DDevice*)ProxyInterface)->GetPickRecords(lpCount, lpD3DPickRec);
}

HRESULT m_IDirect3DDeviceX::CreateMatrix(LPD3DMATRIXHANDLE lpD3DMatHandle)
{
	if (ProxyDirectXVersion != 1)
	{
		Logging::Log() << __FUNCTION__ << " Not Implimented";
		return E_NOTIMPL;
	}

	return ((IDirect3DDevice*)ProxyInterface)->CreateMatrix(lpD3DMatHandle);
}

HRESULT m_IDirect3DDeviceX::SetMatrix(D3DMATRIXHANDLE d3dMatHandle, const LPD3DMATRIX lpD3DMatrix)
{
	if (ProxyDirectXVersion != 1)
	{
		Logging::Log() << __FUNCTION__ << " Not Implimented";
		return E_NOTIMPL;
	}

	return ((IDirect3DDevice*)ProxyInterface)->SetMatrix(d3dMatHandle, lpD3DMatrix);
}

HRESULT m_IDirect3DDeviceX::GetMatrix(D3DMATRIXHANDLE lpD3DMatHandle, LPD3DMATRIX lpD3DMatrix)
{
	if (ProxyDirectXVersion != 1)
	{
		Logging::Log() << __FUNCTION__ << " Not Implimented";
		return E_NOTIMPL;
	}

	return ((IDirect3DDevice*)ProxyInterface)->GetMatrix(lpD3DMatHandle, lpD3DMatrix);
}

HRESULT m_IDirect3DDeviceX::DeleteMatrix(D3DMATRIXHANDLE d3dMatHandle)
{
	if (ProxyDirectXVersion != 1)
	{
		Logging::Log() << __FUNCTION__ << " Not Implimented";
		return E_NOTIMPL;
	}

	return ((IDirect3DDevice*)ProxyInterface)->DeleteMatrix(d3dMatHandle);
}

HRESULT m_IDirect3DDeviceX::SwapTextureHandles(LPDIRECT3DTEXTURE2 lpD3DTex1, LPDIRECT3DTEXTURE2 lpD3DTex2)
{
	if (ProxyDirectXVersion > 2)
	{
		Logging::Log() << __FUNCTION__ << " Not Implimented";
		return E_NOTIMPL;
	}

	if (lpD3DTex1)
	{
		lpD3DTex1 = static_cast<m_IDirect3DTexture2 *>(lpD3DTex1)->GetProxyInterface();
	}
	if (lpD3DTex2)
	{
		lpD3DTex2 = static_cast<m_IDirect3DTexture2 *>(lpD3DTex2)->GetProxyInterface();
	}

	return ((IDirect3DDevice2*)ProxyInterface)->SwapTextureHandles(lpD3DTex1, lpD3DTex2);
}

HRESULT m_IDirect3DDeviceX::EnumTextureFormats(LPD3DENUMTEXTUREFORMATSCALLBACK lpd3dEnumTextureProc, LPVOID lpArg)
{
	if (ProxyDirectXVersion > 2)
	{
		Logging::Log() << __FUNCTION__ << " Not Implimented";
		return E_NOTIMPL;
	}

	return ((IDirect3DDevice2*)ProxyInterface)->EnumTextureFormats(lpd3dEnumTextureProc, lpArg);
}

HRESULT m_IDirect3DDeviceX::GetCaps(LPD3DDEVICEDESC lpD3DHWDevDesc, LPD3DDEVICEDESC lpD3DHELDevDesc)
{
	if (ProxyDirectXVersion == 7)
	{
		D3DDEVICEDESC7 D3DDevDesc;
		HRESULT hr = GetCaps(&D3DDevDesc);

		if (SUCCEEDED(hr))
		{
			if (lpD3DHWDevDesc)
			{
				ConvertDeviceDesc(*lpD3DHWDevDesc, D3DDevDesc);
			}

			if (lpD3DHELDevDesc)
			{
				ConvertDeviceDesc(*lpD3DHELDevDesc, D3DDevDesc);
			}
		}

		return hr;
	}

	return ((IDirect3DDevice3*)ProxyInterface)->GetCaps(lpD3DHWDevDesc, lpD3DHELDevDesc);
}

HRESULT m_IDirect3DDeviceX::GetStats(LPD3DSTATS lpD3DStats)
{
	if (ProxyDirectXVersion == 7)
	{
		Logging::Log() << __FUNCTION__ << " Not Implimented";
		return E_NOTIMPL;
	}

	return ((IDirect3DDevice3*)ProxyInterface)->GetStats(lpD3DStats);
}

HRESULT m_IDirect3DDeviceX::AddViewport(LPDIRECT3DVIEWPORT3 lpDirect3DViewport)
{
	if (ProxyDirectXVersion == 7)
	{
		return D3D_OK;
	}

	if (lpDirect3DViewport)
	{
		lpDirect3DViewport = static_cast<m_IDirect3DViewport3 *>(lpDirect3DViewport)->GetProxyInterface();
	}

	return ((IDirect3DDevice3*)ProxyInterface)->AddViewport(lpDirect3DViewport);
}

HRESULT m_IDirect3DDeviceX::DeleteViewport(LPDIRECT3DVIEWPORT3 lpDirect3DViewport)
{
	if (ProxyDirectXVersion == 7)
	{
		return D3D_OK;
	}

	if (lpDirect3DViewport)
	{
		lpDirect3DViewport = static_cast<m_IDirect3DViewport3 *>(lpDirect3DViewport)->GetProxyInterface();
	}

	return ((IDirect3DDevice3*)ProxyInterface)->DeleteViewport(lpDirect3DViewport);
}

HRESULT m_IDirect3DDeviceX::NextViewport(LPDIRECT3DVIEWPORT3 lpDirect3DViewport, LPDIRECT3DVIEWPORT3 * lplpDirect3DViewport, DWORD dwFlags)
{
	if (ProxyDirectXVersion == 7)
	{
		Logging::Log() << __FUNCTION__ << " Not Implimented";
		return E_NOTIMPL;
	}

	if (lpDirect3DViewport)
	{
		lpDirect3DViewport = static_cast<m_IDirect3DViewport3 *>(lpDirect3DViewport)->GetProxyInterface();
	}

	HRESULT hr = ((IDirect3DDevice3*)ProxyInterface)->NextViewport(lpDirect3DViewport, lplpDirect3DViewport, dwFlags);

	if (SUCCEEDED(hr) && lplpDirect3DViewport)
	{
		*lplpDirect3DViewport = ProxyAddressLookupTable.FindAddress<m_IDirect3DViewport3>(*lplpDirect3DViewport, DirectXVersion);
	}

	return hr;
}

HRESULT m_IDirect3DDeviceX::SetCurrentViewport(LPDIRECT3DVIEWPORT3 lpd3dViewport)
{
	if (ProxyDirectXVersion == 7)
	{
		// ToDo: Validate Viewport address
		lpCurrentViewport = (m_IDirect3DViewportX*)lpd3dViewport;

		return D3D_OK;
	}

	if (lpd3dViewport)
	{
		lpd3dViewport = static_cast<m_IDirect3DViewport3 *>(lpd3dViewport)->GetProxyInterface();
	}

	return ((IDirect3DDevice3*)ProxyInterface)->SetCurrentViewport(lpd3dViewport);
}

HRESULT m_IDirect3DDeviceX::GetCurrentViewport(LPDIRECT3DVIEWPORT3 * lplpd3dViewport)
{
	if (ProxyDirectXVersion == 7)
	{
		if (lplpd3dViewport && lpCurrentViewport)
		{
			// ToDo: Validate current Viewport address
			*lplpd3dViewport = lpCurrentViewport;
			return D3D_OK;
		}
		else if (!lpCurrentViewport)
		{
			return D3DERR_NOCURRENTVIEWPORT;
		}

		return DDERR_INVALIDPARAMS;
	}

	HRESULT hr = ((IDirect3DDevice3*)ProxyInterface)->GetCurrentViewport(lplpd3dViewport);

	if (SUCCEEDED(hr) && lplpd3dViewport)
	{
		*lplpd3dViewport = ProxyAddressLookupTable.FindAddress<m_IDirect3DViewport3>(*lplpd3dViewport, DirectXVersion);
	}

	return hr;
}

HRESULT m_IDirect3DDeviceX::Begin(D3DPRIMITIVETYPE d3dpt, DWORD d3dvt, DWORD dwFlags)
{
	if (ProxyDirectXVersion == 7)
	{
		Logging::Log() << __FUNCTION__ << " Not Implimented";
		return E_NOTIMPL;
	}

	return ((IDirect3DDevice3*)ProxyInterface)->Begin(d3dpt, d3dvt, dwFlags);
}

HRESULT m_IDirect3DDeviceX::BeginIndexed(D3DPRIMITIVETYPE dptPrimitiveType, DWORD dvtVertexType, LPVOID lpvVertices, DWORD dwNumVertices, DWORD dwFlags)
{
	if (ProxyDirectXVersion == 7)
	{
		Logging::Log() << __FUNCTION__ << " Not Implimented";
		return E_NOTIMPL;
	}

	return ((IDirect3DDevice3*)ProxyInterface)->BeginIndexed(dptPrimitiveType, dvtVertexType, lpvVertices, dwNumVertices, dwFlags);
}

HRESULT m_IDirect3DDeviceX::Vertex(LPVOID lpVertexType)
{
	if (ProxyDirectXVersion == 7)
	{
		Logging::Log() << __FUNCTION__ << " Not Implimented";
		return E_NOTIMPL;
	}

	return ((IDirect3DDevice3*)ProxyInterface)->Vertex(lpVertexType);
}

HRESULT m_IDirect3DDeviceX::Index(WORD wVertexIndex)
{
	if (ProxyDirectXVersion == 7)
	{
		Logging::Log() << __FUNCTION__ << " Not Implimented";
		return E_NOTIMPL;
	}

	return ((IDirect3DDevice3*)ProxyInterface)->Index(wVertexIndex);
}

HRESULT m_IDirect3DDeviceX::End(DWORD dwFlags)
{
	if (ProxyDirectXVersion == 7)
	{
		Logging::Log() << __FUNCTION__ << " Not Implimented";
		return E_NOTIMPL;
	}

	return ((IDirect3DDevice3*)ProxyInterface)->End(dwFlags);
}

HRESULT m_IDirect3DDeviceX::GetLightState(D3DLIGHTSTATETYPE dwLightStateType, LPDWORD lpdwLightState)
{
	if (ProxyDirectXVersion == 7)
	{
		Logging::Log() << __FUNCTION__ << " Not Implimented";
		return E_NOTIMPL;
	}

	return ((IDirect3DDevice3*)ProxyInterface)->GetLightState(dwLightStateType, lpdwLightState);
}

HRESULT m_IDirect3DDeviceX::SetLightState(D3DLIGHTSTATETYPE dwLightStateType, DWORD dwLightState)
{
	if (ProxyDirectXVersion == 7)
	{
		Logging::Log() << __FUNCTION__ << " Not Implimented";
		return E_NOTIMPL;
	}

	return ((IDirect3DDevice3*)ProxyInterface)->SetLightState(dwLightStateType, dwLightState);
}

HRESULT m_IDirect3DDeviceX::DrawIndexedPrimitiveVB(D3DPRIMITIVETYPE d3dptPrimitiveType, LPDIRECT3DVERTEXBUFFER lpd3dVertexBuffer, LPWORD lpwIndices, DWORD dwIndexCount, DWORD dwFlags)
{
	if (ProxyDirectXVersion == 7)
	{
		Logging::Log() << __FUNCTION__ << " Not Implimented";
		return E_NOTIMPL;
	}

	if (lpd3dVertexBuffer)
	{
		lpd3dVertexBuffer = static_cast<m_IDirect3DVertexBuffer *>(lpd3dVertexBuffer)->GetProxyInterface();
	}

	return ((IDirect3DDevice3*)ProxyInterface)->DrawIndexedPrimitiveVB(d3dptPrimitiveType, lpd3dVertexBuffer, lpwIndices, dwIndexCount, dwFlags);
}

HRESULT m_IDirect3DDeviceX::GetTexture(DWORD dwStage, LPDIRECT3DTEXTURE2 * lplpTexture)
{
	if (ProxyDirectXVersion == 7)
	{
		Logging::Log() << __FUNCTION__ << " Not Implimented";
		return E_NOTIMPL;
	}

	HRESULT hr = ((IDirect3DDevice3*)ProxyInterface)->GetTexture(dwStage, lplpTexture);

	if (SUCCEEDED(hr) && lplpTexture)
	{
		*lplpTexture = ProxyAddressLookupTable.FindAddress<m_IDirect3DTexture2>(*lplpTexture, DirectXVersion);
	}

	return hr;
}

HRESULT m_IDirect3DDeviceX::SetTexture(DWORD dwStage, LPDIRECT3DTEXTURE2 lpTexture)
{
	if (ProxyDirectXVersion == 7)
	{
		// ToDo: Validate Texture address
		if (lpTexture)
		{
			lpTexture = (((m_IDirect3DTextureX*)lpTexture)->GetProxyInterface());
		}

		return ProxyInterface->SetTexture(dwStage, (LPDIRECTDRAWSURFACE7)lpTexture);
	}

	if (lpTexture)
	{
		lpTexture = static_cast<m_IDirect3DTexture2 *>(lpTexture)->GetProxyInterface();
	}

	return ((IDirect3DDevice3*)ProxyInterface)->SetTexture(dwStage, lpTexture);
}

HRESULT m_IDirect3DDeviceX::GetCaps(LPD3DDEVICEDESC7 lpD3DDevDesc)
{
	return ProxyInterface->GetCaps(lpD3DDevDesc);
}

HRESULT m_IDirect3DDeviceX::EnumTextureFormats(LPD3DENUMPIXELFORMATSCALLBACK lpd3dEnumPixelProc, LPVOID lpArg)
{
	return ProxyInterface->EnumTextureFormats(lpd3dEnumPixelProc, lpArg);
}

HRESULT m_IDirect3DDeviceX::BeginScene()
{
	return ProxyInterface->BeginScene();
}

HRESULT m_IDirect3DDeviceX::EndScene()
{
	return ProxyInterface->EndScene();
}

HRESULT m_IDirect3DDeviceX::GetDirect3D(LPDIRECT3D7 * lplpD3D)
{
	HRESULT hr = ProxyInterface->GetDirect3D(lplpD3D);

	if (SUCCEEDED(hr) && lplpD3D)
	{
		*lplpD3D = ProxyAddressLookupTable.FindAddress<m_IDirect3D7>(*lplpD3D, DirectXVersion);
	}

	return hr;
}

HRESULT m_IDirect3DDeviceX::SetRenderTarget(LPDIRECTDRAWSURFACE7 lpNewRenderTarget, DWORD dwFlags)
{
	if (lpNewRenderTarget)
	{
		lpNewRenderTarget = static_cast<m_IDirectDrawSurface7 *>(lpNewRenderTarget)->GetProxyInterface();
	}

	return ProxyInterface->SetRenderTarget(lpNewRenderTarget, dwFlags);
}

HRESULT m_IDirect3DDeviceX::GetRenderTarget(LPDIRECTDRAWSURFACE7 * lplpRenderTarget)
{
	HRESULT hr = ProxyInterface->GetRenderTarget(lplpRenderTarget);

	if (SUCCEEDED(hr) && lplpRenderTarget)
	{
		*lplpRenderTarget = ProxyAddressLookupTable.FindAddress<m_IDirectDrawSurface7>(*lplpRenderTarget, DirectXVersion);
	}

	return hr;
}

HRESULT m_IDirect3DDeviceX::Clear(DWORD dwCount, LPD3DRECT lpRects, DWORD dwFlags, D3DCOLOR dwColor, D3DVALUE dvZ, DWORD dwStencil)
{
	return ProxyInterface->Clear(dwCount, lpRects, dwFlags, dwColor, dvZ, dwStencil);
}

HRESULT m_IDirect3DDeviceX::SetTransform(D3DTRANSFORMSTATETYPE dtstTransformStateType, LPD3DMATRIX lpD3DMatrix)
{
	return ProxyInterface->SetTransform(dtstTransformStateType, lpD3DMatrix);
}

HRESULT m_IDirect3DDeviceX::GetTransform(D3DTRANSFORMSTATETYPE dtstTransformStateType, LPD3DMATRIX lpD3DMatrix)
{
	return ProxyInterface->GetTransform(dtstTransformStateType, lpD3DMatrix);
}

HRESULT m_IDirect3DDeviceX::SetViewport(LPD3DVIEWPORT7 lpViewport)
{
	return ProxyInterface->SetViewport(lpViewport);
}

HRESULT m_IDirect3DDeviceX::MultiplyTransform(D3DTRANSFORMSTATETYPE dtstTransformStateType, LPD3DMATRIX lpD3DMatrix)
{
	return ProxyInterface->MultiplyTransform(dtstTransformStateType, lpD3DMatrix);
}

HRESULT m_IDirect3DDeviceX::GetViewport(LPD3DVIEWPORT7 lpViewport)
{
	return ProxyInterface->GetViewport(lpViewport);
}

HRESULT m_IDirect3DDeviceX::SetMaterial(LPD3DMATERIAL7 lpMaterial)
{
	return ProxyInterface->SetMaterial(lpMaterial);
}

HRESULT m_IDirect3DDeviceX::GetMaterial(LPD3DMATERIAL7 lpMaterial)
{
	return ProxyInterface->GetMaterial(lpMaterial);
}

HRESULT m_IDirect3DDeviceX::SetLight(DWORD dwLightIndex, LPD3DLIGHT7 lpLight)
{
	return ProxyInterface->SetLight(dwLightIndex, lpLight);
}

HRESULT m_IDirect3DDeviceX::GetLight(DWORD dwLightIndex, LPD3DLIGHT7 lpLight)
{
	return ProxyInterface->GetLight(dwLightIndex, lpLight);
}

HRESULT m_IDirect3DDeviceX::SetRenderState(D3DRENDERSTATETYPE dwRendStateType, DWORD dwRenderState)
{
	return ProxyInterface->SetRenderState(dwRendStateType, dwRenderState);
}

HRESULT m_IDirect3DDeviceX::GetRenderState(D3DRENDERSTATETYPE dwRenderStateType, LPDWORD lpdwRenderState)
{
	return ProxyInterface->GetRenderState(dwRenderStateType, lpdwRenderState);
}

HRESULT m_IDirect3DDeviceX::BeginStateBlock()
{
	return ProxyInterface->BeginStateBlock();
}

HRESULT m_IDirect3DDeviceX::EndStateBlock(LPDWORD lpdwBlockHandle)
{
	return ProxyInterface->EndStateBlock(lpdwBlockHandle);
}

HRESULT m_IDirect3DDeviceX::PreLoad(LPDIRECTDRAWSURFACE7 lpddsTexture)
{
	if (lpddsTexture)
	{
		lpddsTexture = static_cast<m_IDirectDrawSurface7 *>(lpddsTexture)->GetProxyInterface();
	}

	return ProxyInterface->PreLoad(lpddsTexture);
}

HRESULT m_IDirect3DDeviceX::DrawPrimitive(D3DPRIMITIVETYPE dptPrimitiveType, DWORD dwVertexTypeDesc, LPVOID lpVertices, DWORD dwVertexCount, DWORD dwFlags)
{
	return ProxyInterface->DrawPrimitive(dptPrimitiveType, dwVertexTypeDesc, lpVertices, dwVertexCount, dwFlags);
}

HRESULT m_IDirect3DDeviceX::DrawIndexedPrimitive(D3DPRIMITIVETYPE d3dptPrimitiveType, DWORD dwVertexTypeDesc, LPVOID lpvVertices, DWORD dwVertexCount, LPWORD lpwIndices, DWORD dwIndexCount, DWORD dwFlags)
{
	return ProxyInterface->DrawIndexedPrimitive(d3dptPrimitiveType, dwVertexTypeDesc, lpvVertices, dwVertexCount, lpwIndices, dwIndexCount, dwFlags);
}

HRESULT m_IDirect3DDeviceX::SetClipStatus(LPD3DCLIPSTATUS lpD3DClipStatus)
{
	return ProxyInterface->SetClipStatus(lpD3DClipStatus);
}

HRESULT m_IDirect3DDeviceX::GetClipStatus(LPD3DCLIPSTATUS lpD3DClipStatus)
{
	return ProxyInterface->GetClipStatus(lpD3DClipStatus);
}

HRESULT m_IDirect3DDeviceX::DrawPrimitiveStrided(D3DPRIMITIVETYPE dptPrimitiveType, DWORD dwVertexTypeDesc, LPD3DDRAWPRIMITIVESTRIDEDDATA lpVertexArray, DWORD dwVertexCount, DWORD dwFlags)
{
	return ProxyInterface->DrawPrimitiveStrided(dptPrimitiveType, dwVertexTypeDesc, lpVertexArray, dwVertexCount, dwFlags);
}

HRESULT m_IDirect3DDeviceX::DrawIndexedPrimitiveStrided(D3DPRIMITIVETYPE d3dptPrimitiveType, DWORD dwVertexTypeDesc, LPD3DDRAWPRIMITIVESTRIDEDDATA lpVertexArray, DWORD dwVertexCount, LPWORD lpwIndices, DWORD dwIndexCount, DWORD dwFlags)
{
	return ProxyInterface->DrawIndexedPrimitiveStrided(d3dptPrimitiveType, dwVertexTypeDesc, lpVertexArray, dwVertexCount, lpwIndices, dwIndexCount, dwFlags);
}

HRESULT m_IDirect3DDeviceX::DrawPrimitiveVB(D3DPRIMITIVETYPE d3dptPrimitiveType, LPDIRECT3DVERTEXBUFFER7 lpd3dVertexBuffer, DWORD dwStartVertex, DWORD dwNumVertices, DWORD dwFlags)
{
	if (lpd3dVertexBuffer)
	{
		lpd3dVertexBuffer = static_cast<m_IDirect3DVertexBuffer7 *>(lpd3dVertexBuffer)->GetProxyInterface();
	}

	return ProxyInterface->DrawPrimitiveVB(d3dptPrimitiveType, lpd3dVertexBuffer, dwStartVertex, dwNumVertices, dwFlags);
}

HRESULT m_IDirect3DDeviceX::DrawIndexedPrimitiveVB(D3DPRIMITIVETYPE d3dptPrimitiveType, LPDIRECT3DVERTEXBUFFER7 lpd3dVertexBuffer, DWORD dwStartVertex, DWORD dwNumVertices, LPWORD lpwIndices, DWORD dwIndexCount, DWORD dwFlags)
{
	if (lpd3dVertexBuffer)
	{
		lpd3dVertexBuffer = static_cast<m_IDirect3DVertexBuffer7 *>(lpd3dVertexBuffer)->GetProxyInterface();
	}

	return ProxyInterface->DrawIndexedPrimitiveVB(d3dptPrimitiveType, lpd3dVertexBuffer, dwStartVertex, dwNumVertices, lpwIndices, dwIndexCount, dwFlags);
}

HRESULT m_IDirect3DDeviceX::ComputeSphereVisibility(LPD3DVECTOR lpCenters, LPD3DVALUE lpRadii, DWORD dwNumSpheres, DWORD dwFlags, LPDWORD lpdwReturnValues)
{
	return ProxyInterface->ComputeSphereVisibility(lpCenters, lpRadii, dwNumSpheres, dwFlags, lpdwReturnValues);
}

HRESULT m_IDirect3DDeviceX::GetTexture(DWORD dwStage, LPDIRECTDRAWSURFACE7 * lplpTexture)
{
	HRESULT hr = ProxyInterface->GetTexture(dwStage, lplpTexture);

	if (SUCCEEDED(hr) && lplpTexture)
	{
		*lplpTexture = ProxyAddressLookupTable.FindAddress<m_IDirectDrawSurface7>(*lplpTexture, DirectXVersion);
	}

	return hr;
}

HRESULT m_IDirect3DDeviceX::SetTexture(DWORD dwStage, LPDIRECTDRAWSURFACE7 lpTexture)
{
	if (lpTexture)
	{
		lpTexture = static_cast<m_IDirectDrawSurface7 *>(lpTexture)->GetProxyInterface();
	}

	return ProxyInterface->SetTexture(dwStage, lpTexture);
}

HRESULT m_IDirect3DDeviceX::GetTextureStageState(DWORD dwStage, D3DTEXTURESTAGESTATETYPE dwState, LPDWORD lpdwValue)
{
	return ProxyInterface->GetTextureStageState(dwStage, dwState, lpdwValue);
}

HRESULT m_IDirect3DDeviceX::SetTextureStageState(DWORD dwStage, D3DTEXTURESTAGESTATETYPE dwState, DWORD dwValue)
{
	return ProxyInterface->SetTextureStageState(dwStage, dwState, dwValue);
}

HRESULT m_IDirect3DDeviceX::ValidateDevice(LPDWORD lpdwPasses)
{
	return ProxyInterface->ValidateDevice(lpdwPasses);
}

HRESULT m_IDirect3DDeviceX::ApplyStateBlock(DWORD dwBlockHandle)
{
	return ProxyInterface->ApplyStateBlock(dwBlockHandle);
}

HRESULT m_IDirect3DDeviceX::CaptureStateBlock(DWORD dwBlockHandle)
{
	return ProxyInterface->CaptureStateBlock(dwBlockHandle);
}

HRESULT m_IDirect3DDeviceX::DeleteStateBlock(DWORD dwBlockHandle)
{
	return ProxyInterface->DeleteStateBlock(dwBlockHandle);
}

HRESULT m_IDirect3DDeviceX::CreateStateBlock(D3DSTATEBLOCKTYPE d3dsbtype, LPDWORD lpdwBlockHandle)
{
	return ProxyInterface->CreateStateBlock(d3dsbtype, lpdwBlockHandle);
}

HRESULT m_IDirect3DDeviceX::Load(LPDIRECTDRAWSURFACE7 lpDestTex, LPPOINT lpDestPoint, LPDIRECTDRAWSURFACE7 lpSrcTex, LPRECT lprcSrcRect, DWORD dwFlags)
{
	if (lpDestTex)
	{
		lpDestTex = static_cast<m_IDirectDrawSurface7 *>(lpDestTex)->GetProxyInterface();
	}
	if (lpSrcTex)
	{
		lpSrcTex = static_cast<m_IDirectDrawSurface7 *>(lpSrcTex)->GetProxyInterface();
	}

	return ProxyInterface->Load(lpDestTex, lpDestPoint, lpSrcTex, lprcSrcRect, dwFlags);
}

HRESULT m_IDirect3DDeviceX::LightEnable(DWORD dwLightIndex, BOOL bEnable)
{
	return ProxyInterface->LightEnable(dwLightIndex, bEnable);
}

HRESULT m_IDirect3DDeviceX::GetLightEnable(DWORD dwLightIndex, BOOL * pbEnable)
{
	return ProxyInterface->GetLightEnable(dwLightIndex, pbEnable);
}

HRESULT m_IDirect3DDeviceX::SetClipPlane(DWORD dwIndex, D3DVALUE * pPlaneEquation)
{
	return ProxyInterface->SetClipPlane(dwIndex, pPlaneEquation);
}

HRESULT m_IDirect3DDeviceX::GetClipPlane(DWORD dwIndex, D3DVALUE * pPlaneEquation)
{
	return ProxyInterface->GetClipPlane(dwIndex, pPlaneEquation);
}

HRESULT m_IDirect3DDeviceX::GetInfo(DWORD dwDevInfoID, LPVOID pDevInfoStruct, DWORD dwSize)
{
	return ProxyInterface->GetInfo(dwDevInfoID, pDevInfoStruct, dwSize);
}
