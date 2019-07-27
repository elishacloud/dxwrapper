#pragma once

class m_IDirectDrawGammaControl : public IDirectDrawGammaControl, public AddressLookupTableDdrawObject
{
private:
	IDirectDrawGammaControl *ProxyInterface = nullptr;
	REFIID WrapperID = IID_IDirectDrawGammaControl;
	ULONG RefCount = 1;
	DDGAMMARAMP RampData = { NULL };

public:
	m_IDirectDrawGammaControl(IDirectDrawGammaControl *aOriginal) : ProxyInterface(aOriginal)
	{
		LOG_LIMIT(3, "Creating device " << __FUNCTION__ << "(" << this << ")");

		ProxyAddressLookupTable.SaveAddress(this, ProxyInterface);
	}
	m_IDirectDrawGammaControl()
	{
		LOG_LIMIT(3, "Creating device " << __FUNCTION__ << "(" << this << ")");
	}
	~m_IDirectDrawGammaControl()
	{
		LOG_LIMIT(3, __FUNCTION__ << "(" << this << ")" << " deleting device!");

		ProxyAddressLookupTable.DeleteAddress(this);
	}

	DWORD GetDirectXVersion() { return 1; }
	REFIID GetWrapperType() { return WrapperID; }
	IDirectDrawGammaControl *GetProxyInterface() { return ProxyInterface; }
	m_IDirectDrawGammaControl *GetWrapperInterface() { return this; }

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * ppvObj);
	STDMETHOD_(ULONG, AddRef) (THIS);
	STDMETHOD_(ULONG, Release) (THIS);

	/*** IDirectDrawGammaControl methods ***/
	STDMETHOD(GetGammaRamp)(THIS_ DWORD, LPDDGAMMARAMP);
	STDMETHOD(SetGammaRamp)(THIS_ DWORD, LPDDGAMMARAMP);
};
