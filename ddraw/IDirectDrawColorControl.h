#pragma once

class m_IDirectDrawColorControl : public IDirectDrawColorControl, public AddressLookupTableDdrawObject
{
private:
	IDirectDrawColorControl *ProxyInterface;
	REFIID WrapperID = IID_IDirectDrawColorControl;

public:
	m_IDirectDrawColorControl(IDirectDrawColorControl *aOriginal) : ProxyInterface(aOriginal)
	{
		ProxyAddressLookupTable.SaveAddress(this, ProxyInterface);
	}
	~m_IDirectDrawColorControl()
	{
		ProxyAddressLookupTable.DeleteAddress(this);
	}

	DWORD GetDirectXVersion() { return 1; }
	REFIID GetWrapperType() { return WrapperID; }
	IDirectDrawColorControl *GetProxyInterface() { return ProxyInterface; }
	m_IDirectDrawColorControl *GetWrapperInterface() { return this; }

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * ppvObj);
	STDMETHOD_(ULONG, AddRef) (THIS) ;
	STDMETHOD_(ULONG, Release) (THIS);
	/*** IDirectDrawColorControl methods ***/
	STDMETHOD(GetColorControls)(THIS_ LPDDCOLORCONTROL);
	STDMETHOD(SetColorControls)(THIS_ LPDDCOLORCONTROL);
};
