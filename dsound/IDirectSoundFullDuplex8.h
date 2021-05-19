#pragma once

class m_IDirectSoundFullDuplex8 : public IDirectSoundFullDuplex8, public AddressLookupTableDsoundObject
{
private:
	LPDIRECTSOUNDFULLDUPLEX ProxyInterface;

public:
	m_IDirectSoundFullDuplex8(LPDIRECTSOUNDFULLDUPLEX pSound8) : ProxyInterface(pSound8)
	{
		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ")");

		ProxyAddressLookupTableDsound.SaveAddress(this, ProxyInterface);
	}
	~m_IDirectSoundFullDuplex8()
	{
		LOG_LIMIT(3, __FUNCTION__ << " (" << this << ")" << " deleting interface!");

		ProxyAddressLookupTableDsound.DeleteAddress(this);
	}

	// IUnknown methods
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID* ppvObj);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	// IDirectSoundFullDuplex methods
	STDMETHOD(Initialize)(THIS_ _In_ LPCGUID pCaptureGuid, _In_ LPCGUID pRenderGuid, _In_ LPCDSCBUFFERDESC lpDscBufferDesc, _In_ LPCDSBUFFERDESC lpDsBufferDesc, HWND hWnd, DWORD dwLevel,
		_Outptr_ LPLPDIRECTSOUNDCAPTUREBUFFER8 lplpDirectSoundCaptureBuffer8, _Outptr_ LPLPDIRECTSOUNDBUFFER8 lplpDirectSoundBuffer8);

	// Helper functions
	LPDIRECTSOUNDFULLDUPLEX GetProxyInterface() { return ProxyInterface; }
};
