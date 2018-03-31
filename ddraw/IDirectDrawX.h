#pragma once

class m_IDirectDrawX
{
private:
	IDirectDraw7 *ProxyInterface;
	IDirectDraw7 *WrapperInterface;
	DWORD DirectXVersion;
	DWORD ProxyDirectXVersion;
	IID WrapperID;

public:
	m_IDirectDrawX(IDirectDraw7 *aOriginal, DWORD Version, IDirectDraw7 *Interface) : ProxyInterface(aOriginal), DirectXVersion(Version), WrapperInterface(Interface)
	{
		switch (DirectXVersion)
		{
		case 1:
			WrapperID = IID_IDirectDraw;
			break;
		case 2:
			WrapperID = IID_IDirectDraw2;
			break;
		case 3:
			WrapperID = IID_IDirectDraw3;
			break;
		case 4:
			WrapperID = IID_IDirectDraw4;
			break;
		default:
			WrapperID = IID_IDirectDraw7;
			break;
		}

		REFIID ProxyID = ConvertREFIID(WrapperID);
		if (ProxyID == IID_IDirectDraw)
		{
			ProxyDirectXVersion = 1;
		}
		else if (ProxyID == IID_IDirectDraw2)
		{
			ProxyDirectXVersion = 2;
		}
		else if (ProxyID == IID_IDirectDraw3)
		{
			ProxyDirectXVersion = 3;
		}
		else if (ProxyID == IID_IDirectDraw4)
		{
			ProxyDirectXVersion = 4;
		}
		else if (ProxyID == IID_IDirectDraw7)
		{
			ProxyDirectXVersion = 7;
		}
		else
		{
			ProxyDirectXVersion = DirectXVersion;
		}

		if (ProxyDirectXVersion != DirectXVersion)
		{
			Logging::Log() << "Convert DirectDraw v" << DirectXVersion << " to v" << ProxyDirectXVersion;
		}
	}
	~m_IDirectDrawX() { }

	DWORD GetDirectXVersion() { return DirectXVersion; }

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * ppvObj);
	STDMETHOD_(ULONG, AddRef) (THIS);
	STDMETHOD_(ULONG, Release) (THIS);
	/*** IDirectDraw methods ***/
	STDMETHOD(Compact)(THIS);
	STDMETHOD(CreateClipper)(THIS_ DWORD, LPDIRECTDRAWCLIPPER FAR*, IUnknown FAR *);
	STDMETHOD(CreatePalette)(THIS_ DWORD, LPPALETTEENTRY, LPDIRECTDRAWPALETTE FAR*, IUnknown FAR *);
	template <typename T>
	HRESULT CreateSurface(T a, LPDIRECTDRAWSURFACE7 FAR * b, IUnknown FAR * c);
	STDMETHOD(DuplicateSurface)(THIS_ LPDIRECTDRAWSURFACE7, LPDIRECTDRAWSURFACE7 FAR *);
	template <typename T, typename D>
	HRESULT EnumDisplayModes(DWORD a, T b, LPVOID c, D d);
	template <typename T, typename D>
	HRESULT EnumSurfaces(DWORD a, T b, LPVOID c, D d);
	STDMETHOD(FlipToGDISurface)(THIS);
	STDMETHOD(GetCaps)(THIS_ LPDDCAPS, LPDDCAPS);
	template <typename T>
	HRESULT GetDisplayMode(T a);
	STDMETHOD(GetFourCCCodes)(THIS_ LPDWORD, LPDWORD);
	STDMETHOD(GetGDISurface)(THIS_ LPDIRECTDRAWSURFACE7 FAR *);
	STDMETHOD(GetMonitorFrequency)(THIS_ LPDWORD);
	STDMETHOD(GetScanLine)(THIS_ LPDWORD);
	STDMETHOD(GetVerticalBlankStatus)(THIS_ LPBOOL);
	STDMETHOD(Initialize)(THIS_ GUID FAR *);
	STDMETHOD(RestoreDisplayMode)(THIS);
	STDMETHOD(SetCooperativeLevel)(THIS_ HWND, DWORD);
	STDMETHOD(SetDisplayMode)(THIS_ DWORD, DWORD, DWORD, DWORD, DWORD);
	STDMETHOD(WaitForVerticalBlank)(THIS_ DWORD, HANDLE);
	/*** Added in the v2 interface ***/
	template <typename T>
	HRESULT GetAvailableVidMem(T a, LPDWORD b, LPDWORD c);
	/*** Added in the V4 Interface ***/
	STDMETHOD(GetSurfaceFromDC) (THIS_ HDC, LPDIRECTDRAWSURFACE7 *);
	STDMETHOD(RestoreAllSurfaces)(THIS);
	STDMETHOD(TestCooperativeLevel)(THIS);
	template <typename T>
	HRESULT GetDeviceIdentifier(T lpdddi, DWORD dwFlags);
	STDMETHOD(StartModeTest)(THIS_ LPSIZE, DWORD, DWORD);
	STDMETHOD(EvaluateMode)(THIS_ DWORD, DWORD *);
};
