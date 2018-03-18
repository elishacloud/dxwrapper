#pragma once

class m_IDirectDrawSurfaceX
{
private:
	IDirectDrawSurface7 *ProxyInterface;
	DWORD DirectXVersion;
	DWORD ProxyDirectXVersion;
	REFIID RealIID;

public:
	m_IDirectDrawSurfaceX(IDirectDrawSurface7 *pOriginal, DWORD Version, REFIID riid) : ProxyInterface(pOriginal), DirectXVersion(Version), RealIID(riid)
	{
		if (riid == IID_IDirectDrawSurface) { ProxyDirectXVersion = 1; }
		else if (riid == IID_IDirectDrawSurface2) { ProxyDirectXVersion = 2; }
		else if (riid == IID_IDirectDrawSurface3) { ProxyDirectXVersion = 3; }
		else if (riid == IID_IDirectDrawSurface4) { ProxyDirectXVersion = 4; }
		else if (riid == IID_IDirectDrawSurface7) { ProxyDirectXVersion = 7; }
		else { ProxyDirectXVersion = DirectXVersion; }
	}
	~m_IDirectDrawSurfaceX() { }

	DWORD GetDirectXVersion() { return DirectXVersion; }

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
	HRESULT EnumAttachedSurfaces(LPVOID a, T b);
	template <typename T>
	HRESULT EnumOverlayZOrders(DWORD a, LPVOID b, T c);
	STDMETHOD(Flip)(THIS_ LPDIRECTDRAWSURFACE7, DWORD);
	template <typename T>
	HRESULT GetAttachedSurface(T a, LPDIRECTDRAWSURFACE7 FAR * b);
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
	HRESULT GetSurfaceDesc(T a);
	template <typename T>
	HRESULT Initialize(LPDIRECTDRAW a, T b);
	STDMETHOD(IsLost)(THIS);
	template <typename T>
	HRESULT Lock(LPRECT a, T b, DWORD c, HANDLE d);
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
	HRESULT SetSurfaceDesc(T a, DWORD b);
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
