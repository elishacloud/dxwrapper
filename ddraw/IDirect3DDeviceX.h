#pragma once

#include <unordered_map>
#include <unordered_set>
#include "External\DirectXMath\Inc\DirectXMath.h"

struct CONVERTHOMOGENEOUS
{
	bool IsTransformViewSet = false;					// Remembers if game sets the view matrix
	D3DMATRIX ToWorld_ProjectionMatrix;					// Store the projection matrix used to transform the geometry on the gpu
	D3DMATRIX ToWorld_ViewMatrix;						// Store the view matrix used to transform the geometry on the gpu
	D3DMATRIX ToWorld_ViewMatrixOriginal;				// Store the original view matrix, so we can restore it
	DirectX::XMMATRIX ToWorld_ViewMatrixInverse;		// Store the inverse view matrix to transform the geometry on the cpu
	std::vector<uint8_t> ToWorld_IntermediateGeometry;	// Intermediate buffer for the geometry conversion
	float ToWorld_GameCameraYaw = 0.0f;
	float ToWorld_GameCameraPitch = 0.0f;
};

class m_IDirect3DDeviceX : public IUnknown, public AddressLookupTableDdrawObject
{
private:
	IDirect3DDevice7 *ProxyInterface = nullptr;
	DWORD ProxyDirectXVersion;
	ULONG RefCount1 = 0;
	ULONG RefCount2 = 0;
	ULONG RefCount3 = 0;
	ULONG RefCount7 = 0;
	REFCLSID ClassID = IID_IUnknown;

	// Store d3d device version wrappers
	m_IDirect3DDevice* WrapperInterface = nullptr;
	m_IDirect3DDevice2* WrapperInterface2 = nullptr;
	m_IDirect3DDevice3* WrapperInterface3 = nullptr;
	m_IDirect3DDevice7* WrapperInterface7 = nullptr;

	// Convert Device
	m_IDirectDrawX *ddrawParent = nullptr;
	m_IDirect3DX* D3DInterface = nullptr;
	m_IDirectDrawSurfaceX* lpCurrentRenderTargetX = nullptr;
	LPDIRECT3DDEVICE9 *d3d9Device = nullptr;
	LPDIRECT3DPIXELSHADER9* colorkeyPixelShader = nullptr;
	LPDIRECT3DVIEWPORT3 lpCurrentViewport = nullptr;
	m_IDirect3DViewportX* lpCurrentViewportX = nullptr;
	struct {
		m_IDirectDrawSurfaceX* Interface = nullptr;
		DWORD DxVersion = 0;
	} parent3DSurface;

#ifdef ENABLE_PROFILING
	std::chrono::steady_clock::time_point sceneTime;
#endif

	struct {
		struct {
			bool Set = false;
			DWORD State = 0;
		} RenderState[MaxDeviceStates], TextureState[MaxTextureStages][MaxDeviceStates], SamplerState[MaxTextureStages][MaxSamplerStates];
		struct {
			bool Set = false;
			D3DLIGHT9 Light = {};
		} Lights[MAX_LIGHTS];
		struct {
			bool Set = false;
			BOOL Enable = FALSE;
		} LightEnabled[MAX_LIGHTS];
		struct {
			bool Set = false;
			D3DVIEWPORT9 View = {};
		} Viewport;
		struct {
			bool Set = false;
			D3DMATERIAL9 Material = {};
		} Material = {};
		std::unordered_map<D3DTRANSFORMSTATETYPE, D3DMATRIX> Matrix;
	} DeviceStates;

