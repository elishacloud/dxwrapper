#pragma once

class m_IDirectSoundFXCompressor8 : public IDirectSoundFXCompressor8, public AddressLookupTableDsoundObject
{
private:
	LPDIRECTSOUNDFXCOMPRESSOR8 ProxyInterface;

public:
	m_IDirectSoundFXCompressor8(LPDIRECTSOUNDFXCOMPRESSOR8 pSound8) : ProxyInterface(pSound8)
	{
		ProxyAddressLookupTableDsound.SaveAddress(this, ProxyInterface);
	}
	~m_IDirectSoundFXCompressor8()
	{
		ProxyAddressLookupTableDsound.DeleteAddress(this);
	}

	LPDIRECTSOUNDFXCOMPRESSOR8 GetProxyInterface() { return ProxyInterface; }

	// IUnknown methods
	STDMETHOD(QueryInterface)(THIS_ _In_ REFIID, _Outptr_ LPVOID*);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	// IDirectSoundFXCompressor methods
	STDMETHOD(SetAllParameters)(THIS_ _In_ LPCDSFXCompressor pcDsFxCompressor);
	STDMETHOD(GetAllParameters)(THIS_ _Out_ LPDSFXCompressor pDsFxCompressor);
};
