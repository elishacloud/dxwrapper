#pragma once

class m_IDirectSoundFXEcho8 : public IDirectSoundFXEcho8, public AddressLookupTableDsoundObject
{
private:
	LPDIRECTSOUNDFXECHO8 ProxyInterface;

public:
	m_IDirectSoundFXEcho8(LPDIRECTSOUNDFXECHO8 pSound8) : ProxyInterface(pSound8)
	{
		ProxyAddressLookupTableDsound.SaveAddress(this, ProxyInterface);
	}
	~m_IDirectSoundFXEcho8()
	{
		ProxyAddressLookupTableDsound.DeleteAddress(this);
	}

	LPDIRECTSOUNDFXECHO8 GetProxyInterface() { return ProxyInterface; }

	// IUnknown methods
	STDMETHOD(QueryInterface)(THIS_ _In_ REFIID, _Outptr_ LPVOID*);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	// IDirectSoundFXEcho methods
	STDMETHOD(SetAllParameters)(THIS_ _In_ LPCDSFXEcho pcDsFxEcho);
	STDMETHOD(GetAllParameters)(THIS_ _Out_ LPDSFXEcho pDsFxEcho);
};
