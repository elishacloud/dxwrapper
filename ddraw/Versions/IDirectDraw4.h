#pragma once

class m_IDirectDraw4 final : public IDirectDraw4, public AddressLookupTableDdrawObject
{
private:
	m_IDirectDrawX *ProxyInterface;
	const IID WrapperID = IID_IDirectDraw4;
	const DWORD DirectXVersion = 4;

public:
	m_IDirectDraw4(IDirectDraw4 *, m_IDirectDrawX *Interface) : ProxyInterface(Interface)
	{
		ProxyAddressLookupTable.SaveAddress(this, ProxyInterface);
	}
	~m_IDirectDraw4()
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
	IFACEMETHOD(CreateSurface)(THIS_ LPDDSURFACEDESC2, LPDIRECTDRAWSURFACE4 FAR *, IUnknown FAR *) override;
	IFACEMETHOD(DuplicateSurface)(THIS_ LPDIRECTDRAWSURFACE4, LPDIRECTDRAWSURFACE4 FAR *) override;
	IFACEMETHOD(EnumDisplayModes)(THIS_ DWORD, LPDDSURFACEDESC2, LPVOID, LPDDENUMMODESCALLBACK2) override;
	IFACEMETHOD(EnumSurfaces)(THIS_ DWORD, LPDDSURFACEDESC2, LPVOID, LPDDENUMSURFACESCALLBACK2) override;
	IFACEMETHOD(FlipToGDISurface)(THIS) override;
	IFACEMETHOD(GetCaps)(THIS_ LPDDCAPS, LPDDCAPS) override;
	IFACEMETHOD(GetDisplayMode)(THIS_ LPDDSURFACEDESC2) override;
	IFACEMETHOD(GetFourCCCodes)(THIS_ LPDWORD, LPDWORD) override;
	IFACEMETHOD(GetGDISurface)(THIS_ LPDIRECTDRAWSURFACE4 FAR *) override;
	IFACEMETHOD(GetMonitorFrequency)(THIS_ LPDWORD) override;
	IFACEMETHOD(GetScanLine)(THIS_ LPDWORD) override;
	IFACEMETHOD(GetVerticalBlankStatus)(THIS_ LPBOOL) override;
	IFACEMETHOD(Initialize)(THIS_ GUID FAR *) override;
	IFACEMETHOD(RestoreDisplayMode)(THIS) override;
	IFACEMETHOD(SetCooperativeLevel)(THIS_ HWND, DWORD) override;
	IFACEMETHOD(SetDisplayMode)(THIS_ DWORD, DWORD, DWORD, DWORD, DWORD) override;
	IFACEMETHOD(WaitForVerticalBlank)(THIS_ DWORD, HANDLE) override;
	/*** Added in the v2 interface ***/
	IFACEMETHOD(GetAvailableVidMem)(THIS_ LPDDSCAPS2, LPDWORD, LPDWORD) override;
	/*** Added in the V4 Interface ***/
	IFACEMETHOD(GetSurfaceFromDC) (THIS_ HDC, LPDIRECTDRAWSURFACE4 *) override;
	IFACEMETHOD(RestoreAllSurfaces)(THIS) override;
	IFACEMETHOD(TestCooperativeLevel)(THIS) override;
	IFACEMETHOD(GetDeviceIdentifier)(THIS_ LPDDDEVICEIDENTIFIER, DWORD) override;
};
