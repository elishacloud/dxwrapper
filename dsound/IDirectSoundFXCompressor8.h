#pragma once

class m_IDirectSoundFXCompressor8 final : public IDirectSoundFXCompressor8, AddressLookupTableDsoundObject<m_IDirectSoundFXCompressor8>
{
private:
	LPDIRECTSOUNDFXCOMPRESSOR8 ProxyInterface;

public:
	m_IDirectSoundFXCompressor8(LPDIRECTSOUNDFXCOMPRESSOR8 pSound8) : AddressLookupTableDsoundObject(pSound8), ProxyInterface(pSound8)
	{
		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ")");
	}
	~m_IDirectSoundFXCompressor8()
	{
		LOG_LIMIT(3, __FUNCTION__ << " (" << this << ")" << " deleting interface!");
	}

	// IUnknown methods
	IFACEMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) override;
	IFACEMETHOD_(ULONG, AddRef)(THIS) override;
	IFACEMETHOD_(ULONG, Release)(THIS) override;

	// IDirectSoundFXCompressor methods
	IFACEMETHOD(SetAllParameters)(THIS_ _In_ LPCDSFXCompressor pcDsFxCompressor) override;
	IFACEMETHOD(GetAllParameters)(THIS_ _Out_ LPDSFXCompressor pDsFxCompressor) override;

	// Helper functions
	LPDIRECTSOUNDFXCOMPRESSOR8 GetProxyInterface() { return ProxyInterface; }
};
