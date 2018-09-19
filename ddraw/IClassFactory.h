#pragma once

class m_IClassFactory : public IClassFactory, public AddressLookupTableDdrawObject
{
private:
	IClassFactory *ProxyInterface;
	REFIID WrapperID = IID_IClassFactory;
	IID ClassID = IID_IUnknown;
	ULONG RefCount = 1;

public:
	m_IClassFactory(IClassFactory *aOriginal) : ProxyInterface(aOriginal)
	{
		if (ProxyInterface)
		{
			ProxyAddressLookupTable.SaveAddress(this, ProxyInterface);
		}

		Logging::LogDebug() << "Create " << __FUNCTION__;
	}
	~m_IClassFactory()
	{
		ProxyAddressLookupTable.DeleteAddress(this);
	}

	void SetCLSID(REFCLSID rclsid) { ClassID = rclsid; }

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * ppvObj);
	STDMETHOD_(ULONG, AddRef) (THIS);
	STDMETHOD_(ULONG, Release) (THIS);

	/*** IClassFactory methods ***/
	STDMETHOD(CreateInstance)(IUnknown *pUnkOuter, REFIID riid, void **ppvObject);
	STDMETHOD(LockServer)(BOOL fLock);
};
