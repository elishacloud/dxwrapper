#pragma once

class m_IDirect3DTextureX : public IUnknown, public AddressLookupTableDdrawObject
{
private:
	IDirect3DTexture2 *ProxyInterface = nullptr;
	DWORD ProxyDirectXVersion;
	ULONG RefCount1 = 0;
	ULONG RefCount2 = 0;

	// Store version wrappers
	m_IDirect3DTexture* WrapperInterface = nullptr;
	m_IDirect3DTexture2* WrapperInterface2 = nullptr;

	// Convert to Direct3D9
	m_IDirect3DDeviceX **D3DDeviceInterface = nullptr;
	DWORD tHandle = 0;
	struct {
		m_IDirectDrawSurfaceX* Interface = nullptr;
		DWORD DxVersion = 0;
	} parent3DSurface;

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
	void InitInterface(DWORD DirectXVersion);
	void ReleaseInterface();

public:
	m_IDirect3DTextureX(IDirect3DTexture2 *aOriginal, DWORD DirectXVersion) : ProxyInterface(aOriginal)
	{
		ProxyDirectXVersion = GetGUIDVersion(GetWrapperType(DirectXVersion));

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

		InitInterface(DirectXVersion);
	}
	m_IDirect3DTextureX(m_IDirect3DDeviceX **D3DDInterface, DWORD DirectXVersion, m_IDirectDrawSurfaceX *lpSurface, DWORD DXSurfaceVersion) : D3DDeviceInterface(D3DDInterface)
	{
		ProxyDirectXVersion = (Config.Dd7to9) ? 9 : 7;

		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ")" << " converting interface from v" << DirectXVersion << " to v" << ProxyDirectXVersion);

		parent3DSurface.Interface = lpSurface;
		parent3DSurface.DxVersion = DXSurfaceVersion;

		InitInterface(DirectXVersion);
	}
	~m_IDirect3DTextureX()
	{
		LOG_LIMIT(3, __FUNCTION__ << " (" << this << ")" << " deleting interface!");

		ReleaseInterface();
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

	// Handle functions
	HRESULT m_IDirect3DTextureX::SetHandle(DWORD dwHandle);
	void SetD3DDevice(m_IDirect3DDeviceX** D3DDevice) { D3DDeviceInterface = D3DDevice; }
	void ClearD3DDevice() { D3DDeviceInterface = nullptr; }

	// Surface functions
	m_IDirectDrawSurfaceX *GetSurface() const { return parent3DSurface.Interface; }
};
