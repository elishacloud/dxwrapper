#pragma once

class m_IDirectDrawColorControl : public IDirectDrawColorControl, public AddressLookupTableDdrawObject
{
private:
	IDirectDrawColorControl *ProxyInterface = nullptr;
	REFIID WrapperID = IID_IDirectDrawColorControl;
	ULONG RefCount = 1;
	DDCOLORCONTROL ColorControl;

public:
	m_IDirectDrawColorControl(IDirectDrawColorControl *aOriginal) : ProxyInterface(aOriginal)
	{
		ProxyAddressLookupTable.SaveAddress(this, ProxyInterface);

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

		Logging::LogDebug() << "Create " << __FUNCTION__;
	}
	m_IDirectDrawColorControl()
	{
		Logging::LogDebug() << "Create " << __FUNCTION__;
	}
	~m_IDirectDrawColorControl()
	{
		ProxyAddressLookupTable.DeleteAddress(this);
	}

	DWORD GetDirectXVersion() { return 1; }
	REFIID GetWrapperType() { return WrapperID; }
	IDirectDrawColorControl *GetProxyInterface() { return ProxyInterface; }
	m_IDirectDrawColorControl *GetWrapperInterface() { return this; }

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * ppvObj);
	STDMETHOD_(ULONG, AddRef) (THIS);
	STDMETHOD_(ULONG, Release) (THIS);

	/*** IDirectDrawColorControl methods ***/
	STDMETHOD(GetColorControls)(THIS_ LPDDCOLORCONTROL);
	STDMETHOD(SetColorControls)(THIS_ LPDDCOLORCONTROL);
};
