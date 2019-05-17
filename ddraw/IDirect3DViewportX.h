#pragma once

class m_IDirect3DViewportX : public IUnknown
{
private:
	IDirect3DViewport3 *ProxyInterface = nullptr;
	m_IDirect3DViewport3 *WrapperInterface = nullptr;
	DWORD ProxyDirectXVersion;
	ULONG RefCount = 1;

	// Convert Viewport
	m_IDirect3DDeviceX **D3DDeviceInterface = nullptr;
	D3DVIEWPORT ViewPort;
	D3DVIEWPORT2 ViewPort2;
	bool ViewPortSet = false;
	bool ViewPort2Set = false;

	// Store ddraw version wrappers
	std::unique_ptr<m_IDirect3DViewport> UniqueProxyInterface = nullptr;
	std::unique_ptr<m_IDirect3DViewport2> UniqueProxyInterface2 = nullptr;
	std::unique_ptr<m_IDirect3DViewport3> UniqueProxyInterface3 = nullptr;

public:
	m_IDirect3DViewportX(IDirect3DViewport3 *aOriginal, DWORD DirectXVersion, m_IDirect3DViewport3 *Interface) : ProxyInterface(aOriginal), WrapperInterface(Interface)
	{
		ProxyDirectXVersion = GetIIDVersion(ConvertREFIID(GetWrapperType(DirectXVersion)));

		Logging::LogDebug() << "Create " << __FUNCTION__ << " v" << DirectXVersion;
	}
	m_IDirect3DViewportX(m_IDirect3DDeviceX **D3DDInterface, DWORD DirectXVersion) : D3DDeviceInterface(D3DDInterface)
	{
		ProxyDirectXVersion = (!Config.Dd7to9) ? 7 : 9;

		Logging::LogDebug() << "Convert Direct3DViewport v" << DirectXVersion << " to v" << ProxyDirectXVersion;
	}
	~m_IDirect3DViewportX() {}

	DWORD GetDirectXVersion() { return DDWRAPPER_TYPEX; }
	REFIID GetWrapperType() { return IID_IUnknown; }
	IDirect3DViewport3 *GetProxyInterface() { return ProxyInterface; }
	m_IDirect3DViewport3 *GetWrapperInterface() { return WrapperInterface; }

	/*** IUnknown methods ***/
	HRESULT QueryInterface(REFIID riid, LPVOID FAR * ppvObj, DWORD DirectXVersion);
	STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * ppvObj) { return QueryInterface(riid, ppvObj, ProxyDirectXVersion); }
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	/*** IDirect3DViewport methods ***/
	STDMETHOD(Initialize)(THIS_ LPDIRECT3D);
	STDMETHOD(GetViewport)(THIS_ LPD3DVIEWPORT);
	STDMETHOD(SetViewport)(THIS_ LPD3DVIEWPORT);
	STDMETHOD(TransformVertices)(THIS_ DWORD, LPD3DTRANSFORMDATA, DWORD, LPDWORD);
	STDMETHOD(LightElements)(THIS_ DWORD, LPD3DLIGHTDATA);
	STDMETHOD(SetBackground)(THIS_ D3DMATERIALHANDLE);
	STDMETHOD(GetBackground)(THIS_ LPD3DMATERIALHANDLE, LPBOOL);
	STDMETHOD(SetBackgroundDepth)(THIS_ LPDIRECTDRAWSURFACE);
	STDMETHOD(GetBackgroundDepth)(THIS_ LPDIRECTDRAWSURFACE*, LPBOOL);
	STDMETHOD(Clear)(THIS_ DWORD, LPD3DRECT, DWORD);
	STDMETHOD(AddLight)(THIS_ LPDIRECT3DLIGHT);
	STDMETHOD(DeleteLight)(THIS_ LPDIRECT3DLIGHT);
	STDMETHOD(NextLight)(THIS_ LPDIRECT3DLIGHT, LPDIRECT3DLIGHT*, DWORD);

	/*** IDirect3DViewport2 methods ***/
	STDMETHOD(GetViewport2)(THIS_ LPD3DVIEWPORT2);
	STDMETHOD(SetViewport2)(THIS_ LPD3DVIEWPORT2);

	/*** IDirect3DViewport3 methods ***/
	STDMETHOD(SetBackgroundDepth2)(THIS_ LPDIRECTDRAWSURFACE4);
	STDMETHOD(GetBackgroundDepth2)(THIS_ LPDIRECTDRAWSURFACE4*, LPBOOL);
	STDMETHOD(Clear2)(THIS_ DWORD, LPD3DRECT, DWORD, D3DCOLOR, D3DVALUE, DWORD);

	// Wrapper interface functions
	REFIID GetWrapperType(DWORD DirectXVersion)
	{
		return (DirectXVersion == 1) ? IID_IDirect3DViewport :
			(DirectXVersion == 2) ? IID_IDirect3DViewport2 :
			(DirectXVersion == 3) ? IID_IDirect3DViewport3 : IID_IUnknown;
	}
	IDirect3DViewport *GetProxyInterfaceV1() { return (IDirect3DViewport *)ProxyInterface; }
	IDirect3DViewport2 *GetProxyInterfaceV2() { return (IDirect3DViewport2 *)ProxyInterface; }
	IDirect3DViewport3 *GetProxyInterfaceV3() { return ProxyInterface; }
	void *GetWrapperInterfaceX(DWORD DirectXVersion);
};
