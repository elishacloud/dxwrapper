#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "Wrappers\wrapper.h"
#include "Settings\Settings.h"
#include "Logging\Logging.h"

DEFINE_GUID(IID_GetRealInterface, 0x00000000, 0x1c77, 0x4d40, 0xb0, 0xcf, 0x98, 0xfe, 0xfd, 0xff, 0xff, 0xff);
DEFINE_GUID(IID_GetInterfaceX, 0x11111111, 0x1c77, 0x4d40, 0xb0, 0xcf, 0x98, 0xfe, 0xfd, 0xff, 0xff, 0xff);
DEFINE_GUID(IID_GetMipMapLevel, 0x22222222, 0x1c77, 0x4d40, 0xb0, 0xcf, 0x98, 0xfe, 0xfd, 0xff, 0xff, 0xff);

DEFINE_GUID(IID_IDirect3DDxva2Container9, 0x126D0349, 0x4787, 0x4AA6, 0x8E, 0x1B, 0x40, 0xC1, 0x77, 0xC6, 0x0A, 0x01);

typedef void(WINAPI *IQueryInterfaceProc)(REFIID, LPVOID *);
typedef HRESULT(WINAPI *CoCreateInstanceHandleProc)(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID *ppv);

EXPORT_OUT_WRAPPED_PROC(CoCreateInstance, unused);

HRESULT WINAPI CoCreateInstanceHandle(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID *ppv);

class m_IClassFactory : public IClassFactory
{
private:
	IQueryInterfaceProc IQueryInterface;
	IClassFactory *ProxyInterface;
	const IID WrapperID = IID_IClassFactory;
	CLSID ClassID = IID_IUnknown;
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
