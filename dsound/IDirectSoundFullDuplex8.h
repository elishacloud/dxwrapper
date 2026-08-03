#pragma once

class m_IDirectSoundFullDuplex8 final : public IDirectSoundFullDuplex8, AddressLookupTableDsoundObject<m_IDirectSoundFullDuplex8>
{
private:
	LPDIRECTSOUNDFULLDUPLEX ProxyInterface;

public:
	m_IDirectSoundFullDuplex8(LPDIRECTSOUNDFULLDUPLEX pSound8) : AddressLookupTableDsoundObject(pSound8), ProxyInterface(pSound8)
	{
		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ")");
	}
	~m_IDirectSoundFullDuplex8()
	{
		LOG_LIMIT(3, __FUNCTION__ << " (" << this << ")" << " deleting interface!");
	}

	// IUnknown methods
	IFACEMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) override;
	IFACEMETHOD_(ULONG, AddRef)(THIS) override;
	IFACEMETHOD_(ULONG, Release)(THIS) override;

	// IDirectSoundFullDuplex methods
	IFACEMETHOD(Initialize)(THIS_ _In_ LPCGUID pCaptureGuid, _In_ LPCGUID pRenderGuid, _In_ LPCDSCBUFFERDESC lpDscBufferDesc, _In_ LPCDSBUFFERDESC lpDsBufferDesc, HWND hWnd, DWORD dwLevel,
		_Outptr_ LPLPDIRECTSOUNDCAPTUREBUFFER8 lplpDirectSoundCaptureBuffer8, _Outptr_ LPLPDIRECTSOUNDBUFFER8 lplpDirectSoundBuffer8) override;

	// Helper functions
	LPDIRECTSOUNDFULLDUPLEX GetProxyInterface() { return ProxyInterface; }
};
