#pragma once

class m_IDirectSoundCaptureFXNoiseSuppress8 : public IDirectSoundCaptureFXNoiseSuppress8, public AddressLookupTableDsoundObject
{
private:
	LPDIRECTSOUNDCAPTUREFXNOISESUPPRESS8 ProxyInterface;

public:
	m_IDirectSoundCaptureFXNoiseSuppress8(LPDIRECTSOUNDCAPTUREFXNOISESUPPRESS8 pSound8) : ProxyInterface(pSound8)
	{
		LOG_LIMIT(3, "Creating device " << __FUNCTION__ << "(" << this << ")");

		ProxyAddressLookupTableDsound.SaveAddress(this, ProxyInterface);
	}
	~m_IDirectSoundCaptureFXNoiseSuppress8()
	{
		LOG_LIMIT(3, __FUNCTION__ << "(" << this << ")" << " deleting device!");

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
