#pragma once

class m_IDirectInputDevice8A : public IDirectInputDevice8A, public AddressLookupTableDinput8Object
{
private:
	IDirectInputDevice8A *ProxyInterface;

public:
	m_IDirectInputDevice8A(IDirectInputDevice8A *aOriginal) : ProxyInterface(aOriginal)
	{
		LOG_LIMIT(3, "Creating device " << __FUNCTION__ << "(" << this << ")");

		ProxyAddressLookupTableDinput8.SaveAddress(this, ProxyInterface);
	}
	~m_IDirectInputDevice8A()
	{
		LOG_LIMIT(3, __FUNCTION__ << "(" << this << ")" << " deleting device!");

		ProxyAddressLookupTableDinput8.DeleteAddress(this);
	}

	IDirectInputDevice8A *GetProxyInterface() { return ProxyInterface; }

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	/*** IDirectInputDevice8A methods ***/
	STDMETHOD(GetCapabilities)(THIS_ LPDIDEVCAPS);
	STDMETHOD(EnumObjects)(THIS_ LPDIENUMDEVICEOBJECTSCALLBACKA, LPVOID, DWORD);
	STDMETHOD(GetProperty)(THIS_ REFGUID, LPDIPROPHEADER);
	STDMETHOD(SetProperty)(THIS_ REFGUID, LPCDIPROPHEADER);
	STDMETHOD(Acquire)(THIS);
	STDMETHOD(Unacquire)(THIS);
	STDMETHOD(GetDeviceState)(THIS_ DWORD, LPVOID);
	STDMETHOD(GetDeviceData)(THIS_ DWORD, LPDIDEVICEOBJECTDATA, LPDWORD, DWORD);
	STDMETHOD(SetDataFormat)(THIS_ LPCDIDATAFORMAT);
	STDMETHOD(SetEventNotification)(THIS_ HANDLE);
	STDMETHOD(SetCooperativeLevel)(THIS_ HWND, DWORD);
	STDMETHOD(GetObjectInfo)(THIS_ LPDIDEVICEOBJECTINSTANCEA, DWORD, DWORD);
	STDMETHOD(GetDeviceInfo)(THIS_ LPDIDEVICEINSTANCEA);
	STDMETHOD(RunControlPanel)(THIS_ HWND, DWORD);
	STDMETHOD(Initialize)(THIS_ HINSTANCE, DWORD, REFGUID);
	STDMETHOD(CreateEffect)(THIS_ REFGUID, LPCDIEFFECT, LPDIRECTINPUTEFFECT *, LPUNKNOWN);
	STDMETHOD(EnumEffects)(THIS_ LPDIENUMEFFECTSCALLBACKA, LPVOID, DWORD);
	STDMETHOD(GetEffectInfo)(THIS_ LPDIEFFECTINFOA, REFGUID);
	STDMETHOD(GetForceFeedbackState)(THIS_ LPDWORD);
	STDMETHOD(SendForceFeedbackCommand)(THIS_ DWORD);
	STDMETHOD(EnumCreatedEffectObjects)(THIS_ LPDIENUMCREATEDEFFECTOBJECTSCALLBACK, LPVOID, DWORD);
	STDMETHOD(Escape)(THIS_ LPDIEFFESCAPE);
	STDMETHOD(Poll)(THIS);
	STDMETHOD(SendDeviceData)(THIS_ DWORD, LPCDIDEVICEOBJECTDATA, LPDWORD, DWORD);
	STDMETHOD(EnumEffectsInFile)(THIS_ LPCSTR, LPDIENUMEFFECTSINFILECALLBACK, LPVOID, DWORD);
	STDMETHOD(WriteEffectToFile)(THIS_ LPCSTR, DWORD, LPDIFILEEFFECT, DWORD);
	STDMETHOD(BuildActionMap)(THIS_ LPDIACTIONFORMATA, LPCSTR, DWORD);
	STDMETHOD(SetActionMap)(THIS_ LPDIACTIONFORMATA, LPCSTR, DWORD);
	STDMETHOD(GetImageInfo)(THIS_ LPDIDEVICEIMAGEINFOHEADERA);
};
