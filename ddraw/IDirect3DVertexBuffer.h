#pragma once

class m_IDirect3DVertexBuffer : public IDirect3DVertexBuffer, public AddressLookupTableDdrawObject
{
private:
	std::unique_ptr<m_IDirect3DVertexBufferX> ProxyInterface;
	IDirect3DVertexBuffer *RealInterface;

public:
	m_IDirect3DVertexBuffer(IDirect3DVertexBuffer *aOriginal) : RealInterface(aOriginal)
	{
		ProxyInterface = std::make_unique<m_IDirect3DVertexBufferX>((IDirect3DVertexBuffer7*)RealInterface, 1, (m_IDirect3DVertexBuffer7*)this);
		ProxyAddressLookupTable.SaveAddress(this, RealInterface);
	}
	~m_IDirect3DVertexBuffer()
	{
		ProxyAddressLookupTable.DeleteAddress(this);
	}

	IDirect3DVertexBuffer *GetProxyInterface() { return RealInterface; }
	DWORD GetDirectXVersion() { return 1; }

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
