#pragma once

class m_IDirectDrawSurfaceX
{
private:
	IDirectDrawSurface7 *ProxyInterface;
	m_IDirectDrawSurface7 *WrapperInterface;
	DWORD DirectXVersion;
	DWORD ProxyDirectXVersion;
	IID WrapperID;

public:
	m_IDirectDrawSurfaceX(IDirectDrawSurface7 *pOriginal, DWORD Version, m_IDirectDrawSurface7 *Interface) : ProxyInterface(pOriginal), DirectXVersion(Version), WrapperInterface(Interface)
	{
		WrapperID = (DirectXVersion == 1) ? IID_IDirectDrawSurface :
			(DirectXVersion == 2) ? IID_IDirectDrawSurface2 :
			(DirectXVersion == 3) ? IID_IDirectDrawSurface3 :
			(DirectXVersion == 4) ? IID_IDirectDrawSurface4 :
			(DirectXVersion == 7) ? IID_IDirectDrawSurface7 : IID_IDirectDrawSurface7;

		REFIID ProxyID = ConvertREFIID(WrapperID);
		ProxyDirectXVersion = (ProxyID == IID_IDirectDrawSurface) ? 1 :
			(ProxyID == IID_IDirectDrawSurface2) ? 2 :
			(ProxyID == IID_IDirectDrawSurface3) ? 3 :
			(ProxyID == IID_IDirectDrawSurface4) ? 4 :
			(ProxyID == IID_IDirectDrawSurface7) ? 7 : 7;

		if (ProxyDirectXVersion != DirectXVersion)
		{
			Logging::LogDebug() << "Convert DirectDrawSurface v" << DirectXVersion << " to v" << ProxyDirectXVersion;
		}
	}
	~m_IDirectDrawSurfaceX() {}

	DWORD GetDirectXVersion() { return DDWRAPPER_TYPEX; }
	REFIID GetWrapperType() { return WrapperID; }
	IDirectDrawSurface7 *GetProxyInterface() { return ProxyInterface; }
	m_IDirectDrawSurface7 *GetWrapperInterface() { return WrapperInterface; }

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * ppvObj);
	STDMETHOD_(ULONG, AddRef) (THIS);
	STDMETHOD_(ULONG, Release) (THIS);
	/*** IDirectDrawSurface methods ***/
	STDMETHOD(AddAttachedSurface)(THIS_ LPDIRECTDRAWSURFACE7);
	STDMETHOD(AddOverlayDirtyRect)(THIS_ LPRECT);
	STDMETHOD(Blt)(THIS_ LPRECT, LPDIRECTDRAWSURFACE7, LPRECT, DWORD, LPDDBLTFX);
	STDMETHOD(BltBatch)(THIS_ LPDDBLTBATCH, DWORD, DWORD);
	STDMETHOD(BltFast)(THIS_ DWORD, DWORD, LPDIRECTDRAWSURFACE7, LPRECT, DWORD);
	STDMETHOD(DeleteAttachedSurface)(THIS_ DWORD, LPDIRECTDRAWSURFACE7);
	template <typename T>
	HRESULT EnumAttachedSurfaces(LPVOID, T);
	template <typename T>
	HRESULT EnumOverlayZOrders(DWORD, LPVOID, T);
	STDMETHOD(Flip)(THIS_ LPDIRECTDRAWSURFACE7, DWORD);
	template <typename T>
	HRESULT GetAttachedSurface(T, LPDIRECTDRAWSURFACE7 FAR *);
	STDMETHOD(GetBltStatus)(THIS_ DWORD);
	template <typename T>
	HRESULT GetCaps(T lpDDSCaps);
	STDMETHOD(GetClipper)(THIS_ LPDIRECTDRAWCLIPPER FAR*);
	STDMETHOD(GetColorKey)(THIS_ DWORD, LPDDCOLORKEY);
	STDMETHOD(GetDC)(THIS_ HDC FAR *);
	STDMETHOD(GetFlipStatus)(THIS_ DWORD);
	STDMETHOD(GetOverlayPosition)(THIS_ LPLONG, LPLONG);
	STDMETHOD(GetPalette)(THIS_ LPDIRECTDRAWPALETTE FAR*);
	STDMETHOD(GetPixelFormat)(THIS_ LPDDPIXELFORMAT);
	template <typename T>
	HRESULT GetSurfaceDesc(T);
	template <typename T>
	HRESULT Initialize(LPDIRECTDRAW, T);
	STDMETHOD(IsLost)(THIS);
	template <typename T>
	HRESULT Lock(LPRECT, T, DWORD, HANDLE);
	STDMETHOD(ReleaseDC)(THIS_ HDC);
	STDMETHOD(Restore)(THIS);
	STDMETHOD(SetClipper)(THIS_ LPDIRECTDRAWCLIPPER);
	STDMETHOD(SetColorKey)(THIS_ DWORD, LPDDCOLORKEY);
	STDMETHOD(SetOverlayPosition)(THIS_ LONG, LONG);
	STDMETHOD(SetPalette)(THIS_ LPDIRECTDRAWPALETTE);
	STDMETHOD(Unlock)(THIS_ LPRECT);
	STDMETHOD(UpdateOverlay)(THIS_ LPRECT, LPDIRECTDRAWSURFACE7, LPRECT, DWORD, LPDDOVERLAYFX);
	STDMETHOD(UpdateOverlayDisplay)(THIS_ DWORD);
	STDMETHOD(UpdateOverlayZOrder)(THIS_ DWORD, LPDIRECTDRAWSURFACE7);
	/*** Added in the v2 interface ***/
	STDMETHOD(GetDDInterface)(THIS_ LPVOID FAR *);
	STDMETHOD(PageLock)(THIS_ DWORD);
	STDMETHOD(PageUnlock)(THIS_ DWORD);
	/*** Added in the v3 interface ***/
	template <typename T>
	HRESULT SetSurfaceDesc(T, DWORD);
	/*** Added in the v4 interface ***/
	STDMETHOD(SetPrivateData)(THIS_ REFGUID, LPVOID, DWORD, DWORD);
	STDMETHOD(GetPrivateData)(THIS_ REFGUID, LPVOID, LPDWORD);
	STDMETHOD(FreePrivateData)(THIS_ REFGUID);
	STDMETHOD(GetUniquenessValue)(THIS_ LPDWORD);
	STDMETHOD(ChangeUniquenessValue)(THIS);
	/*** Moved Texture7 methods here ***/
	STDMETHOD(SetPriority)(THIS_ DWORD);
	STDMETHOD(GetPriority)(THIS_ LPDWORD);
	STDMETHOD(SetLOD)(THIS_ DWORD);
	STDMETHOD(GetLOD)(THIS_ LPDWORD);
};
