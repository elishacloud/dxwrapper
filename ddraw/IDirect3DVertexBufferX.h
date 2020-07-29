#pragma once

class m_IDirect3DVertexBufferX : public IDirect3DVertexBuffer7, public AddressLookupTableDdrawObject
{
private:
	IDirect3DVertexBuffer7 *ProxyInterface = nullptr;
	DWORD ProxyDirectXVersion;
	ULONG RefCount = 1;

	// Convert Material
	m_IDirect3DDeviceX **D3DDeviceInterface = nullptr;
	D3DVERTEXBUFFERDESC VBDesc = { NULL};

	// Store version wrappers
	m_IDirect3DVertexBuffer *WrapperInterface;
	m_IDirect3DVertexBuffer7 *WrapperInterface7;

	// Wrapper interface functions
	REFIID GetWrapperType(DWORD DirectXVersion)
	{
		return (DirectXVersion == 1) ? IID_IDirect3DVertexBuffer :
			(DirectXVersion == 7) ? IID_IDirect3DVertexBuffer7 : IID_IUnknown;
	}
	IDirect3DVertexBuffer *GetProxyInterfaceV1() { return (IDirect3DVertexBuffer *)ProxyInterface; }
	IDirect3DVertexBuffer7 *GetProxyInterfaceV7() { return ProxyInterface; }

public:
	m_IDirect3DVertexBufferX(IDirect3DVertexBuffer7 *aOriginal, DWORD DirectXVersion) : ProxyInterface(aOriginal)
	{
		ProxyDirectXVersion = GetGUIDVersion(ConvertREFIID(GetWrapperType(DirectXVersion)));

		if (ProxyDirectXVersion != DirectXVersion)
		{
			LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << "(" << this << ")" << " converting interface from v" << DirectXVersion << " to v" << ProxyDirectXVersion);
		}
		else
		{
			LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << "(" << this << ") v" << DirectXVersion);
		}

		WrapperInterface = new m_IDirect3DVertexBuffer((LPDIRECT3DVERTEXBUFFER)ProxyInterface, this);
		WrapperInterface7 = new m_IDirect3DVertexBuffer7((LPDIRECT3DVERTEXBUFFER7)ProxyInterface, this);

		ProxyAddressLookupTable.SaveAddress(this, ProxyInterface);
	}
	m_IDirect3DVertexBufferX(m_IDirect3DDeviceX **D3DDInterface, LPD3DVERTEXBUFFERDESC lpVBDesc, DWORD DirectXVersion) : D3DDeviceInterface(D3DDInterface)
	{
		ProxyDirectXVersion = 9;

		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << "(" << this << ")" << " converting interface from v" << DirectXVersion << " to v" << ProxyDirectXVersion);

		WrapperInterface = new m_IDirect3DVertexBuffer((LPDIRECT3DVERTEXBUFFER)ProxyInterface, this);
		WrapperInterface7 = new m_IDirect3DVertexBuffer7((LPDIRECT3DVERTEXBUFFER7)ProxyInterface, this);

		if (lpVBDesc && lpVBDesc->dwSize)
		{
			VBDesc.dwSize = sizeof(D3DVERTEXBUFFERDESC);
			VBDesc.dwCaps = lpVBDesc->dwCaps;
			VBDesc.dwFVF = lpVBDesc->dwFVF;
			VBDesc.dwNumVertices = lpVBDesc->dwNumVertices;
		}

		ProxyAddressLookupTable.SaveAddress(this, ProxyInterface);
	}
	~m_IDirect3DVertexBufferX()
	{
		LOG_LIMIT(3, __FUNCTION__ << "(" << this << ")" << " deleting interface!");

		WrapperInterface->DeleteMe();
		WrapperInterface7->DeleteMe();

		ProxyAddressLookupTable.DeleteAddress(this);
	}

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * ppvObj);
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

	// Helper functions
	void *GetWrapperInterfaceX(DWORD DirectXVersion);
};
