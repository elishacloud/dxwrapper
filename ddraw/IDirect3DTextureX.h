#pragma once

class m_IDirect3DTextureX : public IDirect3DTexture2
{
private:
	IDirect3DTexture2 *ProxyInterface;
	m_IDirect3DTexture2 *WrapperInterface;
	DWORD DirectXVersion;
	DWORD ProxyDirectXVersion;
	IID WrapperID;
	ULONG RefCount = 1;
	m_IDirect3DDeviceX *D3DDeviceInterface;

public:
	m_IDirect3DTextureX(IDirect3DTexture2 *aOriginal, DWORD Version, m_IDirect3DTexture2 *Interface) : ProxyInterface(aOriginal), DirectXVersion(Version), WrapperInterface(Interface)
	{
		InitWrapper();
	}
	m_IDirect3DTextureX(m_IDirect3DDeviceX *D3DDInterface, DWORD Version) : D3DDeviceInterface(D3DDInterface), DirectXVersion(Version)
	{
		ProxyInterface = nullptr;
		WrapperInterface = nullptr;

		InitWrapper();
	}
	~m_IDirect3DTextureX() {}

	void InitWrapper()
	{
		WrapperID = (DirectXVersion == 1) ? IID_IDirect3DTexture :
			(DirectXVersion == 2) ? IID_IDirect3DTexture2 : IID_IDirect3DTexture2;

		ProxyDirectXVersion = GetIIDVersion(ConvertREFIID(WrapperID));

		if (DirectXVersion == 7)
		{
			DirectXVersion = 3;
			ProxyDirectXVersion = 7;
		}

		if (ProxyDirectXVersion != DirectXVersion)
		{
			Logging::LogDebug() << "Convert Direct3DTexture v" << DirectXVersion << " to v" << ProxyDirectXVersion;
		}
	}

	DWORD GetDirectXVersion() { return DDWRAPPER_TYPEX; }
	REFIID GetWrapperType() { return WrapperID; }
	IDirect3DTexture2 *GetProxyInterface() { return ProxyInterface; }
	m_IDirect3DTexture2 *GetWrapperInterface() { return WrapperInterface; }

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	/*** IDirect3DTexture methods ***/
	STDMETHOD(Initialize)(THIS_ LPDIRECT3DDEVICE, LPDIRECTDRAWSURFACE);
	STDMETHOD(GetHandle)(THIS_ LPDIRECT3DDEVICE2, LPD3DTEXTUREHANDLE);
	STDMETHOD(PaletteChanged)(THIS_ DWORD, DWORD);
	STDMETHOD(Load)(THIS_ LPDIRECT3DTEXTURE2);
	STDMETHOD(Unload)(THIS);
};
