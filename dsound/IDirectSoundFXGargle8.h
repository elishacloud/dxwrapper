#pragma once

class m_IDirectSoundFXGargle8 : public IDirectSoundFXGargle8, public AddressLookupTableDsoundObject
{
private:
	LPDIRECTSOUNDFXGARGLE8 ProxyInterface;

public:
	m_IDirectSoundFXGargle8(LPDIRECTSOUNDFXGARGLE8 pSound8) : ProxyInterface(pSound8)
	{
		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ")");

		ProxyAddressLookupTableDsound.SaveAddress(this, ProxyInterface);
	}
	~m_IDirectSoundFXGargle8()
	{
		LOG_LIMIT(3, __FUNCTION__ << " (" << this << ")" << " deleting interface!");

		ProxyAddressLookupTableDsound.DeleteAddress(this);
	}

	// IUnknown methods
	STDMETHOD(QueryInterface)(THIS_ _In_ REFIID, _Outptr_ LPVOID*);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	// IDirectSoundFXGargle methods
	STDMETHOD(SetAllParameters)(THIS_ _In_ LPCDSFXGargle pcDsFxGargle);
	STDMETHOD(GetAllParameters)(THIS_ _Out_ LPDSFXGargle pDsFxGargle);

	// Helper functions
	LPDIRECTSOUNDFXGARGLE8 GetProxyInterface() { return ProxyInterface; }
};
