#pragma once

class m_IDirectDrawSurface3 : public IDirectDrawSurface3, public AddressLookupTableDdrawObject
{
private:
	m_IDirectDrawSurfaceX *ProxyInterface;
	REFIID WrapperID = IID_IDirectDrawSurface3;
	const DWORD DirectXVersion = 3;
	ULONG RefCount = 1;
	const DWORD MipMapLevel;

public:
	m_IDirectDrawSurface3(m_IDirectDrawSurfaceX* Interface, DWORD Level) : ProxyInterface(Interface), MipMapLevel(Level)
	{
		ProxyAddressLookupTable.SaveAddress(this, ProxyInterface);
	}
	m_IDirectDrawSurface3(IDirectDrawSurface3 *, m_IDirectDrawSurfaceX *Interface) : ProxyInterface(Interface), MipMapLevel(0)
	{
		ProxyAddressLookupTable.SaveAddress(this, ProxyInterface);
	}
	~m_IDirectDrawSurface3()
	{
		ProxyAddressLookupTable.DeleteAddress(this);
	}

	void SetProxy(m_IDirectDrawSurfaceX* NewProxyInterface)
	{
		ProxyInterface = NewProxyInterface;
		if (NewProxyInterface)
		{
			ProxyAddressLookupTable.SaveAddress(this, ProxyInterface);
		}
		else
		{
			ProxyAddressLookupTable.DeleteAddress(this);
		}
	}

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * ppvObj);
	STDMETHOD_(ULONG, AddRef) (THIS);
	STDMETHOD_(ULONG, Release) (THIS);

	/*** IDirectDrawSurface methods ***/
	STDMETHOD(AddAttachedSurface)(THIS_ LPDIRECTDRAWSURFACE3);
	STDMETHOD(AddOverlayDirtyRect)(THIS_ LPRECT);
	STDMETHOD(Blt)(THIS_ LPRECT, LPDIRECTDRAWSURFACE3, LPRECT, DWORD, LPDDBLTFX);
	STDMETHOD(BltBatch)(THIS_ LPDDBLTBATCH, DWORD, DWORD);
	STDMETHOD(BltFast)(THIS_ DWORD, DWORD, LPDIRECTDRAWSURFACE3, LPRECT, DWORD);
	STDMETHOD(DeleteAttachedSurface)(THIS_ DWORD, LPDIRECTDRAWSURFACE3);
	STDMETHOD(EnumAttachedSurfaces)(THIS_ LPVOID, LPDDENUMSURFACESCALLBACK);
	STDMETHOD(EnumOverlayZOrders)(THIS_ DWORD, LPVOID, LPDDENUMSURFACESCALLBACK);
	STDMETHOD(Flip)(THIS_ LPDIRECTDRAWSURFACE3, DWORD);
	STDMETHOD(GetAttachedSurface)(THIS_ LPDDSCAPS, LPDIRECTDRAWSURFACE3 FAR *);
	STDMETHOD(GetBltStatus)(THIS_ DWORD);
	STDMETHOD(GetCaps)(THIS_ LPDDSCAPS);
	STDMETHOD(GetClipper)(THIS_ LPDIRECTDRAWCLIPPER FAR*);
	STDMETHOD(GetColorKey)(THIS_ DWORD, LPDDCOLORKEY);
	STDMETHOD(GetDC)(THIS_ HDC FAR *);
	STDMETHOD(GetFlipStatus)(THIS_ DWORD);
	STDMETHOD(GetOverlayPosition)(THIS_ LPLONG, LPLONG);
	STDMETHOD(GetPalette)(THIS_ LPDIRECTDRAWPALETTE FAR*);
	STDMETHOD(GetPixelFormat)(THIS_ LPDDPIXELFORMAT);
	STDMETHOD(GetSurfaceDesc)(THIS_ LPDDSURFACEDESC);
	STDMETHOD(Initialize)(THIS_ LPDIRECTDRAW, LPDDSURFACEDESC);
	STDMETHOD(IsLost)(THIS);
	STDMETHOD(Lock)(THIS_ LPRECT, LPDDSURFACEDESC, DWORD, HANDLE);
	STDMETHOD(ReleaseDC)(THIS_ HDC);
	STDMETHOD(Restore)(THIS);
	STDMETHOD(SetClipper)(THIS_ LPDIRECTDRAWCLIPPER);
	STDMETHOD(SetColorKey)(THIS_ DWORD, LPDDCOLORKEY);
	STDMETHOD(SetOverlayPosition)(THIS_ LONG, LONG);
	STDMETHOD(SetPalette)(THIS_ LPDIRECTDRAWPALETTE);
	STDMETHOD(Unlock)(THIS_ LPVOID);
	STDMETHOD(UpdateOverlay)(THIS_ LPRECT, LPDIRECTDRAWSURFACE3, LPRECT, DWORD, LPDDOVERLAYFX);
	STDMETHOD(UpdateOverlayDisplay)(THIS_ DWORD);
	STDMETHOD(UpdateOverlayZOrder)(THIS_ DWORD, LPDIRECTDRAWSURFACE3);
	/*** Added in the v2 interface ***/
	STDMETHOD(GetDDInterface)(THIS_ LPVOID FAR *);
	STDMETHOD(PageLock)(THIS_ DWORD);
	STDMETHOD(PageUnlock)(THIS_ DWORD);
	/*** Added in the V3 interface ***/
	STDMETHOD(SetSurfaceDesc)(THIS_ LPDDSURFACEDESC, DWORD);
};
