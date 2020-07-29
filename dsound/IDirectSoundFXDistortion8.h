#pragma once

class m_IDirectSoundFXDistortion8 : public IDirectSoundFXDistortion8, public AddressLookupTableDsoundObject
{
private:
	LPDIRECTSOUNDFXDISTORTION8 ProxyInterface;

public:
	m_IDirectSoundFXDistortion8(LPDIRECTSOUNDFXDISTORTION8 pSound8) : ProxyInterface(pSound8)
	{
		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << "(" << this << ")");

		ProxyAddressLookupTableDsound.SaveAddress(this, ProxyInterface);
	}
	~m_IDirectSoundFXDistortion8()
	{
		LOG_LIMIT(3, __FUNCTION__ << "(" << this << ")" << " deleting interface!");

		ProxyAddressLookupTableDsound.DeleteAddress(this);
	}

	// IUnknown methods
	STDMETHOD(QueryInterface)(THIS_ _In_ REFIID, _Outptr_ LPVOID*);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	// IDirectSoundFXDistortion methods
	STDMETHOD(SetAllParameters)(THIS_ _In_ LPCDSFXDistortion pcDsFxDistortion);
	STDMETHOD(GetAllParameters)(THIS_ _Out_ LPDSFXDistortion pDsFxDistortion);

	// Helper functions
	LPDIRECTSOUNDFXDISTORTION8 GetProxyInterface() { return ProxyInterface; }
};
