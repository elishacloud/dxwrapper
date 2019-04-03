#pragma once

class m_IDirectSoundFXParamEq8 : public IDirectSoundFXParamEq8, public AddressLookupTableDsoundObject
{
private:
	LPDIRECTSOUNDFXPARAMEQ8 ProxyInterface;

public:
	m_IDirectSoundFXParamEq8(LPDIRECTSOUNDFXPARAMEQ8 pSound8) : ProxyInterface(pSound8)
	{
		ProxyAddressLookupTableDsound.SaveAddress(this, ProxyInterface);
	}
	~m_IDirectSoundFXParamEq8()
	{
		ProxyAddressLookupTableDsound.DeleteAddress(this);
	}

	LPDIRECTSOUNDFXPARAMEQ8 GetProxyInterface() { return ProxyInterface; }

	// IUnknown methods
	STDMETHOD(QueryInterface)(THIS_ _In_ REFIID, _Outptr_ LPVOID*);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	// IDirectSoundFXParamEq methods
	STDMETHOD(SetAllParameters)(THIS_ _In_ LPCDSFXParamEq pcDsFxParamEq);
	STDMETHOD(GetAllParameters)(THIS_ _Out_ LPDSFXParamEq pDsFxParamEq);
};
