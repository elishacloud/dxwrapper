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
*/

#include "ddraw.h"

REFIID GetIID(REFIID CalledID)
{
	return (CalledID == CLSID_DirectDraw) ? IID_IDirectDraw :
		(CalledID == CLSID_DirectDraw7) ? IID_IDirectDraw7 :
		(CalledID == CLSID_DirectDrawClipper) ? IID_IDirectDrawClipper :
		(CalledID == CLSID_DirectDrawFactory) ? IID_IDirectDrawFactory :
		CalledID;
}

REFIID ConvertREFIID(REFIID CalledID)
{
	REFIID riid = GetIID(CalledID);

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

HRESULT ProxyQueryInterface(LPVOID ProxyInterface, REFIID CalledID, LPVOID * ppvObj, REFIID WrapperID, LPVOID WrapperInterface)
{
	Logging::LogDebug() << "Query for " << CalledID << " from " << WrapperID;

	REFIID riid = GetIID(CalledID);

	if (Config.Dd7to9 || !ProxyInterface)
	{
		Logging::Log() << __FUNCTION__ << " Query Not Implemented for " << CalledID << " from " << WrapperID;

		return E_NOINTERFACE;
	}

	if ((riid == WrapperID || riid == IID_IUnknown) && ppvObj)
	{
		((IUnknown*)ProxyInterface)->AddRef();

		*ppvObj = WrapperInterface;

		return S_OK;
	}

	HRESULT hr = ((IUnknown*)ProxyInterface)->QueryInterface(ConvertREFIID(riid), ppvObj);

	if (SUCCEEDED(hr))
	{
		genericQueryInterface(riid, ppvObj);
	}
	else
	{
		if (riid == IID_IDirectDrawColorControl)
		{
			*ppvObj = new m_IDirectDrawColorControl(nullptr);

			return DD_OK;
		}

		Logging::LogDebug() << "Query failed for " << riid << " Error " << hr;
	}

	return hr;
}

void genericQueryInterface(REFIID CalledID, LPVOID * ppvObj)
{
	REFIID riid = GetIID(CalledID);

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
	QUERYINTERFACE(IDirectDrawFactory);
	QUERYINTERFACE(IDirectDrawGammaControl);
	QUERYINTERFACE(IDirectDrawPalette);
	QUERYINTERFACE(IDirectDrawSurface);
	QUERYINTERFACE(IDirectDrawSurface2);
	QUERYINTERFACE(IDirectDrawSurface3);
	QUERYINTERFACE(IDirectDrawSurface4);
	QUERYINTERFACE(IDirectDrawSurface7);
}
