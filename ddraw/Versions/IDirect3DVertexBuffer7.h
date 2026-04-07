#pragma once

class m_IDirect3DVertexBuffer7 final : public IDirect3DVertexBuffer7, public AddressLookupTableDdrawObject
{
private:
	m_IDirect3DVertexBufferX *ProxyInterface;
	const IID WrapperID = IID_IDirect3DVertexBuffer7;
	const DWORD DirectXVersion = 7;

public:
	m_IDirect3DVertexBuffer7(IDirect3DVertexBuffer7 *, m_IDirect3DVertexBufferX *Interface) : ProxyInterface(Interface)
	{
		ProxyAddressLookupTable.SaveAddress(this, ProxyInterface);
	}
	~m_IDirect3DVertexBuffer7()
	{
		ProxyAddressLookupTable.DeleteAddress(this);
	}

	void SetProxy(m_IDirect3DVertexBufferX* NewProxyInterface)
	{
		ProxyInterface = NewProxyInterface;
		if (NewProxyInterface)
		{
			ProxyAddressLookupTable.SaveAddress(this, ProxyInterface);
		}
		else
		{
			ProxyAddressLookupTable.DeleteAddress(this);
		}
	}

	/*** IUnknown methods ***/
	IFACEMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj) override;
	IFACEMETHOD_(ULONG, AddRef)(THIS) override;
	IFACEMETHOD_(ULONG, Release)(THIS) override;

	/*** IDirect3DVertexBuffer methods ***/
	IFACEMETHOD(Lock)(THIS_ DWORD, LPVOID*, LPDWORD) override;
	IFACEMETHOD(Unlock)(THIS) override;
	IFACEMETHOD(ProcessVertices)(THIS_ DWORD, DWORD, DWORD, LPDIRECT3DVERTEXBUFFER7, DWORD, LPDIRECT3DDEVICE7, DWORD) override;
	IFACEMETHOD(GetVertexBufferDesc)(THIS_ LPD3DVERTEXBUFFERDESC) override;
	IFACEMETHOD(Optimize)(THIS_ LPDIRECT3DDEVICE7, DWORD) override;
	/*** IDirect3DVertexBuffer7 methods ***/
	IFACEMETHOD(ProcessVerticesStrided)(THIS_ DWORD, DWORD, DWORD, LPD3DDRAWPRIMITIVESTRIDEDDATA, DWORD, LPDIRECT3DDEVICE7, DWORD) override;
};
