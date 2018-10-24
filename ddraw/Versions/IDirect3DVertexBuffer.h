#pragma once

class m_IDirect3DVertexBuffer : public IDirect3DVertexBuffer, public AddressLookupTableDdrawObject
{
private:
	std::unique_ptr<m_IDirect3DVertexBufferX> UniqueProxyInterface;
	m_IDirect3DVertexBufferX *ProxyInterface;
	IDirect3DVertexBuffer *RealInterface;
	REFIID WrapperID = IID_IDirect3DVertexBuffer;
	const DWORD DirectXVersion = 1;

public:
	m_IDirect3DVertexBuffer(IDirect3DVertexBuffer *aOriginal) : RealInterface(aOriginal)
	{
		UniqueProxyInterface = std::make_unique<m_IDirect3DVertexBufferX>((IDirect3DVertexBuffer7*)RealInterface, DirectXVersion, (m_IDirect3DVertexBuffer7*)this);
		ProxyInterface = UniqueProxyInterface.get();
		ProxyAddressLookupTable.SaveAddress(this, RealInterface);
	}
	m_IDirect3DVertexBuffer(m_IDirect3DVertexBufferX *aOriginal) : ProxyInterface(aOriginal)
	{
		RealInterface = nullptr;
	}
	~m_IDirect3DVertexBuffer()
	{
		ProxyAddressLookupTable.DeleteAddress(this);
	}

	DWORD GetDirectXVersion() { return DirectXVersion; }
	REFIID GetWrapperType() { return WrapperID; }
	IDirect3DVertexBuffer *GetProxyInterface() { return RealInterface; }
	m_IDirect3DVertexBufferX *GetWrapperInterface() { return ProxyInterface; }

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	/*** IDirect3DVertexBuffer methods ***/
	STDMETHOD(Lock)(THIS_ DWORD, LPVOID*, LPDWORD);
	STDMETHOD(Unlock)(THIS);
	STDMETHOD(ProcessVertices)(THIS_ DWORD, DWORD, DWORD, LPDIRECT3DVERTEXBUFFER, DWORD, LPDIRECT3DDEVICE3, DWORD);
	STDMETHOD(GetVertexBufferDesc)(THIS_ LPD3DVERTEXBUFFERDESC);
	STDMETHOD(Optimize)(THIS_ LPDIRECT3DDEVICE3, DWORD);
};
