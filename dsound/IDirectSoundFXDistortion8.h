#pragma once

class m_IDirectSoundFXDistortion8 : public IDirectSoundFXDistortion8, public AddressLookupTableDsoundObject
{
private:
	LPDIRECTSOUNDFXDISTORTION8 ProxyInterface;

public:
	m_IDirectSoundFXDistortion8(LPDIRECTSOUNDFXDISTORTION8 pSound8) : ProxyInterface(pSound8)
	{
		ProxyAddressLookupTableDsound.SaveAddress(this, ProxyInterface);
	}
	~m_IDirectSoundFXDistortion8()
	{
		ProxyAddressLookupTableDsound.DeleteAddress(this);
	}

	LPDIRECTSOUNDFXDISTORTION8 GetProxyInterface() { return ProxyInterface; }

	// IUnknown methods
	STDMETHOD(QueryInterface)(THIS_ _In_ REFIID, _Outptr_ LPVOID*);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	// IDirectSoundFXDistortion methods
	STDMETHOD(SetAllParameters)(THIS_ _In_ LPCDSFXDistortion pcDsFxDistortion);
	STDMETHOD(GetAllParameters)(THIS_ _Out_ LPDSFXDistortion pDsFxDistortion);
};
