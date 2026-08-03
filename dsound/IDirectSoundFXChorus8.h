#pragma once

class m_IDirectSoundFXChorus8 final : public IDirectSoundFXChorus8, AddressLookupTableDsoundObject<m_IDirectSoundFXChorus8>
{
private:
	LPDIRECTSOUNDFXCHORUS8 ProxyInterface;

public:
	m_IDirectSoundFXChorus8(LPDIRECTSOUNDFXCHORUS8 pSound8) : AddressLookupTableDsoundObject(pSound8), ProxyInterface(pSound8)
	{
		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ")");
	}
	~m_IDirectSoundFXChorus8()
	{
		LOG_LIMIT(3, __FUNCTION__ << " (" << this << ")" << " deleting interface!");
	}

	// IUnknown methods
	IFACEMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) override;
	IFACEMETHOD_(ULONG, AddRef)(THIS) override;
	IFACEMETHOD_(ULONG, Release)(THIS) override;

	// IDirectSoundFXChorus methods
	IFACEMETHOD(SetAllParameters)(THIS_ _In_ LPCDSFXChorus pcDsFxChorus) override;
	IFACEMETHOD(GetAllParameters)(THIS_ _Out_ LPDSFXChorus pDsFxChorus) override;

	// Helper functions
	LPDIRECTSOUNDFXCHORUS8 GetProxyInterface() { return ProxyInterface; }
};
