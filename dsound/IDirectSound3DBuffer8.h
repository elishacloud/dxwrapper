#pragma once

class m_IDirectSound3DBuffer8 final : public IDirectSound3DBuffer8, AddressLookupTableDsoundObject<m_IDirectSound3DBuffer8>
{
private:
	LPDIRECTSOUND3DBUFFER8 ProxyInterface;

public:
	m_IDirectSound3DBuffer8(LPDIRECTSOUND3DBUFFER8 pSound8) : AddressLookupTableDsoundObject(pSound8), ProxyInterface(pSound8)
	{
		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ")");
	}
	~m_IDirectSound3DBuffer8()
	{
		LOG_LIMIT(3, __FUNCTION__ << " (" << this << ")" << " deleting interface!");
	}

	// IUnknown methods
	IFACEMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) override;
	IFACEMETHOD_(ULONG, AddRef)(THIS) override;
	IFACEMETHOD_(ULONG, Release)(THIS) override;

	// IDirectSound3DBuffer methods
	IFACEMETHOD(GetAllParameters)(THIS_ _Out_ LPDS3DBUFFER pDs3dBuffer) override;
	IFACEMETHOD(GetConeAngles)(THIS_ _Out_ LPDWORD pdwInsideConeAngle, _Out_ LPDWORD pdwOutsideConeAngle) override;
	IFACEMETHOD(GetConeOrientation)(THIS_ _Out_ D3DVECTOR* pvOrientation) override;
	IFACEMETHOD(GetConeOutsideVolume)(THIS_ _Out_ LPLONG plConeOutsideVolume) override;
	IFACEMETHOD(GetMaxDistance)(THIS_ _Out_ D3DVALUE* pflMaxDistance) override;
	IFACEMETHOD(GetMinDistance)(THIS_ _Out_ D3DVALUE* pflMinDistance) override;
	IFACEMETHOD(GetMode)(THIS_ _Out_ LPDWORD pdwMode) override;
	IFACEMETHOD(GetPosition)(THIS_ _Out_ D3DVECTOR* pvPosition) override;
	IFACEMETHOD(GetVelocity)(THIS_ _Out_ D3DVECTOR* pvVelocity) override;
	IFACEMETHOD(SetAllParameters)(THIS_ _In_ LPCDS3DBUFFER pcDs3dBuffer, DWORD dwApply) override;
	IFACEMETHOD(SetConeAngles)(THIS_ DWORD dwInsideConeAngle, DWORD dwOutsideConeAngle, DWORD dwApply) override;
	IFACEMETHOD(SetConeOrientation)(THIS_ D3DVALUE x, D3DVALUE y, D3DVALUE z, DWORD dwApply) override;
	IFACEMETHOD(SetConeOutsideVolume)(THIS_ LONG lConeOutsideVolume, DWORD dwApply) override;
	IFACEMETHOD(SetMaxDistance)(THIS_ D3DVALUE flMaxDistance, DWORD dwApply) override;
	IFACEMETHOD(SetMinDistance)(THIS_ D3DVALUE flMinDistance, DWORD dwApply) override;
	IFACEMETHOD(SetMode)(THIS_ DWORD dwMode, DWORD dwApply) override;
	IFACEMETHOD(SetPosition)(THIS_ D3DVALUE x, D3DVALUE y, D3DVALUE z, DWORD dwApply) override;
	IFACEMETHOD(SetVelocity)(THIS_ D3DVALUE x, D3DVALUE y, D3DVALUE z, DWORD dwApply) override;

	// Helper functions
	LPDIRECTSOUND3DBUFFER8 GetProxyInterface() { return ProxyInterface; }
};