	inline HRESULT SetD9RenderState(D3DRENDERSTATETYPE dwRenderStateType, DWORD dwRenderState);
	inline HRESULT SetD9TextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value);
	inline HRESULT SetD9SamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value);
	inline HRESULT SetD9Light(DWORD Index, CONST D3DLIGHT9* pLight);
	inline HRESULT LightD9Enable(DWORD Index, BOOL bEnable);
	inline HRESULT SetD9Viewport(CONST D3DVIEWPORT9* pViewport);
	inline HRESULT SetD9Material(CONST D3DMATERIAL9* pMaterial);
	inline HRESULT SetD9Transform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix);

	struct {
		DWORD rsClipping = 0;
		DWORD rsLighting = 0;
		DWORD rsExtents = 0;
		DWORD rsAlphaTestEnable = 0;
		DWORD rsAlphaFunc = 0;
		DWORD rsAlphaRef = 0;
		DWORD ssMinFilter[MaxTextureStages] = {};
		DWORD ssMagFilter[MaxTextureStages] = {};
		float lowColorKey[4] = {};
		float highColorKey[4] = {};
	} DrawStates;

	bool bSetDefaults = true;

	bool IsInScene = false;

	// Last clip status
	D3DCLIPSTATUS D3DClipStatus;

	// Light states
	DWORD lsMaterialHandle;

	// Render states
	bool rsAntiAliasChanged;
	DWORD rsAntiAlias;
	DWORD rsTexturePerspective;
	DWORD rsEdgeAntiAlias;
	bool rsTextureWrappingChanged;
	DWORD rsTextureWrappingU;
	DWORD rsTextureWrappingV;
	DWORD rsTextureHandle;
	DWORD rsTextureMin;
	DWORD rsTextureMapBlend;
	DWORD rsAlphaBlendEnabled;
	DWORD rsSrcBlend;
	DWORD rsDestBlend;
	DWORD rsColorKeyEnabled;
	DWORD ssMipFilter[MaxTextureStages] = {};

	// Handle state blocks
	bool IsRecordingState = false;
	std::unordered_set<DWORD> StateBlockTokens;

	// Default settings
	D3DVIEWPORT9 DefaultViewport = {};

	// SetTexture array
	LPDIRECTDRAWSURFACE7 CurrentRenderTarget = nullptr;
	m_IDirectDrawSurfaceX* CurrentTextureSurfaceX[MaxTextureStages] = {};
	LPDIRECTDRAWSURFACE7 AttachedTexture[MaxTextureStages] = {};

	// Texture handle map
	std::unordered_map<D3DTEXTUREHANDLE, m_IDirect3DTextureX*> TextureHandleMap;
	inline m_IDirect3DTextureX* GetTexture(D3DTEXTUREHANDLE TextureHandle)
	{
		m_IDirect3DTextureX* pTextureX = TextureHandleMap[TextureHandle];
		if (!pTextureX)
		{
			TextureHandleMap.erase(TextureHandle);
		}
		return pTextureX;
	}

	// Material handle map
	std::unordered_map<D3DMATERIALHANDLE, m_IDirect3DMaterialX*> MaterialHandleMap;
	inline m_IDirect3DMaterialX* GetMaterial(D3DMATERIALHANDLE MaterialHandle)
	{
		m_IDirect3DMaterialX* pMaterialX = MaterialHandleMap[MaterialHandle];
		if (!pMaterialX)
		{
			MaterialHandleMap.erase(MaterialHandle);
		}
		return pMaterialX;
	}

	// Matrix map
	struct D3DMATRIXSTRUCT {
		bool IsValidMatrix = false;
		D3DMATRIX m = {};
	};
	std::unordered_map<D3DMATRIXHANDLE, D3DMATRIXSTRUCT> MatrixMap;
	inline D3DMATRIX* GetMatrix(D3DMATRIXHANDLE MatrixHandle)
	{
		if (!MatrixMap[MatrixHandle].IsValidMatrix)
		{
			MatrixMap.erase(MatrixHandle);
			return nullptr;
		}
		return &MatrixMap[MatrixHandle].m;
	}

	// Light index map
	std::unordered_map<DWORD, m_IDirect3DLight*> LightIndexMap;

	// ExecuteBuffer array
	std::vector<m_IDirect3DExecuteBuffer*> ExecuteBufferList;

	// Vector temporary buffer cache
	std::vector<BYTE> VertexCache;

	// The data used for rendering Homogeneous
	CONVERTHOMOGENEOUS ConvertHomogeneous;

	// Viewport array
	std::vector<LPDIRECT3DVIEWPORT3> AttachedViewports;

	inline bool IsViewportAttached(LPDIRECT3DVIEWPORT3 ViewportX)
	{
		auto it = std::find_if(AttachedViewports.begin(), AttachedViewports.end(),
			[=](auto pViewport) -> bool { return pViewport == ViewportX; });

		if (it != std::end(AttachedViewports))
		{
			return true;
		}
		return false;
	}

	inline bool DeleteAttachedViewport(LPDIRECT3DVIEWPORT3 ViewportX)
	{
		auto it = std::find_if(AttachedViewports.begin(), AttachedViewports.end(),
			[=](auto pViewport) -> bool { return pViewport == ViewportX; });

		if (it != std::end(AttachedViewports))
		{
			AttachedViewports.erase(it);
			return true;
		}
		return false;
	}

	// Wrapper interface functions
	inline REFIID GetWrapperType(DWORD DirectXVersion)
	{
		return (DirectXVersion == 1) ? IID_IDirect3DDevice :
			(DirectXVersion == 2) ? IID_IDirect3DDevice2 :
			(DirectXVersion == 3) ? IID_IDirect3DDevice3 :
			(DirectXVersion == 7) ? IID_IDirect3DDevice7 : IID_IUnknown;
	}
	inline bool CheckWrapperType(REFIID IID)
	{
		return (IID == IID_IDirect3DDevice ||
			IID == IID_IDirect3DDevice2 ||
			IID == IID_IDirect3DDevice3 ||
			IID == IID_IDirect3DDevice7) ? true : false;
	}
	inline IDirect3DDevice *GetProxyInterfaceV1() { return (IDirect3DDevice *)ProxyInterface; }
	inline IDirect3DDevice2 *GetProxyInterfaceV2() { return (IDirect3DDevice2 *)ProxyInterface; }
	inline IDirect3DDevice3 *GetProxyInterfaceV3() { return (IDirect3DDevice3 *)ProxyInterface; }
	inline IDirect3DDevice7 *GetProxyInterfaceV7() { return ProxyInterface; }

	// Check interfaces
	HRESULT CheckInterface(char *FunctionName, bool CheckD3DDevice);

	// Execute buffer function
	inline void CopyConvertExecuteVertex(BYTE*& DestVertex, DWORD& DestVertexCount, BYTE* SrcVertex, DWORD SrcIndex, DWORD VertexTypeDesc);
	HRESULT DrawExecutePoint(D3DPOINT* point, WORD pointCount, DWORD vertexIndexCount, BYTE* vertexBuffer, DWORD VertexTypeDesc);
	HRESULT DrawExecuteLine(D3DLINE* line, WORD lineCount, DWORD vertexIndexCount, BYTE* vertexBuffer, DWORD VertexTypeDesc);
	HRESULT DrawExecuteTriangle(D3DTRIANGLE* triangle, WORD triangleCount, DWORD vertexIndexCount, BYTE* vertexBuffer, DWORD VertexTypeDesc);

	// Helper functions
	HRESULT RestoreStates();
	void SetDefaults();
	void SetDrawStates(DWORD dwVertexTypeDesc, DWORD& dwFlags, DWORD DirectXVersion);
	void RestoreDrawStates(DWORD dwVertexTypeDesc, DWORD dwFlags, DWORD DirectXVersion);
	void ScaleVertices(DWORD dwVertexTypeDesc, LPVOID& lpVertices, DWORD dwVertexCount);
	void UpdateVertices(DWORD& dwVertexTypeDesc, LPVOID& lpVertices, DWORD dwVertexCount);

	// Interface initialization functions
	void InitInterface(DWORD DirectXVersion);
	void ReleaseInterface();

