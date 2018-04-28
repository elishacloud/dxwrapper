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

public:
	m_IDirect3DTextureX(IDirect3DTexture2 *aOriginal, DWORD Version, m_IDirect3DTexture2 *Interface) : ProxyInterface(aOriginal), DirectXVersion(Version), WrapperInterface(Interface)
	{
		WrapperID = (DirectXVersion == 1) ? IID_IDirect3DTexture :
			(DirectXVersion == 2) ? IID_IDirect3DTexture2 : IID_IDirect3DTexture2;

		REFIID ProxyID = ConvertREFIID(WrapperID);
		ProxyDirectXVersion = (ProxyID == IID_IDirect3DTexture) ? 1 :
			(ProxyID == IID_IDirect3DTexture2) ? 2 : 2;

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
	~m_IDirect3DTextureX() {}

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
