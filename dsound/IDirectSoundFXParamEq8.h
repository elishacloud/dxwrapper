#pragma once

class m_IDirectSoundFXParamEq8 final : public IDirectSoundFXParamEq8, AddressLookupTableDsoundObject<m_IDirectSoundFXParamEq8>
{
private:
	LPDIRECTSOUNDFXPARAMEQ8 ProxyInterface;

public:
	m_IDirectSoundFXParamEq8(LPDIRECTSOUNDFXPARAMEQ8 pSound8) : AddressLookupTableDsoundObject(pSound8), ProxyInterface(pSound8)
	{
		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ")");
	}
	~m_IDirectSoundFXParamEq8()
	{
		LOG_LIMIT(3, __FUNCTION__ << " (" << this << ")" << " deleting interface!");
	}

	// IUnknown methods
	IFACEMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) override;
	IFACEMETHOD_(ULONG, AddRef)(THIS) override;
	IFACEMETHOD_(ULONG, Release)(THIS) override;

	// IDirectSoundFXParamEq methods
	IFACEMETHOD(SetAllParameters)(THIS_ _In_ LPCDSFXParamEq pcDsFxParamEq) override;
	IFACEMETHOD(GetAllParameters)(THIS_ _Out_ LPDSFXParamEq pDsFxParamEq) override;

	// Helper functions
	LPDIRECTSOUNDFXPARAMEQ8 GetProxyInterface() { return ProxyInterface; }
};
