#pragma once

class m_IDirectDrawColorControl final : public IDirectDrawColorControl, public AddressLookupTableDdrawObject
{
private:
	IDirectDrawColorControl *ProxyInterface = nullptr;
	ULONG RefCount = 1;
	const IID WrapperID = IID_IDirectDrawColorControl;

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

		if (Config.Dd7to9)
		{
			Logging::Log() << __FUNCTION__ << " (" << this << ") Warning: created from non-dd7to9 interface!";
		}

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
	IFACEMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * ppvObj) override;
	IFACEMETHOD_(ULONG, AddRef) (THIS) override;
	IFACEMETHOD_(ULONG, Release) (THIS) override;

	/*** IDirectDrawColorControl methods ***/
	IFACEMETHOD(GetColorControls)(THIS_ LPDDCOLORCONTROL) override;
	IFACEMETHOD(SetColorControls)(THIS_ LPDDCOLORCONTROL) override;

	// Functions handling the ddraw parent interface
	void ClearDdraw() { ddrawParent = nullptr; }
	static m_IDirectDrawColorControl* CreateDirectDrawColorControl(IDirectDrawColorControl* aOriginal, m_IDirectDrawX* NewParent);

};
