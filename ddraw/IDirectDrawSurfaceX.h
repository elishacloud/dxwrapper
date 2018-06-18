#pragma once
#include <map>

class m_IDirectDrawSurfaceX : public IDirectDrawSurface7
{
private:
	IDirectDrawSurface7 *ProxyInterface;
	m_IDirectDrawSurface7 *WrapperInterface;
	DWORD DirectXVersion;
	DWORD ProxyDirectXVersion;
	IID WrapperID;
	ULONG RefCount = 1;
	bool ConvertSurfaceDescTo2 = false;

	// Color Key Structure
	struct CKEYS
	{
		DDCOLORKEY Key = { 0, 0 };
		bool IsSet = false;
		bool IdColorSpace = false;
	};

	// Convert to d3d9
	m_IDirectDrawX *ddrawParent = nullptr;
	m_IDirectDrawPalette *attachedPalette = nullptr;	// Associated palette
	DDSURFACEDESC2 surfaceDesc2;
	D3DLOCKED_RECT d3dlrect = { 0, nullptr };
	RECT LockDestRect;
	CKEYS ColorKeys[4];		// Color keys(DDCKEY_DESTBLT, DDCKEY_DESTOVERLAY, DDCKEY_SRCBLT, DDCKEY_SRCOVERLAY)
	LONG overlayX = 0;
	LONG overlayY = 0;
	DWORD BufferSize = 0;
	BYTE *rawVideoBuf = nullptr;						// Virtual video buffer
	bool WriteDirectlyToSurface = false;
	bool IsLocked = false;

	// Display resolution
	DWORD displayWidth = 0;
	DWORD displayHeight = 0;

	// Direct3D9 vars
	LPDIRECT3DDEVICE9 *d3d9Device = nullptr;
	LPDIRECT3DTEXTURE9 surfaceTexture = nullptr;
	LPDIRECT3DVERTEXBUFFER9 vertexBuffer = nullptr;

	// Store a list of attached surfaces
	std::map<DWORD, m_IDirectDrawSurfaceX*> AttachedSurfaceMap;

	// Custom vertex
	struct TLVERTEX
	{
		float x;
		float y;
		float z;
		float rhw;
		float u;
		float v;
	};

public:
	m_IDirectDrawSurfaceX(IDirectDrawSurface7 *pOriginal, DWORD Version, m_IDirectDrawSurface7 *Interface) : ProxyInterface(pOriginal), DirectXVersion(Version), WrapperInterface(Interface)
	{
		InitWrapper();
	}
	m_IDirectDrawSurfaceX(LPDIRECT3DDEVICE9 *lplpDevice, m_IDirectDrawX *Interface, DWORD Version, LPDDSURFACEDESC2 lpDDSurfaceDesc, DWORD Width, DWORD Height) :
		d3d9Device(lplpDevice), ddrawParent(Interface), DirectXVersion(Version), displayWidth(Width), displayHeight(Height)
	{
		ProxyInterface = nullptr;
		WrapperInterface = nullptr;

		InitWrapper();

		// Copy surface description
		memcpy(&surfaceDesc2, lpDDSurfaceDesc, sizeof(DDSURFACEDESC2));

		// Create Surface for d3d9
		CreateD3d9Surface();

		// Store surface
		if (d3d9Device && *d3d9Device && ddrawParent)
		{
			ddrawParent->AddSurfaceToVector(this);
		}
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
			ProxyDirectXVersion = GetIIDVersion(ConvertREFIID(WrapperID));
		}

		if (ProxyDirectXVersion > 3 && DirectXVersion < 4)
		{
			ConvertSurfaceDescTo2 = true;
		}

		if (ProxyDirectXVersion != DirectXVersion)
		{
			Logging::LogDebug() << "Convert DirectDrawSurface v" << DirectXVersion << " to v" << ProxyDirectXVersion;
		}
	}
	~m_IDirectDrawSurfaceX()
	{
		if (rawVideoBuf)
		{
			delete rawVideoBuf;
			rawVideoBuf = nullptr;
		}

		if (d3d9Device && *d3d9Device && ddrawParent)
		{
			ddrawParent->RemoveSurfaceFromVector(this);
			ReleaseD9Surface();
		}
	}

	DWORD GetDirectXVersion() { return DDWRAPPER_TYPEX; }
	REFIID GetWrapperType() { return WrapperID; }
	IDirectDrawSurface7 *GetProxyInterface() { return ProxyInterface; }
	m_IDirectDrawSurface7 *GetWrapperInterface() { return WrapperInterface; }
	LPDIRECT3DTEXTURE9 *GetSurfaceTexture() { return &surfaceTexture; }
	bool IsSurfaceLocked() { return IsLocked; }
	bool NeedsLock() { return !IsLocked && WriteDirectlyToSurface; }

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

	/*** Helper functions ***/
	void ReleaseD9Surface();
	void AlocateVideoBuffer();
	HRESULT CreateD3d9Surface();
	bool CheckSurfaceRect(LPRECT lpOutRect, LPRECT lpInRect);
	HRESULT SetLock();
	HRESULT SetUnLock();
	HRESULT GetSurfaceInfo(D3DLOCKED_RECT *pLockRect, DWORD *lpBitCount, D3DFORMAT *lpFormat);
	HRESULT GetSurfaceDesc2(LPDDSURFACEDESC2 lpDDSurfaceDesc2);
	void AddSurfaceToMap(m_IDirectDrawSurfaceX* lpSurfaceX);
	void RemoveSurfaceFromMap(m_IDirectDrawSurfaceX* lpSurfaceX);
	bool DoesSurfaceExist(m_IDirectDrawSurfaceX* lpSurfaceX);
};
