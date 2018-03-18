#include "ddraw.h"

REFIID GetIID(REFIID CalledID)
{
	return (CalledID == CLSID_DirectDraw) ? IID_IDirectDraw :
		(CalledID == CLSID_DirectDraw7) ? IID_IDirectDraw7 :
		(CalledID == CLSID_DirectDrawClipper) ? IID_IDirectDrawClipper :
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

	return riid;
}

HRESULT ProxyQueryInterface(LPVOID ProxyInterface, REFIID CalledID, LPVOID * ppvObj, LPVOID m_pvObj)
{
	REFIID riid = GetIID(CalledID);

	if (riid == IID_IUnknown)
	{
		((IDirectDraw*)m_pvObj)->AddRef();
		*ppvObj = m_pvObj;
		return S_OK;
	}

	HRESULT hr = ((IDirectDraw*)ProxyInterface)->QueryInterface(ConvertREFIID(riid), ppvObj);

	if (SUCCEEDED(hr))
	{
		genericQueryInterface(riid, ppvObj);
	}

	return hr;
}

void genericQueryInterface(REFIID CalledID, LPVOID * ppvObj)
{
	REFIID riid = GetIID(CalledID);

#define QUERYINTERFACE(x) \
	if (riid == IID_ ## x) \
		{ \
			*ppvObj = ProxyAddressLookupTable.FindAddress<m_ ## x>(*ppvObj, ConvertREFIID(IID_ ## x)); \
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
