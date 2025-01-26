#pragma once

m_IDirectDrawColorControl* CreateDirectDrawColorControl(IDirectDrawColorControl* aOriginal, m_IDirectDrawX* NewParent);

class m_IDirectDrawColorControl : public IDirectDrawColorControl, public AddressLookupTableDdrawObject
{
private:
	IDirectDrawColorControl *ProxyInterface = nullptr;
	REFIID WrapperID = IID_IDirectDrawColorControl;
	ULONG RefCount = 1;

	// Convert to Direct3D9
	m_IDirectDrawX *ddrawParent = nullptr;
	DDCOLORCONTROL ColorControl;

	// Interface initialization functions
	void InitInterface();
	void ReleaseInterface();

public:
	m_IDirectDrawColorControl(IDirectDrawColorControl *aOriginal) : ProxyInterface(aOriginal)
	{
		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ")");

		InitInterface();

		ProxyAddressLookupTable.SaveAddress(this, (ProxyInterface) ? ProxyInterface : (void*)this);
	}
	m_IDirectDrawColorControl(m_IDirectDrawX *Interface) : ddrawParent(Interface)
	{
		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ")");

		InitInterface();

		ProxyAddressLookupTable.SaveAddress(this, (ProxyInterface) ? ProxyInterface : (void*)this);
	}
	~m_IDirectDrawColorControl()
	{
		LOG_LIMIT(3, __FUNCTION__ << " (" << this << ")" << " deleting interface!");

		ReleaseInterface();

		ProxyAddressLookupTable.DeleteAddress(this);
	}

	void SetProxy(IDirectDrawColorControl* NewProxyInterface, m_IDirectDrawX* NewParent)
	{
		if (NewProxyInterface || NewParent)
		{
			RefCount = 1;
			ProxyInterface = NewProxyInterface;
			ddrawParent = NewParent;
			InitInterface();
			ProxyAddressLookupTable.SaveAddress(this, (ProxyInterface) ? ProxyInterface : (void*)this);
		}
		else
		{
			ReleaseInterface();
			ProxyAddressLookupTable.DeleteAddress(this);
			ProxyInterface = nullptr;
			ddrawParent = nullptr;
		}
	}

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * ppvObj);
	STDMETHOD_(ULONG, AddRef) (THIS);
	STDMETHOD_(ULONG, Release) (THIS);

	/*** IDirectDrawColorControl methods ***/
	STDMETHOD(GetColorControls)(THIS_ LPDDCOLORCONTROL);
	STDMETHOD(SetColorControls)(THIS_ LPDDCOLORCONTROL);

	// Functions handling the ddraw parent interface
	void SetDdrawParent(m_IDirectDrawX *ddraw) { ddrawParent = ddraw; }
	void ClearDdraw() { ddrawParent = nullptr; }
};
