/**
* Copyright (C) 2023 Elisha Riedlinger
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
#include <d3dhal.h>

// Enable for testing only
//#define ENABLE_DEBUGOVERLAY

#ifdef ENABLE_DEBUGOVERLAY
#include "DebugOverlay.h"
DebugOverlay DOverlay;
#endif

extern float ScaleDDWidthRatio;
extern float ScaleDDHeightRatio;
extern DWORD ScaleDDPadX;
extern DWORD ScaleDDPadY;

HRESULT m_IDirect3DDeviceX::QueryInterface(REFIID riid, LPVOID FAR * ppvObj, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ") " << riid;

	if (!ppvObj)
	{
		return E_POINTER;
	}

	if (riid == IID_GetRealInterface)
	{
		*ppvObj = ProxyInterface;
		return DD_OK;
	}
	if (riid == IID_GetInterfaceX)
	{
		*ppvObj = this;
		return DD_OK;
	}

	if (DirectXVersion != 1 && DirectXVersion != 2 && DirectXVersion != 3 && DirectXVersion != 7)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: wrapper interface version not found: " << DirectXVersion);
		return DDERR_GENERIC;
	}

	DWORD DxVersion = (CheckWrapperType(riid) && (Config.Dd7to9 || Config.ConvertToDirect3D7)) ? GetGUIDVersion(riid) : DirectXVersion;

	if (riid == GetWrapperType(DxVersion) || riid == IID_IUnknown)
	{
		*ppvObj = GetWrapperInterfaceX(DxVersion);

		AddRef(DxVersion);

		return DD_OK;
	}

	return ProxyQueryInterface(ProxyInterface, riid, ppvObj, GetWrapperType(DxVersion));
}

void *m_IDirect3DDeviceX::GetWrapperInterfaceX(DWORD DirectXVersion)
{
	switch (DirectXVersion)
	{
	case 1:
		return WrapperInterface;
	case 2:
		return WrapperInterface2;
	case 3:
		return WrapperInterface3;
	case 7:
		return WrapperInterface7;
	default:
		LOG_LIMIT(100, __FUNCTION__ << " Error: wrapper interface version not found: " << DirectXVersion);
		return nullptr;
	}
}

ULONG m_IDirect3DDeviceX::AddRef(DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ") v" << DirectXVersion;

	if (Config.Dd7to9)
	{
		switch (DirectXVersion)
		{
		case 1:
			return InterlockedIncrement(&RefCount1);
		case 2:
			return InterlockedIncrement(&RefCount2);
		case 3:
			return InterlockedIncrement(&RefCount3);
		case 7:
			return InterlockedIncrement(&RefCount7);
		default:
			LOG_LIMIT(100, __FUNCTION__ << " Error: wrapper interface version not found: " << DirectXVersion);
			return 0;
		}
	}

	return ProxyInterface->AddRef();
}

ULONG m_IDirect3DDeviceX::Release(DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ") v" << DirectXVersion;

	ULONG ref;

	if (Config.Dd7to9)
	{
		switch (DirectXVersion)
		{
		case 1:
			ref = (InterlockedCompareExchange(&RefCount1, 0, 0)) ? InterlockedDecrement(&RefCount1) : 0;
			break;
		case 2:
			ref = (InterlockedCompareExchange(&RefCount2, 0, 0)) ? InterlockedDecrement(&RefCount2) : 0;
			break;
		case 3:
			ref = (InterlockedCompareExchange(&RefCount3, 0, 0)) ? InterlockedDecrement(&RefCount3) : 0;
			break;
		case 7:
			ref = (InterlockedCompareExchange(&RefCount7, 0, 0)) ? InterlockedDecrement(&RefCount7) : 0;
			break;
		default:
			LOG_LIMIT(100, __FUNCTION__ << " Error: wrapper interface version not found: " << DirectXVersion);
			ref = 0;
		}

		if (InterlockedCompareExchange(&RefCount1, 0, 0) + InterlockedCompareExchange(&RefCount2, 0, 0) +
			InterlockedCompareExchange(&RefCount3, 0, 0) + InterlockedCompareExchange(&RefCount7, 0, 0) == 0)
		{
			delete this;
		}
	}
	else
	{
		ref = ProxyInterface->Release();

		if (ref == 0)
		{
			delete this;
		}
	}

	return ref;
}

HRESULT m_IDirect3DDeviceX::Initialize(LPDIRECT3D lpd3d, LPGUID lpGUID, LPD3DDEVICEDESC lpd3ddvdesc)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (ProxyDirectXVersion != 1)
	{
		return D3D_OK;
	}

	if (lpd3d)
	{
		lpd3d->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpd3d);
	}

	return GetProxyInterfaceV1()->Initialize(lpd3d, lpGUID, lpd3ddvdesc);
}

HRESULT m_IDirect3DDeviceX::CreateExecuteBuffer(LPD3DEXECUTEBUFFERDESC lpDesc, LPDIRECT3DEXECUTEBUFFER * lplpDirect3DExecuteBuffer, IUnknown * pUnkOuter)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (ProxyDirectXVersion != 1)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	HRESULT hr = GetProxyInterfaceV1()->CreateExecuteBuffer(lpDesc, lplpDirect3DExecuteBuffer, pUnkOuter);

	if (SUCCEEDED(hr) && lplpDirect3DExecuteBuffer)
	{
		*lplpDirect3DExecuteBuffer = new m_IDirect3DExecuteBuffer(*lplpDirect3DExecuteBuffer);
	}

	return hr;
}

HRESULT m_IDirect3DDeviceX::Execute(LPDIRECT3DEXECUTEBUFFER lpDirect3DExecuteBuffer, LPDIRECT3DVIEWPORT lpDirect3DViewport, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (ProxyDirectXVersion != 1)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	if (lpDirect3DExecuteBuffer)
	{
		lpDirect3DExecuteBuffer->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpDirect3DExecuteBuffer);
	}
	if (lpDirect3DViewport)
	{
		lpDirect3DViewport->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpDirect3DViewport);
	}

	return GetProxyInterfaceV1()->Execute(lpDirect3DExecuteBuffer, lpDirect3DViewport, dwFlags);
}

HRESULT m_IDirect3DDeviceX::Pick(LPDIRECT3DEXECUTEBUFFER lpDirect3DExecuteBuffer, LPDIRECT3DVIEWPORT lpDirect3DViewport, DWORD dwFlags, LPD3DRECT lpRect)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (ProxyDirectXVersion != 1)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	if (lpDirect3DExecuteBuffer)
	{
		lpDirect3DExecuteBuffer->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpDirect3DExecuteBuffer);
	}
	if (lpDirect3DViewport)
	{
		lpDirect3DViewport->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpDirect3DViewport);
	}

	return GetProxyInterfaceV1()->Pick(lpDirect3DExecuteBuffer, lpDirect3DViewport, dwFlags, lpRect);
}

HRESULT m_IDirect3DDeviceX::GetPickRecords(LPDWORD lpCount, LPD3DPICKRECORD lpD3DPickRec)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (ProxyDirectXVersion != 1)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	return GetProxyInterfaceV1()->GetPickRecords(lpCount, lpD3DPickRec);
}

HRESULT m_IDirect3DDeviceX::CreateMatrix(LPD3DMATRIXHANDLE lpD3DMatHandle)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (ProxyDirectXVersion != 1)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	return GetProxyInterfaceV1()->CreateMatrix(lpD3DMatHandle);
}

HRESULT m_IDirect3DDeviceX::SetMatrix(D3DMATRIXHANDLE d3dMatHandle, const LPD3DMATRIX lpD3DMatrix)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (ProxyDirectXVersion != 1)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	return GetProxyInterfaceV1()->SetMatrix(d3dMatHandle, lpD3DMatrix);
}

HRESULT m_IDirect3DDeviceX::GetMatrix(D3DMATRIXHANDLE lpD3DMatHandle, LPD3DMATRIX lpD3DMatrix)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (ProxyDirectXVersion != 1)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	return GetProxyInterfaceV1()->GetMatrix(lpD3DMatHandle, lpD3DMatrix);
}

HRESULT m_IDirect3DDeviceX::DeleteMatrix(D3DMATRIXHANDLE d3dMatHandle)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (ProxyDirectXVersion != 1)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	return GetProxyInterfaceV1()->DeleteMatrix(d3dMatHandle);
}

HRESULT m_IDirect3DDeviceX::SetTransform(D3DTRANSFORMSTATETYPE dtstTransformStateType, LPD3DMATRIX lpD3DMatrix)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lpD3DMatrix)
		{
			return  DDERR_INVALIDPARAMS;
		}

		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true)))
		{
			return DDERR_GENERIC;
		}

		switch ((DWORD)dtstTransformStateType)
		{
		case D3DTRANSFORMSTATE_WORLD:
			dtstTransformStateType = D3DTS_WORLD;
			break;
		case D3DTRANSFORMSTATE_WORLD1:
			dtstTransformStateType = D3DTS_WORLD1;
			break;
		case D3DTRANSFORMSTATE_WORLD2:
			dtstTransformStateType = D3DTS_WORLD2;
			break;
		case D3DTRANSFORMSTATE_WORLD3:
			dtstTransformStateType = D3DTS_WORLD3;
			break;
		}

		HRESULT hr = (*d3d9Device)->SetTransform(dtstTransformStateType, lpD3DMatrix);

		if (SUCCEEDED(hr))
		{
#ifdef ENABLE_DEBUGOVERLAY
			DOverlay.SetTransform(dtstTransformStateType, lpD3DMatrix);
#endif
		}

		return hr;
	}

	switch (ProxyDirectXVersion)
	{
	case 1:
	default:
		return DDERR_GENERIC;
	case 2:
		return GetProxyInterfaceV2()->SetTransform(dtstTransformStateType, lpD3DMatrix);
	case 3:
		return GetProxyInterfaceV3()->SetTransform(dtstTransformStateType, lpD3DMatrix);
	case 7:
		return GetProxyInterfaceV7()->SetTransform(dtstTransformStateType, lpD3DMatrix);
	}
}

HRESULT m_IDirect3DDeviceX::GetTransform(D3DTRANSFORMSTATETYPE dtstTransformStateType, LPD3DMATRIX lpD3DMatrix)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lpD3DMatrix)
		{
			return  DDERR_INVALIDPARAMS;
		}

		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true)))
		{
			return DDERR_GENERIC;
		}

		switch ((DWORD)dtstTransformStateType)
		{
		case D3DTRANSFORMSTATE_WORLD:
			dtstTransformStateType = D3DTS_WORLD;
			break;
		case D3DTRANSFORMSTATE_WORLD1:
			dtstTransformStateType = D3DTS_WORLD1;
			break;
		case D3DTRANSFORMSTATE_WORLD2:
			dtstTransformStateType = D3DTS_WORLD2;
			break;
		case D3DTRANSFORMSTATE_WORLD3:
			dtstTransformStateType = D3DTS_WORLD3;
			break;
		}

		return (*d3d9Device)->GetTransform(dtstTransformStateType, lpD3DMatrix);
	}

	switch (ProxyDirectXVersion)
	{
	case 1:
	default:
		return DDERR_GENERIC;
	case 2:
		return GetProxyInterfaceV2()->GetTransform(dtstTransformStateType, lpD3DMatrix);
	case 3:
		return GetProxyInterfaceV3()->GetTransform(dtstTransformStateType, lpD3DMatrix);
	case 7:
		return GetProxyInterfaceV7()->GetTransform(dtstTransformStateType, lpD3DMatrix);
	}
}

HRESULT m_IDirect3DDeviceX::PreLoad(LPDIRECTDRAWSURFACE7 lpddsTexture)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		// ToDo: Call PreLoad for the texture.
		// Calling this method indicates that the application will need this managed resource shortly. This method has no effect on nonmanaged resources.
		return D3D_OK;
	}

	if (lpddsTexture)
	{
		lpddsTexture->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpddsTexture);
	}

	return GetProxyInterfaceV7()->PreLoad(lpddsTexture);
}

HRESULT m_IDirect3DDeviceX::Load(LPDIRECTDRAWSURFACE7 lpDestTex, LPPOINT lpDestPoint, LPDIRECTDRAWSURFACE7 lpSrcTex, LPRECT lprcSrcRect, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lpDestTex || !lpSrcTex)
		{
			return  DDERR_INVALIDPARAMS;
		}

		// ToDo: support the following dwFlags: 
		// DDSCAPS2_CUBEMAP_ALLFACES - All faces should be loaded with the image data within the source texture.
		// DDSCAPS2_CUBEMAP_NEGATIVEX, DDSCAPS2_CUBEMAP_NEGATIVEY, or DDSCAPS2_CUBEMAP_NEGATIVEZ
		//     The negative x, y, or z faces should receive the image data.
		// DDSCAPS2_CUBEMAP_POSITIVEX, DDSCAPS2_CUBEMAP_POSITIVEY, or DDSCAPS2_CUBEMAP_POSITIVEZ
		//     The positive x, y, or z faces should receive the image data.

		if (dwFlags)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: flags not supported. dwFlags: " << Logging::hex(dwFlags));
		}

		// ToDo: check if source and destination surfaces are valid

		if (!lprcSrcRect && (!lpDestPoint || (lpDestPoint && lpDestPoint->x == 0 && lpDestPoint->y == 0)))
		{
			return lpDestTex->Blt(nullptr, lpSrcTex, nullptr, 0, nullptr);
		}
		else
		{
			// Get source rect
			RECT SrcRect = {};
			if (lprcSrcRect)
			{
				SrcRect = *lprcSrcRect;
			}
			else
			{
				DDSURFACEDESC2 Desc2 = {};
				Desc2.dwSize = sizeof(DDSURFACEDESC2);
				lpSrcTex->GetSurfaceDesc(&Desc2);

				if ((Desc2.dwFlags & (DDSD_WIDTH | DDSD_HEIGHT)) != (DDSD_WIDTH | DDSD_HEIGHT))
				{
					return DDERR_GENERIC;
				}

				SrcRect = { 0, 0, (LONG)Desc2.dwWidth, (LONG)Desc2.dwHeight };
			}

			// Get destination point
			POINT DestPoint = {};
			if (lpDestPoint)
			{
				DestPoint = *lpDestPoint;
			}

			// Get destination rect
			RECT DestRect = {
				DestPoint.x,									// left
				DestPoint.y,									// top
				DestPoint.x + (SrcRect.right - SrcRect.left),	// right
				DestPoint.y + (SrcRect.bottom - SrcRect.top),	// bottom
			};

			return lpDestTex->Blt(&DestRect, lpSrcTex, &SrcRect, 0, nullptr);
		}
	}

	if (lpDestTex)
	{
		lpDestTex->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpDestTex);
	}
	if (lpSrcTex)
	{
		lpSrcTex->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpSrcTex);
	}

	return GetProxyInterfaceV7()->Load(lpDestTex, lpDestPoint, lpSrcTex, lprcSrcRect, dwFlags);
}

HRESULT m_IDirect3DDeviceX::SwapTextureHandles(LPDIRECT3DTEXTURE2 lpD3DTex1, LPDIRECT3DTEXTURE2 lpD3DTex2)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (ProxyDirectXVersion > 2)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	if (lpD3DTex1)
	{
		lpD3DTex1->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpD3DTex1);
	}
	if (lpD3DTex2)
	{
		lpD3DTex2->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpD3DTex2);
	}

	switch (ProxyDirectXVersion)
	{
	case 1:
		return GetProxyInterfaceV1()->SwapTextureHandles((LPDIRECT3DTEXTURE)lpD3DTex1, (LPDIRECT3DTEXTURE)lpD3DTex2);
	case 2:
		return GetProxyInterfaceV2()->SwapTextureHandles(lpD3DTex1, lpD3DTex2);
	default:
		return DDERR_GENERIC;
	}
}

HRESULT m_IDirect3DDeviceX::EnumTextureFormats(LPD3DENUMTEXTUREFORMATSCALLBACK lpd3dEnumTextureProc, LPVOID lpArg)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	switch (ProxyDirectXVersion)
	{
	case 1:
		return GetProxyInterfaceV1()->EnumTextureFormats(lpd3dEnumTextureProc, lpArg);
	case 2:
		return GetProxyInterfaceV2()->EnumTextureFormats(lpd3dEnumTextureProc, lpArg);
	case 3:
	case 7:
	case 9:
	{
		if (!lpd3dEnumTextureProc)
		{
			return DDERR_INVALIDPARAMS;
		}

		struct EnumPixelFormat
		{
			LPVOID lpContext;
			LPD3DENUMTEXTUREFORMATSCALLBACK lpCallback;

			static HRESULT CALLBACK ConvertCallback(LPDDPIXELFORMAT lpDDPixFmt, LPVOID lpContext)
			{
				EnumPixelFormat *self = (EnumPixelFormat*)lpContext;

				DDSURFACEDESC Desc = {};
				Desc.dwSize = sizeof(DDSURFACEDESC);
				Desc.dwFlags = DDSD_PIXELFORMAT;
				Desc.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
				ConvertPixelFormat(Desc.ddpfPixelFormat, *lpDDPixFmt);

				return self->lpCallback(&Desc, self->lpContext);
			}
		} CallbackContext;
		CallbackContext.lpContext = lpArg;
		CallbackContext.lpCallback = lpd3dEnumTextureProc;

		return EnumTextureFormats(EnumPixelFormat::ConvertCallback, &CallbackContext);
	}
	default:
		return DDERR_GENERIC;
	}
}

HRESULT m_IDirect3DDeviceX::EnumTextureFormats(LPD3DENUMPIXELFORMATSCALLBACK lpd3dEnumPixelProc, LPVOID lpArg)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lpd3dEnumPixelProc)
		{
			return DDERR_GENERIC;
		}

		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, false)))
		{
			return DDERR_GENERIC;
		}

		LPDIRECT3D9 d3d9Object = ddrawParent->GetDirect3D9Object();

		if (!d3d9Object)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: failed to get d3d9 object!");
			return DDERR_GENERIC;
		}

		DDPIXELFORMAT ddpfPixelFormat;
		ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);

		for (D3DFORMAT format : {
			D3DFMT_X1R5G5B5,
			D3DFMT_A1R5G5B5,
			D3DFMT_A4R4G4B4,
			D3DFMT_R5G6B5,
			D3DFMT_X8R8G8B8,
			D3DFMT_A8R8G8B8,
			D3DFMT_V8U8,
			D3DFMT_X8L8V8U8,
			D3DFMT_DXT1,
			D3DFMT_DXT2,
			D3DFMT_DXT3,
			D3DFMT_DXT4,
			D3DFMT_DXT5,
			(D3DFORMAT)MAKEFOURCC('N', 'V', 'C', 'S'),
			(D3DFORMAT)MAKEFOURCC('N', 'V', 'H', 'U'),
			(D3DFORMAT)MAKEFOURCC('N', 'V', 'H', 'S'),
			D3DFMT_L8,
			D3DFMT_A8,
			D3DFMT_A8L8,
			(D3DFORMAT)MAKEFOURCC('N', 'U', 'L', 'L'),
			(D3DFORMAT)MAKEFOURCC('A', 'T', 'I', '1'),
			(D3DFORMAT)MAKEFOURCC('A', 'T', 'I', '2'),
			(D3DFORMAT)MAKEFOURCC('I', 'N', 'T', 'Z'),
			(D3DFORMAT)MAKEFOURCC('3', 'x', '1', '1'),
			(D3DFORMAT)MAKEFOURCC('3', 'x', '1', '6')})
		{
			HRESULT hr = d3d9Object->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8, 0, D3DRTYPE_TEXTURE, format);
			if (SUCCEEDED(hr))
			{
				SetPixelDisplayFormat(format, ddpfPixelFormat);
				lpd3dEnumPixelProc(&ddpfPixelFormat, lpArg);
			}
			else
			{
				Logging::LogDebug() << __FUNCTION__ << " " << format << " " << (DDERR)hr;
			}
		}

		return DD_OK;
	}

	switch (ProxyDirectXVersion)
	{
	case 1:
	case 2:
	default:
		return DDERR_GENERIC;
	case 3:
		return GetProxyInterfaceV3()->EnumTextureFormats(lpd3dEnumPixelProc, lpArg);
	case 7:
		return GetProxyInterfaceV7()->EnumTextureFormats(lpd3dEnumPixelProc, lpArg);
	}
}

HRESULT m_IDirect3DDeviceX::GetTexture(DWORD dwStage, LPDIRECT3DTEXTURE2* lplpTexture)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (ProxyDirectXVersion > 3)
	{
		if (!lplpTexture)
		{
			return DDERR_INVALIDPARAMS;
		}

		*lplpTexture = nullptr;

		// Get surface stage
		LPDIRECTDRAWSURFACE7 pSurface = nullptr;
		HRESULT hr = GetTexture(dwStage, &pSurface);

		if (FAILED(hr))
		{
			return hr;
		}

		// First relese ref for surface
		pSurface->Release();

		// Get surface wrapper
		m_IDirectDrawSurfaceX* pSurfaceX = nullptr;
		pSurface->QueryInterface(IID_GetInterfaceX, (LPVOID*)&pSurfaceX);

		if (!pSurfaceX)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: could not get surface wrapper!");
			return DDERR_GENERIC;
		}

		// Get attached texture from surface
		m_IDirect3DTextureX* pTextureX = pSurfaceX->GetAttachedTexture();
		if (!pTextureX)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: could not get texture!");
			return DDERR_GENERIC;
		}

		// Add ref to texture
		pTextureX->AddRef();

		*lplpTexture = (LPDIRECT3DTEXTURE2)pTextureX->GetWrapperInterfaceX(2);

		return DD_OK;
	}

	HRESULT hr = GetProxyInterfaceV3()->GetTexture(dwStage, lplpTexture);

	if (SUCCEEDED(hr) && lplpTexture)
	{
		*lplpTexture = ProxyAddressLookupTable.FindAddress<m_IDirect3DTexture2>(*lplpTexture, 2);
	}

	return hr;
}

HRESULT m_IDirect3DDeviceX::GetTexture(DWORD dwStage, LPDIRECTDRAWSURFACE7* lplpTexture)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lplpTexture || dwStage > 7)
		{
			return DDERR_INVALIDPARAMS;
		}

		HRESULT hr = DDERR_INVALIDOBJECT;

		*lplpTexture = nullptr;

		if (AttachedTexture[dwStage])
		{
			if (CheckSurfaceExists(AttachedTexture[dwStage]))
			{
				AttachedTexture[dwStage]->AddRef();

				*lplpTexture = AttachedTexture[dwStage];

				hr = DD_OK;
			}
			else
			{
				AttachedTexture[dwStage] = nullptr;
			}
		}

		return hr;
	}

	HRESULT hr = GetProxyInterfaceV7()->GetTexture(dwStage, lplpTexture);

	if (SUCCEEDED(hr) && lplpTexture)
	{
		*lplpTexture = ProxyAddressLookupTable.FindAddress<m_IDirectDrawSurface7>(*lplpTexture, 7);
	}

	return hr;
}

HRESULT m_IDirect3DDeviceX::SetTexture(DWORD dwStage, LPDIRECT3DTEXTURE2 lpTexture)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (ProxyDirectXVersion > 3)
	{
		if (!lpTexture)
		{
			return SetTexture(dwStage, (LPDIRECTDRAWSURFACE7)nullptr);
		}

		m_IDirect3DTextureX *pTextureX = nullptr;
		lpTexture->QueryInterface(IID_GetInterfaceX, (LPVOID*)&pTextureX);

		if (!pTextureX)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: could not get texture wrapper!");
			return DDERR_GENERIC;
		}

		m_IDirectDrawSurfaceX *pSurfaceX = pTextureX->GetSurface();

		if (!pSurfaceX)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: could not get surface!");
			return DDERR_GENERIC;
		}

		return SetTexture(dwStage, (LPDIRECTDRAWSURFACE7)pSurfaceX->GetWrapperInterfaceX(7));
	}

	if (lpTexture)
	{
		lpTexture->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpTexture);
	}

	return GetProxyInterfaceV3()->SetTexture(dwStage, lpTexture);
}

HRESULT m_IDirect3DDeviceX::SetTexture(DWORD dwStage, LPDIRECTDRAWSURFACE7 lpSurface)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true)))
		{
			return DDERR_GENERIC;
		}

		HRESULT hr;

		if (!lpSurface)
		{
			hr = (*d3d9Device)->SetTexture(dwStage, nullptr);
		}
		else
		{
			m_IDirectDrawSurfaceX* lpDDSrcSurfaceX = nullptr;

			lpSurface->QueryInterface(IID_GetInterfaceX, (LPVOID*)&lpDDSrcSurfaceX);

			if (!lpDDSrcSurfaceX)
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: could not get surface wrapper!");
				return DDERR_GENERIC;
			}

			IDirect3DTexture9* pTexture9 = lpDDSrcSurfaceX->Get3DTexture();

			if (!pTexture9)
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: could not get texture!");
				return DDERR_GENERIC;
			}

			hr = (*d3d9Device)->SetTexture(dwStage, pTexture9);
		}

		if (SUCCEEDED(hr) && dwStage < 8)
		{
			AttachedTexture[dwStage] = lpSurface;
		}

		return hr;
	}

	if (lpSurface)
	{
		lpSurface->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpSurface);
	}

	return GetProxyInterfaceV7()->SetTexture(dwStage, lpSurface);
}

HRESULT m_IDirect3DDeviceX::SetRenderTarget(LPDIRECTDRAWSURFACE7 lpNewRenderTarget, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lpNewRenderTarget)
		{
			Logging::Log() << __FUNCTION__ " Error: nullptr";
			return DDERR_GENERIC;
		}

		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true)))
		{
			return DDERR_GENERIC;
		}

		m_IDirectDrawSurfaceX *lpDDSrcSurfaceX = nullptr;

		lpNewRenderTarget->QueryInterface(IID_GetInterfaceX, (LPVOID*)&lpDDSrcSurfaceX);

		if (!lpDDSrcSurfaceX)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: could not get surface wrapper!");
			return DDERR_GENERIC;
		}

		IDirect3DSurface9* pRenderTarget9 = lpDDSrcSurfaceX->Get3DSurface();

		if (!pRenderTarget9)
		{
			Logging::Log() << __FUNCTION__ " Error: d3d9 surface does not exist!";
			return DDERR_GENERIC;
		}

		return (*d3d9Device)->SetRenderTarget(0, pRenderTarget9);
	}

	if (lpNewRenderTarget)
	{
		lpNewRenderTarget->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpNewRenderTarget);
	}

	switch (ProxyDirectXVersion)
	{
	case 1:
	default:
		return DDERR_GENERIC;
	case 2:
		return GetProxyInterfaceV2()->SetRenderTarget((LPDIRECTDRAWSURFACE)lpNewRenderTarget, dwFlags);
	case 3:
		return GetProxyInterfaceV3()->SetRenderTarget((LPDIRECTDRAWSURFACE4)lpNewRenderTarget, dwFlags);
	case 7:
		return GetProxyInterfaceV7()->SetRenderTarget(lpNewRenderTarget, dwFlags);
	}
}

HRESULT m_IDirect3DDeviceX::GetRenderTarget(LPDIRECTDRAWSURFACE7 * lplpRenderTarget, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	HRESULT hr = DDERR_GENERIC;

	switch (ProxyDirectXVersion)
	{
	case 2:
		hr = GetProxyInterfaceV2()->GetRenderTarget((LPDIRECTDRAWSURFACE*)lplpRenderTarget);
		break;
	case 3:
		hr = GetProxyInterfaceV3()->GetRenderTarget((LPDIRECTDRAWSURFACE4*)lplpRenderTarget);
		break;
	case 7:
		hr = GetProxyInterfaceV7()->GetRenderTarget(lplpRenderTarget);
		break;
	}

	if (SUCCEEDED(hr) && lplpRenderTarget)
	{
		*lplpRenderTarget = ProxyAddressLookupTable.FindAddress<m_IDirectDrawSurface7>(*lplpRenderTarget, DirectXVersion);
	}

	return hr;
}

HRESULT m_IDirect3DDeviceX::GetTextureStageState(DWORD dwStage, D3DTEXTURESTAGESTATETYPE dwState, LPDWORD lpdwValue)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lpdwValue)
		{
			return DDERR_INVALIDPARAMS;
		}

		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true)))
		{
			return DDERR_GENERIC;
		}

		switch ((DWORD)dwState)
		{
		case D3DTSS_ADDRESS:
		{
			DWORD ValueU = 0, ValueV = 0;
			(*d3d9Device)->GetSamplerState(dwStage, D3DSAMP_ADDRESSU, &ValueU);
			(*d3d9Device)->GetSamplerState(dwStage, D3DSAMP_ADDRESSV, &ValueV);
			if (ValueU == ValueV)
			{
				*lpdwValue = ValueU;
				return D3D_OK;
			}
			else
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: AddressU and AddressV don't match");
				*lpdwValue = 0;
				return DDERR_GENERIC;
			}
		}
		case D3DTSS_ADDRESSU:
			return (*d3d9Device)->GetSamplerState(dwStage, D3DSAMP_ADDRESSU, lpdwValue);
		case D3DTSS_ADDRESSV:
			return (*d3d9Device)->GetSamplerState(dwStage, D3DSAMP_ADDRESSV, lpdwValue);
		case D3DTSS_ADDRESSW:
			return (*d3d9Device)->GetSamplerState(dwStage, D3DSAMP_ADDRESSW, lpdwValue);
		case D3DTSS_BORDERCOLOR:
			return (*d3d9Device)->GetSamplerState(dwStage, D3DSAMP_BORDERCOLOR, lpdwValue);
		case D3DTSS_MAGFILTER:
			return (*d3d9Device)->GetSamplerState(dwStage, D3DSAMP_MAGFILTER, lpdwValue);
		case D3DTSS_MINFILTER:
			return (*d3d9Device)->GetSamplerState(dwStage, D3DSAMP_MINFILTER, lpdwValue);
		case D3DTSS_MIPFILTER:
			return (*d3d9Device)->GetSamplerState(dwStage, D3DSAMP_MIPFILTER, lpdwValue);
		case D3DTSS_MIPMAPLODBIAS:
			return (*d3d9Device)->GetSamplerState(dwStage, D3DSAMP_MIPMAPLODBIAS, lpdwValue);
		case D3DTSS_MAXMIPLEVEL:
			return (*d3d9Device)->GetSamplerState(dwStage, D3DSAMP_MAXMIPLEVEL, lpdwValue);
		case D3DTSS_MAXANISOTROPY:
			return (*d3d9Device)->GetSamplerState(dwStage, D3DSAMP_MAXANISOTROPY, lpdwValue);
		}

		if (!CheckTextureStageStateType(dwState))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: Texture state type not implemented: " << dwState);
		}

		return (*d3d9Device)->GetTextureStageState(dwStage, dwState, lpdwValue);
	}

	switch (ProxyDirectXVersion)
	{
	case 1:
	case 2:
	default:
		return DDERR_GENERIC;
	case 3:
		return GetProxyInterfaceV3()->GetTextureStageState(dwStage, dwState, lpdwValue);
	case 7:
		return GetProxyInterfaceV7()->GetTextureStageState(dwStage, dwState, lpdwValue);
	}
}

HRESULT m_IDirect3DDeviceX::SetTextureStageState(DWORD dwStage, D3DTEXTURESTAGESTATETYPE dwState, DWORD dwValue)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true)))
		{
			return DDERR_GENERIC;
		}

		switch ((DWORD)dwState)
		{
		case D3DTSS_ADDRESS:
			if (SUCCEEDED((*d3d9Device)->SetSamplerState(dwStage, D3DSAMP_ADDRESSU, dwValue)) &&
				SUCCEEDED((*d3d9Device)->SetSamplerState(dwStage, D3DSAMP_ADDRESSV, dwValue)))
			{
				return D3D_OK;
			}
			else
			{
				return DDERR_GENERIC;
			}
		case D3DTSS_ADDRESSU:
			return (*d3d9Device)->SetSamplerState(dwStage, D3DSAMP_ADDRESSU, dwValue);
		case D3DTSS_ADDRESSV:
			return (*d3d9Device)->SetSamplerState(dwStage, D3DSAMP_ADDRESSV, dwValue);
		case D3DTSS_ADDRESSW:
			return (*d3d9Device)->SetSamplerState(dwStage, D3DSAMP_ADDRESSW, dwValue);
		case D3DTSS_BORDERCOLOR:
			return (*d3d9Device)->SetSamplerState(dwStage, D3DSAMP_BORDERCOLOR, dwValue);
		case D3DTSS_MAGFILTER:
			return (*d3d9Device)->SetSamplerState(dwStage, D3DSAMP_MAGFILTER, dwValue);
		case D3DTSS_MINFILTER:
			return (*d3d9Device)->SetSamplerState(dwStage, D3DSAMP_MINFILTER, dwValue);
		case D3DTSS_MIPFILTER:
			return (*d3d9Device)->SetSamplerState(dwStage, D3DSAMP_MIPFILTER, dwValue);
		case D3DTSS_MIPMAPLODBIAS:
			return (*d3d9Device)->SetSamplerState(dwStage, D3DSAMP_MIPMAPLODBIAS, dwValue);
		case D3DTSS_MAXMIPLEVEL:
			return (*d3d9Device)->SetSamplerState(dwStage, D3DSAMP_MAXMIPLEVEL, dwValue);
		case D3DTSS_MAXANISOTROPY:
			return (*d3d9Device)->SetSamplerState(dwStage, D3DSAMP_MAXANISOTROPY, dwValue);
		}

		if (!CheckTextureStageStateType(dwState))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: Texture state type not implemented: " << dwState);
		}

		return (*d3d9Device)->SetTextureStageState(dwStage, dwState, dwValue);
	}

	switch (ProxyDirectXVersion)
	{
	case 1:
	case 2:
	default:
		return DDERR_GENERIC;
	case 3:
		return GetProxyInterfaceV3()->SetTextureStageState(dwStage, dwState, dwValue);
	case 7:
		return GetProxyInterfaceV7()->SetTextureStageState(dwStage, dwState, dwValue);
	}
}

HRESULT m_IDirect3DDeviceX::GetCaps(LPD3DDEVICEDESC lpD3DHWDevDesc, LPD3DDEVICEDESC lpD3DHELDevDesc)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	switch (ProxyDirectXVersion)
	{
	case 1:
		return GetProxyInterfaceV1()->GetCaps(lpD3DHWDevDesc, lpD3DHELDevDesc);
	case 2:
		return GetProxyInterfaceV2()->GetCaps(lpD3DHWDevDesc, lpD3DHELDevDesc);
	case 3:
		return GetProxyInterfaceV3()->GetCaps(lpD3DHWDevDesc, lpD3DHELDevDesc);
	case 7:
	case 9:
	{
		if ((!lpD3DHWDevDesc && !lpD3DHELDevDesc) ||
			(lpD3DHWDevDesc && lpD3DHWDevDesc->dwSize != D3DDEVICEDESC1_SIZE && lpD3DHWDevDesc->dwSize != D3DDEVICEDESC5_SIZE && lpD3DHWDevDesc->dwSize != D3DDEVICEDESC6_SIZE) ||
			(lpD3DHELDevDesc && lpD3DHELDevDesc->dwSize != D3DDEVICEDESC1_SIZE && lpD3DHELDevDesc->dwSize != D3DDEVICEDESC5_SIZE && lpD3DHELDevDesc->dwSize != D3DDEVICEDESC6_SIZE))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Incorrect dwSize: " << ((lpD3DHWDevDesc) ? lpD3DHWDevDesc->dwSize : -1) << " " << ((lpD3DHELDevDesc) ? lpD3DHELDevDesc->dwSize : -1));
			return DDERR_INVALIDPARAMS;
		}

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
	default:
		return DDERR_GENERIC;
	}
}

HRESULT m_IDirect3DDeviceX::GetCaps(LPD3DDEVICEDESC7 lpD3DDevDesc)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lpD3DDevDesc)
		{
			return DDERR_INVALIDPARAMS;
		}

		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true)))
		{
			return DDERR_GENERIC;
		}

		D3DCAPS9 Caps9 = {};

		HRESULT hr = (*d3d9Device)->GetDeviceCaps(&Caps9);

		if (SUCCEEDED(hr))
		{
			ConvertDeviceDesc(*lpD3DDevDesc, Caps9);
		}

		return hr;
	}

	return GetProxyInterfaceV7()->GetCaps(lpD3DDevDesc);
}

HRESULT m_IDirect3DDeviceX::GetStats(LPD3DSTATS lpD3DStats)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	switch (ProxyDirectXVersion)
	{
	case 1:
		return GetProxyInterfaceV1()->GetStats(lpD3DStats);
	case 2:
		return GetProxyInterfaceV2()->GetStats(lpD3DStats);
	case 3:
		return GetProxyInterfaceV3()->GetStats(lpD3DStats);
	default:
		LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");
		return DDERR_UNSUPPORTED;
	}
}

HRESULT m_IDirect3DDeviceX::AddViewport(LPDIRECT3DVIEWPORT3 lpDirect3DViewport)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9 || ProxyDirectXVersion == 7)
	{
		// This method will fail, returning DDERR_INVALIDPARAMS, if you attempt to add a viewport that has already been assigned to the device.
		if (!lpDirect3DViewport || IsViewportAttached(lpDirect3DViewport))
		{
			return DDERR_INVALIDPARAMS;
		}

		// ToDo: Validate Viewport address
		AttachedViewports.push_back(lpDirect3DViewport);

		lpDirect3DViewport->AddRef();

		return D3D_OK;
	}

	if (lpDirect3DViewport)
	{
		lpDirect3DViewport->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpDirect3DViewport);
	}

	switch (ProxyDirectXVersion)
	{
	case 1:
		return GetProxyInterfaceV1()->AddViewport(lpDirect3DViewport);
	case 2:
		return GetProxyInterfaceV2()->AddViewport(lpDirect3DViewport);
	case 3:
		return GetProxyInterfaceV3()->AddViewport(lpDirect3DViewport);
	default:
		return DDERR_GENERIC;
	}
}

HRESULT m_IDirect3DDeviceX::DeleteViewport(LPDIRECT3DVIEWPORT3 lpDirect3DViewport)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (ProxyDirectXVersion > 3)
	{
		if (!lpDirect3DViewport)
		{
			return DDERR_INVALIDPARAMS;
		}

		// ToDo: Figure out what to do if an attempting to delete the SetCurrentViewport
		bool ret = DeleteAttachedViewport(lpDirect3DViewport);

		if (!ret)
		{
			return DDERR_INVALIDPARAMS;
		}

		// ToDo: Validate Viewport address
		lpDirect3DViewport->Release();

		return D3D_OK;
	}

	if (lpDirect3DViewport)
	{
		lpDirect3DViewport->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpDirect3DViewport);
	}

	switch (ProxyDirectXVersion)
	{
	case 1:
		return GetProxyInterfaceV1()->DeleteViewport(lpDirect3DViewport);
	case 2:
		return GetProxyInterfaceV2()->DeleteViewport(lpDirect3DViewport);
	case 3:
		return GetProxyInterfaceV3()->DeleteViewport(lpDirect3DViewport);
	default:
		return DDERR_GENERIC;
	}
}

HRESULT m_IDirect3DDeviceX::NextViewport(LPDIRECT3DVIEWPORT3 lpDirect3DViewport, LPDIRECT3DVIEWPORT3* lplpDirect3DViewport, DWORD dwFlags, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (ProxyDirectXVersion > 3)
	{
		if (!lplpDirect3DViewport || (dwFlags == D3DNEXT_NEXT && !lpDirect3DViewport))
		{
			return DDERR_INVALIDPARAMS;
		}

		*lplpDirect3DViewport = nullptr;

		if (AttachedViewports.size() == 0)
		{
			return D3DERR_NOVIEWPORTS;
		}

		switch (dwFlags)
		{
		case D3DNEXT_HEAD:
			// Retrieve the item at the beginning of the list.
			*lplpDirect3DViewport = AttachedViewports.front();
			break;
		case D3DNEXT_TAIL:
			// Retrieve the item at the end of the list.
			*lplpDirect3DViewport = AttachedViewports.back();
			break;
		case D3DNEXT_NEXT:
			// Retrieve the next item in the list.
			// If you attempt to retrieve the next viewport in the list when you are at the end of the list, this method returns D3D_OK but lplpAnotherViewport is NULL.
			for (UINT x = 1; x < AttachedViewports.size(); x++)
			{
				if (AttachedViewports[x - 1] == lpDirect3DViewport)
				{
					*lplpDirect3DViewport = AttachedViewports[x];
					break;
				}
			}
			break;
		default:
			return DDERR_INVALIDPARAMS;
			break;
		}

		// ToDo: Validate return Viewport address
		return D3D_OK;
	}

	if (lpDirect3DViewport)
	{
		lpDirect3DViewport->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpDirect3DViewport);
	}

	HRESULT hr = DDERR_GENERIC;

	switch (ProxyDirectXVersion)
	{
	case 1:
		hr = GetProxyInterfaceV1()->NextViewport(lpDirect3DViewport, (LPDIRECT3DVIEWPORT*)lplpDirect3DViewport, dwFlags);
		break;
	case 2:
		hr = GetProxyInterfaceV2()->NextViewport(lpDirect3DViewport, (LPDIRECT3DVIEWPORT2*)lplpDirect3DViewport, dwFlags);
		break;
	case 3:
		hr = GetProxyInterfaceV3()->NextViewport(lpDirect3DViewport, lplpDirect3DViewport, dwFlags);
		break;
	}

	if (SUCCEEDED(hr) && lplpDirect3DViewport)
	{
		*lplpDirect3DViewport = ProxyAddressLookupTable.FindAddress<m_IDirect3DViewport3>(*lplpDirect3DViewport, DirectXVersion);
	}

	return hr;
}

HRESULT m_IDirect3DDeviceX::SetCurrentViewport(LPDIRECT3DVIEWPORT3 lpd3dViewport)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9 || ProxyDirectXVersion == 7)
	{
		// Before calling this method, applications must have already called the AddViewport method to add the viewport to the device.
		if (!lpd3dViewport || !IsViewportAttached(lpd3dViewport))
		{
			return DDERR_INVALIDPARAMS;
		}

		D3DVIEWPORT Viewport = {};
		Viewport.dwSize = sizeof(D3DVIEWPORT);

		// ToDo: Validate Viewport address
		HRESULT hr = lpd3dViewport->GetViewport(&Viewport);

		if (SUCCEEDED(hr))
		{
			D3DVIEWPORT7 Viewport7;

			ConvertViewport(Viewport7, Viewport);

			hr = SetViewport(&Viewport7);

			if (SUCCEEDED(hr))
			{
				lpCurrentViewport = lpd3dViewport;

				lpCurrentViewport->AddRef();
			}
		}

		return hr;
	}

	if (lpd3dViewport)
	{
		lpd3dViewport->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpd3dViewport);
	}

	switch (ProxyDirectXVersion)
	{
	case 1:
	default:
		return DDERR_GENERIC;
	case 2:
		return GetProxyInterfaceV2()->SetCurrentViewport(lpd3dViewport);
	case 3:
		return GetProxyInterfaceV3()->SetCurrentViewport(lpd3dViewport);
	}
}

HRESULT m_IDirect3DDeviceX::GetCurrentViewport(LPDIRECT3DVIEWPORT3* lplpd3dViewport, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9 || ProxyDirectXVersion == 7)
	{
		if (!lplpd3dViewport)
		{
			return DDERR_INVALIDPARAMS;
		}

		if (!lpCurrentViewport)
		{
			return D3DERR_NOCURRENTVIEWPORT;
		}

		// ToDo: Validate Viewport address
		*lplpd3dViewport = lpCurrentViewport;

		lpCurrentViewport->AddRef();

		return D3D_OK;
	}

	HRESULT hr = DDERR_GENERIC;

	switch (ProxyDirectXVersion)
	{
	case 2:
		hr = GetProxyInterfaceV2()->GetCurrentViewport((LPDIRECT3DVIEWPORT2*)lplpd3dViewport);
		break;
	case 3:
		hr = GetProxyInterfaceV3()->GetCurrentViewport(lplpd3dViewport);
		break;
	}

	if (SUCCEEDED(hr) && lplpd3dViewport)
	{
		*lplpd3dViewport = ProxyAddressLookupTable.FindAddress<m_IDirect3DViewport3>(*lplpd3dViewport, DirectXVersion);
	}

	return hr;
}

HRESULT m_IDirect3DDeviceX::SetViewport(LPD3DVIEWPORT7 lpViewport)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lpViewport)
		{
			return DDERR_INVALIDPARAMS;
		}

		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true)))
		{
			return DDERR_GENERIC;
		}

		return (*d3d9Device)->SetViewport((D3DVIEWPORT9*)lpViewport);
	}

	D3DVIEWPORT7 Viewport7;
	if (Config.DdrawUseNativeResolution && lpViewport)
	{
		ConvertViewport(Viewport7, *lpViewport);
		Viewport7.dwX = (LONG)(Viewport7.dwX * ScaleDDWidthRatio) + ScaleDDPadX;
		Viewport7.dwY = (LONG)(Viewport7.dwY * ScaleDDHeightRatio) + ScaleDDPadY;
		Viewport7.dwWidth = (LONG)(Viewport7.dwWidth * ScaleDDWidthRatio);
		Viewport7.dwHeight = (LONG)(Viewport7.dwHeight * ScaleDDHeightRatio);
		lpViewport = &Viewport7;
	}

	return GetProxyInterfaceV7()->SetViewport(lpViewport);
}

HRESULT m_IDirect3DDeviceX::GetViewport(LPD3DVIEWPORT7 lpViewport)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lpViewport)
		{
			return DDERR_INVALIDPARAMS;
		}

		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true)))
		{
			return DDERR_GENERIC;
		}

		return (*d3d9Device)->GetViewport((D3DVIEWPORT9*)lpViewport);
	}

	return GetProxyInterfaceV7()->GetViewport(lpViewport);
}

HRESULT m_IDirect3DDeviceX::Begin(D3DPRIMITIVETYPE d3dpt, DWORD d3dvt, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (ProxyDirectXVersion > 3)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	switch (ProxyDirectXVersion)
	{
	case 1:
	default:
		return DDERR_GENERIC;
	case 2:
		return GetProxyInterfaceV2()->Begin(d3dpt, (D3DVERTEXTYPE)d3dvt, dwFlags);
	case 3:
		return GetProxyInterfaceV3()->Begin(d3dpt, d3dvt, dwFlags);
	}
}

HRESULT m_IDirect3DDeviceX::BeginIndexed(D3DPRIMITIVETYPE dptPrimitiveType, DWORD dvtVertexType, LPVOID lpvVertices, DWORD dwNumVertices, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (ProxyDirectXVersion > 3)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	switch (ProxyDirectXVersion)
	{
	case 1:
	default:
		return DDERR_GENERIC;
	case 2:
		return GetProxyInterfaceV2()->BeginIndexed(dptPrimitiveType, (D3DVERTEXTYPE)dvtVertexType, lpvVertices, dwNumVertices, dwFlags);
	case 3:
		return GetProxyInterfaceV3()->BeginIndexed(dptPrimitiveType, dvtVertexType, lpvVertices, dwNumVertices, dwFlags);
	}
}

HRESULT m_IDirect3DDeviceX::Vertex(LPVOID lpVertexType)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (ProxyDirectXVersion > 3)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	switch (ProxyDirectXVersion)
	{
	case 1:
	default:
		return DDERR_GENERIC;
	case 2:
		return GetProxyInterfaceV2()->Vertex(lpVertexType);
	case 3:
		return GetProxyInterfaceV3()->Vertex(lpVertexType);
	}
}

HRESULT m_IDirect3DDeviceX::Index(WORD wVertexIndex)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (ProxyDirectXVersion > 3)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	switch (ProxyDirectXVersion)
	{
	case 1:
	default:
		return DDERR_GENERIC;
	case 2:
		return GetProxyInterfaceV2()->Index(wVertexIndex);
	case 3:
		return GetProxyInterfaceV3()->Index(wVertexIndex);
	}
}

HRESULT m_IDirect3DDeviceX::End(DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (ProxyDirectXVersion > 3)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	switch (ProxyDirectXVersion)
	{
	case 1:
	default:
		return DDERR_GENERIC;
	case 2:
		return GetProxyInterfaceV2()->End(dwFlags);
	case 3:
		return GetProxyInterfaceV3()->End(dwFlags);
	}
}

HRESULT m_IDirect3DDeviceX::BeginScene()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true)))
		{
			return DDERR_GENERIC;
		}

		// Set 3D Enabled
		ddrawParent->Enable3D();

		HRESULT hr = (*d3d9Device)->BeginScene();

#ifdef ENABLE_DEBUGOVERLAY
		DOverlay.BeginScene();
#endif

		return hr;
	}

	switch (ProxyDirectXVersion)
	{
	case 1:
		return GetProxyInterfaceV1()->BeginScene();

	case 2:
		return GetProxyInterfaceV2()->BeginScene();

	case 3:
		return GetProxyInterfaceV3()->BeginScene();

	case 7:
	default:
		return GetProxyInterfaceV7()->BeginScene();
	}
}

HRESULT m_IDirect3DDeviceX::EndScene()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true)))
		{
			return DDERR_GENERIC;
		}

#ifdef ENABLE_DEBUGOVERLAY
		DOverlay.EndScene();
#endif

		// The IDirect3DDevice7::EndScene method ends a scene that was begun by calling the IDirect3DDevice7::BeginScene method.
		// When this method succeeds, the scene has been rendered, and the device surface holds the rendered scene.

		(*d3d9Device)->EndScene();

		return (*d3d9Device)->Present(nullptr, nullptr, nullptr, nullptr);
	}

	switch (ProxyDirectXVersion)
	{
	case 1:
		return GetProxyInterfaceV1()->EndScene();
	case 2:
		return GetProxyInterfaceV2()->EndScene();
	case 3:
		return GetProxyInterfaceV3()->EndScene();
	case 7:
		return GetProxyInterfaceV7()->EndScene();
	default:
		return DDERR_GENERIC;
	}
}

HRESULT m_IDirect3DDeviceX::Clear(DWORD dwCount, LPD3DRECT lpRects, DWORD dwFlags, D3DCOLOR dwColor, D3DVALUE dvZ, DWORD dwStencil)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true)))
		{
			return DDERR_GENERIC;
		}

		return (*d3d9Device)->Clear(dwCount, lpRects, dwFlags, dwColor, dvZ, dwStencil);
	}

	return GetProxyInterfaceV7()->Clear(dwCount, lpRects, dwFlags, dwColor, dvZ, dwStencil);
}

HRESULT m_IDirect3DDeviceX::GetDirect3D(LPDIRECT3D7* lplpD3D, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lplpD3D)
		{
			return DDERR_INVALIDPARAMS;
		}

		*lplpD3D = nullptr;

		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, false)))
		{
			return DDERR_GENERIC;
		}

		m_IDirect3DX** lplpD3DX = ddrawParent->GetCurrentD3D();

		if (!(*lplpD3DX))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: missing Direct3D wrapper!");
			return DDERR_GENERIC;
		}

		*lplpD3D = (LPDIRECT3D7)(*lplpD3DX)->GetWrapperInterfaceX(DirectXVersion);

		if (!(*lplpD3D))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: could not get Direct3D interface!");
			return DDERR_GENERIC;
		}

		return D3D_OK;
	}

	HRESULT hr = DDERR_GENERIC;

	switch (ProxyDirectXVersion)
	{
	case 1:
		hr = GetProxyInterfaceV1()->GetDirect3D((LPDIRECT3D*)lplpD3D);
		break;
	case 2:
		hr = GetProxyInterfaceV2()->GetDirect3D((LPDIRECT3D2*)lplpD3D);
		break;
	case 3:
		hr = GetProxyInterfaceV3()->GetDirect3D((LPDIRECT3D3*)lplpD3D);
		break;
	case 7:
		hr = GetProxyInterfaceV7()->GetDirect3D(lplpD3D);
		break;
	}

	if (SUCCEEDED(hr) && lplpD3D)
	{
		*lplpD3D = ProxyAddressLookupTable.FindAddress<m_IDirect3D7>(*lplpD3D, DirectXVersion);
	}

	return hr;
}

HRESULT m_IDirect3DDeviceX::GetLightState(D3DLIGHTSTATETYPE dwLightStateType, LPDWORD lpdwLightState)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (ProxyDirectXVersion > 3)
	{
		if (dwLightStateType == D3DLIGHTSTATE_MATERIAL || dwLightStateType == D3DLIGHTSTATE_COLORMODEL)
		{
			LOG_LIMIT(100, __FUNCTION__ << " LightStateType: " << dwLightStateType << " Not Implemented");
			return DDERR_UNSUPPORTED;
		}

		DWORD RenderState = 0;
		switch (dwLightStateType)
		{
		case D3DLIGHTSTATE_AMBIENT:
			RenderState = D3DRENDERSTATE_AMBIENT;
			break;
		case D3DLIGHTSTATE_FOGMODE:
			RenderState = D3DRENDERSTATE_FOGVERTEXMODE;
			break;
		case D3DLIGHTSTATE_FOGSTART:
			RenderState = D3DRENDERSTATE_FOGSTART;
			break;
		case D3DLIGHTSTATE_FOGEND:
			RenderState = D3DRENDERSTATE_FOGEND;
			break;
		case D3DLIGHTSTATE_FOGDENSITY:
			RenderState = D3DRENDERSTATE_FOGDENSITY;
			break;
		case D3DLIGHTSTATE_COLORVERTEX:
			RenderState = D3DRENDERSTATE_COLORVERTEX;
			break;
		default:
			break;
		}

		if (!RenderState)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: unknown LightStateType: " << dwLightStateType);
			return DDERR_HEIGHTALIGN;
		}

		return GetRenderState((D3DRENDERSTATETYPE)RenderState, lpdwLightState);
	}

	switch (ProxyDirectXVersion)
	{
	case 1:
	default:
		return DDERR_GENERIC;
	case 2:
		return GetProxyInterfaceV2()->GetLightState(dwLightStateType, lpdwLightState);
	case 3:
		return GetProxyInterfaceV3()->GetLightState(dwLightStateType, lpdwLightState);
	}
}

HRESULT m_IDirect3DDeviceX::SetLightState(D3DLIGHTSTATETYPE dwLightStateType, DWORD dwLightState)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (ProxyDirectXVersion > 3)
	{
		if (dwLightStateType == D3DLIGHTSTATE_MATERIAL || dwLightStateType == D3DLIGHTSTATE_COLORMODEL)
		{
			LOG_LIMIT(100, __FUNCTION__ << " LightStateType: " << dwLightStateType << " Not Implemented");
			return DDERR_UNSUPPORTED;
		}

		DWORD RenderState = 0;
		switch (dwLightStateType)
		{
		case D3DLIGHTSTATE_AMBIENT:
			RenderState = D3DRENDERSTATE_AMBIENT;
			break;
		case D3DLIGHTSTATE_FOGMODE:
			RenderState = D3DRENDERSTATE_FOGVERTEXMODE;
			break;
		case D3DLIGHTSTATE_FOGSTART:
			RenderState = D3DRENDERSTATE_FOGSTART;
			break;
		case D3DLIGHTSTATE_FOGEND:
			RenderState = D3DRENDERSTATE_FOGEND;
			break;
		case D3DLIGHTSTATE_FOGDENSITY:
			RenderState = D3DRENDERSTATE_FOGDENSITY;
			break;
		case D3DLIGHTSTATE_COLORVERTEX:
			RenderState = D3DRENDERSTATE_COLORVERTEX;
			break;
		default:
			break;
		}

		if (!RenderState)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: unknown LightStateType: " << dwLightStateType);
			return DDERR_HEIGHTALIGN;
		}

		return SetRenderState((D3DRENDERSTATETYPE)RenderState, dwLightState);
	}

	switch (ProxyDirectXVersion)
	{
	case 1:
	default:
		return DDERR_GENERIC;
	case 2:
		return GetProxyInterfaceV2()->SetLightState(dwLightStateType, dwLightState);
	case 3:
		return GetProxyInterfaceV3()->SetLightState(dwLightStateType, dwLightState);
	}
}

HRESULT m_IDirect3DDeviceX::SetLight(DWORD dwLightIndex, LPD3DLIGHT7 lpLight)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lpLight)
		{
			return DDERR_INVALIDPARAMS;
		}

		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true)))
		{
			return DDERR_GENERIC;
		}

		D3DLIGHT9 Light = *(D3DLIGHT9*)lpLight;

		// Make spot light work more like it did in Direct3D7
		if (Light.Type == D3DLIGHTTYPE::D3DLIGHT_SPOT)
		{
			// Theta must be in the range from 0 through the value specified by Phi
			if (Light.Theta <= Light.Phi)
			{
				Light.Theta /= 1.75f;
			}
		}

		HRESULT hr = (*d3d9Device)->SetLight(dwLightIndex, &Light);

		if (SUCCEEDED(hr))
		{
#ifdef ENABLE_DEBUGOVERLAY
			DOverlay.SetLight(dwLightIndex, lpLight);
#endif
		}

		return hr;
	}

	return GetProxyInterfaceV7()->SetLight(dwLightIndex, lpLight);
}

HRESULT m_IDirect3DDeviceX::GetLight(DWORD dwLightIndex, LPD3DLIGHT7 lpLight)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lpLight)
		{
			return DDERR_INVALIDPARAMS;
		}

		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true)))
		{
			return DDERR_GENERIC;
		}

		return (*d3d9Device)->GetLight(dwLightIndex, (D3DLIGHT9*)lpLight);
	}

	return GetProxyInterfaceV7()->GetLight(dwLightIndex, lpLight);
}

HRESULT m_IDirect3DDeviceX::LightEnable(DWORD dwLightIndex, BOOL bEnable)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true)))
		{
			return DDERR_GENERIC;
		}

		HRESULT hr = (*d3d9Device)->LightEnable(dwLightIndex, bEnable);

		if (SUCCEEDED(hr))
		{
#ifdef ENABLE_DEBUGOVERLAY
			DOverlay.LightEnable(dwLightIndex, bEnable);
#endif
		}

		return hr;
	}

	return GetProxyInterfaceV7()->LightEnable(dwLightIndex, bEnable);
}

HRESULT m_IDirect3DDeviceX::GetLightEnable(DWORD dwLightIndex, BOOL* pbEnable)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!pbEnable)
		{
			return DDERR_INVALIDPARAMS;
		}

		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true)))
		{
			return DDERR_GENERIC;
		}

		return (*d3d9Device)->GetLightEnable(dwLightIndex, pbEnable);
	}

	return GetProxyInterfaceV7()->GetLightEnable(dwLightIndex, pbEnable);
}

HRESULT m_IDirect3DDeviceX::MultiplyTransform(D3DTRANSFORMSTATETYPE dtstTransformStateType, LPD3DMATRIX lpD3DMatrix)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true)))
		{
			return DDERR_GENERIC;
		}

		switch ((DWORD)dtstTransformStateType)
		{
		case D3DTRANSFORMSTATE_WORLD:
			dtstTransformStateType = D3DTS_WORLD;
			break;
		case D3DTRANSFORMSTATE_WORLD1:
			dtstTransformStateType = D3DTS_WORLD1;
			break;
		case D3DTRANSFORMSTATE_WORLD2:
			dtstTransformStateType = D3DTS_WORLD2;
			break;
		case D3DTRANSFORMSTATE_WORLD3:
			dtstTransformStateType = D3DTS_WORLD3;
			break;
		}

		return (*d3d9Device)->MultiplyTransform(dtstTransformStateType, lpD3DMatrix);
	}

	switch (ProxyDirectXVersion)
	{
	case 1:
	default:
		return DDERR_GENERIC;
	case 2:
		return GetProxyInterfaceV2()->MultiplyTransform(dtstTransformStateType, lpD3DMatrix);
	case 3:
		return GetProxyInterfaceV3()->MultiplyTransform(dtstTransformStateType, lpD3DMatrix);
	case 7:
		return GetProxyInterfaceV7()->MultiplyTransform(dtstTransformStateType, lpD3DMatrix);
	}
}

HRESULT m_IDirect3DDeviceX::SetMaterial(LPD3DMATERIAL7 lpMaterial)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lpMaterial)
		{
			return DDERR_INVALIDPARAMS;
		}

		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true)))
		{
			return DDERR_GENERIC;
		}

		return (*d3d9Device)->SetMaterial((D3DMATERIAL9*)lpMaterial);
	}

	return GetProxyInterfaceV7()->SetMaterial(lpMaterial);
}

HRESULT m_IDirect3DDeviceX::GetMaterial(LPD3DMATERIAL7 lpMaterial)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lpMaterial)
		{
			return DDERR_INVALIDPARAMS;
		}

		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true)))
		{
			return DDERR_GENERIC;
		}

		return (*d3d9Device)->GetMaterial((D3DMATERIAL9*)lpMaterial);
	}

	return GetProxyInterfaceV7()->GetMaterial(lpMaterial);
}

HRESULT m_IDirect3DDeviceX::SetRenderState(D3DRENDERSTATETYPE dwRenderStateType, DWORD dwRenderState)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true)))
		{
			return DDERR_GENERIC;
		}

		switch ((DWORD)dwRenderStateType)
		{
		case D3DRENDERSTATE_ANTIALIAS:
			dwRenderStateType = D3DRS_MULTISAMPLEANTIALIAS;
			dwRenderState = (dwRenderState == D3DANTIALIAS_SORTDEPENDENT || dwRenderState == D3DANTIALIAS_SORTINDEPENDENT);
			break;
		case D3DRENDERSTATE_EDGEANTIALIAS:
			dwRenderStateType = D3DRS_ANTIALIASEDLINEENABLE;
			break;
		case D3DRENDERSTATE_ZBIAS:
		{
			FLOAT Biased = static_cast<FLOAT>(dwRenderState) * -0.000005f;
			dwRenderState = *reinterpret_cast<const DWORD*>(&Biased);
			dwRenderStateType = D3DRS_DEPTHBIAS;
			break;
		}
		case D3DRENDERSTATE_TEXTUREPERSPECTIVE:
			return D3D_OK;		// As long as the device's D3DPTEXTURECAPS_PERSPECTIVE is enabled, the correction will be applied automatically.
		case D3DRENDERSTATE_LINEPATTERN:
			LOG_LIMIT(100, __FUNCTION__ << " Warning: 'D3DRENDERSTATE_LINEPATTERN' not implemented!");
			return D3D_OK;
		case D3DRENDERSTATE_ZVISIBLE:
			LOG_LIMIT(100, __FUNCTION__ << " Warning: 'D3DRENDERSTATE_ZVISIBLE' not implemented!");
			return D3D_OK;
		case D3DRENDERSTATE_STIPPLEDALPHA:
			LOG_LIMIT(100, __FUNCTION__ << " Warning: 'D3DRENDERSTATE_STIPPLEDALPHA' not implemented!");
			return D3D_OK;
		case D3DRENDERSTATE_EXTENTS:
			LOG_LIMIT(100, __FUNCTION__ << " Warning: 'D3DRENDERSTATE_EXTENTS' not implemented!");
			return D3D_OK;
		case D3DRENDERSTATE_COLORKEYENABLE:
			LOG_LIMIT(100, __FUNCTION__ << " Warning: 'D3DRENDERSTATE_COLORKEYENABLE' not implemented!");
			return D3D_OK;
		case D3DRENDERSTATE_COLORKEYBLENDENABLE:
			LOG_LIMIT(100, __FUNCTION__ << " Warning: 'D3DRENDERSTATE_COLORKEYBLENDENABLE' not implemented!");
			return D3D_OK;
		}

		if (!CheckRenderStateType(dwRenderStateType))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: Render state type not implemented: " << dwRenderStateType);
		}

		return (*d3d9Device)->SetRenderState(dwRenderStateType, dwRenderState);
	}

	switch (ProxyDirectXVersion)
	{
	case 1:
	default:
		return DDERR_GENERIC;
	case 2:
		return GetProxyInterfaceV2()->SetRenderState(dwRenderStateType, dwRenderState);
	case 3:
		return GetProxyInterfaceV3()->SetRenderState(dwRenderStateType, dwRenderState);
	case 7:
		return GetProxyInterfaceV7()->SetRenderState(dwRenderStateType, dwRenderState);
	}
}

HRESULT m_IDirect3DDeviceX::GetRenderState(D3DRENDERSTATETYPE dwRenderStateType, LPDWORD lpdwRenderState)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lpdwRenderState)
		{
			return DDERR_INVALIDPARAMS;
		}

		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true)))
		{
			return DDERR_GENERIC;
		}

		switch ((DWORD)dwRenderStateType)
		{
		case D3DRENDERSTATE_ANTIALIAS:
			dwRenderStateType = D3DRS_MULTISAMPLEANTIALIAS;
			break;
		case D3DRENDERSTATE_EDGEANTIALIAS:
			dwRenderStateType = D3DRS_ANTIALIASEDLINEENABLE;
			break;
		case D3DRENDERSTATE_ZBIAS:
		{
			HRESULT hr = (*d3d9Device)->GetRenderState(D3DRS_DEPTHBIAS, lpdwRenderState);
			*lpdwRenderState = static_cast<DWORD>(*reinterpret_cast<const FLOAT*>(lpdwRenderState) * -200000.0f);
			return hr;
		}
		case D3DRENDERSTATE_TEXTUREPERSPECTIVE:
			*lpdwRenderState = TRUE;	// As long as the device's D3DPTEXTURECAPS_PERSPECTIVE is enabled, the correction will be applied automatically.
			return D3D_OK;
		case D3DRENDERSTATE_LINEPATTERN:
			LOG_LIMIT(100, __FUNCTION__ << " Warning: 'D3DRENDERSTATE_LINEPATTERN' not implemented!");
			*lpdwRenderState = 0;
			return D3D_OK;
		case D3DRENDERSTATE_ZVISIBLE:
			LOG_LIMIT(100, __FUNCTION__ << " Warning: 'D3DRENDERSTATE_ZVISIBLE' not implemented!");
			*lpdwRenderState = FALSE;
			return D3D_OK;
		case D3DRENDERSTATE_STIPPLEDALPHA:
			LOG_LIMIT(100, __FUNCTION__ << " Warning: 'D3DRENDERSTATE_STIPPLEDALPHA' not implemented!");
			*lpdwRenderState = FALSE;
			return D3D_OK;
		case D3DRENDERSTATE_EXTENTS:
			LOG_LIMIT(100, __FUNCTION__ << " Warning: 'D3DRENDERSTATE_EXTENTS' not implemented!");
			*lpdwRenderState = FALSE;
			return D3D_OK;
		case D3DRENDERSTATE_COLORKEYENABLE:
			LOG_LIMIT(100, __FUNCTION__ << " Warning: 'D3DRENDERSTATE_COLORKEYENABLE' not implemented!");
			*lpdwRenderState = FALSE;
			return D3D_OK;
		case D3DRENDERSTATE_COLORKEYBLENDENABLE:
			LOG_LIMIT(100, __FUNCTION__ << " Warning: 'D3DRENDERSTATE_COLORKEYBLENDENABLE' not implemented!");
			*lpdwRenderState = FALSE;
			return D3D_OK;
		}

		if (!CheckRenderStateType(dwRenderStateType))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: Render state type not implemented: " << dwRenderStateType);
		}

		return (*d3d9Device)->GetRenderState(dwRenderStateType, lpdwRenderState);
	}

	switch (ProxyDirectXVersion)
	{
	case 1:
	default:
		return DDERR_GENERIC;
	case 2:
		return GetProxyInterfaceV2()->GetRenderState(dwRenderStateType, lpdwRenderState);
	case 3:
		return GetProxyInterfaceV3()->GetRenderState(dwRenderStateType, lpdwRenderState);
	case 7:
		return GetProxyInterfaceV7()->GetRenderState(dwRenderStateType, lpdwRenderState);
	}
}

HRESULT m_IDirect3DDeviceX::BeginStateBlock()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true)))
		{
			return DDERR_GENERIC;
		}

		return (*d3d9Device)->BeginStateBlock();
	}

	return GetProxyInterfaceV7()->BeginStateBlock();
}

HRESULT m_IDirect3DDeviceX::EndStateBlock(LPDWORD lpdwBlockHandle)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lpdwBlockHandle)
		{
			return DDERR_INVALIDPARAMS;
		}

		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true)))
		{
			return DDERR_GENERIC;
		}

		// ToDo: Validate BlockHandle
		return (*d3d9Device)->EndStateBlock(reinterpret_cast<IDirect3DStateBlock9**>(lpdwBlockHandle));
	}

	return GetProxyInterfaceV7()->EndStateBlock(lpdwBlockHandle);
}

HRESULT m_IDirect3DDeviceX::DrawPrimitive(D3DPRIMITIVETYPE dptPrimitiveType, DWORD dwVertexTypeDesc, LPVOID lpVertices, DWORD dwVertexCount, DWORD dwFlags, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lpVertices)
		{
			return DDERR_INVALIDPARAMS;
		}

		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true)))
		{
			return DDERR_GENERIC;
		}

		// dwFlags (D3DDP_WAIT) can be ignored safely

		// Handle dwFlags
		DWORD rsClipping = 0, rsLighting = 0, rsExtents = 0;
		SetDrawFlags(rsClipping, rsLighting, rsExtents, dwVertexTypeDesc, dwFlags, DirectXVersion);

		HRESULT hr;
		if (D3DFVF_LVERTEX == dwVertexTypeDesc)
		{
			D3DLVERTEX *lFVF = (D3DLVERTEX*)lpVertices;
			std::vector<D3DLVERTEX9> lFVF9(dwVertexCount);

			for (UINT x = 0; x < dwVertexCount; x++)
			{
				lFVF9[x].x = lFVF[x].x;
				lFVF9[x].y = lFVF[x].y;
				lFVF9[x].z = lFVF[x].z;
				lFVF9[x].diffuse = lFVF[x].color;
				lFVF9[x].specular = lFVF[x].specular;
				lFVF9[x].tu = lFVF[x].tu;
				lFVF9[x].tv = lFVF[x].tv;
			}

			// Set fixed function vertex type
			(*d3d9Device)->SetFVF(D3DFVF_LVERTEX9);

			// Draw primitive UP
			hr = (*d3d9Device)->DrawPrimitiveUP(dptPrimitiveType, GetNumberOfPrimitives(dptPrimitiveType, dwVertexCount), &lFVF9[0], sizeof(D3DLVERTEX9));
		}
		else
		{
			// Set fixed function vertex type
			(*d3d9Device)->SetFVF(dwVertexTypeDesc);

			// Draw primitive UP
			hr = (*d3d9Device)->DrawPrimitiveUP(dptPrimitiveType, GetNumberOfPrimitives(dptPrimitiveType, dwVertexCount), lpVertices, GetVertexStride(dwVertexTypeDesc));
		}

		// Handle dwFlags
		UnSetDrawFlags(rsClipping, rsLighting, rsExtents, dwVertexTypeDesc, dwFlags, DirectXVersion);

		return hr;
	}

	std::vector<D3DTLVERTEX> pVert;
	if (Config.DdrawUseNativeResolution && dwVertexTypeDesc == 3)
	{
		CopyScaleVertex(lpVertices, pVert, dwVertexCount);
		lpVertices = &pVert[0];
	}

	switch (ProxyDirectXVersion)
	{
	case 1:
	default:
		return DDERR_GENERIC;
	case 2:
		return GetProxyInterfaceV2()->DrawPrimitive(dptPrimitiveType, (D3DVERTEXTYPE)dwVertexTypeDesc, lpVertices, dwVertexCount, dwFlags);
	case 3:
		return GetProxyInterfaceV3()->DrawPrimitive(dptPrimitiveType, dwVertexTypeDesc, lpVertices, dwVertexCount, dwFlags);
	case 7:
		if (DirectXVersion != 7)
		{
			// Handle dwFlags
			DWORD rsClipping = 0, rsLighting = 0, rsExtents = 0;
			SetDrawFlags(rsClipping, rsLighting, rsExtents, dwVertexTypeDesc, dwFlags, DirectXVersion);

			HRESULT hr = GetProxyInterfaceV7()->DrawPrimitive(dptPrimitiveType, dwVertexTypeDesc, lpVertices, dwVertexCount, dwFlags);

			// Handle dwFlags
			UnSetDrawFlags(rsClipping, rsLighting, rsExtents, dwVertexTypeDesc, dwFlags, DirectXVersion);

			return hr;
		}
		else
		{
			return GetProxyInterfaceV7()->DrawPrimitive(dptPrimitiveType, dwVertexTypeDesc, lpVertices, dwVertexCount, dwFlags);
		}
	}
}

HRESULT m_IDirect3DDeviceX::DrawPrimitiveStrided(D3DPRIMITIVETYPE dptPrimitiveType, DWORD dwVertexTypeDesc, LPD3DDRAWPRIMITIVESTRIDEDDATA lpVertexArray, DWORD dwVertexCount, DWORD dwFlags, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	switch (ProxyDirectXVersion)
	{
	case 1:
	case 2:
	default:
		return DDERR_GENERIC;
	case 3:
		return GetProxyInterfaceV3()->DrawPrimitiveStrided(dptPrimitiveType, dwVertexTypeDesc, lpVertexArray, dwVertexCount, dwFlags);
	case 7:
		if (DirectXVersion != 7)
		{
			// Handle dwFlags
			DWORD rsClipping = 0, rsLighting = 0, rsExtents = 0;
			SetDrawFlags(rsClipping, rsLighting, rsExtents, dwVertexTypeDesc, dwFlags, DirectXVersion);

			HRESULT hr = GetProxyInterfaceV7()->DrawPrimitiveStrided(dptPrimitiveType, dwVertexTypeDesc, lpVertexArray, dwVertexCount, dwFlags);

			// Handle dwFlags
			UnSetDrawFlags(rsClipping, rsLighting, rsExtents, dwVertexTypeDesc, dwFlags, DirectXVersion);

			return hr;
		}
		else
		{
			return GetProxyInterfaceV7()->DrawPrimitiveStrided(dptPrimitiveType, dwVertexTypeDesc, lpVertexArray, dwVertexCount, dwFlags);
		}
	}
}

HRESULT m_IDirect3DDeviceX::DrawPrimitiveVB(D3DPRIMITIVETYPE d3dptPrimitiveType, LPDIRECT3DVERTEXBUFFER7 lpd3dVertexBuffer, DWORD dwStartVertex, DWORD dwNumVertices, DWORD dwFlags, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	if (lpd3dVertexBuffer)
	{
		lpd3dVertexBuffer->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpd3dVertexBuffer);
	}

	switch (ProxyDirectXVersion)
	{
	case 1:
	case 2:
	default:
		return DDERR_GENERIC;
	case 3:
		return GetProxyInterfaceV3()->DrawPrimitiveVB(d3dptPrimitiveType, (LPDIRECT3DVERTEXBUFFER)lpd3dVertexBuffer, dwStartVertex, dwNumVertices, dwFlags);
	case 7:
		if (DirectXVersion != 7)
		{
			D3DVERTEXBUFFERDESC BufferDesc = {};
			if (lpd3dVertexBuffer)
			{
				lpd3dVertexBuffer->GetVertexBufferDesc(&BufferDesc);
			}

			// Handle dwFlags
			DWORD rsClipping = 0, rsLighting = 0, rsExtents = 0;
			SetDrawFlags(rsClipping, rsLighting, rsExtents, BufferDesc.dwFVF, dwFlags, DirectXVersion);

			HRESULT hr = GetProxyInterfaceV7()->DrawPrimitiveVB(d3dptPrimitiveType, lpd3dVertexBuffer, dwStartVertex, dwNumVertices, dwFlags);

			// Handle dwFlags
			UnSetDrawFlags(rsClipping, rsLighting, rsExtents, BufferDesc.dwFVF, dwFlags, DirectXVersion);

			return hr;
		}
		else
		{
			return GetProxyInterfaceV7()->DrawPrimitiveVB(d3dptPrimitiveType, lpd3dVertexBuffer, dwStartVertex, dwNumVertices, dwFlags);
		}
	}
}

HRESULT m_IDirect3DDeviceX::DrawIndexedPrimitive(D3DPRIMITIVETYPE dptPrimitiveType, DWORD dwVertexTypeDesc, LPVOID lpVertices, DWORD dwVertexCount, LPWORD lpIndices, DWORD dwIndexCount, DWORD dwFlags, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lpVertices)
		{
			return DDERR_INVALIDPARAMS;
		}

		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true)))
		{
			return DDERR_GENERIC;
		}

		// dwFlags (D3DDP_WAIT) can be ignored safely

		// Handle dwFlags
		DWORD rsClipping = 0, rsLighting = 0, rsExtents = 0;
		SetDrawFlags(rsClipping, rsLighting, rsExtents, dwVertexTypeDesc, dwFlags, DirectXVersion);

		HRESULT hr;
		if (D3DFVF_LVERTEX == dwVertexTypeDesc)
		{
			D3DLVERTEX *lFVF = (D3DLVERTEX*)lpVertices;
			std::vector<D3DLVERTEX9> lFVF9(dwVertexCount);

			for (UINT x = 0; x < dwVertexCount; x++)
			{
				lFVF9[x].x = lFVF[x].x;
				lFVF9[x].y = lFVF[x].y;
				lFVF9[x].z = lFVF[x].z;
				lFVF9[x].diffuse = lFVF[x].color;
				lFVF9[x].specular = lFVF[x].specular;
				lFVF9[x].tu = lFVF[x].tu;
				lFVF9[x].tv = lFVF[x].tv;
			}

			// Set fixed function vertex type
			(*d3d9Device)->SetFVF(D3DFVF_LVERTEX9);

			// Draw indexed primitive UP
			hr = (*d3d9Device)->DrawIndexedPrimitiveUP(dptPrimitiveType, 0, dwVertexCount, GetNumberOfPrimitives(dptPrimitiveType, dwIndexCount), lpIndices, D3DFMT_INDEX16, &lFVF9[0], sizeof(D3DLVERTEX9));
		}
		else
		{
			// Set fixed function vertex type
			(*d3d9Device)->SetFVF(dwVertexTypeDesc);

			// Draw indexed primitive UP
			hr = (*d3d9Device)->DrawIndexedPrimitiveUP(dptPrimitiveType, 0, dwVertexCount, GetNumberOfPrimitives(dptPrimitiveType, dwIndexCount), lpIndices, D3DFMT_INDEX16, lpVertices, GetVertexStride(dwVertexTypeDesc));
		}

		// Handle dwFlags
		UnSetDrawFlags(rsClipping, rsLighting, rsExtents, dwVertexTypeDesc, dwFlags, DirectXVersion);

		return hr;
	}

	std::vector<D3DTLVERTEX> pVert;
	if (Config.DdrawUseNativeResolution && dwVertexTypeDesc == 3)
	{
		CopyScaleVertex(lpVertices, pVert, dwVertexCount);
		lpVertices = &pVert[0];
	}

	switch (ProxyDirectXVersion)
	{
	case 1:
	default:
		return DDERR_GENERIC;
	case 2:
		return GetProxyInterfaceV2()->DrawIndexedPrimitive(dptPrimitiveType, (D3DVERTEXTYPE)dwVertexTypeDesc, lpVertices, dwVertexCount, lpIndices, dwIndexCount, dwFlags);
	case 3:
		return GetProxyInterfaceV3()->DrawIndexedPrimitive(dptPrimitiveType, dwVertexTypeDesc, lpVertices, dwVertexCount, lpIndices, dwIndexCount, dwFlags);
	case 7:
		if (DirectXVersion != 7)
		{
			// Handle dwFlags
			DWORD rsClipping = 0, rsLighting = 0, rsExtents = 0;
			SetDrawFlags(rsClipping, rsLighting, rsExtents, dwVertexTypeDesc, dwFlags, DirectXVersion);

			HRESULT hr = GetProxyInterfaceV7()->DrawIndexedPrimitive(dptPrimitiveType, dwVertexTypeDesc, lpVertices, dwVertexCount, lpIndices, dwIndexCount, dwFlags);

			// Handle dwFlags
			UnSetDrawFlags(rsClipping, rsLighting, rsExtents, dwVertexTypeDesc, dwFlags, DirectXVersion);

			return hr;
		}
		else
		{
			return GetProxyInterfaceV7()->DrawIndexedPrimitive(dptPrimitiveType, dwVertexTypeDesc, lpVertices, dwVertexCount, lpIndices, dwIndexCount, dwFlags);
		}
	}
}

HRESULT m_IDirect3DDeviceX::DrawIndexedPrimitiveStrided(D3DPRIMITIVETYPE d3dptPrimitiveType, DWORD dwVertexTypeDesc, LPD3DDRAWPRIMITIVESTRIDEDDATA lpVertexArray, DWORD dwVertexCount, LPWORD lpwIndices, DWORD dwIndexCount, DWORD dwFlags, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	switch (ProxyDirectXVersion)
	{
	case 1:
	case 2:
	default:
		return DDERR_GENERIC;
	case 3:
		return GetProxyInterfaceV3()->DrawIndexedPrimitiveStrided(d3dptPrimitiveType, dwVertexTypeDesc, lpVertexArray, dwVertexCount, lpwIndices, dwIndexCount, dwFlags);
	case 7:
		if (DirectXVersion != 7)
		{
			// Handle dwFlags
			DWORD rsClipping = 0, rsLighting = 0, rsExtents = 0;
			SetDrawFlags(rsClipping, rsLighting, rsExtents, dwVertexTypeDesc, dwFlags, DirectXVersion);

			HRESULT hr = GetProxyInterfaceV7()->DrawIndexedPrimitiveStrided(d3dptPrimitiveType, dwVertexTypeDesc, lpVertexArray, dwVertexCount, lpwIndices, dwIndexCount, dwFlags);

			// Handle dwFlags
			UnSetDrawFlags(rsClipping, rsLighting, rsExtents, dwVertexTypeDesc, dwFlags, DirectXVersion);

			return hr;
		}
		else
		{
			return GetProxyInterfaceV7()->DrawIndexedPrimitiveStrided(d3dptPrimitiveType, dwVertexTypeDesc, lpVertexArray, dwVertexCount, lpwIndices, dwIndexCount, dwFlags);
		}
	}
}

HRESULT m_IDirect3DDeviceX::DrawIndexedPrimitiveVB(D3DPRIMITIVETYPE d3dptPrimitiveType, LPDIRECT3DVERTEXBUFFER7 lpd3dVertexBuffer, DWORD dwStartVertex, DWORD dwNumVertices, LPWORD lpwIndices, DWORD dwIndexCount, DWORD dwFlags, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	if (lpd3dVertexBuffer)
	{
		lpd3dVertexBuffer->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpd3dVertexBuffer);
	}

	switch (ProxyDirectXVersion)
	{
	case 1:
	case 2:
	default:
		return DDERR_GENERIC;
	case 3:
		return GetProxyInterfaceV3()->DrawIndexedPrimitiveVB(d3dptPrimitiveType, (LPDIRECT3DVERTEXBUFFER)lpd3dVertexBuffer, lpwIndices, dwIndexCount, dwFlags);
	case 7:
		if (DirectXVersion != 7)
		{
			D3DVERTEXBUFFERDESC BufferDesc = {};
			if (lpd3dVertexBuffer)
			{
				lpd3dVertexBuffer->GetVertexBufferDesc(&BufferDesc);
			}

			// Handle dwFlags
			DWORD rsClipping = 0, rsLighting = 0, rsExtents = 0;
			SetDrawFlags(rsClipping, rsLighting, rsExtents, BufferDesc.dwFVF, dwFlags, DirectXVersion);

			HRESULT hr = GetProxyInterfaceV7()->DrawIndexedPrimitiveVB(d3dptPrimitiveType, lpd3dVertexBuffer, dwStartVertex, dwNumVertices, lpwIndices, dwIndexCount, dwFlags);

			// Handle dwFlags
			UnSetDrawFlags(rsClipping, rsLighting, rsExtents, BufferDesc.dwFVF, dwFlags, DirectXVersion);

			return hr;
		}
		else
		{
			return GetProxyInterfaceV7()->DrawIndexedPrimitiveVB(d3dptPrimitiveType, lpd3dVertexBuffer, dwStartVertex, dwNumVertices, lpwIndices, dwIndexCount, dwFlags);
		}
	}
}

HRESULT m_IDirect3DDeviceX::ComputeSphereVisibility(LPD3DVECTOR lpCenters, LPD3DVALUE lpRadii, DWORD dwNumSpheres, DWORD dwFlags, LPDWORD lpdwReturnValues)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lpCenters || !lpRadii || !dwNumSpheres || !lpdwReturnValues)
		{
			return DDERR_INVALIDPARAMS;
		}

		// Sphere visibility is computed by back-transforming the viewing frustum to the model space, using the inverse of the combined world, view, or projection matrices.
		// If the combined matrix can't be inverted (if the determinant is 0), the method will fail, returning D3DERR_INVALIDMATRIX.
		for (UINT x = 0; x < dwNumSpheres; x++)
		{
			// If a sphere is completely visible, the corresponding entry in lpdwReturnValues is 0.
			lpdwReturnValues[x] = 0;	// Just return all is visible for now
		}

		return D3D_OK;
	}

	switch (ProxyDirectXVersion)
	{
	case 1:
	case 2:
	default:
		return DDERR_GENERIC;
	case 3:
		return GetProxyInterfaceV3()->ComputeSphereVisibility(lpCenters, lpRadii, dwNumSpheres, dwFlags, lpdwReturnValues);
	case 7:
		return GetProxyInterfaceV7()->ComputeSphereVisibility(lpCenters, lpRadii, dwNumSpheres, dwFlags, lpdwReturnValues);
	}
}

HRESULT m_IDirect3DDeviceX::ValidateDevice(LPDWORD lpdwPasses)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lpdwPasses)
		{
			return DDERR_INVALIDPARAMS;
		}

		// Address to be filled with the number of rendering passes to complete the desired effect through multipass rendering.
		*lpdwPasses = 1;

		return DD_OK;
	}

	switch (ProxyDirectXVersion)
	{
	case 1:
	case 2:
	default:
		return DDERR_GENERIC;
	case 3:
		return GetProxyInterfaceV3()->ValidateDevice(lpdwPasses);
	case 7:
		return GetProxyInterfaceV7()->ValidateDevice(lpdwPasses);
	}
}

HRESULT m_IDirect3DDeviceX::ApplyStateBlock(DWORD dwBlockHandle)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!dwBlockHandle)
		{
			return DDERR_INVALIDPARAMS;
		}

		// ToDo: Validate BlockHandle
		return reinterpret_cast<IDirect3DStateBlock9*>(dwBlockHandle)->Apply();
	}

	return GetProxyInterfaceV7()->ApplyStateBlock(dwBlockHandle);
}

HRESULT m_IDirect3DDeviceX::CaptureStateBlock(DWORD dwBlockHandle)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!dwBlockHandle)
		{
			return DDERR_INVALIDPARAMS;
		}

		// ToDo: Validate BlockHandle
		return reinterpret_cast<IDirect3DStateBlock9*>(dwBlockHandle)->Capture();
	}

	return GetProxyInterfaceV7()->CaptureStateBlock(dwBlockHandle);
}

HRESULT m_IDirect3DDeviceX::DeleteStateBlock(DWORD dwBlockHandle)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!dwBlockHandle)
		{
			return DDERR_INVALIDPARAMS;
		}

		// ToDo: Validate BlockHandle
		reinterpret_cast<IDirect3DStateBlock9*>(dwBlockHandle)->Release();

		return DD_OK;
	}

	return GetProxyInterfaceV7()->DeleteStateBlock(dwBlockHandle);
}

HRESULT m_IDirect3DDeviceX::CreateStateBlock(D3DSTATEBLOCKTYPE d3dsbtype, LPDWORD lpdwBlockHandle)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lpdwBlockHandle)
		{
			return DDERR_INVALIDPARAMS;
		}

		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true)))
		{
			return DDERR_GENERIC;
		}

		// ToDo: Validate BlockHandle
		return (*d3d9Device)->CreateStateBlock(d3dsbtype, reinterpret_cast<IDirect3DStateBlock9**>(lpdwBlockHandle));
	}

	return GetProxyInterfaceV7()->CreateStateBlock(d3dsbtype, lpdwBlockHandle);
}

HRESULT m_IDirect3DDeviceX::SetClipStatus(LPD3DCLIPSTATUS lpD3DClipStatus)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	switch (ProxyDirectXVersion)
	{
	case 1:
	default:
		return DDERR_GENERIC;
	case 2:
		return GetProxyInterfaceV2()->SetClipStatus(lpD3DClipStatus);
	case 3:
		return GetProxyInterfaceV3()->SetClipStatus(lpD3DClipStatus);
	case 7:
		return GetProxyInterfaceV7()->SetClipStatus(lpD3DClipStatus);
	}
}

HRESULT m_IDirect3DDeviceX::GetClipStatus(LPD3DCLIPSTATUS lpD3DClipStatus)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	switch (ProxyDirectXVersion)
	{
	case 1:
	default:
		return DDERR_GENERIC;
	case 2:
		return GetProxyInterfaceV2()->GetClipStatus(lpD3DClipStatus);
	case 3:
		return GetProxyInterfaceV3()->GetClipStatus(lpD3DClipStatus);
	case 7:
		return GetProxyInterfaceV7()->GetClipStatus(lpD3DClipStatus);
	}
}

HRESULT m_IDirect3DDeviceX::SetClipPlane(DWORD dwIndex, D3DVALUE* pPlaneEquation)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true)))
		{
			return DDERR_GENERIC;
		}

		return (*d3d9Device)->SetClipPlane(dwIndex, pPlaneEquation);
	}

	return GetProxyInterfaceV7()->SetClipPlane(dwIndex, pPlaneEquation);
}

HRESULT m_IDirect3DDeviceX::GetClipPlane(DWORD dwIndex, D3DVALUE* pPlaneEquation)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!pPlaneEquation)
		{
			return DDERR_INVALIDPARAMS;
		}

		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true)))
		{
			return DDERR_GENERIC;
		}

		return (*d3d9Device)->SetClipPlane(dwIndex, pPlaneEquation);
	}

	return GetProxyInterfaceV7()->GetClipPlane(dwIndex, pPlaneEquation);
}

HRESULT m_IDirect3DDeviceX::GetInfo(DWORD dwDevInfoID, LPVOID pDevInfoStruct, DWORD dwSize)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		// This method is intended to be used for performance tracking and debugging during product development (on the debug version of DirectX). 
		// The method can succeed, returning S_FALSE, without retrieving device data.
		// This occurs when the retail version of the DirectX runtime is installed on the host system.
		return S_FALSE;
	}

	return GetProxyInterfaceV7()->GetInfo(dwDevInfoID, pDevInfoStruct, dwSize);
}

/************************/
/*** Helper functions ***/
/************************/

