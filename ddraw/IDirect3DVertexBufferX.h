#pragma once

class m_IDirect3DVertexBufferX : public IDirect3DVertexBuffer7
{
private:
	IDirect3DVertexBuffer7 *ProxyInterface = nullptr;
	m_IDirect3DVertexBuffer7 *WrapperInterface = nullptr;
	DWORD ProxyDirectXVersion;
	ULONG RefCount = 1;

	// Convert Material
	m_IDirect3DDeviceX **D3DDeviceInterface = nullptr;
	D3DVERTEXBUFFERDESC VBDesc = { NULL};

	// Store ddraw version wrappers
	std::unique_ptr<m_IDirect3DVertexBuffer> UniqueProxyInterface = nullptr;
	std::unique_ptr<m_IDirect3DVertexBuffer7> UniqueProxyInterface7 = nullptr;

public:
	m_IDirect3DVertexBufferX(IDirect3DVertexBuffer7 *aOriginal, DWORD DirectXVersion, m_IDirect3DVertexBuffer7 *Interface) : ProxyInterface(aOriginal), WrapperInterface(Interface)
	{
		ProxyDirectXVersion = GetIIDVersion(ConvertREFIID(GetWrapperType(DirectXVersion)));

		if (ProxyDirectXVersion != DirectXVersion)
		{
			LOG_LIMIT(3, "Creating device " << __FUNCTION__ << "(" << this << ")" << " converting device from v" << DirectXVersion << " to v" << ProxyDirectXVersion);
		}
		else
		{
			LOG_LIMIT(3, "Creating device " << __FUNCTION__ << "(" << this << ") v" << DirectXVersion);
		}
	}
	m_IDirect3DVertexBufferX(m_IDirect3DDeviceX **D3DDInterface, LPD3DVERTEXBUFFERDESC lpVBDesc, DWORD DirectXVersion) : D3DDeviceInterface(D3DDInterface)
	{
		ProxyDirectXVersion = 9;

		LOG_LIMIT(3, "Creating device " << __FUNCTION__ << "(" << this << ")" << " converting device from v" << DirectXVersion << " to v" << ProxyDirectXVersion);

		if (lpVBDesc && lpVBDesc->dwSize)
		{
			VBDesc.dwSize = sizeof(D3DVERTEXBUFFERDESC);
			VBDesc.dwCaps = lpVBDesc->dwCaps;
			VBDesc.dwFVF = lpVBDesc->dwFVF;
			VBDesc.dwNumVertices = lpVBDesc->dwNumVertices;
		}
	}
	~m_IDirect3DVertexBufferX()
	{
		LOG_LIMIT(3, __FUNCTION__ << "(" << this << ")" << " deleting device!");
	}

	DWORD GetDirectXVersion() { return DDWRAPPER_TYPEX; }
	REFIID GetWrapperType() { return IID_IUnknown; }
	IDirect3DVertexBuffer7 *GetProxyInterface() { return ProxyInterface; }
	m_IDirect3DVertexBuffer7 *GetWrapperInterface() { return WrapperInterface; }

	/*** IUnknown methods ***/
	HRESULT QueryInterface(REFIID riid, LPVOID FAR * ppvObj, DWORD DirectXVersion);
	STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * ppvObj) { return QueryInterface(riid, ppvObj, ProxyDirectXVersion); }
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

	// Wrapper interface functions
	REFIID GetWrapperType(DWORD DirectXVersion)
	{
		return (DirectXVersion == 1) ? IID_IDirect3DVertexBuffer :
			(DirectXVersion == 7) ? IID_IDirect3DVertexBuffer7 : IID_IUnknown;
	}
	IDirect3DVertexBuffer *GetProxyInterfaceV1() { return (IDirect3DVertexBuffer *)ProxyInterface; }
	IDirect3DVertexBuffer7 *GetProxyInterfaceV7() { return ProxyInterface; }
	void *GetWrapperInterfaceX(DWORD DirectXVersion);
};
