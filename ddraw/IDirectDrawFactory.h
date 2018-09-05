#pragma once

class m_IDirectDrawFactory : public IDirectDrawFactory, public AddressLookupTableDdrawObject
{
private:
	REFIID WrapperID = IID_IDirectDrawFactory;
	ULONG RefCount = 1;

public:
	m_IDirectDrawFactory() { }
	~m_IDirectDrawFactory() { }

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * ppvObj);
	STDMETHOD_(ULONG, AddRef) (THIS) ;
	STDMETHOD_(ULONG, Release) (THIS);
	/*** IDirectDrawFactory methods ***/
	STDMETHOD(CreateDirectDraw) (THIS_ GUID * pGUID, HWND hWnd, DWORD dwCoopLevelFlags, DWORD dwReserved, IUnknown *pUnkOuter, IDirectDraw **ppDirectDraw);
	STDMETHOD(DirectDrawEnumerateA) (THIS_ LPDDENUMCALLBACKA lpCallback, LPVOID lpContext);
	STDMETHOD(DirectDrawEnumerateW) (THIS_ LPDDENUMCALLBACKW lpCallback, LPVOID lpContext);
};
