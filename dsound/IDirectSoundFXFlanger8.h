#pragma once

class m_IDirectSoundFXFlanger8 final : public IDirectSoundFXFlanger8, AddressLookupTableDsoundObject<m_IDirectSoundFXFlanger8>
{
private:
	LPDIRECTSOUNDFXFLANGER8 ProxyInterface;

public:
	m_IDirectSoundFXFlanger8(LPDIRECTSOUNDFXFLANGER8 pSound8) : AddressLookupTableDsoundObject(pSound8), ProxyInterface(pSound8)
	{
		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ")");
	}
	~m_IDirectSoundFXFlanger8()
	{
		LOG_LIMIT(3, __FUNCTION__ << " (" << this << ")" << " deleting interface!");
	}

	// IUnknown methods
	IFACEMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) override;
	IFACEMETHOD_(ULONG, AddRef)(THIS) override;
	IFACEMETHOD_(ULONG, Release)(THIS) override;

	// IDirectSoundFXFlanger methods
	IFACEMETHOD(SetAllParameters)(THIS_ _In_ LPCDSFXFlanger pcDsFxFlanger) override;
	IFACEMETHOD(GetAllParameters)(THIS_ _Out_ LPDSFXFlanger pDsFxFlanger) override;

	// Helper functions
	LPDIRECTSOUNDFXFLANGER8 GetProxyInterface() { return ProxyInterface; }
};
