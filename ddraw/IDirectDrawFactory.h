#pragma once

class m_IDirectDrawFactory : public IDirectDrawFactory
{
private:
	IDirectDrawFactory *ProxyInterface = nullptr;
	ULONG RefCount = 1;
	REFIID WrapperID = IID_IDirectDrawFactory;

public:
	m_IDirectDrawFactory(IDirectDrawFactory *aOriginal) : ProxyInterface(aOriginal)
	{
		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ")");
	}
	~m_IDirectDrawFactory()
	{
		LOG_LIMIT(3, __FUNCTION__ << " (" << this << ")" << " deleting interface!");
	}

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * ppvObj);
	STDMETHOD_(ULONG, AddRef) (THIS);
	STDMETHOD_(ULONG, Release) (THIS);

	/*** IDirectDrawFactory methods ***/
	STDMETHOD(CreateDirectDraw) (THIS_ GUID * pGUID, HWND hWnd, DWORD dwCoopLevelFlags, DWORD dwReserved, IUnknown *pUnkOuter, IDirectDraw **ppDirectDraw);
	STDMETHOD(DirectDrawEnumerateA) (THIS_ LPDDENUMCALLBACKA lpCallback, LPVOID lpContext);
	STDMETHOD(DirectDrawEnumerateW) (THIS_ LPDDENUMCALLBACKW lpCallback, LPVOID lpContext);
};