void m_IDirect3DDeviceX::InitDevice(DWORD DirectXVersion)
{
	WrapperInterface = new m_IDirect3DDevice((LPDIRECT3DDEVICE)ProxyInterface, this);
	WrapperInterface2 = new m_IDirect3DDevice2((LPDIRECT3DDEVICE2)ProxyInterface, this);
	WrapperInterface3 = new m_IDirect3DDevice3((LPDIRECT3DDEVICE3)ProxyInterface, this);
	WrapperInterface7 = new m_IDirect3DDevice7((LPDIRECT3DDEVICE7)ProxyInterface, this);

	if (!Config.Dd7to9)
	{
		return;
	}

	AddRef(DirectXVersion);
}

void m_IDirect3DDeviceX::ReleaseDevice()
{

	WrapperInterface->DeleteMe();
	WrapperInterface2->DeleteMe();
	WrapperInterface3->DeleteMe();
	WrapperInterface7->DeleteMe();

	// Teardown debug overlay
	if (Config.Dd7to9)
	{
#ifdef ENABLE_DEBUGOVERLAY
		DOverlay.Shutdown();
#endif
	}

	if (ddrawParent && !Config.Exiting)
	{
		ddrawParent->ClearD3DDevice();
	}
}

HRESULT m_IDirect3DDeviceX::CheckInterface(char *FunctionName, bool CheckD3DDevice)
{
	// Check for device
	if (!ddrawParent)
	{
		LOG_LIMIT(100, FunctionName << " Error: no ddraw parent!");
		return DDERR_GENERIC;
	}

	// Check for device, if not then create it
	if (CheckD3DDevice && (!d3d9Device || !*d3d9Device))
	{
		d3d9Device = ddrawParent->GetDirect3D9Device();

		// For concurrency
		SetCriticalSection();
		bool flag = (!d3d9Device || !*d3d9Device);
		ReleaseCriticalSection();

		// Create d3d9 device
		if (flag && FAILED(ddrawParent->CreateD3D9Device()))
		{
			LOG_LIMIT(100, FunctionName << " Error: d3d9 device not setup!");
			return DDERR_GENERIC;
		}

#ifdef ENABLE_DEBUGOVERLAY
		DOverlay.Setup(ddrawParent->GetHwnd(), *d3d9Device);
#endif
	}

	return DD_OK;
}

