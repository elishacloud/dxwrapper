#pragma once

class m_IDirectInputEffect8 : public IDirectInputEffect, public AddressLookupTableDinput8Object
{
private:
	IDirectInputEffect *ProxyInterface;
	const IID WrapperID;

public:
	m_IDirectInputEffect8(IDirectInputEffect *aOriginal, REFIID riid) : ProxyInterface(aOriginal), WrapperID(riid)
	{
		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ")");

		if (IsEqualIID(riid, IID_IUnknown))
		{
			Logging::Log() << __FUNCTION__ << " Error: could not get riid when creating interface!";
		}

		ProxyAddressLookupTableDinput8.SaveAddress(this, ProxyInterface);
	}
	~m_IDirectInputEffect8()
	{
		LOG_LIMIT(3, __FUNCTION__ << " (" << this << ")" << " deleting interface!");

		ProxyAddressLookupTableDinput8.DeleteAddress(this);
	}

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	/*** IDirectInputEffect methods ***/
	STDMETHOD(Initialize)(THIS_ HINSTANCE, DWORD, REFGUID);
	STDMETHOD(GetEffectGuid)(THIS_ LPGUID);
	STDMETHOD(GetParameters)(THIS_ LPDIEFFECT, DWORD);
	STDMETHOD(SetParameters)(THIS_ LPCDIEFFECT, DWORD);
	STDMETHOD(Start)(THIS_ DWORD, DWORD);
	STDMETHOD(Stop)(THIS);
	STDMETHOD(GetEffectStatus)(THIS_ LPDWORD);
	STDMETHOD(Download)(THIS);
	STDMETHOD(Unload)(THIS);
	STDMETHOD(Escape)(THIS_ LPDIEFFESCAPE);

	// Helper functions
	IDirectInputEffect *GetProxyInterface() { return ProxyInterface; }
};