public:
	m_IDirect3DDeviceX(IDirect3DDevice7 *aOriginal, DWORD DirectXVersion) : ProxyInterface(aOriginal), ClassID(IID_IDirect3DHALDevice)
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
		if (Config.Dd7to9)
		{
			Logging::Log() << __FUNCTION__ << " (" << this << ") Warning: created from non-dd7to9 interface!";
		}

		InitInterface(DirectXVersion);
	}
	m_IDirect3DDeviceX(m_IDirectDrawX* lpDdraw, m_IDirect3DX* lpD3D, LPDIRECTDRAWSURFACE7 pRenderTarget, REFCLSID rclsid, DWORD DirectXVersion) :
		ddrawParent(lpDdraw), D3DInterface(lpD3D), CurrentRenderTarget(pRenderTarget), ClassID(rclsid)
	{
		ProxyDirectXVersion = 9;

		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ")" << " converting interface from v" << DirectXVersion << " to v" << ProxyDirectXVersion);

		InitInterface(DirectXVersion);
	}
	~m_IDirect3DDeviceX()
	{
		LOG_LIMIT(3, __FUNCTION__ << " (" << this << ")" << " deleting interface!");

		ReleaseInterface();
	}

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * ppvObj) { return QueryInterface(riid, ppvObj, 0); }
	STDMETHOD_(ULONG, AddRef) (THIS) { return AddRef(0); }
	STDMETHOD_(ULONG, Release) (THIS) { return Release(0); }

	/*** IDirect3DDevice methods ***/
	STDMETHOD(Initialize)(THIS_ LPDIRECT3D, LPGUID, LPD3DDEVICEDESC);
	STDMETHOD(CreateExecuteBuffer)(THIS_ LPD3DEXECUTEBUFFERDESC, LPDIRECT3DEXECUTEBUFFER*, IUnknown*);
	STDMETHOD(Execute)(THIS_ LPDIRECT3DEXECUTEBUFFER, LPDIRECT3DVIEWPORT, DWORD);
	STDMETHOD(Pick)(THIS_ LPDIRECT3DEXECUTEBUFFER, LPDIRECT3DVIEWPORT, DWORD, LPD3DRECT);
	STDMETHOD(GetPickRecords)(THIS_ LPDWORD, LPD3DPICKRECORD);
	STDMETHOD(CreateMatrix)(THIS_ LPD3DMATRIXHANDLE);
	STDMETHOD(SetMatrix)(THIS_ D3DMATRIXHANDLE, const LPD3DMATRIX);
	STDMETHOD(GetMatrix)(THIS_ D3DMATRIXHANDLE, LPD3DMATRIX);
	STDMETHOD(DeleteMatrix)(THIS_ D3DMATRIXHANDLE);
	STDMETHOD(SetTransform)(THIS_ D3DTRANSFORMSTATETYPE, LPD3DMATRIX);
	STDMETHOD(GetTransform)(THIS_ D3DTRANSFORMSTATETYPE, LPD3DMATRIX);
	STDMETHOD(MultiplyTransform)(THIS_ D3DTRANSFORMSTATETYPE, LPD3DMATRIX);
	STDMETHOD(PreLoad)(THIS_ LPDIRECTDRAWSURFACE7);
	STDMETHOD(Load)(THIS_ LPDIRECTDRAWSURFACE7, LPPOINT, LPDIRECTDRAWSURFACE7, LPRECT, DWORD);
	STDMETHOD(SwapTextureHandles)(THIS_ LPDIRECT3DTEXTURE2, LPDIRECT3DTEXTURE2);
	STDMETHOD(EnumTextureFormats)(THIS_ LPD3DENUMTEXTUREFORMATSCALLBACK, LPVOID);
	STDMETHOD(EnumTextureFormats)(THIS_ LPD3DENUMPIXELFORMATSCALLBACK, LPVOID);
	STDMETHOD(GetTexture)(THIS_ DWORD, LPDIRECT3DTEXTURE2 *);
	STDMETHOD(GetTexture)(THIS_ DWORD, LPDIRECTDRAWSURFACE7 *);
	STDMETHOD(SetTexture)(THIS_ DWORD, LPDIRECT3DTEXTURE2);
	STDMETHOD(SetTexture)(THIS_ DWORD, LPDIRECTDRAWSURFACE7);
	STDMETHOD(SetRenderTarget)(THIS_ LPDIRECTDRAWSURFACE7, DWORD);
	STDMETHOD(GetRenderTarget)(THIS_ LPDIRECTDRAWSURFACE7 *, DWORD);
	STDMETHOD(GetTextureStageState)(THIS_ DWORD, D3DTEXTURESTAGESTATETYPE, LPDWORD);
	STDMETHOD(SetTextureStageState)(THIS_ DWORD, D3DTEXTURESTAGESTATETYPE, DWORD);
	STDMETHOD(GetCaps)(THIS_ LPD3DDEVICEDESC, LPD3DDEVICEDESC);
	STDMETHOD(GetCaps)(THIS_ LPD3DDEVICEDESC7);
	STDMETHOD(GetStats)(THIS_ LPD3DSTATS, DWORD);
	STDMETHOD(AddViewport)(THIS_ LPDIRECT3DVIEWPORT3);
	STDMETHOD(DeleteViewport)(THIS_ LPDIRECT3DVIEWPORT3);
	STDMETHOD(NextViewport)(THIS_ LPDIRECT3DVIEWPORT3, LPDIRECT3DVIEWPORT3*, DWORD, DWORD);
	STDMETHOD(SetCurrentViewport)(THIS_ LPDIRECT3DVIEWPORT3);
	STDMETHOD(GetCurrentViewport)(THIS_ LPDIRECT3DVIEWPORT3 *, DWORD);
	STDMETHOD(SetViewport)(THIS_ LPD3DVIEWPORT7);
	STDMETHOD(GetViewport)(THIS_ LPD3DVIEWPORT7);
	STDMETHOD(Begin)(THIS_ D3DPRIMITIVETYPE, DWORD, DWORD);
	STDMETHOD(BeginIndexed)(THIS_ D3DPRIMITIVETYPE, DWORD, LPVOID, DWORD, DWORD);
	STDMETHOD(Vertex)(THIS_ LPVOID);
	STDMETHOD(Index)(THIS_ WORD);
	STDMETHOD(End)(THIS_ DWORD);
	STDMETHOD(BeginScene)(THIS);
	STDMETHOD(EndScene)(THIS);
	STDMETHOD(Clear)(THIS_ DWORD, LPD3DRECT, DWORD, D3DCOLOR, D3DVALUE, DWORD);
	STDMETHOD(GetDirect3D)(THIS_ LPDIRECT3D7*, DWORD);
	STDMETHOD(GetLightState)(THIS_ D3DLIGHTSTATETYPE, LPDWORD);
	STDMETHOD(SetLightState)(THIS_ D3DLIGHTSTATETYPE, DWORD);
	STDMETHOD(SetLight)(THIS_ m_IDirect3DLight*, LPD3DLIGHT);
	STDMETHOD(SetLight)(THIS_ DWORD, LPD3DLIGHT7);
	STDMETHOD(GetLight)(THIS_ DWORD, LPD3DLIGHT7);
	STDMETHOD(LightEnable)(THIS_ DWORD, BOOL);
	STDMETHOD(GetLightEnable)(THIS_ m_IDirect3DLight*, BOOL*);
	STDMETHOD(GetLightEnable)(THIS_ DWORD, BOOL*);
	STDMETHOD(SetMaterial)(THIS_ LPD3DMATERIAL);
	STDMETHOD(SetMaterial)(THIS_ LPD3DMATERIAL7);
	STDMETHOD(GetMaterial)(THIS_ LPD3DMATERIAL7);
	STDMETHOD(SetRenderState)(THIS_ D3DRENDERSTATETYPE, DWORD);
	STDMETHOD(GetRenderState)(THIS_ D3DRENDERSTATETYPE, LPDWORD);
	STDMETHOD(BeginStateBlock)(THIS);
	STDMETHOD(EndStateBlock)(THIS_ LPDWORD);
	STDMETHOD(DrawPrimitive)(THIS_ D3DPRIMITIVETYPE, DWORD, LPVOID, DWORD, DWORD, DWORD);
	STDMETHOD(DrawPrimitiveStrided)(THIS_ D3DPRIMITIVETYPE, DWORD, LPD3DDRAWPRIMITIVESTRIDEDDATA, DWORD, DWORD, DWORD);
	STDMETHOD(DrawPrimitiveVB)(THIS_ D3DPRIMITIVETYPE, LPDIRECT3DVERTEXBUFFER7, DWORD, DWORD, DWORD, DWORD);
	STDMETHOD(DrawIndexedPrimitive)(THIS_ D3DPRIMITIVETYPE, DWORD, LPVOID, DWORD, LPWORD, DWORD, DWORD, DWORD);
	STDMETHOD(DrawIndexedPrimitiveStrided)(THIS_ D3DPRIMITIVETYPE, DWORD, LPD3DDRAWPRIMITIVESTRIDEDDATA, DWORD, LPWORD, DWORD, DWORD, DWORD);
	STDMETHOD(DrawIndexedPrimitiveVB)(THIS_ D3DPRIMITIVETYPE, LPDIRECT3DVERTEXBUFFER7, DWORD, DWORD, LPWORD, DWORD, DWORD, DWORD);
	STDMETHOD(ComputeSphereVisibility)(THIS_ LPD3DVECTOR, LPD3DVALUE, DWORD, DWORD, LPDWORD);
	STDMETHOD(ValidateDevice)(THIS_ LPDWORD);
	STDMETHOD(ApplyStateBlock)(THIS_ DWORD);
	STDMETHOD(CaptureStateBlock)(THIS_ DWORD);
	STDMETHOD(DeleteStateBlock)(THIS_ DWORD);
	STDMETHOD(CreateStateBlock)(THIS_ D3DSTATEBLOCKTYPE, LPDWORD);
	STDMETHOD(SetClipStatus)(THIS_ LPD3DCLIPSTATUS);
	STDMETHOD(GetClipStatus)(THIS_ LPD3DCLIPSTATUS);
	STDMETHOD(SetClipPlane)(THIS_ DWORD, D3DVALUE*);
	STDMETHOD(GetClipPlane)(THIS_ DWORD, D3DVALUE*);
	STDMETHOD(GetInfo)(THIS_ DWORD, LPVOID, DWORD);

	// Helper functions
	HRESULT QueryInterface(REFIID riid, LPVOID FAR * ppvObj, DWORD DirectXVersion);
	void *GetWrapperInterfaceX(DWORD DirectXVersion);
	ULONG AddRef(DWORD DirectXVersion);
	ULONG Release(DWORD DirectXVersion);
	bool IsDeviceInScene() const { return IsInScene; }
	inline void SetParent3DSurface(m_IDirectDrawSurfaceX* lpSurfaceX, DWORD DxVersion) { parent3DSurface = { lpSurfaceX, DxVersion }; }

	// ExecuteBuffer
	void AddExecuteBuffer(m_IDirect3DExecuteBuffer* lpExecuteBuffer);
	void ClearExecuteBuffer(m_IDirect3DExecuteBuffer* lpExecuteBuffer);

	// Viewport functions
	inline void GetDefaultViewport(D3DVIEWPORT9& Viewport) const { Viewport = DefaultViewport; }
	inline bool CheckIfViewportSet(m_IDirect3DViewportX* pViewport) { return (pViewport == lpCurrentViewportX); }
	void ClearViewport(m_IDirect3DViewportX* lpViewportX);

	// Texture handle function
	void ClearTextureHandle(D3DTEXTUREHANDLE tHandle);
	HRESULT SetTextureHandle(D3DTEXTUREHANDLE& tHandle, m_IDirect3DTextureX* pTextureX);

	// Material handle function
	void ClearMaterialHandle(D3DMATERIALHANDLE mHandle);
	HRESULT SetMaterialHandle(D3DMATERIALHANDLE& mHandle, m_IDirect3DMaterialX* lpMaterial);
	inline bool CheckIfMaterialSet(D3DMATERIALHANDLE mHandle) const { return (mHandle == lsMaterialHandle); }

	// Light index function
	void ClearLight(m_IDirect3DLight* lpLight);

	// Functions handling the Direct3D parent interface
	void SetD3D(m_IDirect3DX* lpD3D);
	void ClearD3D(m_IDirect3DX* lpD3D);

	// Functions handling the ddraw parent interface
	void ClearSurface(m_IDirectDrawSurfaceX* lpSurfaceX);
	void SetDdrawParent(m_IDirectDrawX* ddraw);
	void ClearDdraw();
	void BeforeResetDevice();
	void AfterResetDevice();
	void ReleaseAllStateBlocks();
};
