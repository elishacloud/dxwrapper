#pragma once

class m_IDirectSoundCapture8 final : public IDirectSoundCapture8, AddressLookupTableDsoundObject<m_IDirectSoundCapture8>
{
private:
	LPDIRECTSOUNDCAPTURE8 ProxyInterface;

public:
	m_IDirectSoundCapture8(LPDIRECTSOUNDCAPTURE8 pSound8) : AddressLookupTableDsoundObject(pSound8), ProxyInterface(pSound8)
	{
		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ")");
	}
	~m_IDirectSoundCapture8()
	{
		LOG_LIMIT(3, __FUNCTION__ << " (" << this << ")" << " deleting interface!");
	}

	// IUnknown methods
	IFACEMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) override;
	IFACEMETHOD_(ULONG, AddRef)(THIS) override;
	IFACEMETHOD_(ULONG, Release)(THIS) override;

	// IDirectSoundCapture methods
	IFACEMETHOD(CreateCaptureBuffer)(THIS_ _In_ LPCDSCBUFFERDESC pcDSCBufferDesc, _Outptr_ LPDIRECTSOUNDCAPTUREBUFFER *ppDSCBuffer, _Pre_null_ LPUNKNOWN pUnkOuter) override;
	IFACEMETHOD(GetCaps)(THIS_ _Out_ LPDSCCAPS pDSCCaps) override;
	IFACEMETHOD(Initialize)(THIS_ _In_opt_ LPCGUID pcGuidDevice) override;

	// Helper functions
	LPDIRECTSOUNDCAPTURE8 GetProxyInterface() { return ProxyInterface; }
};
