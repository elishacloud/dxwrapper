#pragma once

class m_IDirect3DTextureX : public IUnknown
{
private:
	IDirect3DTexture2 *ProxyInterface;
	m_IDirect3DTexture2 *WrapperInterface;
	DWORD ProxyDirectXVersion;
	ULONG RefCount = 1;
	m_IDirect3DDeviceX *D3DDeviceInterface;

	// Store d3d texture version wrappers
	std::unique_ptr<m_IDirect3DTexture> UniqueProxyInterface = nullptr;
	std::unique_ptr<m_IDirect3DTexture2> UniqueProxyInterface2 = nullptr;

public:
	m_IDirect3DTextureX(IDirect3DTexture2 *aOriginal, DWORD DirectXVersion, m_IDirect3DTexture2 *Interface) : ProxyInterface(aOriginal), WrapperInterface(Interface)
	{
		InitWrapper(DirectXVersion);
	}
	m_IDirect3DTextureX(m_IDirect3DDeviceX *D3DDInterface, DWORD DirectXVersion, IDirectDrawSurface7 *lpSurface) : D3DDeviceInterface(D3DDInterface), ProxyInterface((IDirect3DTexture2*)lpSurface)
	{
		WrapperInterface = nullptr;

		InitWrapper(DirectXVersion);
	}
	void InitWrapper(DWORD DirectXVersion)
	{
		if (DirectXVersion == 7)
		{
			DirectXVersion = 3;
			ProxyDirectXVersion = 7;
		}
		else
		{
			ProxyDirectXVersion = GetIIDVersion(ConvertREFIID(GetWrapperType(DirectXVersion)));
		}

		if (ProxyDirectXVersion != DirectXVersion)
		{
			Logging::LogDebug() << "Convert Direct3DTexture v" << DirectXVersion << " to v" << ProxyDirectXVersion;
		}
	}
	~m_IDirect3DTextureX() {}

	DWORD GetDirectXVersion() { return DDWRAPPER_TYPEX; }
	REFIID GetWrapperType(DWORD DirectXVersion)
	{
		return (DirectXVersion == 1) ? IID_IDirect3DTexture :
			(DirectXVersion == 2) ? IID_IDirect3DTexture2 : IID_IUnknown;
	}
	IDirect3DTexture *GetProxyInterfaceV1() { return (IDirect3DTexture *)ProxyInterface; }
	IDirect3DTexture2 *GetProxyInterface() { return ProxyInterface; }
	m_IDirect3DTexture2 *GetWrapperInterface() { return WrapperInterface; }
	void *GetWrapperInterfaceX(DWORD DirectXVersion);

	/*** IUnknown methods ***/
	HRESULT QueryInterface(REFIID riid, LPVOID FAR * ppvObj, DWORD DirectXVersion);
	STDMETHOD(QueryInterface) (THIS_ REFIID, LPVOID FAR *) { return E_NOINTERFACE; }
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	/*** IDirect3DTexture methods ***/
	STDMETHOD(Initialize)(THIS_ LPDIRECT3DDEVICE, LPDIRECTDRAWSURFACE);
	STDMETHOD(GetHandle)(THIS_ LPDIRECT3DDEVICE2, LPD3DTEXTUREHANDLE);
	STDMETHOD(PaletteChanged)(THIS_ DWORD, DWORD);
	STDMETHOD(Load)(THIS_ LPDIRECT3DTEXTURE2);
	STDMETHOD(Unload)(THIS);
};
