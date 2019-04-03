#pragma once

class m_IDirectSoundFullDuplex8 : public IDirectSoundFullDuplex8, public AddressLookupTableDsoundObject
{
private:
	LPDIRECTSOUNDFULLDUPLEX ProxyInterface;

public:
	m_IDirectSoundFullDuplex8(LPDIRECTSOUNDFULLDUPLEX pSound8) : ProxyInterface(pSound8)
	{
		ProxyAddressLookupTableDsound.SaveAddress(this, ProxyInterface);
	}
	~m_IDirectSoundFullDuplex8()
	{
		ProxyAddressLookupTableDsound.DeleteAddress(this);
	}

	LPDIRECTSOUNDFULLDUPLEX GetProxyInterface() { return ProxyInterface; }

	// IUnknown methods
	STDMETHOD(QueryInterface)(THIS_ _In_ REFIID, _Outptr_ LPVOID*);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	// IDirectSoundFullDuplex methods
	STDMETHOD(Initialize)(THIS_ _In_ LPCGUID pCaptureGuid, _In_ LPCGUID pRenderGuid, _In_ LPCDSCBUFFERDESC lpDscBufferDesc, _In_ LPCDSBUFFERDESC lpDsBufferDesc, HWND hWnd, DWORD dwLevel,
		_Outptr_ LPLPDIRECTSOUNDCAPTUREBUFFER8 lplpDirectSoundCaptureBuffer8, _Outptr_ LPLPDIRECTSOUNDBUFFER8 lplpDirectSoundBuffer8);
};
