#pragma once

class m_IDirectSound3DListener8 final : public IDirectSound3DListener8, AddressLookupTableDsoundObject<m_IDirectSound3DListener8>
{
private:
	LPDIRECTSOUND3DLISTENER8 ProxyInterface;

public:
	m_IDirectSound3DListener8(LPDIRECTSOUND3DLISTENER8 pSound8) : AddressLookupTableDsoundObject(pSound8), ProxyInterface(pSound8)
	{
		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ")");
	}
	~m_IDirectSound3DListener8()
	{
		LOG_LIMIT(3, __FUNCTION__ << " (" << this << ")" << " deleting interface!");
	}

	// IUnknown methods
	IFACEMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) override;
	IFACEMETHOD_(ULONG, AddRef)(THIS) override;
	IFACEMETHOD_(ULONG, Release)(THIS) override;

	// IDirectSound3DListener methods
	IFACEMETHOD(GetAllParameters)(THIS_ _Out_ LPDS3DLISTENER pListener) override;
	IFACEMETHOD(GetDistanceFactor)(THIS_ _Out_ D3DVALUE* pflDistanceFactor) override;
	IFACEMETHOD(GetDopplerFactor)(THIS_ _Out_ D3DVALUE* pflDopplerFactor) override;
	IFACEMETHOD(GetOrientation)(THIS_ _Out_ D3DVECTOR* pvOrientFront, _Out_ D3DVECTOR* pvOrientTop) override;
	IFACEMETHOD(GetPosition)(THIS_ _Out_ D3DVECTOR* pvPosition) override;
	IFACEMETHOD(GetRolloffFactor)(THIS_ _Out_ D3DVALUE* pflRolloffFactor) override;
	IFACEMETHOD(GetVelocity)(THIS_ _Out_ D3DVECTOR* pvVelocity) override;
	IFACEMETHOD(SetAllParameters)(THIS_ _In_ LPCDS3DLISTENER pcListener, DWORD dwApply) override;
	IFACEMETHOD(SetDistanceFactor)(THIS_ D3DVALUE flDistanceFactor, DWORD dwApply) override;
	IFACEMETHOD(SetDopplerFactor)(THIS_ D3DVALUE flDopplerFactor, DWORD dwApply) override;
	IFACEMETHOD(SetOrientation)(THIS_ D3DVALUE xFront, D3DVALUE yFront, D3DVALUE zFront,
		D3DVALUE xTop, D3DVALUE yTop, D3DVALUE zTop, DWORD dwApply) override;
	IFACEMETHOD(SetPosition)(THIS_ D3DVALUE x, D3DVALUE y, D3DVALUE z, DWORD dwApply) override;
	IFACEMETHOD(SetRolloffFactor)(THIS_ D3DVALUE flRolloffFactor, DWORD dwApply) override;
	IFACEMETHOD(SetVelocity)(THIS_ D3DVALUE x, D3DVALUE y, D3DVALUE z, DWORD dwApply) override;
	IFACEMETHOD(CommitDeferredSettings)(THIS) override;

	// Helper functions
	LPDIRECTSOUND3DLISTENER8 GetProxyInterface() { return ProxyInterface; }
};
