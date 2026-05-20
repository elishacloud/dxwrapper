#pragma once

class m_IDirect3DVertexBufferX final : public IUnknown, public AddressLookupTableDdrawObject
{
private:
	IDirect3DVertexBuffer7 *ProxyInterface = nullptr;
	DWORD ProxyDirectXVersion;
	LONG RefCount1 = 0;
	LONG RefCount7 = 0;

	// Store version wrappers
	m_IDirect3DVertexBuffer* WrapperInterface = nullptr;
	m_IDirect3DVertexBuffer7* WrapperInterface7 = nullptr;

	// Convert to Direct3D9
	m_IDirectDrawX* ddrawParent = nullptr;
	m_IDirect3DX* D3DInterface = nullptr;
	LPDIRECT3DDEVICE9* d3d9Device = nullptr;
	LPDIRECT3DVERTEXBUFFER9 d3d9VertexBuffer = nullptr;

	// Vertex buffer desc
	struct {
		D3DVERTEXBUFFERDESC Desc = {};
		DWORD Size = 0;
	} VB;
	D3DVERTEXBUFFER_DESC d3d9VBDesc = {};

	// Vector buffer data
	std::vector<BYTE, aligned_allocator<BYTE, 4>> VertexData;
	bool IsVBEmulated = false;
	struct {
		bool IsLocked = false;
		void* Addr = nullptr;
		DWORD Flags = 0;
	} LastLock;

	// Lighting data
	struct LightingState
	{
		bool UseSpecular;
		bool LocalViewer;
		bool NormalizeNormals;
		bool ColorVertex;

		DWORD DiffuseMaterialSource;
		DWORD AmbientMaterialSource;
		DWORD SpecularMaterialSource;
		DWORD EmissiveMaterialSource;

		D3DCOLOR AmbientRenderState;

		D3DMATRIX ViewMatrix;

		D3DMATERIAL7 Material;
	};

	// Direct3D9 interface functions
	HRESULT CreateD3D9VertexBuffer();
	void ReleaseD3D9VertexBuffer();

	// Check interfaces
	HRESULT CheckInterface(char* FunctionName, bool CheckD3DDevice, bool CheckD3DVertexBuffer);

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
	void InitInterface(DWORD DirectXVersion);
	void ReleaseInterface();

public:
	m_IDirect3DVertexBufferX(IDirect3DVertexBuffer7 *aOriginal, DWORD DirectXVersion) : ProxyInterface(aOriginal)
	{
		ProxyDirectXVersion = GetGUIDVersion(GetWrapperType(DirectXVersion));

		if (ProxyDirectXVersion != DirectXVersion)
		{
			LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ")" << " converting interface from v" << DirectXVersion << " to v" << ProxyDirectXVersion);
		}
		else
		{
			LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ") v" << DirectXVersion);
		}
		if (Config.Dd7to9)
		{
			Logging::Log() << __FUNCTION__ << " (" << this << ") Warning: created from non-dd7to9 interface!";
		}

		InitInterface(DirectXVersion);
	}
	m_IDirect3DVertexBufferX(m_IDirectDrawX* lpDdraw, m_IDirect3DX* D3D, LPD3DVERTEXBUFFERDESC lpVBDesc, DWORD DirectXVersion) : ddrawParent(lpDdraw), D3DInterface(D3D)
	{
		ProxyDirectXVersion = 9;

		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ")" << " converting interface from v" << DirectXVersion << " to v" << ProxyDirectXVersion);

		if (lpVBDesc)
		{
			VB.Desc.dwSize = sizeof(D3DVERTEXBUFFERDESC);
			VB.Desc.dwCaps = lpVBDesc->dwCaps;
			VB.Desc.dwFVF = lpVBDesc->dwFVF;
			VB.Desc.dwNumVertices = lpVBDesc->dwNumVertices;
		}
		d3d9VBDesc.Type = D3DRTYPE_VERTEXBUFFER;

		InitInterface(DirectXVersion);
	}
	~m_IDirect3DVertexBufferX()
	{
		LOG_LIMIT(3, __FUNCTION__ << " (" << this << ")" << " deleting interface!");

		ReleaseInterface();
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
	void ClearDdraw() { ddrawParent = nullptr; d3d9Device = nullptr; }

	// Direct3D9 interfaces
	const LPDIRECT3DVERTEXBUFFER9 GetCurrentD9VertexBuffer() const { return d3d9VertexBuffer; };
	void ClearD3D() { D3DInterface = nullptr; }
	void ReleaseD9Buffer(bool BackupData, bool ResetBuffer);

	DWORD GetFVF9() const { return d3d9VBDesc.FVF; };

	// Static functions
	static HRESULT InterleaveStridedVertexData(std::vector<BYTE, aligned_allocator<BYTE, 4>>& outputBuffer, const D3DDRAWPRIMITIVESTRIDEDDATA* sd, const DWORD dwVertexStart, const DWORD dwNumVertices, const DWORD dwVertexTypeDesc);
	static HRESULT ProcessVerticesUP(DWORD dwVertexOp, LPVOID lpDestBuffer, DWORD dwDestVertexTypeDesc, DWORD dwDestIndex, DWORD dwCount, LPVOID lpSrcBuffer, DWORD dwSrcVertexTypeDesc, DWORD dwSrcIndex, D3DRECT& drExtent, LPDIRECT3DDEVICE7 lpD3DDevice, DWORD dwFlags);
	template <typename T>
	static HRESULT TransformVertexUP(m_IDirect3DDeviceX* pDirect3DDeviceX, T* srcVertex, D3DTLVERTEX* destVertex, D3DHVERTEX* pHOut, const DWORD dwCount, D3DRECT& drExtent);
	static void ComputeLighting(const D3DVECTOR& Position, const D3DVECTOR& Normal, const std::vector<DXLIGHT7>& lights, const LightingState* s, D3DCOLOR& inoutColor, D3DCOLOR& inoutSpecular);
};
