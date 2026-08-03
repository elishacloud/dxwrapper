#pragma once

class m_IDirectSoundFXI3DL2Reverb8 final : public IDirectSoundFXI3DL2Reverb8, AddressLookupTableDsoundObject<m_IDirectSoundFXI3DL2Reverb8>
{
private:
	LPDIRECTSOUNDFXI3DL2REVERB8 ProxyInterface;

public:
	m_IDirectSoundFXI3DL2Reverb8(LPDIRECTSOUNDFXI3DL2REVERB8 pSound8) : AddressLookupTableDsoundObject(pSound8), ProxyInterface(pSound8)
	{
		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ")");
	}
	~m_IDirectSoundFXI3DL2Reverb8()
	{
		LOG_LIMIT(3, __FUNCTION__ << " (" << this << ")" << " deleting interface!");
	}

	// IUnknown methods
	IFACEMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) override;
	IFACEMETHOD_(ULONG, AddRef)(THIS) override;
	IFACEMETHOD_(ULONG, Release)(THIS) override;

	// IDirectSoundFXI3DL2Reverb methods
	IFACEMETHOD(SetAllParameters)(THIS_ _In_ LPCDSFXI3DL2Reverb pcDsFxI3DL2Reverb) override;
	IFACEMETHOD(GetAllParameters)(THIS_ _Out_ LPDSFXI3DL2Reverb pDsFxI3DL2Reverb) override;
	IFACEMETHOD(SetPreset)(THIS_ DWORD dwPreset) override;
	IFACEMETHOD(GetPreset)(THIS_ _Out_ LPDWORD pdwPreset) override;
	IFACEMETHOD(SetQuality)(THIS_ LONG lQuality) override;
	IFACEMETHOD(GetQuality)(THIS_ _Out_ LONG *plQuality) override;

	// Helper functions
	LPDIRECTSOUNDFXI3DL2REVERB8 GetProxyInterface() { return ProxyInterface; }
};
