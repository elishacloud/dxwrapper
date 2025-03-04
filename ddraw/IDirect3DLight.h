#pragma once

m_IDirect3DLight* CreateDirect3DLight(IDirect3DLight* aOriginal, m_IDirect3DX* NewD3DInterface);

class m_IDirect3DLight : public IDirect3DLight, public AddressLookupTableDdrawObject
{
private:
	IDirect3DLight *ProxyInterface = nullptr;
	REFIID WrapperID = IID_IDirect3DLight;
	ULONG RefCount = 1;

	// Convert Light
	m_IDirect3DX* D3DInterface = nullptr;
	m_IDirect3DDeviceX** D3DDeviceInterface = nullptr;
	D3DLIGHT2 Light;
	bool LightSet = false;

	// Helper functions
	HRESULT CheckInterface(char* FunctionName);

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
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	/*** IDirect3DLight methods ***/
	STDMETHOD(Initialize)(THIS_ LPDIRECT3D);
	STDMETHOD(SetLight)(THIS_ LPD3DLIGHT);
	STDMETHOD(GetLight)(THIS_ LPD3DLIGHT);

	// Helper function
	m_IDirect3DDeviceX* GetD3DDevice();
	inline void ClearD3D() { D3DInterface = nullptr; D3DDeviceInterface = nullptr; }
};
