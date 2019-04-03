#pragma once

class m_IDirectSoundCaptureFXNoiseSuppress8 : public IDirectSoundCaptureFXNoiseSuppress8, public AddressLookupTableDsoundObject
{
private:
	LPDIRECTSOUNDCAPTUREFXNOISESUPPRESS8 ProxyInterface;

public:
	m_IDirectSoundCaptureFXNoiseSuppress8(LPDIRECTSOUNDCAPTUREFXNOISESUPPRESS8 pSound8) : ProxyInterface(pSound8)
	{
		ProxyAddressLookupTableDsound.SaveAddress(this, ProxyInterface);
	}
	~m_IDirectSoundCaptureFXNoiseSuppress8()
	{
		ProxyAddressLookupTableDsound.DeleteAddress(this);
	}

	LPDIRECTSOUNDCAPTUREFXNOISESUPPRESS8 GetProxyInterface() { return ProxyInterface; }

	// IUnknown methods
	STDMETHOD(QueryInterface)(THIS_ _In_ REFIID, _Outptr_ LPVOID*);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	// IDirectSoundCaptureFXNoiseSuppress methods
	STDMETHOD(SetAllParameters)(THIS_ _In_ LPCDSCFXNoiseSuppress pcDscFxNoiseSuppress);
	STDMETHOD(GetAllParameters)(THIS_ _Out_ LPDSCFXNoiseSuppress pDscFxNoiseSuppress);
	STDMETHOD(Reset)(THIS);
};
