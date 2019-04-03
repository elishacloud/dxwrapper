#pragma once

class m_IDirectSound3DBuffer8 : public IDirectSound3DBuffer8, public AddressLookupTableDsoundObject
{
private:
	LPDIRECTSOUND3DBUFFER8 ProxyInterface;

public:
	m_IDirectSound3DBuffer8(LPDIRECTSOUND3DBUFFER8 pSound8) : ProxyInterface(pSound8)
	{
		ProxyAddressLookupTableDsound.SaveAddress(this, ProxyInterface);
	}
	~m_IDirectSound3DBuffer8()
	{
		ProxyAddressLookupTableDsound.DeleteAddress(this);
	}

	LPDIRECTSOUND3DBUFFER8 GetProxyInterface() { return ProxyInterface; }

	// IUnknown methods
	STDMETHOD(QueryInterface)(THIS_ _In_ REFIID, _Outptr_ LPVOID*);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	// IDirectSound3DBuffer methods
	STDMETHOD(GetAllParameters)(THIS_ _Out_ LPDS3DBUFFER pDs3dBuffer);
	STDMETHOD(GetConeAngles)(THIS_ _Out_ LPDWORD pdwInsideConeAngle, _Out_ LPDWORD pdwOutsideConeAngle);
	STDMETHOD(GetConeOrientation)(THIS_ _Out_ D3DVECTOR* pvOrientation);
	STDMETHOD(GetConeOutsideVolume)(THIS_ _Out_ LPLONG plConeOutsideVolume);
	STDMETHOD(GetMaxDistance)(THIS_ _Out_ D3DVALUE* pflMaxDistance);
	STDMETHOD(GetMinDistance)(THIS_ _Out_ D3DVALUE* pflMinDistance);
	STDMETHOD(GetMode)(THIS_ _Out_ LPDWORD pdwMode);
	STDMETHOD(GetPosition)(THIS_ _Out_ D3DVECTOR* pvPosition);
	STDMETHOD(GetVelocity)(THIS_ _Out_ D3DVECTOR* pvVelocity);
	STDMETHOD(SetAllParameters)(THIS_ _In_ LPCDS3DBUFFER pcDs3dBuffer, DWORD dwApply);
	STDMETHOD(SetConeAngles)(THIS_ DWORD dwInsideConeAngle, DWORD dwOutsideConeAngle, DWORD dwApply);
	STDMETHOD(SetConeOrientation)(THIS_ D3DVALUE x, D3DVALUE y, D3DVALUE z, DWORD dwApply);
	STDMETHOD(SetConeOutsideVolume)(THIS_ LONG lConeOutsideVolume, DWORD dwApply);
	STDMETHOD(SetMaxDistance)(THIS_ D3DVALUE flMaxDistance, DWORD dwApply);
	STDMETHOD(SetMinDistance)(THIS_ D3DVALUE flMinDistance, DWORD dwApply);
	STDMETHOD(SetMode)(THIS_ DWORD dwMode, DWORD dwApply);
	STDMETHOD(SetPosition)(THIS_ D3DVALUE x, D3DVALUE y, D3DVALUE z, DWORD dwApply);
	STDMETHOD(SetVelocity)(THIS_ D3DVALUE x, D3DVALUE y, D3DVALUE z, DWORD dwApply);
};
