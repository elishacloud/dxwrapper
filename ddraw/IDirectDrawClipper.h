#pragma once

class m_IDirectDrawClipper : public IDirectDrawClipper, public AddressLookupTableDdrawObject
{
private:
	IDirectDrawClipper *ProxyInterface = nullptr;
	ULONG RefCount = 1;
	const IID WrapperID = IID_IDirectDrawClipper;

	// Convert to Direct3D9
	m_IDirectDrawX* ddrawParent = nullptr;
	DWORD clipperCaps = 0;						// Clipper flags
	HWND cliphWnd = nullptr;
	std::vector<BYTE> ClipList;
	bool IsClipListSet = false;
	RECT LastClipBounds = {};

	// Helper functions
	bool CheckHwnd();

	// Interface initialization functions
	void InitInterface(DWORD dwFlags);
	void ReleaseInterface();

public:
	m_IDirectDrawClipper(IDirectDrawClipper *aOriginal) : ProxyInterface(aOriginal)
	{
		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ")");

		if (Config.Dd7to9)
		{
			Logging::Log() << __FUNCTION__ << " (" << this << ") Warning: created from non-dd7to9 interface!";
		}

		InitInterface(0);

		ProxyAddressLookupTable.SaveAddress(this, (ProxyInterface) ? ProxyInterface : (void*)this);
	}
	m_IDirectDrawClipper(m_IDirectDrawX* Interface, DWORD dwFlags) : ddrawParent(Interface)
	{
		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ")");

		InitInterface(dwFlags);

		ProxyAddressLookupTable.SaveAddress(this, (ProxyInterface) ? ProxyInterface : (void*)this);
	}
	~m_IDirectDrawClipper()
	{
		LOG_LIMIT(3, __FUNCTION__ << " (" << this << ")" << " deleting interface!");

		ReleaseInterface();

		ProxyAddressLookupTable.DeleteAddress(this);
	}

	void SetProxy(IDirectDrawClipper* NewProxyInterface, m_IDirectDrawX* NewParent, DWORD dwFlags)
	{
		if (NewProxyInterface || NewParent)
		{
			RefCount = 1;
			ProxyInterface = NewProxyInterface;
			ddrawParent = NewParent;
			InitInterface(dwFlags);
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

	/*** IDirectDrawClipper methods ***/
	STDMETHOD(GetClipList)(THIS_ LPRECT, LPRGNDATA, LPDWORD);
	STDMETHOD(GetHWnd)(THIS_ HWND FAR *);
	STDMETHOD(Initialize)(THIS_ LPDIRECTDRAW, DWORD);
	STDMETHOD(IsClipListChanged)(THIS_ BOOL FAR *);
	STDMETHOD(SetClipList)(THIS_ LPRGNDATA, DWORD);
	STDMETHOD(SetHWnd)(THIS_ DWORD, HWND);

	// Functions handling the ddraw parent interface
	void SetDdrawParent(m_IDirectDrawX* ddraw) { ddrawParent = ddraw; }
	void ClearDdraw() { ddrawParent = nullptr; }
	bool HasClipList() const { return IsClipListSet; }
	bool GetClipBoundsFromData(RECT& bounds);
	HRESULT GetClipRegion(HRGN hOutRgn);
	static m_IDirectDrawClipper* CreateDirectDrawClipper(IDirectDrawClipper* aOriginal, m_IDirectDrawX* NewParent, DWORD dwFlags);
};
