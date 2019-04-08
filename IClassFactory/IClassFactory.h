#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Unknwnbase.h>
#include "Settings\Settings.h"
#include "Logging\Logging.h"

typedef void(WINAPI *IQueryInterfaceProc)(REFIID, LPVOID *);

class m_IClassFactory : public IClassFactory
{
private:
	IQueryInterfaceProc IQueryInterface;
	IClassFactory *ProxyInterface;
	REFIID WrapperID = IID_IClassFactory;
	IID ClassID = IID_IUnknown;
	ULONG RefCount = 1;

public:
	m_IClassFactory(IClassFactory *aOriginal, IQueryInterfaceProc p_QueryInterface) : ProxyInterface(aOriginal), IQueryInterface(p_QueryInterface)
	{
		Logging::LogDebug() << "Create " << __FUNCTION__;
		if (!ProxyInterface || !IQueryInterface)
		{
			ProxyInterface = nullptr;
		}
	}
	~m_IClassFactory() {}

	void SetCLSID(REFCLSID rclsid) { ClassID = rclsid; }

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * ppvObj);
	STDMETHOD_(ULONG, AddRef) (THIS);
	STDMETHOD_(ULONG, Release) (THIS);

	/*** IClassFactory methods ***/
	STDMETHOD(CreateInstance)(IUnknown *pUnkOuter, REFIID riid, void **ppvObject);
	STDMETHOD(LockServer)(BOOL fLock);
};
