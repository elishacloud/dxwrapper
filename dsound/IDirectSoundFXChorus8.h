#pragma once

class m_IDirectSoundFXChorus8 : public IDirectSoundFXChorus8, public AddressLookupTableDsoundObject
{
private:
	LPDIRECTSOUNDFXCHORUS8 ProxyInterface;

public:
	m_IDirectSoundFXChorus8(LPDIRECTSOUNDFXCHORUS8 pSound8) : ProxyInterface(pSound8)
	{
		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ")");

		ProxyAddressLookupTableDsound.SaveAddress(this, ProxyInterface);
	}
	~m_IDirectSoundFXChorus8()
	{
		LOG_LIMIT(3, __FUNCTION__ << " (" << this << ")" << " deleting interface!");

		ProxyAddressLookupTableDsound.DeleteAddress(this);
	}

	// IUnknown methods
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID* ppvObj);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	// IDirectSoundFXChorus methods
	STDMETHOD(SetAllParameters)(THIS_ _In_ LPCDSFXChorus pcDsFxChorus);
	STDMETHOD(GetAllParameters)(THIS_ _Out_ LPDSFXChorus pDsFxChorus);

	// Helper functions
	LPDIRECTSOUNDFXCHORUS8 GetProxyInterface() { return ProxyInterface; }
};
