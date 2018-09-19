#pragma once

class m_IDirect3DMaterialX : public IDirect3DMaterial3
{
private:
	IDirect3DMaterial3 *ProxyInterface;
	m_IDirect3DMaterial3 *WrapperInterface;
	DWORD DirectXVersion;
	DWORD ProxyDirectXVersion;
	IID WrapperID;
	ULONG RefCount = 1;
	m_IDirect3DDeviceX *D3DDeviceInterface;

public:
	m_IDirect3DMaterialX(IDirect3DMaterial3 *aOriginal, DWORD Version, m_IDirect3DMaterial3 *Interface) : ProxyInterface(aOriginal), DirectXVersion(Version), WrapperInterface(Interface)
	{
		InitWrapper();
	}
	m_IDirect3DMaterialX(m_IDirect3DDeviceX *D3DDInterface, DWORD Version) : D3DDeviceInterface(D3DDInterface), DirectXVersion(Version)
	{
		ProxyInterface = nullptr;
		WrapperInterface = nullptr;

		InitWrapper();
	}
	void InitWrapper()
	{
		WrapperID = (DirectXVersion == 1) ? IID_IDirect3DMaterial :
			(DirectXVersion == 2) ? IID_IDirect3DMaterial2 :
			(DirectXVersion == 3) ? IID_IDirect3DMaterial3 : IID_IDirect3DMaterial3;

		if (DirectXVersion == 7)
		{
			DirectXVersion = 3;
			ProxyDirectXVersion = 7;
		}
		else
		{
			ProxyDirectXVersion = GetIIDVersion(ConvertREFIID(WrapperID));
		}

		if (ProxyDirectXVersion != DirectXVersion)
		{
			Logging::LogDebug() << "Convert Direct3DMaterial v" << DirectXVersion << " to v" << ProxyDirectXVersion;
		}
		else
		{
			Logging::LogDebug() << "Create " << __FUNCTION__ << " v" << DirectXVersion;
		}
	}
	~m_IDirect3DMaterialX() {}

	DWORD GetDirectXVersion() { return DDWRAPPER_TYPEX; }
	REFIID GetWrapperType() { return WrapperID; }
	IDirect3DMaterial3 *GetProxyInterface() { return ProxyInterface; }
	m_IDirect3DMaterial3 *GetWrapperInterface() { return WrapperInterface; }

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	/*** IDirect3DMaterial methods ***/
	STDMETHOD(Initialize)(THIS_ LPDIRECT3D);
	STDMETHOD(SetMaterial)(THIS_ LPD3DMATERIAL);
	STDMETHOD(GetMaterial)(THIS_ LPD3DMATERIAL);
	STDMETHOD(GetHandle)(THIS_ LPDIRECT3DDEVICE3, LPD3DMATERIALHANDLE);
	STDMETHOD(Reserve)(THIS);
	STDMETHOD(Unreserve)(THIS);
};
