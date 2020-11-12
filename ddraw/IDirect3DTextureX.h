#pragma once

class m_IDirect3DTextureX : public IUnknown, public AddressLookupTableDdrawObject
{
private:
	IDirect3DTexture2 *ProxyInterface = nullptr;
	DWORD ProxyDirectXVersion;
	ULONG RefCount1 = 0;
	ULONG RefCount2 = 0;

	// Convert Texture
	m_IDirect3DDeviceX **D3DDeviceInterface = nullptr;
	m_IDirectDrawSurfaceX *DDrawSurface = nullptr;
	DWORD tHandle = 0;

	// Store d3d texture version wrappers
	m_IDirect3DTexture *WrapperInterface;
	m_IDirect3DTexture2 *WrapperInterface2;

	// Wrapper interface functions
	inline REFIID GetWrapperType(DWORD DirectXVersion)
	{
		return (DirectXVersion == 1) ? IID_IDirect3DTexture :
			(DirectXVersion == 2) ? IID_IDirect3DTexture2 : IID_IUnknown;
	}
	inline bool CheckWrapperType(REFIID IID)
	{
		return (IID == IID_IDirect3DTexture ||
			IID == IID_IDirect3DTexture2) ? true : false;
	}
	inline IDirect3DTexture *GetProxyInterfaceV1() { return (IDirect3DTexture *)ProxyInterface; }
	inline IDirect3DTexture2 *GetProxyInterfaceV2() { return ProxyInterface; }

	// Interface initialization functions
	void InitTexture(DWORD DirectXVersion);
	void ReleaseTexture();

public:
	m_IDirect3DTextureX(IDirect3DTexture2 *aOriginal, DWORD DirectXVersion) : ProxyInterface(aOriginal)
	{
		ProxyDirectXVersion = GetGUIDVersion(ConvertREFIID(GetWrapperType(DirectXVersion)));

		if (ProxyDirectXVersion != DirectXVersion)
		{
			LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << "(" << this << ")" << " converting interface from v" << DirectXVersion << " to v" << ProxyDirectXVersion);
		}
		else
		{
			LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << "(" << this << ") v" << DirectXVersion);
		}

		InitTexture(DirectXVersion);

		ProxyAddressLookupTable.SaveAddress(this, (ProxyInterface) ? ProxyInterface : (void*)this);
	}
	m_IDirect3DTextureX(m_IDirect3DDeviceX **D3DDInterface, DWORD DirectXVersion, m_IDirectDrawSurfaceX *lpSurface) : D3DDeviceInterface(D3DDInterface), DDrawSurface(lpSurface)
	{
		ProxyDirectXVersion = (Config.Dd7to9) ? 9 : 7;

		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << "(" << this << ")" << " converting interface from v" << DirectXVersion << " to v" << ProxyDirectXVersion);

		InitTexture(DirectXVersion);

		ProxyAddressLookupTable.SaveAddress(this, (ProxyInterface) ? ProxyInterface : (void*)this);
	}
	~m_IDirect3DTextureX()
	{
		LOG_LIMIT(3, __FUNCTION__ << "(" << this << ")" << " deleting interface!");

		ReleaseTexture();

		ProxyAddressLookupTable.DeleteAddress(this);
	}

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * ppvObj) { return QueryInterface(riid, ppvObj, 0); }
	STDMETHOD_(ULONG, AddRef) (THIS) { return AddRef(0); }
	STDMETHOD_(ULONG, Release) (THIS) { return Release(0); }

	/*** IDirect3DTexture methods ***/
	STDMETHOD(Initialize)(THIS_ LPDIRECT3DDEVICE, LPDIRECTDRAWSURFACE);
	STDMETHOD(GetHandle)(THIS_ LPDIRECT3DDEVICE2, LPD3DTEXTUREHANDLE);
	STDMETHOD(PaletteChanged)(THIS_ DWORD, DWORD);
	STDMETHOD(Load)(THIS_ LPDIRECT3DTEXTURE2);
	STDMETHOD(Unload)(THIS);

	// Helper functions
	HRESULT QueryInterface(REFIID riid, LPVOID FAR * ppvObj, DWORD DirectXVersion);
	void *GetWrapperInterfaceX(DWORD DirectXVersion);
	ULONG AddRef(DWORD DirectXVersion);
	ULONG Release(DWORD DirectXVersion);

	// Surface functions
	void ClearSurface() { DDrawSurface = nullptr; }
	m_IDirectDrawSurfaceX *GetSurface() { return DDrawSurface; }
};
