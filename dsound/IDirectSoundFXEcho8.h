#pragma once

class m_IDirectSoundFXEcho8 final : public IDirectSoundFXEcho8, AddressLookupTableDsoundObject<m_IDirectSoundFXEcho8>
{
private:
	LPDIRECTSOUNDFXECHO8 ProxyInterface;

public:
	m_IDirectSoundFXEcho8(LPDIRECTSOUNDFXECHO8 pSound8) : AddressLookupTableDsoundObject(pSound8), ProxyInterface(pSound8)
	{
		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ")");
	}
	~m_IDirectSoundFXEcho8()
	{
		LOG_LIMIT(3, __FUNCTION__ << " (" << this << ")" << " deleting interface!");
	}

	// IUnknown methods
	IFACEMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) override;
	IFACEMETHOD_(ULONG, AddRef)(THIS) override;
	IFACEMETHOD_(ULONG, Release)(THIS) override;

	// IDirectSoundFXEcho methods
	IFACEMETHOD(SetAllParameters)(THIS_ _In_ LPCDSFXEcho pcDsFxEcho) override;
	IFACEMETHOD(GetAllParameters)(THIS_ _Out_ LPDSFXEcho pDsFxEcho) override;

	// Helper functions
	LPDIRECTSOUNDFXECHO8 GetProxyInterface() { return ProxyInterface; }
};
