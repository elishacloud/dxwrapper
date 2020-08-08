#pragma once

class m_IDirectDrawGammaControl : public IDirectDrawGammaControl, public AddressLookupTableDdrawObject
{
private:
	IDirectDrawGammaControl *ProxyInterface = nullptr;
	REFIID WrapperID = IID_IDirectDrawGammaControl;
	ULONG RefCount = 1;

	// Convert to Direct3D9
	m_IDirectDrawX *ddrawParent = nullptr;
	DDGAMMARAMP RampData = { NULL };

public:
	m_IDirectDrawGammaControl(IDirectDrawGammaControl *aOriginal) : ProxyInterface(aOriginal)
	{
		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << "(" << this << ")");

		ProxyAddressLookupTable.SaveAddress(this, (ProxyInterface) ? ProxyInterface : (void*)this);
	}
	m_IDirectDrawGammaControl(m_IDirectDrawX *Interface) : ddrawParent(Interface)
	{
		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << "(" << this << ")");

		ProxyAddressLookupTable.SaveAddress(this, (ProxyInterface) ? ProxyInterface : (void*)this);
	}
	~m_IDirectDrawGammaControl()
	{
		LOG_LIMIT(3, __FUNCTION__ << "(" << this << ")" << " deleting interface!");

		ProxyAddressLookupTable.DeleteAddress(this);

		if (!ProxyInterface && !Config.Exiting)
		{
			if (ddrawParent)
			{
				ddrawParent->ClearGammaInterface();
			}
		}
	}

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * ppvObj);
	STDMETHOD_(ULONG, AddRef) (THIS);
	STDMETHOD_(ULONG, Release) (THIS);

	/*** IDirectDrawGammaControl methods ***/
	STDMETHOD(GetGammaRamp)(THIS_ DWORD, LPDDGAMMARAMP);
	STDMETHOD(SetGammaRamp)(THIS_ DWORD, LPDDGAMMARAMP);

	// Functions handling the ddraw parent interface
	void SetDdrawParent(m_IDirectDrawX *ddraw) { ddrawParent = ddraw; }
	void ClearDdraw() { ddrawParent = nullptr; }
};