void m_IDirect3DDeviceX::ResetDevice()
{
	// Reset textures after device reset
	for (UINT x = 0; x < 8; x++)
	{
		if (AttachedTexture[x])
		{
			if (CheckSurfaceExists(AttachedTexture[x]))
			{
				SetTexture(x, AttachedTexture[x]);
			}
			else
			{
				AttachedTexture[x] = nullptr;
			}
		}
	}
}

void m_IDirect3DDeviceX::SetDrawFlags(DWORD &rsClipping, DWORD &rsLighting, DWORD &rsExtents, DWORD dwVertexTypeDesc, DWORD dwFlags, DWORD DirectXVersion)
{
	if (DirectXVersion != 7)
	{
		// Handle dwFlags
		if (dwFlags & D3DDP_DONOTCLIP)
		{
			GetRenderState(D3DRENDERSTATE_CLIPPING, &rsClipping);
			SetRenderState(D3DRENDERSTATE_CLIPPING, FALSE);
		}
		if ((dwFlags & D3DDP_DONOTLIGHT) || !(dwVertexTypeDesc & D3DFVF_NORMAL))
		{
			GetRenderState(D3DRENDERSTATE_LIGHTING, &rsLighting);
			SetRenderState(D3DRENDERSTATE_LIGHTING, FALSE);
		}
		if (dwFlags & D3DDP_DONOTUPDATEEXTENTS)
		{
			GetRenderState(D3DRENDERSTATE_EXTENTS, &rsExtents);
			SetRenderState(D3DRENDERSTATE_EXTENTS, FALSE);
		}
	}
}

