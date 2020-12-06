#pragma once

class m_IDirectSoundCaptureBuffer8 : public IDirectSoundCaptureBuffer8, public AddressLookupTableDsoundObject
{
private:
	LPDIRECTSOUNDCAPTUREBUFFER8 ProxyInterface;

public:
	m_IDirectSoundCaptureBuffer8(LPDIRECTSOUNDCAPTUREBUFFER8 pSound8) : ProxyInterface(pSound8)
	{
		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ")");

		ProxyAddressLookupTableDsound.SaveAddress(this, ProxyInterface);
	}
	~m_IDirectSoundCaptureBuffer8()
	{
		LOG_LIMIT(3, __FUNCTION__ << " (" << this << ")" << " deleting interface!");

		ProxyAddressLookupTableDsound.DeleteAddress(this);
	}

	// IUnknown methods
	STDMETHOD(QueryInterface)(THIS_ _In_ REFIID, _Outptr_ LPVOID*);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	// IDirectSoundCaptureBuffer methods
	STDMETHOD(GetCaps)(THIS_ _Out_ LPDSCBCAPS pDSCBCaps);
	STDMETHOD(GetCurrentPosition)(THIS_ _Out_opt_ LPDWORD pdwCapturePosition, _Out_opt_ LPDWORD pdwReadPosition);
	STDMETHOD(GetFormat)(THIS_ _Out_writes_bytes_opt_(dwSizeAllocated) LPWAVEFORMATEX pwfxFormat, DWORD dwSizeAllocated, _Out_opt_ LPDWORD pdwSizeWritten);
	STDMETHOD(GetStatus)(THIS_ _Out_ LPDWORD pdwStatus);
	STDMETHOD(Initialize)(THIS_ _In_ LPDIRECTSOUNDCAPTURE pDirectSoundCapture, _In_ LPCDSCBUFFERDESC pcDSCBufferDesc);
	STDMETHOD(Lock)(THIS_ DWORD dwOffset, DWORD dwBytes,
		_Outptr_result_bytebuffer_(*pdwAudioBytes1) LPVOID *ppvAudioPtr1, _Out_ LPDWORD pdwAudioBytes1,
		_Outptr_opt_result_bytebuffer_(*pdwAudioBytes2) LPVOID *ppvAudioPtr2, _Out_opt_ LPDWORD pdwAudioBytes2, DWORD dwFlags);
	STDMETHOD(Start)(THIS_ DWORD dwFlags);
	STDMETHOD(Stop)(THIS);
	STDMETHOD(Unlock)(THIS_ _In_reads_bytes_(dwAudioBytes1) LPVOID pvAudioPtr1, DWORD dwAudioBytes1,
		_In_reads_bytes_opt_(dwAudioBytes2) LPVOID pvAudioPtr2, DWORD dwAudioBytes2);

	// IDirectSoundCaptureBuffer8 methods
	STDMETHOD(GetObjectInPath)(THIS_ _In_ REFGUID rguidObject, DWORD dwIndex, _In_ REFGUID rguidInterface, _Outptr_ LPVOID *ppObject);
	STDMETHOD(GetFXStatus)(DWORD dwEffectsCount, _Out_writes_(dwEffectsCount) LPDWORD pdwFXStatus);

	// Helper functions
	LPDIRECTSOUNDCAPTUREBUFFER8 GetProxyInterface() { return ProxyInterface; }
};
