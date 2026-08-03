#pragma once

class m_IDirectSoundCaptureFXNoiseSuppress8 final : public IDirectSoundCaptureFXNoiseSuppress8, AddressLookupTableDsoundObject<m_IDirectSoundCaptureFXNoiseSuppress8>
{
private:
	LPDIRECTSOUNDCAPTUREFXNOISESUPPRESS8 ProxyInterface;

public:
	m_IDirectSoundCaptureFXNoiseSuppress8(LPDIRECTSOUNDCAPTUREFXNOISESUPPRESS8 pSound8) : AddressLookupTableDsoundObject(pSound8), ProxyInterface(pSound8)
	{
		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ")");
	}
	~m_IDirectSoundCaptureFXNoiseSuppress8()
	{
		LOG_LIMIT(3, __FUNCTION__ << " (" << this << ")" << " deleting interface!");
	}

	// IUnknown methods
	IFACEMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) override;
	IFACEMETHOD_(ULONG, AddRef)(THIS) override;
	IFACEMETHOD_(ULONG, Release)(THIS) override;

	// IDirectSoundCaptureFXNoiseSuppress methods
	IFACEMETHOD(SetAllParameters)(THIS_ _In_ LPCDSCFXNoiseSuppress pcDscFxNoiseSuppress) override;
	IFACEMETHOD(GetAllParameters)(THIS_ _Out_ LPDSCFXNoiseSuppress pDscFxNoiseSuppress) override;
	IFACEMETHOD(Reset)(THIS) override;

	// Helper functions
	LPDIRECTSOUNDCAPTUREFXNOISESUPPRESS8 GetProxyInterface() { return ProxyInterface; }
};
