#pragma once

class m_IDirect3DViewportX : public IDirect3DViewport3
{
private:
	IDirect3DViewport3 *ProxyInterface;
	m_IDirect3DViewport3 *WrapperInterface;
	DWORD DirectXVersion;
	DWORD ProxyDirectXVersion;
	IID WrapperID;

public:
	m_IDirect3DViewportX(IDirect3DViewport3 *aOriginal, DWORD Version, m_IDirect3DViewport3 *Interface) : ProxyInterface(aOriginal), DirectXVersion(Version), WrapperInterface(Interface)
	{
		WrapperID = (DirectXVersion == 1) ? IID_IDirect3DViewport :
			(DirectXVersion == 2) ? IID_IDirect3DViewport2 :
			(DirectXVersion == 3) ? IID_IDirect3DViewport3 : IID_IDirect3DViewport3;

		REFIID ProxyID = ConvertREFIID(WrapperID);
		ProxyDirectXVersion = (ProxyID == IID_IDirect3DViewport) ? 1 :
			(ProxyID == IID_IDirect3DViewport2) ? 2 :
			(ProxyID == IID_IDirect3DViewport3) ? 3 : 3;

		if (ProxyDirectXVersion != DirectXVersion)
		{
			Logging::LogDebug() << "Convert Direct3DViewport v" << DirectXVersion << " to v" << ProxyDirectXVersion;
		}
	}
	~m_IDirect3DViewportX() {}

	DWORD GetDirectXVersion() { return DirectXVersion; }

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	/*** IDirect3DViewport2 methods ***/
	STDMETHOD(Initialize)(THIS_ LPDIRECT3D);
	STDMETHOD(GetViewport)(THIS_ LPD3DVIEWPORT);
	STDMETHOD(SetViewport)(THIS_ LPD3DVIEWPORT);
	STDMETHOD(TransformVertices)(THIS_ DWORD, LPD3DTRANSFORMDATA, DWORD, LPDWORD);
	STDMETHOD(LightElements)(THIS_ DWORD, LPD3DLIGHTDATA);
	STDMETHOD(SetBackground)(THIS_ D3DMATERIALHANDLE);
	STDMETHOD(GetBackground)(THIS_ LPD3DMATERIALHANDLE, LPBOOL);
	STDMETHOD(SetBackgroundDepth)(THIS_ LPDIRECTDRAWSURFACE);
	STDMETHOD(GetBackgroundDepth)(THIS_ LPDIRECTDRAWSURFACE*, LPBOOL);
	STDMETHOD(Clear)(THIS_ DWORD, LPD3DRECT, DWORD);
	STDMETHOD(AddLight)(THIS_ LPDIRECT3DLIGHT);
	STDMETHOD(DeleteLight)(THIS_ LPDIRECT3DLIGHT);
	STDMETHOD(NextLight)(THIS_ LPDIRECT3DLIGHT, LPDIRECT3DLIGHT*, DWORD);
	STDMETHOD(GetViewport2)(THIS_ LPD3DVIEWPORT2);
	STDMETHOD(SetViewport2)(THIS_ LPD3DVIEWPORT2);
	STDMETHOD(SetBackgroundDepth2)(THIS_ LPDIRECTDRAWSURFACE4);
	STDMETHOD(GetBackgroundDepth2)(THIS_ LPDIRECTDRAWSURFACE4*, LPBOOL);
	STDMETHOD(Clear2)(THIS_ DWORD, LPD3DRECT, DWORD, D3DCOLOR, D3DVALUE, DWORD);
};
