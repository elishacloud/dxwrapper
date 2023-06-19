#pragma once

class m_IDirect3DVertexBufferX : public IDirect3DVertexBuffer7, public AddressLookupTableDdrawObject
{
private:
	IDirect3DVertexBuffer7 *ProxyInterface = nullptr;
	DWORD ProxyDirectXVersion;
	ULONG RefCount1 = 0;
	ULONG RefCount7 = 0;
	m_IDirectDrawX* ddrawParent = nullptr;
	LPDIRECT3DDEVICE9* d3d9Device = nullptr;

	// Vertex buffer desc
	D3DVERTEXBUFFERDESC VBDesc = {};
	D3DVERTEXBUFFER_DESC d3d9VBDesc = {};

	// Vector buffer data
	std::vector<BYTE> VertexData;
	void* LastLockAddr = nullptr;
	DWORD LastLockFlags = 0;

	// Index buffer data
	DWORD IndexBufferSize = 0;

	// Store d3d interface
	LPDIRECT3DVERTEXBUFFER9 d3d9VertexBuffer = nullptr;
	LPDIRECT3DINDEXBUFFER9 d3d9IndexBuffer = nullptr;

	// Store version wrappers
	m_IDirect3DVertexBuffer *WrapperInterface;
	m_IDirect3DVertexBuffer7 *WrapperInterface7;

	// Wrapper interface functions
	inline REFIID GetWrapperType(DWORD DirectXVersion)
	{
		return (DirectXVersion == 1) ? IID_IDirect3DVertexBuffer :
			(DirectXVersion == 7) ? IID_IDirect3DVertexBuffer7 : IID_IUnknown;
	}
	inline bool CheckWrapperType(REFIID IID)
	{
		return (IID == IID_IDirect3DVertexBuffer ||
			IID == IID_IDirect3DVertexBuffer7) ? true : false;
	}
	inline IDirect3DVertexBuffer *GetProxyInterfaceV1() { return (IDirect3DVertexBuffer *)ProxyInterface; }
	inline IDirect3DVertexBuffer7 *GetProxyInterfaceV7() { return ProxyInterface; }

	// Interface initialization functions
	void InitVertexBuffer(DWORD DirectXVersion);
	void ReleaseVertexBuffer();

	// Check interfaces
	HRESULT CheckInterface(char* FunctionName, bool CheckD3DVertexBuffer, bool CheckD3DDevice);

	// Direct3D9 interface functions
	HRESULT CreateD3D9VertexBuffer();
	void ReleaseD3D9VertexBuffer();
	void ReleaseD3D9IndexBuffer();

public:
	m_IDirect3DVertexBufferX(IDirect3DVertexBuffer7 *aOriginal, DWORD DirectXVersion) : ProxyInterface(aOriginal)
	{
		ProxyDirectXVersion = GetGUIDVersion(ConvertREFIID(GetWrapperType(DirectXVersion)));

		if (ProxyDirectXVersion != DirectXVersion)
		{
			LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ")" << " converting interface from v" << DirectXVersion << " to v" << ProxyDirectXVersion);
		}
		else
		{
			LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ") v" << DirectXVersion);
		}

		InitVertexBuffer(DirectXVersion);
	}
	m_IDirect3DVertexBufferX(m_IDirectDrawX* lpDdraw, LPD3DVERTEXBUFFERDESC lpVBDesc, DWORD DirectXVersion) : ddrawParent(lpDdraw)
	{
		ProxyDirectXVersion = 9;

		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ")" << " converting interface from v" << DirectXVersion << " to v" << ProxyDirectXVersion);

		if (lpVBDesc)
		{
			VBDesc.dwSize = sizeof(D3DVERTEXBUFFERDESC);
			VBDesc.dwCaps = lpVBDesc->dwCaps;
			VBDesc.dwFVF = lpVBDesc->dwFVF;
			VBDesc.dwNumVertices = lpVBDesc->dwNumVertices;
		}
		d3d9VBDesc.Type = D3DRTYPE_VERTEXBUFFER;

		InitVertexBuffer(DirectXVersion);
	}
	~m_IDirect3DVertexBufferX()
	{
		LOG_LIMIT(3, __FUNCTION__ << " (" << this << ")" << " deleting interface!");

		ReleaseVertexBuffer();
	}

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * ppvObj) { return QueryInterface(riid, ppvObj, 0); }
	STDMETHOD_(ULONG, AddRef) (THIS) { return AddRef(0); }
	STDMETHOD_(ULONG, Release) (THIS) { return Release(0); }

	/*** IDirect3DVertexBuffer methods ***/
	STDMETHOD(Lock)(THIS_ DWORD, LPVOID*, LPDWORD);
	STDMETHOD(Unlock)(THIS);
	STDMETHOD(ProcessVertices)(THIS_ DWORD, DWORD, DWORD, LPDIRECT3DVERTEXBUFFER7, DWORD, LPDIRECT3DDEVICE7, DWORD);
	STDMETHOD(GetVertexBufferDesc)(THIS_ LPD3DVERTEXBUFFERDESC);
	STDMETHOD(Optimize)(THIS_ LPDIRECT3DDEVICE7, DWORD);

	/*** IDirect3DVertexBuffer7 methods ***/
	STDMETHOD(ProcessVerticesStrided)(THIS_ DWORD, DWORD, DWORD, LPD3DDRAWPRIMITIVESTRIDEDDATA, DWORD, LPDIRECT3DDEVICE7, DWORD);

	// Helper functions
	HRESULT QueryInterface(REFIID riid, LPVOID FAR * ppvObj, DWORD DirectXVersion);
	void *GetWrapperInterfaceX(DWORD DirectXVersion);
	ULONG AddRef(DWORD DirectXVersion);
	ULONG Release(DWORD DirectXVersion);

	// Functions handling the ddraw parent interface
	void SetDdrawParent(m_IDirectDrawX* ddraw) { ddrawParent = ddraw; }
	void ClearDdraw() { ddrawParent = nullptr; }

	// Direct3D9 interfaces
	LPDIRECT3DVERTEXBUFFER9 GetCurrentD9VertexBuffer() { return d3d9VertexBuffer; };
	LPDIRECT3DINDEXBUFFER9 SetupIndexBuffer(LPWORD lpwIndices, DWORD dwIndexCount);
	void ReleaseD9Buffers(bool BackupData);

	DWORD GetFVF9() { return d3d9VBDesc.FVF; };
};
