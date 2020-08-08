#pragma once

class m_IDirectDrawColorControl : public IDirectDrawColorControl, public AddressLookupTableDdrawObject
{
private:
	IDirectDrawColorControl *ProxyInterface = nullptr;
	REFIID WrapperID = IID_IDirectDrawColorControl;
	ULONG RefCount = 1;

	// Convert to Direct3D9
	m_IDirectDrawX *ddrawParent = nullptr;
	DDCOLORCONTROL ColorControl;

public:
	m_IDirectDrawColorControl(IDirectDrawColorControl *aOriginal) : ProxyInterface(aOriginal)
	{
		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << "(" << this << ")");

		InitColorControl();

		ProxyAddressLookupTable.SaveAddress(this, (ProxyInterface) ? ProxyInterface : (void*)this);
	}
	m_IDirectDrawColorControl(m_IDirectDrawX *Interface) : ddrawParent(Interface)
	{
		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << "(" << this << ")");

		InitColorControl();

		ProxyAddressLookupTable.SaveAddress(this, (ProxyInterface) ? ProxyInterface : (void*)this);
	}
	void InitColorControl()
	{
		ColorControl.dwSize = sizeof(DDCOLORCONTROL);
		ColorControl.dwFlags = DDCOLOR_BRIGHTNESS | DDCOLOR_CONTRAST | DDCOLOR_HUE | DDCOLOR_SATURATION | DDCOLOR_SHARPNESS | DDCOLOR_GAMMA | DDCOLOR_COLORENABLE;
		ColorControl.lBrightness = 750;
		ColorControl.lContrast = 10000;
		ColorControl.lHue = 0;
		ColorControl.lSaturation = 10000;
		ColorControl.lSharpness = 5;
		ColorControl.lGamma = 1;
		ColorControl.lColorEnable = 1;
		ColorControl.dwReserved1 = 0;
	}
	~m_IDirectDrawColorControl()
	{
		LOG_LIMIT(3, __FUNCTION__ << "(" << this << ")" << " deleting interface!");

		ProxyAddressLookupTable.DeleteAddress(this);

		if (!ProxyInterface && !Config.Exiting)
		{
			if (ddrawParent)
			{
				ddrawParent->ClearColorInterface();
			}
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
