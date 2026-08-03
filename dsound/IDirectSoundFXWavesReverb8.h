#pragma once

class m_IDirectSoundFXWavesReverb8 final : public IDirectSoundFXWavesReverb8, AddressLookupTableDsoundObject<m_IDirectSoundFXWavesReverb8>
{
private:
	LPDIRECTSOUNDFXWAVESREVERB8 ProxyInterface;

public:
	m_IDirectSoundFXWavesReverb8(LPDIRECTSOUNDFXWAVESREVERB8 pSound8) : AddressLookupTableDsoundObject(pSound8), ProxyInterface(pSound8)
	{
		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ")");
	}
	~m_IDirectSoundFXWavesReverb8()
	{
		LOG_LIMIT(3, __FUNCTION__ << " (" << this << ")" << " deleting interface!");
	}

	// IUnknown methods
	IFACEMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) override;
	IFACEMETHOD_(ULONG, AddRef)(THIS) override;
	IFACEMETHOD_(ULONG, Release)(THIS) override;

	// IDirectSoundFXWavesReverb methods
	IFACEMETHOD(SetAllParameters)(THIS_ _In_ LPCDSFXWavesReverb pcDsFxWavesReverb) override;
	IFACEMETHOD(GetAllParameters)(THIS_ _Out_ LPDSFXWavesReverb pDsFxWavesReverb) override;

	// Helper functions
	LPDIRECTSOUNDFXWAVESREVERB8 GetProxyInterface() { return ProxyInterface; }
};
