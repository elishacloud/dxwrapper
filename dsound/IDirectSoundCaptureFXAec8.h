#pragma once

class m_IDirectSoundCaptureFXAec8 : public IDirectSoundCaptureFXAec8, public AddressLookupTableDsoundObject
{
private:
	LPDIRECTSOUNDCAPTUREFXAEC8 ProxyInterface;

public:
	m_IDirectSoundCaptureFXAec8(LPDIRECTSOUNDCAPTUREFXAEC8 pSound8) : ProxyInterface(pSound8)
	{
		LOG_LIMIT(3, "Creating device " << __FUNCTION__ << "(" << this << ")");

		ProxyAddressLookupTableDsound.SaveAddress(this, ProxyInterface);
	}
	~m_IDirectSoundCaptureFXAec8()
	{
		LOG_LIMIT(3, __FUNCTION__ << "(" << this << ")" << " deleting device!");

		ProxyAddressLookupTableDsound.DeleteAddress(this);
	}

	LPDIRECTSOUNDCAPTUREFXAEC8 GetProxyInterface() { return ProxyInterface; }

	// IUnknown methods
	STDMETHOD(QueryInterface)(THIS_ _In_ REFIID, _Outptr_ LPVOID*);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	// IDirectSoundCaptureFXAec methods
	STDMETHOD(SetAllParameters)(THIS_ _In_ LPCDSCFXAec pDscFxAec);
	STDMETHOD(GetAllParameters)(THIS_ _Out_ LPDSCFXAec pDscFxAec);
	STDMETHOD(GetStatus)(THIS_ _Out_ LPDWORD pdwStatus);
	STDMETHOD(Reset)(THIS);
};
