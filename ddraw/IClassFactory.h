#pragma once

class m_IClassFactory : public IClassFactory
{
private:
	REFIID WrapperID = IID_IClassFactory;
	ULONG RefCount = 1;

public:
	m_IClassFactory() {}
	~m_IClassFactory() {}

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * ppvObj);
	STDMETHOD_(ULONG, AddRef) (THIS);
	STDMETHOD_(ULONG, Release) (THIS);

	/*** IClassFactory methods ***/
	STDMETHOD(CreateInstance)(IUnknown *pUnkOuter, REFIID riid, void **ppvObject);
	STDMETHOD(LockServer)(BOOL fLock);
};
