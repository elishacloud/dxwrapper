#pragma once

class m_IDirectSoundFXDistortion8 final : public IDirectSoundFXDistortion8, AddressLookupTableDsoundObject<m_IDirectSoundFXDistortion8>
{
private:
	LPDIRECTSOUNDFXDISTORTION8 ProxyInterface;

public:
	m_IDirectSoundFXDistortion8(LPDIRECTSOUNDFXDISTORTION8 pSound8) : AddressLookupTableDsoundObject(pSound8), ProxyInterface(pSound8)
	{
		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ")");
	}
	~m_IDirectSoundFXDistortion8()
	{
		LOG_LIMIT(3, __FUNCTION__ << " (" << this << ")" << " deleting interface!");
	}

	// IUnknown methods
	IFACEMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) override;
	IFACEMETHOD_(ULONG, AddRef)(THIS) override;
	IFACEMETHOD_(ULONG, Release)(THIS) override;

	// IDirectSoundFXDistortion methods
	IFACEMETHOD(SetAllParameters)(THIS_ _In_ LPCDSFXDistortion pcDsFxDistortion) override;
	IFACEMETHOD(GetAllParameters)(THIS_ _Out_ LPDSFXDistortion pDsFxDistortion) override;

	// Helper functions
	LPDIRECTSOUNDFXDISTORTION8 GetProxyInterface() { return ProxyInterface; }
};