void m_IDirect3DDeviceX::UnSetDrawFlags(DWORD rsClipping, DWORD rsLighting, DWORD rsExtents, DWORD dwVertexTypeDesc, DWORD dwFlags, DWORD DirectXVersion)
{
	if (DirectXVersion != 7)
	{
		// Handle dwFlags
		if (dwFlags & D3DDP_DONOTCLIP)
		{
			SetRenderState(D3DRENDERSTATE_CLIPPING, rsClipping);
		}
		if ((dwFlags & D3DDP_DONOTLIGHT) || !(dwVertexTypeDesc & D3DFVF_NORMAL))
		{
			SetRenderState(D3DRENDERSTATE_LIGHTING, rsLighting);
		}
		if (dwFlags & D3DDP_DONOTUPDATEEXTENTS)
		{
			SetRenderState(D3DRENDERSTATE_EXTENTS, rsExtents);
		}
	}
}

void m_IDirect3DDeviceX::CopyScaleVertex(LPVOID lpVertices, std::vector<D3DTLVERTEX>& pVert, DWORD dwVertexCount)
{
	if (!lpVertices)
	{
		return;
	}
	pVert.resize(dwVertexCount);
	memcpy(&pVert[0], lpVertices, sizeof(D3DTLVERTEX) * dwVertexCount);
	for (DWORD x = 0; x < dwVertexCount; x++)
	{
		pVert[x].sx = (D3DVALUE)(pVert[x].sx * ScaleDDWidthRatio) + ScaleDDPadX;
		pVert[x].sy = (D3DVALUE)(pVert[x].sy * ScaleDDHeightRatio) + ScaleDDPadY;
	}
}

