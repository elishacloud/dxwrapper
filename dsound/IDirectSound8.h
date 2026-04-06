#pragma once

class m_IDirectSound8 final : public IDirectSound8, AddressLookupTableDsoundObject<m_IDirectSound8>
{
private:
	LPDIRECTSOUND8 ProxyInterface;

public:
	m_IDirectSound8(LPDIRECTSOUND8 pSound8) : AddressLookupTableDsoundObject(pSound8), ProxyInterface(pSound8)
	{
		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ")");
	}
	~m_IDirectSound8()
	{
		LOG_LIMIT(3, __FUNCTION__ << " (" << this << ")" << " deleting interface!");
	}

	// IUnknown methods
	IFACEMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) override;
	IFACEMETHOD_(ULONG, AddRef)(THIS) override;
	IFACEMETHOD_(ULONG, Release)(THIS) override;

	// IDirectSound methods
	IFACEMETHOD(CreateSoundBuffer)(THIS_ _In_ LPCDSBUFFERDESC pcDSBufferDesc, _Out_ LPDIRECTSOUNDBUFFER *ppDSBuffer, _Pre_null_ LPUNKNOWN pUnkOuter) override;
	IFACEMETHOD(GetCaps)(THIS_ _Out_ LPDSCAPS pDSCaps) override;
	IFACEMETHOD(DuplicateSoundBuffer)(THIS_ _In_ LPDIRECTSOUNDBUFFER pDSBufferOriginal, _Out_ LPDIRECTSOUNDBUFFER *ppDSBufferDuplicate) override;
	IFACEMETHOD(SetCooperativeLevel)(THIS_ HWND hwnd, DWORD dwLevel) override;
	IFACEMETHOD(Compact)(THIS) override;
	IFACEMETHOD(GetSpeakerConfig)(THIS_ _Out_ LPDWORD pdwSpeakerConfig) override;
	IFACEMETHOD(SetSpeakerConfig)(THIS_ DWORD dwSpeakerConfig) override;
	IFACEMETHOD(Initialize)(THIS_ _In_opt_ LPCGUID pcGuidDevice) override;

	// IDirectSound8 methods
	IFACEMETHOD(VerifyCertification)(THIS_ _Out_ LPDWORD pdwCertified) override;

	// Helper functions
	LPDIRECTSOUND8 GetProxyInterface() { return ProxyInterface; }
};
