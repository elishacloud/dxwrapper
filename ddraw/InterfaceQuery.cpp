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

DWORD GetIIDVersion(REFIID riid)
{
	return (riid == IID_IDirectDraw || riid == IID_IDirectDrawSurface || riid == IID_IDirect3D || riid == IID_IDirect3DDevice ||
		riid == IID_IDirect3DMaterial || riid == IID_IDirect3DTexture || riid == IID_IDirect3DVertexBuffer || riid == IID_IDirect3DViewport) ? 1 :
		(riid == IID_IDirectDraw2 || riid == IID_IDirectDrawSurface2 || riid == IID_IDirect3D2 || riid == IID_IDirect3DDevice2 ||
			riid == IID_IDirect3DMaterial2 || riid == IID_IDirect3DTexture2 || riid == IID_IDirect3DViewport2) ? 2 :
		(riid == IID_IDirectDraw3 || riid == IID_IDirectDrawSurface3 || riid == IID_IDirect3D3 || riid == IID_IDirect3DDevice3 ||
			riid == IID_IDirect3DMaterial3 || riid == IID_IDirect3DViewport3) ? 3 :
		(riid == IID_IDirectDraw4 || riid == IID_IDirectDrawSurface4) ? 4 :
		(riid == IID_IDirectDraw7 || riid == IID_IDirectDrawSurface7 || riid == IID_IDirect3D7 || riid == IID_IDirect3DDevice7 ||
			riid == IID_IDirect3DVertexBuffer7) ? 7 : 7;
}

REFIID ConvertREFIID(REFIID riid)
{
	if (Config.ConvertToDirectDraw7)
	{
		if (riid == IID_IDirectDraw ||
			riid == IID_IDirectDraw2 ||
			riid == IID_IDirectDraw3 ||
			riid == IID_IDirectDraw4)
		{
			return IID_IDirectDraw7;
		}
		else if (riid == IID_IDirectDrawSurface ||
			riid == IID_IDirectDrawSurface2 ||
			riid == IID_IDirectDrawSurface3 ||
			riid == IID_IDirectDrawSurface4)
		{
			return IID_IDirectDrawSurface7;
		}
	}
	if (Config.ConvertToDirect3D7)
	{
		if (riid == IID_IDirect3D ||
			riid == IID_IDirect3D2 ||
			riid == IID_IDirect3D3)
		{
			return IID_IDirect3D7;
		}
		else if (riid == IID_IDirect3DDevice ||
			riid == IID_IDirect3DDevice2 ||
			riid == IID_IDirect3DDevice3)
		{
			return IID_IDirect3DDevice7;
		}
		else if (riid == IID_IDirect3DMaterial ||
			riid == IID_IDirect3DMaterial2)
		{
			return IID_IDirect3DMaterial3;
		}
		else if (riid == IID_IDirect3DTexture)
		{
			return IID_IDirect3DTexture2;
		}
		else if (riid == IID_IDirect3DViewport ||
			riid == IID_IDirect3DViewport2)
		{
			return IID_IDirect3DViewport3;
		}
		else if (riid == IID_IDirect3DVertexBuffer)
		{
			return IID_IDirect3DVertexBuffer7;
		}
	}

	return riid;
}

HRESULT ProxyQueryInterface(LPVOID ProxyInterface, REFIID riid, LPVOID * ppvObj, REFIID WrapperID, LPVOID WrapperInterface)
{
	Logging::LogDebug() << "Query for " << riid << " from " << WrapperID;

	if (!ppvObj)
	{
		return E_FAIL;
	}

	if (Config.Dd7to9 || !ProxyInterface)
	{
		if (riid == IID_IClassFactory)
		{
			*ppvObj = new m_IClassFactory(nullptr);
			return DD_OK;
		}
		if (riid == IID_IDirectDrawFactory)
		{
			*ppvObj = new m_IDirectDrawFactory(nullptr);
			return DD_OK;
		}
		if (riid == IID_IDirectDrawColorControl)
		{
			*ppvObj = new m_IDirectDrawColorControl(nullptr);
			return DD_OK;
		}

		Logging::Log() << __FUNCTION__ << " Query Not Implemented for " << riid << " from " << WrapperID;
		*ppvObj = nullptr;
		return E_NOINTERFACE;
	}

	if (riid == WrapperID || riid == IID_IUnknown)
	{
		((IUnknown*)ProxyInterface)->AddRef();

		*ppvObj = WrapperInterface;

		return DD_OK;
	}

	HRESULT hr = ((IUnknown*)ProxyInterface)->QueryInterface(ConvertREFIID(riid), ppvObj);

	if (SUCCEEDED(hr))
	{
		hr = genericQueryInterface(riid, ppvObj);
	}
	else
	{
		Logging::LogDebug() << "Query failed for " << riid << " Error " << hr;
	}

	return hr;
}

HRESULT genericQueryInterface(REFIID riid, LPVOID * ppvObj)
{
	if (!ppvObj)
	{
		return E_FAIL;
	}

#define QUERYINTERFACE(x) \
	if (riid == IID_ ## x) \
		{ \
			*ppvObj = ProxyAddressLookupTable.FindAddress<m_ ## x>(*ppvObj); \
			return DD_OK; \
		}

	QUERYINTERFACE(IClassFactory);
	QUERYINTERFACE(IDirectDrawFactory);
	QUERYINTERFACE(IDirect3D);
	QUERYINTERFACE(IDirect3D2);
	QUERYINTERFACE(IDirect3D3);
	QUERYINTERFACE(IDirect3D7);
	QUERYINTERFACE(IDirect3DDevice);
	QUERYINTERFACE(IDirect3DDevice2);
	QUERYINTERFACE(IDirect3DDevice3);
	QUERYINTERFACE(IDirect3DDevice7);
	QUERYINTERFACE(IDirect3DExecuteBuffer);
	QUERYINTERFACE(IDirect3DLight);
	QUERYINTERFACE(IDirect3DMaterial);
	QUERYINTERFACE(IDirect3DMaterial2);
	QUERYINTERFACE(IDirect3DMaterial3);
	QUERYINTERFACE(IDirect3DTexture);
	QUERYINTERFACE(IDirect3DTexture2);
	QUERYINTERFACE(IDirect3DVertexBuffer);
	QUERYINTERFACE(IDirect3DVertexBuffer7);
	QUERYINTERFACE(IDirect3DViewport);
	QUERYINTERFACE(IDirect3DViewport2);
	QUERYINTERFACE(IDirect3DViewport3);
	QUERYINTERFACE(IDirectDraw);
	QUERYINTERFACE(IDirectDraw2);
	QUERYINTERFACE(IDirectDraw3);
	QUERYINTERFACE(IDirectDraw4);
	QUERYINTERFACE(IDirectDraw7);
	QUERYINTERFACE(IDirectDrawClipper);
	QUERYINTERFACE(IDirectDrawColorControl);
	QUERYINTERFACE(IDirectDrawGammaControl);
	QUERYINTERFACE(IDirectDrawPalette);
	QUERYINTERFACE(IDirectDrawSurface);
	QUERYINTERFACE(IDirectDrawSurface2);
	QUERYINTERFACE(IDirectDrawSurface3);
	QUERYINTERFACE(IDirectDrawSurface4);
	QUERYINTERFACE(IDirectDrawSurface7);

	*ppvObj = nullptr;
	return E_NOINTERFACE;
}