UINT m_IDirect3DDeviceX::GetNumberOfPrimitives(D3DPRIMITIVETYPE dptPrimitiveType, DWORD dwVertexCount)
{
	return
		(dptPrimitiveType == D3DPT_POINTLIST) ? dwVertexCount :
		(dptPrimitiveType == D3DPT_LINELIST) ? dwVertexCount / 2 :
		(dptPrimitiveType == D3DPT_LINESTRIP) ? dwVertexCount - 1 :
		(dptPrimitiveType == D3DPT_TRIANGLELIST) ? dwVertexCount / 3 :
		(dptPrimitiveType == D3DPT_TRIANGLESTRIP) ? dwVertexCount - 2 :
		(dptPrimitiveType == D3DPT_TRIANGLEFAN) ? dwVertexCount - 2 :
		0;
}

UINT m_IDirect3DDeviceX::GetVertexStride(DWORD dwVertexTypeDesc)
{
	// Reserved:
	// #define D3DFVF_RESERVED0        0x001  // (DX7)
	// #define D3DFVF_RESERVED0        0x001  // (DX9)

	// #define D3DFVF_RESERVED1        0x020  // (DX7)
	// #define D3DFVF_PSIZE            0x020  // (DX9)

	// #define D3DFVF_RESERVED2        0xf000  // 4 reserved bits (DX7)
	// #define D3DFVF_RESERVED2        0x6000  // 2 reserved bits (DX9)

	// Check for unsupported vertex types
	DWORD UnSupportedVertexTypes = dwVertexTypeDesc & ~(D3DFVF_POSITION_MASK | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEXCOUNT_MASK);
	if (UnSupportedVertexTypes)
	{
		LOG_LIMIT(100, __FUNCTION__ " Warning: Unsupported FVF type: " << Logging::hex(UnSupportedVertexTypes));
	}

	return
		(((dwVertexTypeDesc & D3DFVF_POSITION_MASK) == D3DFVF_XYZ) ? sizeof(float) * 3 : 0) +
		(((dwVertexTypeDesc & D3DFVF_POSITION_MASK) == D3DFVF_XYZRHW) ? sizeof(float) * 4 : 0) +
		(((dwVertexTypeDesc & D3DFVF_POSITION_MASK) == D3DFVF_XYZB1) ? sizeof(float) * 4 : 0) +
		(((dwVertexTypeDesc & D3DFVF_POSITION_MASK) == D3DFVF_XYZB2) ? sizeof(float) * 5 : 0) +
		(((dwVertexTypeDesc & D3DFVF_POSITION_MASK) == D3DFVF_XYZB3) ? sizeof(float) * 6 : 0) +
		(((dwVertexTypeDesc & D3DFVF_POSITION_MASK) == D3DFVF_XYZB4) ? sizeof(float) * 6 + sizeof(DWORD) : 0) +
		(((dwVertexTypeDesc & D3DFVF_POSITION_MASK) == D3DFVF_XYZB5) ? sizeof(float) * 7 + sizeof(DWORD) : 0) +
		((dwVertexTypeDesc & D3DFVF_NORMAL) ? sizeof(float) * 3 : 0) +
		((dwVertexTypeDesc & D3DFVF_DIFFUSE) ? sizeof(D3DCOLOR) : 0) +
		((dwVertexTypeDesc & D3DFVF_SPECULAR) ? sizeof(D3DCOLOR) : 0) +
		(((dwVertexTypeDesc & D3DFVF_TEXCOUNT_MASK) == D3DFVF_TEX1) ? sizeof(float) * 2 : 0) +
		(((dwVertexTypeDesc & D3DFVF_TEXCOUNT_MASK) == D3DFVF_TEX2) ? sizeof(float) * 4 : 0) +
		(((dwVertexTypeDesc & D3DFVF_TEXCOUNT_MASK) == D3DFVF_TEX3) ? sizeof(float) * 6 : 0) +
		(((dwVertexTypeDesc & D3DFVF_TEXCOUNT_MASK) == D3DFVF_TEX4) ? sizeof(float) * 8 : 0) +
		(((dwVertexTypeDesc & D3DFVF_TEXCOUNT_MASK) == D3DFVF_TEX5) ? sizeof(float) * 10 : 0) +
		(((dwVertexTypeDesc & D3DFVF_TEXCOUNT_MASK) == D3DFVF_TEX6) ? sizeof(float) * 12 : 0) +
		(((dwVertexTypeDesc & D3DFVF_TEXCOUNT_MASK) == D3DFVF_TEX7) ? sizeof(float) * 14 : 0) +
		(((dwVertexTypeDesc & D3DFVF_TEXCOUNT_MASK) == D3DFVF_TEX8) ? sizeof(float) * 16 : 0) +
		0;
}
