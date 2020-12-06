#pragma once

class m_IDirectSound3DListener8 : public IDirectSound3DListener8, public AddressLookupTableDsoundObject
{
private:
	LPDIRECTSOUND3DLISTENER8 ProxyInterface;

public:
	m_IDirectSound3DListener8(LPDIRECTSOUND3DLISTENER8 pSound8) : ProxyInterface(pSound8)
	{
		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ")");

		ProxyAddressLookupTableDsound.SaveAddress(this, ProxyInterface);
	}
	~m_IDirectSound3DListener8()
	{
		LOG_LIMIT(3, __FUNCTION__ << " (" << this << ")" << " deleting interface!");

		ProxyAddressLookupTableDsound.DeleteAddress(this);
	}

	// IUnknown methods
	STDMETHOD(QueryInterface)(THIS_ _In_ REFIID, _Outptr_ LPVOID*);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	// IDirectSound3DListener methods
	STDMETHOD(GetAllParameters)(THIS_ _Out_ LPDS3DLISTENER pListener);
	STDMETHOD(GetDistanceFactor)(THIS_ _Out_ D3DVALUE* pflDistanceFactor);
	STDMETHOD(GetDopplerFactor)(THIS_ _Out_ D3DVALUE* pflDopplerFactor);
	STDMETHOD(GetOrientation)(THIS_ _Out_ D3DVECTOR* pvOrientFront, _Out_ D3DVECTOR* pvOrientTop);
	STDMETHOD(GetPosition)(THIS_ _Out_ D3DVECTOR* pvPosition);
	STDMETHOD(GetRolloffFactor)(THIS_ _Out_ D3DVALUE* pflRolloffFactor);
	STDMETHOD(GetVelocity)(THIS_ _Out_ D3DVECTOR* pvVelocity);
	STDMETHOD(SetAllParameters)(THIS_ _In_ LPCDS3DLISTENER pcListener, DWORD dwApply);
	STDMETHOD(SetDistanceFactor)(THIS_ D3DVALUE flDistanceFactor, DWORD dwApply);
	STDMETHOD(SetDopplerFactor)(THIS_ D3DVALUE flDopplerFactor, DWORD dwApply);
	STDMETHOD(SetOrientation)(THIS_ D3DVALUE xFront, D3DVALUE yFront, D3DVALUE zFront,
		D3DVALUE xTop, D3DVALUE yTop, D3DVALUE zTop, DWORD dwApply);
	STDMETHOD(SetPosition)(THIS_ D3DVALUE x, D3DVALUE y, D3DVALUE z, DWORD dwApply);
	STDMETHOD(SetRolloffFactor)(THIS_ D3DVALUE flRolloffFactor, DWORD dwApply);
	STDMETHOD(SetVelocity)(THIS_ D3DVALUE x, D3DVALUE y, D3DVALUE z, DWORD dwApply);
	STDMETHOD(CommitDeferredSettings)(THIS);

	// Helper functions
	LPDIRECTSOUND3DLISTENER8 GetProxyInterface() { return ProxyInterface; }
};
