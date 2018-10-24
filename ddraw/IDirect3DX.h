#pragma once

class m_IDirect3DX : public IUnknown
{
private:
	IDirect3D7 *ProxyInterface;
	m_IDirect3D7 *WrapperInterface;
	DWORD ProxyDirectXVersion;
	ULONG RefCount = 1;
	m_IDirectDrawX *ddrawParent = nullptr;

	// Store d3d version wrappers
	std::unique_ptr<m_IDirect3D> UniqueProxyInterface = nullptr;
	std::unique_ptr<m_IDirect3D2> UniqueProxyInterface2 = nullptr;
	std::unique_ptr<m_IDirect3D3> UniqueProxyInterface3 = nullptr;
	std::unique_ptr<m_IDirect3D7> UniqueProxyInterface7 = nullptr;

public:
	m_IDirect3DX(IDirect3D7 *aOriginal, DWORD DirectXVersion, m_IDirect3D7 *Interface) : ProxyInterface(aOriginal), WrapperInterface(Interface)
	{
		if (Config.Dd7to9)
		{
			ddrawParent = (m_IDirectDrawX *)aOriginal;

			ProxyInterface = nullptr;
			WrapperInterface = nullptr;

			ProxyDirectXVersion = 9;
		}
		else
		{
			ProxyDirectXVersion = GetIIDVersion(ConvertREFIID(GetWrapperType(DirectXVersion)));
		}

		if (ProxyDirectXVersion != DirectXVersion)
		{
			Logging::LogDebug() << "Convert Direct3D v" << DirectXVersion << " to v" << ProxyDirectXVersion;
		}
		else
		{
			Logging::LogDebug() << "Create " << __FUNCTION__ << " v" << DirectXVersion;
		}
	}
	~m_IDirect3DX()
	{
		if (Config.Dd7to9)
		{
			ReleaseD3DInterface();
		}
	}

	DWORD GetDirectXVersion() { return DDWRAPPER_TYPEX; }
	REFIID GetWrapperType(DWORD DirectXVersion)
	{
		return (DirectXVersion == 1) ? IID_IDirect3D :
			(DirectXVersion == 2) ? IID_IDirect3D2 :
			(DirectXVersion == 3) ? IID_IDirect3D3 :
			(DirectXVersion == 7) ? IID_IDirect3D7 : IID_IDirect3D7;
	}
	IDirect3D *GetProxyInterfaceV1() { return (IDirect3D *)ProxyInterface; }
	IDirect3D3 *GetProxyInterfaceV3() { return (IDirect3D3 *)ProxyInterface; }
	IDirect3D7 *GetProxyInterface() { return ProxyInterface; }
	m_IDirect3D7 *GetWrapperInterface() { return WrapperInterface; }
	void *GetWrapperInterfaceX(DWORD DirectXVersion);

	/*** IUnknown methods ***/
	HRESULT QueryInterface(REFIID riid, LPVOID FAR * ppvObj, DWORD DirectXVersion);
	STDMETHOD(QueryInterface) (THIS_ REFIID, LPVOID FAR *) { return E_NOINTERFACE; }
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	/*** IDirect3D methods ***/
	STDMETHOD(Initialize)(THIS_ REFCLSID);
	HRESULT EnumDevices(LPD3DENUMDEVICESCALLBACK, LPVOID);
	HRESULT EnumDevices7(LPD3DENUMDEVICESCALLBACK7, LPVOID, bool = false);
	STDMETHOD(CreateLight)(THIS_ LPDIRECT3DLIGHT*, LPUNKNOWN);
	STDMETHOD(CreateMaterial)(THIS_ LPDIRECT3DMATERIAL3*, LPUNKNOWN, DWORD);
	STDMETHOD(CreateViewport)(THIS_ LPDIRECT3DVIEWPORT3*, LPUNKNOWN, DWORD);
	STDMETHOD(FindDevice)(THIS_ LPD3DFINDDEVICESEARCH, LPD3DFINDDEVICERESULT);
	STDMETHOD(CreateDevice)(THIS_ REFCLSID, LPDIRECTDRAWSURFACE7, LPDIRECT3DDEVICE7*, LPUNKNOWN, DWORD);
	STDMETHOD(CreateVertexBuffer)(THIS_ LPD3DVERTEXBUFFERDESC, LPDIRECT3DVERTEXBUFFER7*, DWORD, LPUNKNOWN, DWORD);
	STDMETHOD(EnumZBufferFormats)(THIS_ REFCLSID, LPD3DENUMPIXELFORMATSCALLBACK, LPVOID);
	STDMETHOD(EvictManagedTextures)(THIS);

	// Helper functions
	void ReleaseD3DInterface();
};
