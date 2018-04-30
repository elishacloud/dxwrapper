#pragma once

class m_IDirectDrawFactory : public IDirectDrawFactory, public AddressLookupTableDdrawObject
{
private:
	IDirectDrawFactory *ProxyInterface;
	REFIID WrapperID = IID_IDirectDrawFactory;

public:
	m_IDirectDrawFactory(IDirectDrawFactory *aOriginal) : ProxyInterface(aOriginal)
	{
		ProxyAddressLookupTable.SaveAddress(this, ProxyInterface);
	}
	~m_IDirectDrawFactory()
	{
		ProxyAddressLookupTable.DeleteAddress(this);
	}

	DWORD GetDirectXVersion() { return 1; }
	REFIID GetWrapperType() { return WrapperID; }
	IDirectDrawFactory *GetProxyInterface() { return ProxyInterface; }
	m_IDirectDrawFactory *GetWrapperInterface() { return this; }

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * ppvObj);
	STDMETHOD_(ULONG, AddRef) (THIS) ;
	STDMETHOD_(ULONG, Release) (THIS);
	/*** IDirectDrawFactory methods ***/
	STDMETHOD(CreateDirectDraw) (THIS_ GUID * pGUID, HWND hWnd, DWORD dwCoopLevelFlags, DWORD dwReserved, IUnknown *pUnkOuter, IDirectDraw **ppDirectDraw);
	STDMETHOD(DirectDrawEnumerateA) (THIS_ LPDDENUMCALLBACKA lpCallback, LPVOID lpContext);
	STDMETHOD(DirectDrawEnumerateW) (THIS_ LPDDENUMCALLBACKW lpCallback, LPVOID lpContext);
};
