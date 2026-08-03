#pragma once

class m_IDirectSoundCaptureFXAec8 final : public IDirectSoundCaptureFXAec8, AddressLookupTableDsoundObject<m_IDirectSoundCaptureFXAec8>
{
private:
	LPDIRECTSOUNDCAPTUREFXAEC8 ProxyInterface;

public:
	m_IDirectSoundCaptureFXAec8(LPDIRECTSOUNDCAPTUREFXAEC8 pSound8) : AddressLookupTableDsoundObject(pSound8), ProxyInterface(pSound8)
	{
		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ")");
	}
	~m_IDirectSoundCaptureFXAec8()
	{
		LOG_LIMIT(3, __FUNCTION__ << " (" << this << ")" << " deleting interface!");
	}

	// IUnknown methods
	IFACEMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) override;
	IFACEMETHOD_(ULONG, AddRef)(THIS) override;
	IFACEMETHOD_(ULONG, Release)(THIS) override;

	// IDirectSoundCaptureFXAec methods
	IFACEMETHOD(SetAllParameters)(THIS_ _In_ LPCDSCFXAec pDscFxAec) override;
	IFACEMETHOD(GetAllParameters)(THIS_ _Out_ LPDSCFXAec pDscFxAec) override;
	IFACEMETHOD(GetStatus)(THIS_ _Out_ LPDWORD pdwStatus) override;
	IFACEMETHOD(Reset)(THIS) override;

	// Helper functions
	LPDIRECTSOUNDCAPTUREFXAEC8 GetProxyInterface() { return ProxyInterface; }
};
