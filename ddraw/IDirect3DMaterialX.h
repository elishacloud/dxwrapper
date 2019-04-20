#pragma once

class m_IDirect3DMaterialX : public IUnknown
{
private:
	IDirect3DMaterial3 *ProxyInterface = nullptr;
	m_IDirect3DMaterial3 *WrapperInterface = nullptr;
	DWORD ProxyDirectXVersion;
	ULONG RefCount = 1;

	// Convert Material
	m_IDirect3DDeviceX **D3DDeviceInterface = nullptr;

	// Store d3d material version wrappers
	std::unique_ptr<m_IDirect3DMaterial> UniqueProxyInterface = nullptr;
	std::unique_ptr<m_IDirect3DMaterial2> UniqueProxyInterface2 = nullptr;
	std::unique_ptr<m_IDirect3DMaterial3> UniqueProxyInterface3 = nullptr;

public:
	m_IDirect3DMaterialX(IDirect3DMaterial3 *aOriginal, DWORD DirectXVersion, m_IDirect3DMaterial3 *Interface) : ProxyInterface(aOriginal), WrapperInterface(Interface)
	{
		ProxyDirectXVersion = GetIIDVersion(ConvertREFIID(GetWrapperType(DirectXVersion)));

		Logging::LogDebug() << "Create " << __FUNCTION__ << " v" << DirectXVersion;
	}
	m_IDirect3DMaterialX(m_IDirect3DDeviceX **D3DDInterface, DWORD DirectXVersion) : D3DDeviceInterface(D3DDInterface)
	{
		ProxyDirectXVersion = (!Config.Dd7to9) ? 7 : 9;

		Logging::LogDebug() << "Convert Direct3DMaterial v" << DirectXVersion << " to v" << ProxyDirectXVersion;
	}
	~m_IDirect3DMaterialX() {}

	DWORD GetDirectXVersion() { return DDWRAPPER_TYPEX; }
	REFIID GetWrapperType() { return IID_IUnknown; }
	IDirect3DMaterial3 *GetProxyInterface() { return ProxyInterface; }
	m_IDirect3DMaterial3 *GetWrapperInterface() { return WrapperInterface; }

	/*** IUnknown methods ***/
	HRESULT QueryInterface(REFIID riid, LPVOID FAR * ppvObj, DWORD DirectXVersion);
	STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * ppvObj) { return QueryInterface(riid, ppvObj, ProxyDirectXVersion); }
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	/*** IDirect3DMaterial methods ***/
	STDMETHOD(Initialize)(THIS_ LPDIRECT3D);
	STDMETHOD(SetMaterial)(THIS_ LPD3DMATERIAL);
	STDMETHOD(GetMaterial)(THIS_ LPD3DMATERIAL);
	STDMETHOD(GetHandle)(THIS_ LPDIRECT3DDEVICE3, LPD3DMATERIALHANDLE);
	STDMETHOD(Reserve)(THIS);
	STDMETHOD(Unreserve)(THIS);

	// Helper functions
	REFIID GetWrapperType(DWORD DirectXVersion)
	{
		return (DirectXVersion == 1) ? IID_IDirect3DMaterial :
			(DirectXVersion == 2) ? IID_IDirect3DMaterial2 :
			(DirectXVersion == 3) ? IID_IDirect3DMaterial3 : IID_IUnknown;
	}
	IDirect3DMaterial *GetProxyInterfaceV1() { return (IDirect3DMaterial *)ProxyInterface; }
	IDirect3DMaterial2 *GetProxyInterfaceV2() { return (IDirect3DMaterial2 *)ProxyInterface; }
	IDirect3DMaterial3 *GetProxyInterfaceV3() { return ProxyInterface; }
	void *GetWrapperInterfaceX(DWORD DirectXVersion);
};
