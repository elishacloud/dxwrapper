#pragma once

class m_IDirectSoundFXChorus8 : public IDirectSoundFXChorus8, public AddressLookupTableDsoundObject
{
private:
	LPDIRECTSOUNDFXCHORUS8 ProxyInterface;

public:
	m_IDirectSoundFXChorus8(LPDIRECTSOUNDFXCHORUS8 pSound8) : ProxyInterface(pSound8)
	{
		ProxyAddressLookupTableDsound.SaveAddress(this, ProxyInterface);
	}
	~m_IDirectSoundFXChorus8()
	{
		ProxyAddressLookupTableDsound.DeleteAddress(this);
	}

	LPDIRECTSOUNDFXCHORUS8 GetProxyInterface() { return ProxyInterface; }

	// IUnknown methods
	STDMETHOD(QueryInterface)(THIS_ _In_ REFIID, _Outptr_ LPVOID*);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	// IDirectSoundFXChorus methods
	STDMETHOD(SetAllParameters)(THIS_ _In_ LPCDSFXChorus pcDsFxChorus);
	STDMETHOD(GetAllParameters)(THIS_ _Out_ LPDSFXChorus pDsFxChorus);
};
