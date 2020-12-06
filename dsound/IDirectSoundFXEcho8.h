#pragma once

class m_IDirectSoundFXEcho8 : public IDirectSoundFXEcho8, public AddressLookupTableDsoundObject
{
private:
	LPDIRECTSOUNDFXECHO8 ProxyInterface;

public:
	m_IDirectSoundFXEcho8(LPDIRECTSOUNDFXECHO8 pSound8) : ProxyInterface(pSound8)
	{
		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ")");

		ProxyAddressLookupTableDsound.SaveAddress(this, ProxyInterface);
	}
	~m_IDirectSoundFXEcho8()
	{
		LOG_LIMIT(3, __FUNCTION__ << " (" << this << ")" << " deleting interface!");

		ProxyAddressLookupTableDsound.DeleteAddress(this);
	}

	// IUnknown methods
	STDMETHOD(QueryInterface)(THIS_ _In_ REFIID, _Outptr_ LPVOID*);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	// IDirectSoundFXEcho methods
	STDMETHOD(SetAllParameters)(THIS_ _In_ LPCDSFXEcho pcDsFxEcho);
	STDMETHOD(GetAllParameters)(THIS_ _Out_ LPDSFXEcho pDsFxEcho);

	// Helper functions
	LPDIRECTSOUNDFXECHO8 GetProxyInterface() { return ProxyInterface; }
};
