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

#include "ddraw.h"
#include "DirectShow\IAMMediaStream.h"
#include "IClassFactory\IClassFactory.h"

DWORD DdrawWrapper::GetGUIDVersion(REFIID riid)
{
	return (riid == IID_IDirectDraw || riid == IID_IDirectDrawSurface || riid == IID_IDirect3D || riid == IID_IDirect3DDevice ||
		riid == IID_IDirect3DMaterial || riid == IID_IDirect3DTexture || riid == IID_IDirect3DVertexBuffer || riid == IID_IDirect3DViewport ||
		riid == IID_IDirectDrawClipper || riid == IID_IDirectDrawColorControl || riid == IID_IDirectDrawGammaControl ||
		riid == IID_IDirectDrawPalette || riid == IID_IDirectDrawFactory || riid == IID_IDirect3DLight || riid == IID_IDirect3DExecuteBuffer) ? 1 :
		(riid == IID_IDirectDraw2 || riid == IID_IDirectDrawSurface2 || riid == IID_IDirect3D2 || riid == IID_IDirect3DDevice2 ||
			riid == IID_IDirect3DMaterial2 || riid == IID_IDirect3DTexture2 || riid == IID_IDirect3DViewport2) ? 2 :
		(riid == IID_IDirectDraw3 || riid == IID_IDirectDrawSurface3 || riid == IID_IDirect3D3 || riid == IID_IDirect3DDevice3 ||
			riid == IID_IDirect3DMaterial3 || riid == IID_IDirect3DViewport3) ? 3 :
		(riid == IID_IDirectDraw4 || riid == IID_IDirectDrawSurface4) ? 4 :
		(riid == IID_IDirectDraw7 || riid == IID_IDirectDrawSurface7 || riid == IID_IDirect3D7 || riid == IID_IDirect3DDevice7 ||
			riid == IID_IDirect3DVertexBuffer7) ? 7 : 0;
}

REFIID DdrawWrapper::ReplaceIIDUnknown(REFIID riid, REFIID guid)
{
	return (riid == IID_IUnknown) ? guid : riid;
}

HRESULT DdrawWrapper::ProxyQueryInterface(LPVOID ProxyInterface, REFIID riid, LPVOID * ppvObj, REFIID WrapperID)
{
	Logging::LogDebug() << __FUNCTION__ << " Query for " << riid << " from " << WrapperID;

	if (!ppvObj)
	{
		return E_POINTER;
	}
	*ppvObj = nullptr;

	if (Config.Dd7to9 || !ProxyInterface)
	{
		genericQueryInterface(riid, ppvObj);

		if (*ppvObj)
		{
			return DD_OK;
		}

		LOG_LIMIT(100, __FUNCTION__ << " Query Not Implemented for " << riid << " from " << WrapperID);

		return E_NOINTERFACE;
	}

	HRESULT hr = ((IUnknown*)ProxyInterface)->QueryInterface(riid, ppvObj);

	if (SUCCEEDED(hr))
	{
		genericQueryInterface(riid, ppvObj);
	}
	else
	{
		genericQueryInterface(riid, ppvObj);

		if (*ppvObj)
		{
			return DD_OK;
		}

		LOG_LIMIT(100, __FUNCTION__ << " Query failed for " << riid << " Error: " << (DDERR)hr);
	}

	return hr;
}

void WINAPI DdrawWrapper::genericQueryInterface(REFIID riid, LPVOID *ppvObj)
{
	if (!ppvObj)
	{
		return;
	}

	if (!*ppvObj)
	{
		if (Config.DirectShowEmulation && (riid == IID_IMediaStream || riid == IID_IAMMediaStream))
		{
			*ppvObj = new m_IAMMediaStream(riid);
		}
		else if (riid == IID_IClassFactory)
		{
			*ppvObj = new m_IClassFactory(nullptr, genericQueryInterface);
		}
		else if (riid == IID_IDirectDrawFactory)
		{
			*ppvObj = new m_IDirectDrawFactory(nullptr);
		}

		if (*ppvObj && !Config.Dd7to9)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: Emulating empty class: " << riid);
		}

		return;
	}

#define QUERYINTERFACE(x) \
	if (riid == IID_ ## x) \
		{ \
			*ppvObj = ProxyAddressLookupTable.FindAddress<m_ ## x>(*ppvObj); \
		}

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

	if (riid == IID_IClassFactory)
	{
		*ppvObj = new m_IClassFactory((IClassFactory*)*ppvObj, genericQueryInterface);
	}
	if (riid == IID_IDirectDrawFactory)
	{
		*ppvObj = new m_IDirectDrawFactory((IDirectDrawFactory*)*ppvObj);
	}
}
