#pragma once

class m_IDirectDrawClipper : public IDirectDrawClipper, public AddressLookupTableDdrawObject
{
private:
	IDirectDrawClipper *ProxyInterface;

public:
	m_IDirectDrawClipper(IDirectDrawClipper *aOriginal, REFIID) : ProxyInterface(aOriginal)
	{
		ProxyAddressLookupTable.SaveAddress(this, ProxyInterface);
	}
	~m_IDirectDrawClipper() {}

	IDirectDrawClipper *GetProxyInterface() { return ProxyInterface; }

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
