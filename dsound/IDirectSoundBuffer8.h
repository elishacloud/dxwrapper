#pragma once

struct AUDIOCLIP
{
	DWORD ds_ThreadID = 0;
	CRITICAL_SECTION dics = {};
	LPDIRECTSOUNDBUFFER8 ProxyInterface = nullptr;
	LONG CurrentVolume = 0;
	HANDLE hTriggerEvent = nullptr;
	bool PendingStop = false;
};

class m_IDirectSoundBuffer8 final : public IDirectSoundBuffer8, AddressLookupTableDsoundObject<m_IDirectSoundBuffer8>
{
private:
	LPDIRECTSOUNDBUFFER8 ProxyInterface;

	// Set variables
	AUDIOCLIP AudioClip;

protected:
	DWORD m_dwOldWriteCursorPos = 0;
	BYTE m_nWriteCursorIdent = 0;

	bool m_bIsPrimary = false;

public:
	m_IDirectSoundBuffer8(LPDIRECTSOUNDBUFFER8 pSound8) : AddressLookupTableDsoundObject(pSound8), ProxyInterface(pSound8)
	{
		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ")");

		AudioClip.ProxyInterface = ProxyInterface;

		// Initialize Critical Section
		InitializeCriticalSection(&AudioClip.dics);
		char EventName[MAX_PATH];
		sprintf_s(EventName, MAX_PATH, "Local\\SH2EAudioClipDetection-%u", (DWORD)this);
		AudioClip.hTriggerEvent = CreateEvent(nullptr, FALSE, FALSE, EventName);
	}
	~m_IDirectSoundBuffer8()
	{
		LOG_LIMIT(3, __FUNCTION__ << " (" << this << ")" << " deleting interface!");

		// Delete Critical Section
		DeleteCriticalSection(&AudioClip.dics);
		CloseHandle(AudioClip.hTriggerEvent);
	}

	// IUnknown methods
	IFACEMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) override;
	IFACEMETHOD_(ULONG, AddRef)(THIS) override;
	IFACEMETHOD_(ULONG, Release)(THIS) override;

	// IDirectSoundBuffer8 methods
	IFACEMETHOD(GetCaps)(THIS_ _Out_ LPDSBCAPS pDSBufferCaps) override;
	IFACEMETHOD(GetCurrentPosition)(THIS_ _Out_opt_ LPDWORD pdwCurrentPlayCursor, _Out_opt_ LPDWORD pdwCurrentWriteCursor) override;
	IFACEMETHOD(GetFormat)(THIS_ _Out_writes_bytes_opt_(dwSizeAllocated) LPWAVEFORMATEX pwfxFormat, DWORD dwSizeAllocated, _Out_opt_ LPDWORD pdwSizeWritten) override;
	IFACEMETHOD(GetVolume)(THIS_ _Out_ LPLONG plVolume) override;
	IFACEMETHOD(GetPan)(THIS_ _Out_ LPLONG plPan) override;
	IFACEMETHOD(GetFrequency)(THIS_ _Out_ LPDWORD pdwFrequency) override;
	IFACEMETHOD(GetStatus)(THIS_ _Out_ LPDWORD pdwStatus) override;
	IFACEMETHOD(Initialize)(THIS_ _In_ LPDIRECTSOUND pDirectSound, _In_ LPCDSBUFFERDESC pcDSBufferDesc) override;
	IFACEMETHOD(Lock)(THIS_ DWORD dwOffset, DWORD dwBytes,
		_Outptr_result_bytebuffer_(*pdwAudioBytes1) LPVOID *ppvAudioPtr1, _Out_ LPDWORD pdwAudioBytes1,
		_Outptr_opt_result_bytebuffer_(*pdwAudioBytes2) LPVOID *ppvAudioPtr2, _Out_opt_ LPDWORD pdwAudioBytes2, DWORD dwFlags) override;
	IFACEMETHOD(Play)(THIS_ DWORD dwReserved1, DWORD dwPriority, DWORD dwFlags) override;
	IFACEMETHOD(SetCurrentPosition)(THIS_ DWORD dwNewPosition) override;
	IFACEMETHOD(SetFormat)(THIS_ _In_ LPCWAVEFORMATEX pcfxFormat) override;
	IFACEMETHOD(SetVolume)(THIS_ LONG lVolume) override;
	IFACEMETHOD(SetPan)(THIS_ LONG lPan) override;
	IFACEMETHOD(SetFrequency)(THIS_ DWORD dwFrequency) override;
	IFACEMETHOD(Stop)(THIS) override;
	IFACEMETHOD(Unlock)(THIS_ _In_reads_bytes_(dwAudioBytes1) LPVOID pvAudioPtr1, DWORD dwAudioBytes1,
		_In_reads_bytes_opt_(dwAudioBytes2) LPVOID pvAudioPtr2, DWORD dwAudioBytes2) override;
	IFACEMETHOD(Restore)(THIS) override;

	// IDirectSoundBuffer8 methods
	IFACEMETHOD(SetFX)(THIS_ DWORD dwEffectsCount, _In_reads_opt_(dwEffectsCount) LPDSEFFECTDESC pDSFXDesc, _Out_writes_opt_(dwEffectsCount) LPDWORD pdwResultCodes) override;
	IFACEMETHOD(AcquireResources)(THIS_ DWORD dwFlags, DWORD dwEffectsCount, _Out_writes_(dwEffectsCount) LPDWORD pdwResultCodes) override;
	IFACEMETHOD(GetObjectInPath)(THIS_ _In_ REFGUID rguidObject, DWORD dwIndex, _In_ REFGUID rguidInterface, _Outptr_ LPVOID *ppObject) override;

	// Helper functions
	bool CheckThreadRunning();
	void StopThread();
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
