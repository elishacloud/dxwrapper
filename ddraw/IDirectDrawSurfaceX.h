#pragma once

class m_IDirectDrawSurfaceX : public IDirectDrawSurface7
{
private:
	IDirectDrawSurface7 *ProxyInterface;
	m_IDirectDrawSurface7 *WrapperInterface;
	DWORD DirectXVersion;
	DWORD ProxyDirectXVersion;
	IID WrapperID;
	ULONG RefCount = 1;

	// Convert to d3d9
	m_IDirectDrawX *ddrawParent = nullptr;
	LPDIRECT3DDEVICE9 *d3d9Device = nullptr;
	DDSURFACEDESC2 surfaceDesc;
	D3DLOCKED_RECT d3dlrect = { 0, nullptr };
	DWORD surfaceWidth = 0;
	DWORD surfaceHeight = 0;
	DDCOLORKEY colorKeys[4];		// Color keys(DDCKEY_DESTBLT, DDCKEY_DESTOVERLAY, DDCKEY_SRCBLT, DDCKEY_SRCOVERLAY)
	LONG overlayX = 0;
	LONG overlayY = 0;
	m_IDirectDrawPalette *attachedPalette = nullptr;	// Associated palette
	UINT32 *rgbVideoBuf = nullptr;						// RGB video buffer
	BYTE *rawVideoBuf = nullptr;						// Virtual video buffer
	DWORD BufferSize = 0;
	RECT DestRect;

public:
	m_IDirectDrawSurfaceX(IDirectDrawSurface7 *pOriginal, DWORD Version, m_IDirectDrawSurface7 *Interface) : ProxyInterface(pOriginal), DirectXVersion(Version), WrapperInterface(Interface)
	{
		InitWrapper();
	}
	m_IDirectDrawSurfaceX(LPDIRECT3DDEVICE9 *lplpd3d9Device, m_IDirectDrawX *Interface, DWORD Version, LPDDSURFACEDESC2 lpDDSurfaceDesc, DWORD Width, DWORD Height) :
		d3d9Device(lplpd3d9Device), ddrawParent(Interface), DirectXVersion(Version), surfaceWidth(Width), surfaceHeight(Height)
	{
		ProxyInterface = nullptr;
		WrapperInterface = nullptr;

		InitWrapper();

		// Init color keys
		for (int i = 0; i < 4; i++)
		{
			memset(&colorKeys[i], 0, sizeof(DDCOLORKEY));
		}

		// Alocate memory for video buffer
		AlocateVideoBuffer();

		// Copy surface description
		memcpy(&surfaceDesc, lpDDSurfaceDesc, sizeof(DDSURFACEDESC2));
	}
	~m_IDirectDrawSurfaceX()
	{
		FreeVideoBuffer();
	}

	void FreeVideoBuffer()
	{
		if (rawVideoBuf)
		{
			delete rawVideoBuf;
			rawVideoBuf = nullptr;
		}
		if (rgbVideoBuf)
		{
			delete rgbVideoBuf;
			rgbVideoBuf = nullptr;
		}
	}

	void AlocateVideoBuffer()
	{
		// Free memory for internal structures in case there is an old one
		FreeVideoBuffer();

		// Buffer size
		BufferSize = surfaceWidth * surfaceHeight;

		// Allocate the raw video buffer
		rawVideoBuf = new BYTE[BufferSize * 4];

		// Clear raw memory
		ZeroMemory(rawVideoBuf, BufferSize * 4 * sizeof(BYTE));

		// Allocate virtual video memory RGB
		rgbVideoBuf = new UINT32[BufferSize];

		// Clear rgb memory
		ZeroMemory(rgbVideoBuf, BufferSize * sizeof(UINT32));
	}

	void InitWrapper()
	{
		WrapperID = (DirectXVersion == 1) ? IID_IDirectDrawSurface :
			(DirectXVersion == 2) ? IID_IDirectDrawSurface2 :
			(DirectXVersion == 3) ? IID_IDirectDrawSurface3 :
			(DirectXVersion == 4) ? IID_IDirectDrawSurface4 :
			(DirectXVersion == 7) ? IID_IDirectDrawSurface7 : IID_IDirectDrawSurface7;

		if (Config.Dd7to9)
		{
			ProxyDirectXVersion = 9;
		}
		else
		{
			REFIID ProxyID = ConvertREFIID(WrapperID);
			ProxyDirectXVersion = (ProxyID == IID_IDirectDrawSurface) ? 1 :
				(ProxyID == IID_IDirectDrawSurface2) ? 2 :
				(ProxyID == IID_IDirectDrawSurface3) ? 3 :
				(ProxyID == IID_IDirectDrawSurface4) ? 4 :
				(ProxyID == IID_IDirectDrawSurface7) ? 7 : 7;
		}

		if (ProxyDirectXVersion != DirectXVersion)
		{
			Logging::LogDebug() << "Convert DirectDrawSurface v" << DirectXVersion << " to v" << ProxyDirectXVersion;
		}
	}

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
	STDMETHOD(EnumAttachedSurfaces)(THIS_ LPVOID, LPDDENUMSURFACESCALLBACK7);
	STDMETHOD(EnumOverlayZOrders)(THIS_ DWORD, LPVOID, LPDDENUMSURFACESCALLBACK7);
	STDMETHOD(Flip)(THIS_ LPDIRECTDRAWSURFACE7, DWORD);
	STDMETHOD(GetAttachedSurface)(THIS_ LPDDSCAPS2, LPDIRECTDRAWSURFACE7 FAR *);
	STDMETHOD(GetBltStatus)(THIS_ DWORD);
	STDMETHOD(GetCaps)(THIS_ LPDDSCAPS2);
	STDMETHOD(GetClipper)(THIS_ LPDIRECTDRAWCLIPPER FAR*);
	STDMETHOD(GetColorKey)(THIS_ DWORD, LPDDCOLORKEY);
	STDMETHOD(GetDC)(THIS_ HDC FAR *);
	STDMETHOD(GetFlipStatus)(THIS_ DWORD);
	STDMETHOD(GetOverlayPosition)(THIS_ LPLONG, LPLONG);
	STDMETHOD(GetPalette)(THIS_ LPDIRECTDRAWPALETTE FAR*);
	STDMETHOD(GetPixelFormat)(THIS_ LPDDPIXELFORMAT);
	STDMETHOD(GetSurfaceDesc)(THIS_ LPDDSURFACEDESC2);
	STDMETHOD(Initialize)(THIS_ LPDIRECTDRAW, LPDDSURFACEDESC2);
	STDMETHOD(IsLost)(THIS);
	STDMETHOD(Lock)(THIS_ LPRECT, LPDDSURFACEDESC2, DWORD, HANDLE);
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
	STDMETHOD(SetSurfaceDesc)(THIS_ LPDDSURFACEDESC2, DWORD);
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
