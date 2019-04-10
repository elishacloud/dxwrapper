#pragma once

class m_IDirect3DVertexBuffer7 : public IDirect3DVertexBuffer7, public AddressLookupTableDdrawObject
{
private:
	std::unique_ptr<m_IDirect3DVertexBufferX> UniqueProxyInterface;
	m_IDirect3DVertexBufferX *ProxyInterface;
	IDirect3DVertexBuffer7 *RealInterface;
	REFIID WrapperID = IID_IDirect3DVertexBuffer7;
	const DWORD DirectXVersion = 7;

public:
	m_IDirect3DVertexBuffer7(IDirect3DVertexBuffer7 *aOriginal) : RealInterface(aOriginal)
	{
		UniqueProxyInterface = std::make_unique<m_IDirect3DVertexBufferX>((IDirect3DVertexBuffer7*)RealInterface, DirectXVersion, (m_IDirect3DVertexBuffer7*)this);
		ProxyInterface = UniqueProxyInterface.get();
		ProxyAddressLookupTable.SaveAddress(this, RealInterface);
	}
	m_IDirect3DVertexBuffer7(m_IDirect3DVertexBufferX *aOriginal) : ProxyInterface(aOriginal)
	{
		RealInterface = nullptr;
	}
	~m_IDirect3DVertexBuffer7()
	{
		ProxyAddressLookupTable.DeleteAddress(this);
	}

	DWORD GetDirectXVersion() { return DirectXVersion; }
	REFIID GetWrapperType() { return WrapperID; }
	IDirect3DVertexBuffer7 *GetProxyInterface() { return RealInterface; }
	m_IDirect3DVertexBufferX *GetWrapperInterface() { return ProxyInterface; }

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	/*** IDirect3DVertexBuffer methods ***/
	STDMETHOD(Lock)(THIS_ DWORD, LPVOID*, LPDWORD);
	STDMETHOD(Unlock)(THIS);
	STDMETHOD(ProcessVertices)(THIS_ DWORD, DWORD, DWORD, LPDIRECT3DVERTEXBUFFER7, DWORD, LPDIRECT3DDEVICE7, DWORD);
	STDMETHOD(GetVertexBufferDesc)(THIS_ LPD3DVERTEXBUFFERDESC);
	STDMETHOD(Optimize)(THIS_ LPDIRECT3DDEVICE7, DWORD);
	/*** IDirect3DVertexBuffer7 methods ***/
	STDMETHOD(ProcessVerticesStrided)(THIS_ DWORD, DWORD, DWORD, LPD3DDRAWPRIMITIVESTRIDEDDATA, DWORD, LPDIRECT3DDEVICE7, DWORD);
};
