#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Unknwnbase.h>
#include "Settings\Settings.h"
#include "Logging\Logging.h"

DEFINE_GUID(IID_GetRealInterface, 0x254e1ffd, 0x10f9, 0x10c0, 0xc1, 0xb8, 0x21, 0x7d, 0x59, 0xd1, 0xe5, 0xb2);
DEFINE_GUID(IID_GetInterfaceX, 0x254e1ffd, 0x10f9, 0x10c0, 0xc1, 0xb8, 0x21, 0x7d, 0x59, 0xd1, 0xe5, 0xb4);

typedef void(WINAPI *IQueryInterfaceProc)(REFIID, LPVOID *);
typedef HRESULT(WINAPI *CoCreateInstanceHandleProc)(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID *ppv);

extern CoCreateInstanceHandleProc p_CoCreateInstance;

HRESULT WINAPI CoCreateInstanceHandle(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID *ppv);

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
