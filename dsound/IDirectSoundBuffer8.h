#pragma once

struct AUDIOCLIP
{
	DWORD ds_ThreadID = 0;
	CRITICAL_SECTION dics;
	LPDIRECTSOUNDBUFFER8 ProxyInterface = nullptr;
	LONG CurrentVolume = 0;
	LARGE_INTEGER StartingTime, EndingTime, Frequency;
	bool PendingStop = false;
};

class m_IDirectSoundBuffer8 : public IDirectSoundBuffer8, public AddressLookupTableDsoundObject
{
private:
	LPDIRECTSOUNDBUFFER8 ProxyInterface;

	// Set varables
	AUDIOCLIP AudioClip;

protected:
	DWORD m_dwOldWriteCursorPos = 0;
	BYTE m_nWriteCursorIdent = 0;

	bool m_bIsPrimary = false;

public:
	m_IDirectSoundBuffer8(LPDIRECTSOUNDBUFFER8 pSound8) : ProxyInterface(pSound8)
	{
		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ")");

		AudioClip.ProxyInterface = ProxyInterface;

		// Initialize Critical Section
		InitializeCriticalSection(&AudioClip.dics);

		ProxyAddressLookupTableDsound.SaveAddress(this, ProxyInterface);
	}
	~m_IDirectSoundBuffer8()
	{
		LOG_LIMIT(3, __FUNCTION__ << " (" << this << ")" << " deleting interface!");

		// Delete Critical Section
		DeleteCriticalSection(&AudioClip.dics);

		ProxyAddressLookupTableDsound.DeleteAddress(this);
	}

	// IUnknown methods
	STDMETHOD(QueryInterface)(THIS_ _In_ REFIID, _Outptr_ LPVOID*);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	// IDirectSoundBuffer8 methods
	STDMETHOD(GetCaps)(THIS_ _Out_ LPDSBCAPS pDSBufferCaps);
	STDMETHOD(GetCurrentPosition)(THIS_ _Out_opt_ LPDWORD pdwCurrentPlayCursor, _Out_opt_ LPDWORD pdwCurrentWriteCursor);
	STDMETHOD(GetFormat)(THIS_ _Out_writes_bytes_opt_(dwSizeAllocated) LPWAVEFORMATEX pwfxFormat, DWORD dwSizeAllocated, _Out_opt_ LPDWORD pdwSizeWritten);
	STDMETHOD(GetVolume)(THIS_ _Out_ LPLONG plVolume);
	STDMETHOD(GetPan)(THIS_ _Out_ LPLONG plPan);
	STDMETHOD(GetFrequency)(THIS_ _Out_ LPDWORD pdwFrequency);
	STDMETHOD(GetStatus)(THIS_ _Out_ LPDWORD pdwStatus);
	STDMETHOD(Initialize)(THIS_ _In_ LPDIRECTSOUND pDirectSound, _In_ LPCDSBUFFERDESC pcDSBufferDesc);
	STDMETHOD(Lock)(THIS_ DWORD dwOffset, DWORD dwBytes,
		_Outptr_result_bytebuffer_(*pdwAudioBytes1) LPVOID *ppvAudioPtr1, _Out_ LPDWORD pdwAudioBytes1,
		_Outptr_opt_result_bytebuffer_(*pdwAudioBytes2) LPVOID *ppvAudioPtr2, _Out_opt_ LPDWORD pdwAudioBytes2, DWORD dwFlags);
	STDMETHOD(Play)(THIS_ DWORD dwReserved1, DWORD dwPriority, DWORD dwFlags);
	STDMETHOD(SetCurrentPosition)(THIS_ DWORD dwNewPosition);
	STDMETHOD(SetFormat)(THIS_ _In_ LPCWAVEFORMATEX pcfxFormat);
	STDMETHOD(SetVolume)(THIS_ LONG lVolume);
	STDMETHOD(SetPan)(THIS_ LONG lPan);
	STDMETHOD(SetFrequency)(THIS_ DWORD dwFrequency);
	STDMETHOD(Stop)(THIS);
	STDMETHOD(Unlock)(THIS_ _In_reads_bytes_(dwAudioBytes1) LPVOID pvAudioPtr1, DWORD dwAudioBytes1,
		_In_reads_bytes_opt_(dwAudioBytes2) LPVOID pvAudioPtr2, DWORD dwAudioBytes2);
	STDMETHOD(Restore)(THIS);

	// IDirectSoundBuffer8 methods
	STDMETHOD(SetFX)(THIS_ DWORD dwEffectsCount, _In_reads_opt_(dwEffectsCount) LPDSEFFECTDESC pDSFXDesc, _Out_writes_opt_(dwEffectsCount) LPDWORD pdwResultCodes);
	STDMETHOD(AcquireResources)(THIS_ DWORD dwFlags, DWORD dwEffectsCount, _Out_writes_(dwEffectsCount) LPDWORD pdwResultCodes);
	STDMETHOD(GetObjectInPath)(THIS_ _In_ REFGUID rguidObject, DWORD dwIndex, _In_ REFGUID rguidInterface, _Outptr_ LPVOID *ppObject);

	// Helper functions
	LPDIRECTSOUNDBUFFER8 GetProxyInterface() { return ProxyInterface; }
	bool GetPrimaryBuffer()
	{
		return m_bIsPrimary;
	};
	void SetPrimaryBuffer(bool bIsPrimary)
	{
		m_bIsPrimary = bIsPrimary;
	};
};
