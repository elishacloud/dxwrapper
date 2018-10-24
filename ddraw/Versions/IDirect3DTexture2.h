#pragma once

class m_IDirect3DTexture2 : public IDirect3DTexture2, public AddressLookupTableDdrawObject
{
private:
	std::unique_ptr<m_IDirect3DTextureX> UniqueProxyInterface;
	m_IDirect3DTextureX *ProxyInterface;
	IDirect3DTexture2 *RealInterface;
	REFIID WrapperID = IID_IDirect3DTexture2;
	const DWORD DirectXVersion = 2;

public:
	m_IDirect3DTexture2(IDirect3DTexture2 *aOriginal) : RealInterface(aOriginal)
	{
		UniqueProxyInterface = std::make_unique<m_IDirect3DTextureX>((IDirect3DTexture2*)RealInterface, DirectXVersion, (m_IDirect3DTexture2*)this);
		ProxyInterface = UniqueProxyInterface.get();
		ProxyAddressLookupTable.SaveAddress(this, RealInterface);
	}
	m_IDirect3DTexture2(m_IDirect3DTextureX *aOriginal) : ProxyInterface(aOriginal)
	{
		RealInterface = nullptr;
	}
	~m_IDirect3DTexture2()
	{
		ProxyAddressLookupTable.DeleteAddress(this);
	}

	DWORD GetDirectXVersion() { return DirectXVersion; }
	REFIID GetWrapperType() { return WrapperID; }
	IDirect3DTexture2 *GetProxyInterface() { return RealInterface; }
	m_IDirect3DTextureX *GetWrapperInterface() { return ProxyInterface; }

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	/*** IDirect3DTexture2 methods ***/
	STDMETHOD(GetHandle)(THIS_ LPDIRECT3DDEVICE2, LPD3DTEXTUREHANDLE);
	STDMETHOD(PaletteChanged)(THIS_ DWORD, DWORD);
	STDMETHOD(Load)(THIS_ LPDIRECT3DTEXTURE2);
};
