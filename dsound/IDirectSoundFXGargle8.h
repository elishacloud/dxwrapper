#pragma once

class m_IDirectSoundFXGargle8 final : public IDirectSoundFXGargle8, AddressLookupTableDsoundObject<m_IDirectSoundFXGargle8>
{
private:
	LPDIRECTSOUNDFXGARGLE8 ProxyInterface;

public:
	m_IDirectSoundFXGargle8(LPDIRECTSOUNDFXGARGLE8 pSound8) : AddressLookupTableDsoundObject(pSound8), ProxyInterface(pSound8)
	{
		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ")");
	}
	~m_IDirectSoundFXGargle8()
	{
		LOG_LIMIT(3, __FUNCTION__ << " (" << this << ")" << " deleting interface!");
	}

	// IUnknown methods
	IFACEMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) override;
	IFACEMETHOD_(ULONG, AddRef)(THIS) override;
	IFACEMETHOD_(ULONG, Release)(THIS) override;

	// IDirectSoundFXGargle methods
	IFACEMETHOD(SetAllParameters)(THIS_ _In_ LPCDSFXGargle pcDsFxGargle) override;
	IFACEMETHOD(GetAllParameters)(THIS_ _Out_ LPDSFXGargle pDsFxGargle) override;

	// Helper functions
	LPDIRECTSOUNDFXGARGLE8 GetProxyInterface() { return ProxyInterface; }
};
