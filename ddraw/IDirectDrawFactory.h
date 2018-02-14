#pragma once

class m_IDirectDrawFactory : public IDirectDrawFactory, public AddressLookupTableObject
{
private:
	IDirectDrawFactory *ProxyInterface;

public:
	m_IDirectDrawFactory(IDirectDrawFactory *aOriginal, void *) : ProxyInterface(aOriginal)
	{
		ProxyAddressLookupTable.SaveAddress(this, ProxyInterface);
	}
	~m_IDirectDrawFactory() {}

	IDirectDrawFactory *GetProxyInterface() { return ProxyInterface; }

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * ppvObj);
	STDMETHOD_(ULONG, AddRef) (THIS) ;
	STDMETHOD_(ULONG, Release) (THIS);
	/*** IDirectDrawFactory methods ***/
	STDMETHOD(CreateDirectDraw) (THIS_ GUID * pGUID, HWND hWnd, DWORD dwCoopLevelFlags, DWORD dwReserved, IUnknown *pUnkOuter, IDirectDraw **ppDirectDraw);
	STDMETHOD(DirectDrawEnumerate) (THIS_ LPDDENUMCALLBACK lpCallback, LPVOID lpContext);
};
