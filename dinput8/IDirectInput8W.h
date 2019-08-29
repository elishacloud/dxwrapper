#pragma once

class m_IDirectInput8W : public IDirectInput8W, public AddressLookupTableDinput8Object
{
private:
	IDirectInput8W *ProxyInterface;

public:
	m_IDirectInput8W(IDirectInput8W *aOriginal) : ProxyInterface(aOriginal)
	{
		LOG_LIMIT(3, "Creating device " << __FUNCTION__ << "(" << this << ")");

		ProxyAddressLookupTableDinput8.SaveAddress(this, ProxyInterface);
	}
	~m_IDirectInput8W()
	{
		LOG_LIMIT(3, __FUNCTION__ << "(" << this << ")" << " deleting device!");

		ProxyAddressLookupTableDinput8.DeleteAddress(this);
	}

	IDirectInput8W *GetProxyInterface() { return ProxyInterface; }

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	/*** IDirectInput8W methods ***/
	STDMETHOD(CreateDevice)(THIS_ REFGUID, LPDIRECTINPUTDEVICE8W *, LPUNKNOWN);
	STDMETHOD(EnumDevices)(THIS_ DWORD, LPDIENUMDEVICESCALLBACKW, LPVOID, DWORD);
	STDMETHOD(GetDeviceStatus)(THIS_ REFGUID);
	STDMETHOD(RunControlPanel)(THIS_ HWND, DWORD);
	STDMETHOD(Initialize)(THIS_ HINSTANCE, DWORD);
	STDMETHOD(FindDevice)(THIS_ REFGUID, LPCWSTR, LPGUID);
	STDMETHOD(EnumDevicesBySemantics)(THIS_ LPCWSTR, LPDIACTIONFORMATW, LPDIENUMDEVICESBYSEMANTICSCBW, LPVOID, DWORD);
	STDMETHOD(ConfigureDevices)(THIS_ LPDICONFIGUREDEVICESCALLBACK, LPDICONFIGUREDEVICESPARAMSW, DWORD, LPVOID);
};
