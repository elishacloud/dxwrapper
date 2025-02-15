#pragma once

class m_IDirect3DX : public IUnknown, public AddressLookupTableDdrawObject
{
private:
	IDirect3D7 *ProxyInterface = nullptr;
	DWORD ProxyDirectXVersion;
	ULONG RefCount1 = 0;
	ULONG RefCount2 = 0;
	ULONG RefCount3 = 0;
	ULONG RefCount7 = 0;
	m_IDirectDrawX *ddrawParent = nullptr;
	DWORD DDrawVersion = 0;

	// Store d3d version wrappers
	m_IDirect3D *WrapperInterface = nullptr;
	m_IDirect3D2 *WrapperInterface2 = nullptr;
	m_IDirect3D3 *WrapperInterface3 = nullptr;
	m_IDirect3D7 *WrapperInterface7 = nullptr;

	// Device interface pointers
	m_IDirect3DDeviceX* D3DDeviceInterface = nullptr;

	// Cache Cap9
	struct DUALCAP9 {
		D3DCAPS9 REF = {};
		D3DCAPS9 HAL = {};
	};
	std::vector<DUALCAP9> Cap9Cache;

	// Light array
	std::vector<m_IDirect3DLight*> LightList;

	// Material array
	std::vector<m_IDirect3DMaterialX*> MaterialList;

	// VertexBuffer array
	struct VERTEXBUFFERLIST {
		m_IDirect3DVertexBufferX* Interface = nullptr;
		DWORD DxVersion = 0;
		DWORD RefCount = 0;
	};
	std::vector<VERTEXBUFFERLIST> VertexBufferList;

	// Viewport array
	std::vector<m_IDirect3DViewportX*> ViewportList;

	// Wrapper interface functions
	inline REFIID GetWrapperType(DWORD DirectXVersion)
	{
		return (DirectXVersion == 1) ? IID_IDirect3D :
			(DirectXVersion == 2) ? IID_IDirect3D2 :
			(DirectXVersion == 3) ? IID_IDirect3D3 :
			(DirectXVersion == 7) ? IID_IDirect3D7 : IID_IUnknown;
	}
	inline bool CheckWrapperType(REFIID IID)
	{
		return (IID == IID_IDirect3D ||
			IID == IID_IDirect3D2 ||
			IID == IID_IDirect3D3 ||
			IID == IID_IDirect3D7) ? true : false;
	}
	inline IDirect3D *GetProxyInterfaceV1() { return (IDirect3D *)ProxyInterface; }
	inline IDirect3D2 *GetProxyInterfaceV2() { return (IDirect3D2 *)ProxyInterface; }
	inline IDirect3D3 *GetProxyInterfaceV3() { return (IDirect3D3 *)ProxyInterface; }
	inline IDirect3D7 *GetProxyInterfaceV7() { return ProxyInterface; }

	// Helper functions
	void GetCap9Cache();
	void ResolutionHack();

	// Interface initialization functions
	void InitInterface();
	void ReleaseInterface();

public:
	m_IDirect3DX(IDirect3D7 *aOriginal, DWORD DirectXVersion) : ProxyInterface(aOriginal)
	{
		ProxyDirectXVersion = GetGUIDVersion(ConvertREFIID(GetWrapperType(DirectXVersion)));

		if (ProxyDirectXVersion != DirectXVersion)
		{
			LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ")" << " converting interface from v" << DirectXVersion << " to v" << ProxyDirectXVersion);
		}
		else
		{
			LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ") v" << DirectXVersion);
		}
		if (Config.Dd7to9)
		{
			Logging::Log() << __FUNCTION__ << " (" << this << ") Warning: created from non-dd7to9 interface!";
		}

		InitInterface();
	}
	m_IDirect3DX(m_IDirectDrawX *lpDdraw, DWORD DirectXVersion, DWORD DXDrawVersion) : ddrawParent(lpDdraw), DDrawVersion(DXDrawVersion)
	{
		ProxyDirectXVersion = 9;

		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ")" << " converting interface from v" << DirectXVersion << " to v" << ProxyDirectXVersion);

		InitInterface();
	}
	~m_IDirect3DX()
	{
		LOG_LIMIT(3, __FUNCTION__ << " (" << this << ")" << " deleting interface!");

		ReleaseInterface();
	}

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * ppvObj) { return QueryInterface(riid, ppvObj, 0); }
	STDMETHOD_(ULONG, AddRef) (THIS) { return AddRef(0); }
	STDMETHOD_(ULONG, Release) (THIS) { return Release(0); }

	/*** IDirect3D methods ***/
	STDMETHOD(Initialize)(THIS_ REFCLSID);
	HRESULT EnumDevices(LPD3DENUMDEVICESCALLBACK, LPVOID, DWORD);
	HRESULT EnumDevices7(LPD3DENUMDEVICESCALLBACK7, LPVOID, DWORD);
	STDMETHOD(CreateLight)(THIS_ LPDIRECT3DLIGHT*, LPUNKNOWN);
	STDMETHOD(CreateMaterial)(THIS_ LPDIRECT3DMATERIAL3*, LPUNKNOWN, DWORD);
	STDMETHOD(CreateViewport)(THIS_ LPDIRECT3DVIEWPORT3*, LPUNKNOWN, DWORD);
	STDMETHOD(FindDevice)(THIS_ LPD3DFINDDEVICESEARCH, LPD3DFINDDEVICERESULT);
	STDMETHOD(CreateDevice)(THIS_ REFCLSID, LPDIRECTDRAWSURFACE7, LPDIRECT3DDEVICE7*, LPUNKNOWN, DWORD);
	STDMETHOD(CreateVertexBuffer)(THIS_ LPD3DVERTEXBUFFERDESC, LPDIRECT3DVERTEXBUFFER7*, DWORD, LPUNKNOWN, DWORD);
	STDMETHOD(EnumZBufferFormats)(THIS_ REFCLSID, LPD3DENUMPIXELFORMATSCALLBACK, LPVOID);
	STDMETHOD(EvictManagedTextures)(THIS);

	// Helper functions
	HRESULT QueryInterface(REFIID riid, LPVOID FAR * ppvObj, DWORD DirectXVersion);
	void* GetWrapperInterfaceX(DWORD DirectXVersion);
	inline m_IDirect3DDeviceX** GetD3DDevice() { return &D3DDeviceInterface; }
	void SetD3DDevice(m_IDirect3DDeviceX* lpD3DDevice);
	void ClearD3DDevice(m_IDirect3DDeviceX* lpD3DDevice);
	void AddLight(m_IDirect3DLight* lpLight);
	void ClearLight(m_IDirect3DLight* lpLight);
	void AddMaterial(m_IDirect3DMaterialX* lpMaterialX);
	void ClearMaterial(m_IDirect3DMaterialX* lpMaterialX);
	void AddVertexBuffer(m_IDirect3DVertexBufferX* lpVertexBufferX);
	void ClearVertexBuffer(m_IDirect3DVertexBufferX* lpVertexBufferX);
	void AddViewport(m_IDirect3DViewportX* lpViewportX);
	void ClearViewport(m_IDirect3DViewportX* lpViewportX);
	ULONG AddRef(DWORD DirectXVersion);
	ULONG Release(DWORD DirectXVersion);

	// Functions handling the ddraw parent interface
	inline void SetDdrawParent(m_IDirectDrawX* ddraw) { ddrawParent = ddraw; GetCap9Cache(); }
	inline void ClearDdraw() { ddrawParent = nullptr; }
};
