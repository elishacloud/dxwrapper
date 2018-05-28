#pragma once

class m_IDirectDrawClipper : public IDirectDrawClipper, public AddressLookupTableDdrawObject
{
private:
	IDirectDrawClipper *ProxyInterface;
	REFIID WrapperID = IID_IDirectDrawClipper;
	ULONG RefCount = 1;
	HWND cliphWnd = nullptr;

public:
	m_IDirectDrawClipper(IDirectDrawClipper *aOriginal) : ProxyInterface(aOriginal)
	{
		if (ProxyInterface)
		{
			ProxyAddressLookupTable.SaveAddress(this, ProxyInterface);
		}
	}
	~m_IDirectDrawClipper()
	{
		ProxyAddressLookupTable.DeleteAddress(this);
	}

	DWORD GetDirectXVersion() { return 1; }
	REFIID GetWrapperType() { return WrapperID; }
	IDirectDrawClipper *GetProxyInterface() { return ProxyInterface; }
	m_IDirectDrawClipper *GetWrapperInterface() { return this; }

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * ppvObj);
	STDMETHOD_(ULONG, AddRef) (THIS) ;
	STDMETHOD_(ULONG, Release) (THIS);
	/*** IDirectDrawClipper methods ***/
	STDMETHOD(GetClipList)(THIS_ LPRECT, LPRGNDATA, LPDWORD);
	STDMETHOD(GetHWnd)(THIS_ HWND FAR *);
	STDMETHOD(Initialize)(THIS_ LPDIRECTDRAW, DWORD);
	STDMETHOD(IsClipListChanged)(THIS_ BOOL FAR *);
	STDMETHOD(SetClipList)(THIS_ LPRGNDATA, DWORD);
	STDMETHOD(SetHWnd)(THIS_ DWORD, HWND);
};
