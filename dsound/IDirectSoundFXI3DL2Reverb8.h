#pragma once

class m_IDirectSoundFXI3DL2Reverb8 : public IDirectSoundFXI3DL2Reverb8, public AddressLookupTableDsoundObject
{
private:
	LPDIRECTSOUNDFXI3DL2REVERB8 ProxyInterface;

public:
	m_IDirectSoundFXI3DL2Reverb8(LPDIRECTSOUNDFXI3DL2REVERB8 pSound8) : ProxyInterface(pSound8)
	{
		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ")");

		ProxyAddressLookupTableDsound.SaveAddress(this, ProxyInterface);
	}
	~m_IDirectSoundFXI3DL2Reverb8()
	{
		LOG_LIMIT(3, __FUNCTION__ << " (" << this << ")" << " deleting interface!");

		ProxyAddressLookupTableDsound.DeleteAddress(this);
	}

	// IUnknown methods
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID* ppvObj);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	// IDirectSoundFXI3DL2Reverb methods
	STDMETHOD(SetAllParameters)(THIS_ _In_ LPCDSFXI3DL2Reverb pcDsFxI3DL2Reverb);
	STDMETHOD(GetAllParameters)(THIS_ _Out_ LPDSFXI3DL2Reverb pDsFxI3DL2Reverb);
	STDMETHOD(SetPreset)(THIS_ DWORD dwPreset);
	STDMETHOD(GetPreset)(THIS_ _Out_ LPDWORD pdwPreset);
	STDMETHOD(SetQuality)(THIS_ LONG lQuality);
	STDMETHOD(GetQuality)(THIS_ _Out_ LONG *plQuality);

	// Helper functions
	LPDIRECTSOUNDFXI3DL2REVERB8 GetProxyInterface() { return ProxyInterface; }
};
