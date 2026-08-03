#pragma once

class m_IDirectSoundCaptureBuffer8 final : public IDirectSoundCaptureBuffer8, AddressLookupTableDsoundObject<m_IDirectSoundCaptureBuffer8>
{
private:
	LPDIRECTSOUNDCAPTUREBUFFER8 ProxyInterface;

public:
	m_IDirectSoundCaptureBuffer8(LPDIRECTSOUNDCAPTUREBUFFER8 pSound8) : AddressLookupTableDsoundObject(pSound8), ProxyInterface(pSound8)
	{
		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ")");
	}
	~m_IDirectSoundCaptureBuffer8()
	{
		LOG_LIMIT(3, __FUNCTION__ << " (" << this << ")" << " deleting interface!");
	}

	// IUnknown methods
	IFACEMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) override;
	IFACEMETHOD_(ULONG, AddRef)(THIS) override;
	IFACEMETHOD_(ULONG, Release)(THIS) override;

	// IDirectSoundCaptureBuffer methods
	IFACEMETHOD(GetCaps)(THIS_ _Out_ LPDSCBCAPS pDSCBCaps) override;
	IFACEMETHOD(GetCurrentPosition)(THIS_ _Out_opt_ LPDWORD pdwCapturePosition, _Out_opt_ LPDWORD pdwReadPosition) override;
	IFACEMETHOD(GetFormat)(THIS_ _Out_writes_bytes_opt_(dwSizeAllocated) LPWAVEFORMATEX pwfxFormat, DWORD dwSizeAllocated, _Out_opt_ LPDWORD pdwSizeWritten) override;
	IFACEMETHOD(GetStatus)(THIS_ _Out_ LPDWORD pdwStatus) override;
	IFACEMETHOD(Initialize)(THIS_ _In_ LPDIRECTSOUNDCAPTURE pDirectSoundCapture, _In_ LPCDSCBUFFERDESC pcDSCBufferDesc) override;
	IFACEMETHOD(Lock)(THIS_ DWORD dwOffset, DWORD dwBytes,
		_Outptr_result_bytebuffer_(*pdwAudioBytes1) LPVOID *ppvAudioPtr1, _Out_ LPDWORD pdwAudioBytes1,
		_Outptr_opt_result_bytebuffer_(*pdwAudioBytes2) LPVOID *ppvAudioPtr2, _Out_opt_ LPDWORD pdwAudioBytes2, DWORD dwFlags) override;
	IFACEMETHOD(Start)(THIS_ DWORD dwFlags) override;
	IFACEMETHOD(Stop)(THIS) override;
	IFACEMETHOD(Unlock)(THIS_ _In_reads_bytes_(dwAudioBytes1) LPVOID pvAudioPtr1, DWORD dwAudioBytes1,
		_In_reads_bytes_opt_(dwAudioBytes2) LPVOID pvAudioPtr2, DWORD dwAudioBytes2) override;

	// IDirectSoundCaptureBuffer8 methods
	IFACEMETHOD(GetObjectInPath)(THIS_ _In_ REFGUID rguidObject, DWORD dwIndex, _In_ REFGUID rguidInterface, _Outptr_ LPVOID *ppObject) override;
	IFACEMETHOD(GetFXStatus)(DWORD dwEffectsCount, _Out_writes_(dwEffectsCount) LPDWORD pdwFXStatus) override;

	// Helper functions
	LPDIRECTSOUNDCAPTUREBUFFER8 GetProxyInterface() { return ProxyInterface; }
};
