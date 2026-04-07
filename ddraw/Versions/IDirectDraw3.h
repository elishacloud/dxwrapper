#pragma once

class m_IDirectDraw3 final : public IDirectDraw3, public AddressLookupTableDdrawObject
{
private:
	m_IDirectDrawX *ProxyInterface;
	const IID WrapperID = IID_IDirectDraw3;
	const DWORD DirectXVersion = 3;

public:
	m_IDirectDraw3(IDirectDraw3 *, m_IDirectDrawX *Interface) : ProxyInterface(Interface)
	{
		ProxyAddressLookupTable.SaveAddress(this, ProxyInterface);
	}
	~m_IDirectDraw3()
	{
		ProxyAddressLookupTable.DeleteAddress(this);
	}

	void SetProxy(m_IDirectDrawX* NewProxyInterface)
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

	/*** IDirectDraw methods ***/
	IFACEMETHOD(Compact)(THIS) override;
	IFACEMETHOD(CreateClipper)(THIS_ DWORD, LPDIRECTDRAWCLIPPER FAR*, IUnknown FAR *) override;
	IFACEMETHOD(CreatePalette)(THIS_ DWORD, LPPALETTEENTRY, LPDIRECTDRAWPALETTE FAR*, IUnknown FAR *) override;
	IFACEMETHOD(CreateSurface)(THIS_ LPDDSURFACEDESC, LPDIRECTDRAWSURFACE FAR *, IUnknown FAR *) override;
	IFACEMETHOD(DuplicateSurface)(THIS_ LPDIRECTDRAWSURFACE, LPDIRECTDRAWSURFACE FAR *) override;
	IFACEMETHOD(EnumDisplayModes)(THIS_ DWORD, LPDDSURFACEDESC, LPVOID, LPDDENUMMODESCALLBACK) override;
	IFACEMETHOD(EnumSurfaces)(THIS_ DWORD, LPDDSURFACEDESC, LPVOID, LPDDENUMSURFACESCALLBACK) override;
	IFACEMETHOD(FlipToGDISurface)(THIS) override;
	IFACEMETHOD(GetCaps)(THIS_ LPDDCAPS, LPDDCAPS) override;
	IFACEMETHOD(GetDisplayMode)(THIS_ LPDDSURFACEDESC) override;
	IFACEMETHOD(GetFourCCCodes)(THIS_ LPDWORD, LPDWORD) override;
	IFACEMETHOD(GetGDISurface)(THIS_ LPDIRECTDRAWSURFACE FAR *) override;
	IFACEMETHOD(GetMonitorFrequency)(THIS_ LPDWORD) override;
	IFACEMETHOD(GetScanLine)(THIS_ LPDWORD) override;
	IFACEMETHOD(GetVerticalBlankStatus)(THIS_ LPBOOL) override;
	IFACEMETHOD(Initialize)(THIS_ GUID FAR *) override;
	IFACEMETHOD(RestoreDisplayMode)(THIS) override;
	IFACEMETHOD(SetCooperativeLevel)(THIS_ HWND, DWORD) override;
	IFACEMETHOD(SetDisplayMode)(THIS_ DWORD, DWORD, DWORD, DWORD, DWORD) override;
	IFACEMETHOD(WaitForVerticalBlank)(THIS_ DWORD, HANDLE) override;
	/*** Added in the v2 interface ***/
	IFACEMETHOD(GetAvailableVidMem)(THIS_ LPDDSCAPS, LPDWORD, LPDWORD) override;
	/*** IDirectDraw3 methods ***/
	IFACEMETHOD(GetSurfaceFromDC) (THIS_ HDC, IDirectDrawSurface **) override;
};
