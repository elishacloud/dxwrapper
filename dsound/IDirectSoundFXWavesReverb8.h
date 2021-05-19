#pragma once

class m_IDirectSoundFXWavesReverb8 : public IDirectSoundFXWavesReverb8, public AddressLookupTableDsoundObject
{
private:
	LPDIRECTSOUNDFXWAVESREVERB8 ProxyInterface;

public:
	m_IDirectSoundFXWavesReverb8(LPDIRECTSOUNDFXWAVESREVERB8 pSound8) : ProxyInterface(pSound8)
	{
		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ")");

		ProxyAddressLookupTableDsound.SaveAddress(this, ProxyInterface);
	}
	~m_IDirectSoundFXWavesReverb8()
	{
		LOG_LIMIT(3, __FUNCTION__ << " (" << this << ")" << " deleting interface!");

		ProxyAddressLookupTableDsound.DeleteAddress(this);
	}

	// IUnknown methods
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID* ppvObj);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	// IDirectSoundFXWavesReverb methods
	STDMETHOD(SetAllParameters)(THIS_ _In_ LPCDSFXWavesReverb pcDsFxWavesReverb);
	STDMETHOD(GetAllParameters)(THIS_ _Out_ LPDSFXWavesReverb pDsFxWavesReverb);

	// Helper functions
	LPDIRECTSOUNDFXWAVESREVERB8 GetProxyInterface() { return ProxyInterface; }
};
