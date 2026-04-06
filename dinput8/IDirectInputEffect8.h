#pragma once

class m_IDirectInputEffect8 final : public IDirectInputEffect, AddressLookupTableDinput8Object<m_IDirectInputEffect8>, ModuleObjectCount::CountedObject
{
private:
	IDirectInputEffect *ProxyInterface;

	LONG RefCount = 1;

public:
	m_IDirectInputEffect8(IDirectInputEffect *aOriginal) : AddressLookupTableDinput8Object(aOriginal), ProxyInterface(aOriginal)
	{
		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ")");
	}
	~m_IDirectInputEffect8()
	{
		LOG_LIMIT(3, __FUNCTION__ << " (" << this << ")" << " deleting interface!");
	}

	/*** IUnknown methods ***/
	IFACEMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) override;
	IFACEMETHOD_(ULONG, AddRef)(THIS) override;
	IFACEMETHOD_(ULONG, Release)(THIS) override;

	/*** IDirectInputEffect methods ***/
	IFACEMETHOD(Initialize)(THIS_ HINSTANCE, DWORD, REFGUID) override;
	IFACEMETHOD(GetEffectGuid)(THIS_ LPGUID) override;
	IFACEMETHOD(GetParameters)(THIS_ LPDIEFFECT, DWORD) override;
	IFACEMETHOD(SetParameters)(THIS_ LPCDIEFFECT, DWORD) override;
	IFACEMETHOD(Start)(THIS_ DWORD, DWORD) override;
	IFACEMETHOD(Stop)(THIS) override;
	IFACEMETHOD(GetEffectStatus)(THIS_ LPDWORD) override;
	IFACEMETHOD(Download)(THIS) override;
	IFACEMETHOD(Unload)(THIS) override;
	IFACEMETHOD(Escape)(THIS_ LPDIEFFESCAPE) override;

	// Helper functions
	IDirectInputEffect *GetProxyInterface() { return ProxyInterface; }
};
