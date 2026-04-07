#pragma once

class m_IDirect3DLight final : public IDirect3DLight, public AddressLookupTableDdrawObject
{
private:
	IDirect3DLight *ProxyInterface = nullptr;
	const IID WrapperID = IID_IDirect3DLight;
	ULONG RefCount = 1;

	// Convert Light
	m_IDirect3DX* D3DInterface = nullptr;
	D3DLIGHT2 Light;
	bool LightSet = false;

	// Interface initialization functions
	void InitInterface();
	void ReleaseInterface();

public:
	m_IDirect3DLight(IDirect3DLight *aOriginal) : ProxyInterface(aOriginal)
	{
		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ")");

		if (Config.Dd7to9)
		{
			Logging::Log() << __FUNCTION__ << " (" << this << ") Warning: created from non-dd7to9 interface!";
		}

		InitInterface();

		ProxyAddressLookupTable.SaveAddress(this, (ProxyInterface) ? ProxyInterface : (void*)this);
	}
	m_IDirect3DLight(m_IDirect3DX *D3D) : D3DInterface(D3D)
	{
		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ")");

		InitInterface();

		ProxyAddressLookupTable.SaveAddress(this, (ProxyInterface) ? ProxyInterface : (void*)this);
	}
	~m_IDirect3DLight()
	{
		LOG_LIMIT(3, __FUNCTION__ << " (" << this << ")" << " deleting interface!");

		ReleaseInterface();

		ProxyAddressLookupTable.DeleteAddress(this);
	}

	void SetProxy(IDirect3DLight* NewProxyInterface, m_IDirect3DX* NewD3DInterface)
	{
		if (NewProxyInterface || NewD3DInterface)
		{
			RefCount = 1;
			ProxyInterface = NewProxyInterface;
			D3DInterface = NewD3DInterface;
			InitInterface();
			ProxyAddressLookupTable.SaveAddress(this, (ProxyInterface) ? ProxyInterface : (void*)this);
		}
		else
		{
			ReleaseInterface();
			ProxyAddressLookupTable.DeleteAddress(this);
			ProxyInterface = nullptr;
			D3DInterface = nullptr;
		}
	}

	/*** IUnknown methods ***/
	IFACEMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj) override;
	IFACEMETHOD_(ULONG, AddRef)(THIS) override;
	IFACEMETHOD_(ULONG, Release)(THIS) override;

	/*** IDirect3DLight methods ***/
	IFACEMETHOD(Initialize)(THIS_ LPDIRECT3D) override;
	IFACEMETHOD(SetLight)(THIS_ LPD3DLIGHT) override;
	IFACEMETHOD(GetLight)(THIS_ LPD3DLIGHT) override;

	// Helper function
	void ClearD3D() { D3DInterface = nullptr; }
	static m_IDirect3DLight* CreateDirect3DLight(IDirect3DLight* aOriginal, m_IDirect3DX* NewD3DInterface);
};
