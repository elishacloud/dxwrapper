#pragma once

class m_IDirectSoundFXFlanger8 : public IDirectSoundFXFlanger8, public AddressLookupTableDsoundObject
{
private:
	LPDIRECTSOUNDFXFLANGER8 ProxyInterface;

public:
	m_IDirectSoundFXFlanger8(LPDIRECTSOUNDFXFLANGER8 pSound8) : ProxyInterface(pSound8)
	{
		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ")");

		ProxyAddressLookupTableDsound.SaveAddress(this, ProxyInterface);
	}
	~m_IDirectSoundFXFlanger8()
	{
		LOG_LIMIT(3, __FUNCTION__ << " (" << this << ")" << " deleting interface!");

		ProxyAddressLookupTableDsound.DeleteAddress(this);
	}

	// IUnknown methods
	STDMETHOD(QueryInterface)(THIS_ _In_ REFIID, _Outptr_ LPVOID*);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	// IDirectSoundFXFlanger methods
	STDMETHOD(SetAllParameters)(THIS_ _In_ LPCDSFXFlanger pcDsFxFlanger);
	STDMETHOD(GetAllParameters)(THIS_ _Out_ LPDSFXFlanger pDsFxFlanger);

	// Helper functions
	LPDIRECTSOUNDFXFLANGER8 GetProxyInterface() { return ProxyInterface; }
};
