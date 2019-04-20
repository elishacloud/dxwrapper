#pragma once

class m_IDirect3DTextureX : public IUnknown
{
private:
	IDirect3DTexture2 *ProxyInterface = nullptr;
	m_IDirect3DTexture2 *WrapperInterface = nullptr;
	DWORD ProxyDirectXVersion;
	ULONG RefCount = 1;

	// Convert Texture
	m_IDirect3DDeviceX **D3DDeviceInterface = nullptr;

	// Store d3d texture version wrappers
	std::unique_ptr<m_IDirect3DTexture> UniqueProxyInterface = nullptr;
	std::unique_ptr<m_IDirect3DTexture2> UniqueProxyInterface2 = nullptr;

public:
	m_IDirect3DTextureX(IDirect3DTexture2 *aOriginal, DWORD DirectXVersion, m_IDirect3DTexture2 *Interface) : ProxyInterface(aOriginal), WrapperInterface(Interface)
	{
		ProxyDirectXVersion = GetIIDVersion(ConvertREFIID(GetWrapperType(DirectXVersion)));

		Logging::LogDebug() << "Create " << __FUNCTION__ << " v" << DirectXVersion;
	}
	m_IDirect3DTextureX(m_IDirect3DDeviceX **D3DDInterface, DWORD DirectXVersion, IDirectDrawSurface7 *lpSurface) : D3DDeviceInterface(D3DDInterface), ProxyInterface((IDirect3DTexture2*)lpSurface)
	{
		ProxyDirectXVersion = (!Config.Dd7to9) ? 7 : 9;

		Logging::LogDebug() << "Convert Direct3DTexture v" << DirectXVersion << " to v" << ProxyDirectXVersion;
	}
	~m_IDirect3DTextureX() {}

	DWORD GetDirectXVersion() { return DDWRAPPER_TYPEX; }
	REFIID GetWrapperType() { return IID_IUnknown; }
	IDirect3DTexture2 *GetProxyInterface() { return ProxyInterface; }
	m_IDirect3DTexture2 *GetWrapperInterface() { return WrapperInterface; }

	/*** IUnknown methods ***/
	HRESULT QueryInterface(REFIID riid, LPVOID FAR * ppvObj, DWORD DirectXVersion);
	STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * ppvObj) { return QueryInterface(riid, ppvObj, ProxyDirectXVersion); }
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	/*** IDirect3DTexture methods ***/
	STDMETHOD(Initialize)(THIS_ LPDIRECT3DDEVICE, LPDIRECTDRAWSURFACE);
	STDMETHOD(GetHandle)(THIS_ LPDIRECT3DDEVICE2, LPD3DTEXTUREHANDLE);
	STDMETHOD(PaletteChanged)(THIS_ DWORD, DWORD);
	STDMETHOD(Load)(THIS_ LPDIRECT3DTEXTURE2);
	STDMETHOD(Unload)(THIS);

	// Helper functions
	REFIID GetWrapperType(DWORD DirectXVersion)
	{
		return (DirectXVersion == 1) ? IID_IDirect3DTexture :
			(DirectXVersion == 2) ? IID_IDirect3DTexture2 : IID_IUnknown;
	}
	IDirect3DTexture *GetProxyInterfaceV1() { return (IDirect3DTexture *)ProxyInterface; }
	IDirect3DTexture2 *GetProxyInterfaceV2() { return ProxyInterface; }
	void *GetWrapperInterfaceX(DWORD DirectXVersion);
};
