#pragma once

class m_IDirect3DLight : public IDirect3DLight, public AddressLookupTableDdrawObject
{
private:
	IDirect3DLight *ProxyInterface = nullptr;
	REFIID WrapperID = IID_IDirect3DLight;
	ULONG RefCount = 1;

	// Convert Material
	m_IDirect3DDeviceX **D3DDeviceInterface = nullptr;

	// Interface initialization functions
	void InitLight();
	void ReleaseLight();

public:
	m_IDirect3DLight(IDirect3DLight *aOriginal) : ProxyInterface(aOriginal)
	{
		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << "(" << this << ")");

		InitLight();

		ProxyAddressLookupTable.SaveAddress(this, (ProxyInterface) ? ProxyInterface : (void*)this);
	}
	m_IDirect3DLight(m_IDirect3DDeviceX **D3DDInterface) : D3DDeviceInterface(D3DDInterface)
	{
		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << "(" << this << ")");

		InitLight();

		ProxyAddressLookupTable.SaveAddress(this, (ProxyInterface) ? ProxyInterface : (void*)this);
	}
	~m_IDirect3DLight()
	{
		LOG_LIMIT(3, __FUNCTION__ << "(" << this << ")" << " deleting interface!");

		ReleaseLight();

		ProxyAddressLookupTable.DeleteAddress(this);
	}

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	/*** IDirect3DLight methods ***/
	STDMETHOD(Initialize)(THIS_ LPDIRECT3D);
	STDMETHOD(SetLight)(THIS_ LPD3DLIGHT);
	STDMETHOD(GetLight)(THIS_ LPD3DLIGHT);
};
