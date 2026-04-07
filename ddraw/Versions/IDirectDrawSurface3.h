#pragma once

class m_IDirectDrawSurface3 final : public IDirectDrawSurface3, public AddressLookupTableDdrawObject
{
private:
	m_IDirectDrawSurfaceX *ProxyInterface;
	const IID WrapperID = IID_IDirectDrawSurface3;
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
	IFACEMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * ppvObj) override;
	IFACEMETHOD_(ULONG, AddRef) (THIS) override;
	IFACEMETHOD_(ULONG, Release) (THIS) override;

	/*** IDirectDrawSurface methods ***/
	IFACEMETHOD(AddAttachedSurface)(THIS_ LPDIRECTDRAWSURFACE3) override;
	IFACEMETHOD(AddOverlayDirtyRect)(THIS_ LPRECT) override;
	IFACEMETHOD(Blt)(THIS_ LPRECT, LPDIRECTDRAWSURFACE3, LPRECT, DWORD, LPDDBLTFX) override;
	IFACEMETHOD(BltBatch)(THIS_ LPDDBLTBATCH, DWORD, DWORD) override;
	IFACEMETHOD(BltFast)(THIS_ DWORD, DWORD, LPDIRECTDRAWSURFACE3, LPRECT, DWORD) override;
	IFACEMETHOD(DeleteAttachedSurface)(THIS_ DWORD, LPDIRECTDRAWSURFACE3) override;
	IFACEMETHOD(EnumAttachedSurfaces)(THIS_ LPVOID, LPDDENUMSURFACESCALLBACK) override;
	IFACEMETHOD(EnumOverlayZOrders)(THIS_ DWORD, LPVOID, LPDDENUMSURFACESCALLBACK) override;
	IFACEMETHOD(Flip)(THIS_ LPDIRECTDRAWSURFACE3, DWORD) override;
	IFACEMETHOD(GetAttachedSurface)(THIS_ LPDDSCAPS, LPDIRECTDRAWSURFACE3 FAR *) override;
	IFACEMETHOD(GetBltStatus)(THIS_ DWORD) override;
	IFACEMETHOD(GetCaps)(THIS_ LPDDSCAPS) override;
	IFACEMETHOD(GetClipper)(THIS_ LPDIRECTDRAWCLIPPER FAR*) override;
	IFACEMETHOD(GetColorKey)(THIS_ DWORD, LPDDCOLORKEY) override;
	IFACEMETHOD(GetDC)(THIS_ HDC FAR *) override;
	IFACEMETHOD(GetFlipStatus)(THIS_ DWORD) override;
	IFACEMETHOD(GetOverlayPosition)(THIS_ LPLONG, LPLONG) override;
	IFACEMETHOD(GetPalette)(THIS_ LPDIRECTDRAWPALETTE FAR*) override;
	IFACEMETHOD(GetPixelFormat)(THIS_ LPDDPIXELFORMAT) override;
	IFACEMETHOD(GetSurfaceDesc)(THIS_ LPDDSURFACEDESC) override;
	IFACEMETHOD(Initialize)(THIS_ LPDIRECTDRAW, LPDDSURFACEDESC) override;
	IFACEMETHOD(IsLost)(THIS) override;
	IFACEMETHOD(Lock)(THIS_ LPRECT, LPDDSURFACEDESC, DWORD, HANDLE) override;
	IFACEMETHOD(ReleaseDC)(THIS_ HDC) override;
	IFACEMETHOD(Restore)(THIS) override;
	IFACEMETHOD(SetClipper)(THIS_ LPDIRECTDRAWCLIPPER) override;
	IFACEMETHOD(SetColorKey)(THIS_ DWORD, LPDDCOLORKEY) override;
	IFACEMETHOD(SetOverlayPosition)(THIS_ LONG, LONG) override;
	IFACEMETHOD(SetPalette)(THIS_ LPDIRECTDRAWPALETTE) override;
	IFACEMETHOD(Unlock)(THIS_ LPVOID) override;
	IFACEMETHOD(UpdateOverlay)(THIS_ LPRECT, LPDIRECTDRAWSURFACE3, LPRECT, DWORD, LPDDOVERLAYFX) override;
	IFACEMETHOD(UpdateOverlayDisplay)(THIS_ DWORD) override;
	IFACEMETHOD(UpdateOverlayZOrder)(THIS_ DWORD, LPDIRECTDRAWSURFACE3) override;
	/*** Added in the v2 interface ***/
	IFACEMETHOD(GetDDInterface)(THIS_ LPVOID FAR *) override;
	IFACEMETHOD(PageLock)(THIS_ DWORD) override;
	IFACEMETHOD(PageUnlock)(THIS_ DWORD) override;
	/*** Added in the V3 interface ***/
	IFACEMETHOD(SetSurfaceDesc)(THIS_ LPDDSURFACEDESC, DWORD) override;
};
