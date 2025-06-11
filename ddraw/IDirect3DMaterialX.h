#pragma once

class m_IDirect3DMaterialX : public IUnknown, public AddressLookupTableDdrawObject
{
private:
	IDirect3DMaterial3 *ProxyInterface = nullptr;
	DWORD ProxyDirectXVersion;
	ULONG RefCount1 = 0;
	ULONG RefCount2 = 0;
	ULONG RefCount3 = 0;

	// Store version wrappers
	m_IDirect3DMaterial* WrapperInterface = nullptr;
	m_IDirect3DMaterial2* WrapperInterface2 = nullptr;
	m_IDirect3DMaterial3* WrapperInterface3 = nullptr;

	// Convert to Direct3D9
	m_IDirect3DX* D3DInterface = nullptr;
	D3DMATERIAL Material = {};		// Defaults to all null
	D3DMATERIALHANDLE mHandle = 0;

	// Wrapper interface functions
	inline REFIID GetWrapperType(DWORD DirectXVersion)
	{
		return (DirectXVersion == 1) ? IID_IDirect3DMaterial :
			(DirectXVersion == 2) ? IID_IDirect3DMaterial2 :
			(DirectXVersion == 3) ? IID_IDirect3DMaterial3 : IID_IUnknown;
	}
	inline bool CheckWrapperType(REFIID IID)
	{
		return (IID == IID_IDirect3DMaterial ||
			IID == IID_IDirect3DMaterial2 ||
			IID == IID_IDirect3DMaterial3) ? true : false;
	}
	inline IDirect3DMaterial *GetProxyInterfaceV1() { return (IDirect3DMaterial *)ProxyInterface; }
	inline IDirect3DMaterial2 *GetProxyInterfaceV2() { return (IDirect3DMaterial2 *)ProxyInterface; }
	inline IDirect3DMaterial3 *GetProxyInterfaceV3() { return ProxyInterface; }

	// Interface initialization functions
	void InitInterface(DWORD DirectXVersion);
	void ReleaseInterface();

public:
	m_IDirect3DMaterialX(IDirect3DMaterial3 *aOriginal, DWORD DirectXVersion) : ProxyInterface(aOriginal)
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
	m_IDirect3DMaterialX(m_IDirect3DX *D3D, DWORD DirectXVersion) : D3DInterface(D3D)
	{
		ProxyDirectXVersion = (!Config.Dd7to9) ? 3 : 9;

		if (ProxyDirectXVersion != DirectXVersion)
		{
			LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ")" << " converting interface from v" << DirectXVersion << " to v" << ProxyDirectXVersion);
		}
		else
		{
			LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ") v" << DirectXVersion);
		}

		InitInterface(DirectXVersion);
	}
	~m_IDirect3DMaterialX()
	{
		LOG_LIMIT(3, __FUNCTION__ << " (" << this << ")" << " deleting interface!");

		ReleaseInterface();
	}

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * ppvObj) { return QueryInterface(riid, ppvObj, 0); }
	STDMETHOD_(ULONG, AddRef) (THIS) { return AddRef(0); }
	STDMETHOD_(ULONG, Release) (THIS) { return Release(0); }

	/*** IDirect3DMaterial methods ***/
	STDMETHOD(Initialize)(THIS_ LPDIRECT3D);
	STDMETHOD(SetMaterial)(THIS_ LPD3DMATERIAL);
	STDMETHOD(GetMaterial)(THIS_ LPD3DMATERIAL);
	STDMETHOD(GetHandle)(THIS_ LPDIRECT3DDEVICE3, LPD3DMATERIALHANDLE);
	STDMETHOD(Reserve)(THIS);
	STDMETHOD(Unreserve)(THIS);

	// Helper function
	HRESULT QueryInterface(REFIID riid, LPVOID FAR * ppvObj, DWORD DirectXVersion);
	void *GetWrapperInterfaceX(DWORD DirectXVersion);
	void ClearD3D() { D3DInterface = nullptr; }
	ULONG AddRef(DWORD DirectXVersion);
	ULONG Release(DWORD DirectXVersion);
};
