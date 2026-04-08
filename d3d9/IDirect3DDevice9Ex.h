#pragma once

static constexpr size_t MAX_CLIP_PLANES = 6;
static constexpr size_t MAX_TEXTURE_STAGES = 8;
const std::chrono::seconds FPS_CALCULATION_WINDOW(1);	// Define a constant for the desired duration of FPS calculation

struct DEVICEDETAILS
{
	// Device window handle and size
	DWORD ClientDirectXVersion = 0;
	bool IsWindowMode = false;
	bool AppRequestedWindowMode = false;
	bool IsDirectDrawDevice = false;
	UINT Adapter = D3DADAPTER_DEFAULT;
	D3DDEVTYPE DeviceType = D3DDEVTYPE_HAL;
	DWORD BackBufferCount = 0;
	HMONITOR hMonitor = nullptr;
	HWND DeviceWindow = nullptr;
	LONG BufferWidth = 0, BufferHeight = 0;
	LONG screenWidth = 0, screenHeight = 0;

	// For AntiAliasing
	bool DeviceMultiSampleFlag = false;
	bool SetMultiSampleState = false;
	bool UseAppMultiSampleState = false;
	bool SetSSAA = false;
	bool SetATOC = false;
	D3DMULTISAMPLE_TYPE DeviceMultiSampleType = D3DMULTISAMPLE_NONE;
	DWORD DeviceMultiSampleQuality = 0;
};

extern std::unordered_map<UINT, std::unique_ptr<DEVICEDETAILS>> DeviceDetailsMap;

#include "IDirect3D9Ex.h"

class m_IDirect3DDevice9Ex final : public IDirect3DDevice9Ex
{
private:
	LPDIRECT3DDEVICE9 ProxyInterface;
	LPDIRECT3DDEVICE9EX ProxyInterfaceEx = nullptr;
	m_IDirect3D9Ex* m_pD3DEx;
	const IID WrapperID;
	std::unique_ptr<ShadowSurfaceStorage> ShadowBackbuffer = std::make_unique<ShadowSurfaceStorage>();
	std::vector<IDirect3DSurface9*> BackBufferList;

	LONG RefCount = 1;

	inline LONG InterlockedDecrementIfPositive(LONG* value)
	{
		while (true)
		{
			LONG current = *value;

			if (current <= 0)
			{
				return 0;
			}

			if (_InterlockedCompareExchange(value, current - 1, current) == current)
			{
				return current - 1;
			}
		}
	}

	AddressLookupTableD3d9 ProxyAddressLookupTable9;

	DEVICEDETAILS DeviceDetails;

	CRITICAL_SECTION d9cs = {};

	D3DCAPS9 Caps = {};
	bool AnisotropyMin = false;
	bool AnisotropyMag = false;

	// Begin/End Scene
	bool IsInScene = false;
	bool BeginSceneCalled = false;

	std::unordered_set<m_IDirect3DSurface9*> EmulatedSurfaceList;

	StateBlockCache StateBlockTable;
	StateBlockCache DeletedStateBlocks;

	// FPS display
	LPD3DXFONT pFont = nullptr;
	DWORD FontRefCount = 0;
	LPD3DXSPRITE pSprite = nullptr;
	DWORD SprintRefCount = 0;

	// Frame counter
	double AverageFPSCounter = 0.0;
	std::deque<std::pair<std::chrono::steady_clock::time_point, std::chrono::duration<double>>> frameTimes;	// Store frame times in a deque
	std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();	// Store start time for PFS counter

	// Limit frame rate
	struct {
		DWORD FrameCounter = 0;
		LARGE_INTEGER LastPresentTime = {};
	} Counter;

	// State block
	IDirect3DStateBlock9* pStateBlock = nullptr;

	// For environment map cube
	bool isTextureCubeMap[MAX_TEXTURE_STAGES] = {};
	bool isTransformCubeMap[MAX_TEXTURE_STAGES] = {};
	DWORD texCoordIndex[MAX_TEXTURE_STAGES] = {};
	DWORD texTransformFlags[MAX_TEXTURE_STAGES] = {};
	bool isBlankTextureUsed = false;
	IDirect3DBaseTexture9* pCurrentTexture = nullptr;
	IDirect3DCubeTexture9* BlankTexture = nullptr;

	// For CacheClipPlane
	bool isClipPlaneSet = false;
	DWORD ClipPlaneRenderState = 0;
	float StoredClipPlanes[MAX_CLIP_PLANES][4] = {};

	// For gamma
	bool IsGammaSet = false;
	bool UsingShader32f = true;
	D3DGAMMARAMP RampData = {};
	D3DGAMMARAMP DefaultRampData = {};
	LPDIRECT3DTEXTURE9 GammaLUTTexture = nullptr;
	LPDIRECT3DTEXTURE9 ScreenCopyTexture = nullptr;
	LPDIRECT3DPIXELSHADER9 gammaPixelShader = nullptr;

	// Anisotropic Filtering
	DWORD MaxAnisotropy = 0;

	// Antialiasing
	struct {
		bool MultiSampleMismatch = false;
		bool RenderTargetNonMultiSampled = false;
		bool DepthStencilNonMultiSampled = false;
		bool NullDepthStencil = false;
		m_IDirect3DSurface9* RenderTarget = nullptr;
	} msaa;

	void ApplyPreDrawFixes();
	void ApplyPostDrawFixes();
	void ApplyPrePresentFixes();
	void ApplyPostPresentFixes();

	void ApplyClipPlanes();

	void AfterBeginScene();
	void BeforeEndScene();

	inline bool RequirePresentHandling() const;
	inline bool UsingShadowBackBuffer(DWORD iSwapChain = 0) const;

	// Limit frame rate
	void LimitFrameRate();

	// Frame counter
	void CalculateFPS();
	void DrawFPS(float fps, const RECT& presentRect, DWORD position);

	// Gamma
	HRESULT SetBrightnessLevel(D3DGAMMARAMP& Ramp);
	LPDIRECT3DPIXELSHADER9 GetGammaPixelShader();
	void ApplyBrightnessLevel();
	DWORD GetResourceRefCount();
	void ReleaseResources(bool isReset);

	// For environment map cube
	void CheckTransformForCubeMap(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix);
	bool CheckTextureStageForCubeMap() const;
	void SetEnvironmentCubeMapTexture();

	// For Reset & ResetEx
	void ReInitInterface();
	void CreateShadowBackbuffer();
	void ReleaseShadowBackbuffer();
	void ClearVars();
	typedef HRESULT(WINAPI* fReset)(D3DPRESENT_PARAMETERS* pPresentationParameters);
	typedef HRESULT(WINAPI* fResetEx)(D3DPRESENT_PARAMETERS* pPresentationParameters, D3DDISPLAYMODEEX* pFullscreenDisplayMode);
	template <typename T>
	HRESULT ResetT(T, D3DPRESENT_PARAMETERS* pPresentationParameters, bool IsEx, D3DDISPLAYMODEEX* pFullscreenDisplayMode);
	inline HRESULT ResetT(fReset, D3DPRESENT_PARAMETERS* pPresentationParameters, D3DDISPLAYMODEEX*)
	{ return ProxyInterface->Reset(pPresentationParameters); }
	inline HRESULT ResetT(fResetEx, D3DPRESENT_PARAMETERS* pPresentationParameters, D3DDISPLAYMODEEX* pFullscreenDisplayMode)
	{ return (ProxyInterfaceEx) ? ProxyInterfaceEx->ResetEx(pPresentationParameters, pFullscreenDisplayMode) : D3DERR_INVALIDCALL; }

	// Information
	inline bool IsForcingD3d9to9Ex() const { return (Config.D3d9to9Ex && ProxyInterface == ProxyInterfaceEx); }

public:
	m_IDirect3DDevice9Ex(LPDIRECT3DDEVICE9EX pDevice, m_IDirect3D9Ex* pD3D, REFIID DeviceID, DEVICEDETAILS Details) : ProxyInterface(pDevice), m_pD3DEx(pD3D), WrapperID(DeviceID), DeviceDetails(Details)
	{
		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ") " << WrapperID << " game interface v" << DeviceDetails.ClientDirectXVersion);

		if (WrapperID == IID_IDirect3DDevice9Ex)
		{
			ProxyInterfaceEx = pDevice;
		}

		D3d9Wrapper::TestAllDeviceRefs(ProxyInterface);

		// Check for SSAA
		if (DeviceDetails.DeviceMultiSampleType && m_pD3DEx)
		{
			if (m_pD3DEx->CheckDeviceFormat(DeviceDetails.Adapter, DeviceDetails.DeviceType, D3DFMT_X8R8G8B8, 0, D3DRTYPE_SURFACE, (D3DFORMAT)MAKEFOURCC('S', 'S', 'A', 'A')) == S_OK)
			{
				DeviceDetails.SetSSAA = true;
			}
			if (Config.EnableMultisamplingATOC &&
				m_pD3DEx->CheckDeviceFormat(DeviceDetails.Adapter, DeviceDetails.DeviceType, D3DFMT_X8R8G8B8, 0, D3DRTYPE_SURFACE, (D3DFORMAT)MAKEFOURCC('A', 'T', 'O', 'C')) == S_OK)
			{
				DeviceDetails.SetATOC = true;
			}
		}

		if (!InitializeCriticalSectionAndSpinCount(&d9cs, 4000))
		{
			Logging::Log() << __FUNCTION__ << " Warning: failed to initialize CriticalSectionAndSpinCount for d9cs.  Failing over to CriticalSection!";
			InitializeCriticalSection(&d9cs);
		}

		ReInitInterface();
	}
	~m_IDirect3DDevice9Ex()
	{
		LOG_LIMIT(3, __FUNCTION__ << " (" << this << ")" << " deleting interface!");

		DeleteCriticalSection(&d9cs);
	}

	/*** IUnknown methods ***/
	IFACEMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj) override;
	IFACEMETHOD_(ULONG, AddRef)(THIS) override;
	IFACEMETHOD_(ULONG, Release)(THIS) override;

	/*** IDirect3DDevice9 methods ***/
	IFACEMETHOD(TestCooperativeLevel)(THIS) override;
	IFACEMETHOD_(UINT, GetAvailableTextureMem)(THIS) override;
	IFACEMETHOD(EvictManagedResources)(THIS) override;
	IFACEMETHOD(GetDirect3D)(THIS_ IDirect3D9** ppD3D9) override;
	IFACEMETHOD(GetDeviceCaps)(THIS_ D3DCAPS9* pCaps) override;
	IFACEMETHOD(GetDisplayMode)(THIS_ UINT iSwapChain, D3DDISPLAYMODE* pMode) override;
	IFACEMETHOD(GetCreationParameters)(THIS_ D3DDEVICE_CREATION_PARAMETERS *pParameters) override;
	IFACEMETHOD(SetCursorProperties)(THIS_ UINT XHotSpot, UINT YHotSpot, IDirect3DSurface9* pCursorBitmap) override;
	IFACEMETHOD_(void, SetCursorPosition)(THIS_ int X, int Y, DWORD Flags) override;
	IFACEMETHOD_(BOOL, ShowCursor)(THIS_ BOOL bShow) override;
	IFACEMETHOD(CreateAdditionalSwapChain)(THIS_ D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DSwapChain9** pSwapChain) override;
	IFACEMETHOD(GetSwapChain)(THIS_ UINT iSwapChain, IDirect3DSwapChain9** pSwapChain) override;
	IFACEMETHOD_(UINT, GetNumberOfSwapChains)(THIS) override;
	IFACEMETHOD(Reset)(THIS_ D3DPRESENT_PARAMETERS* pPresentationParameters) override;
	IFACEMETHOD(Present)(THIS_ CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion) override;
	IFACEMETHOD(GetBackBuffer)(THIS_ UINT iSwapChain, UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface9** ppBackBuffer) override;
	IFACEMETHOD(GetRasterStatus)(THIS_ UINT iSwapChain, D3DRASTER_STATUS* pRasterStatus) override;
	IFACEMETHOD(SetDialogBoxMode)(THIS_ BOOL bEnableDialogs) override;
	IFACEMETHOD_(void, SetGammaRamp)(THIS_ UINT iSwapChain, DWORD Flags, CONST D3DGAMMARAMP* pRamp) override;
	IFACEMETHOD_(void, GetGammaRamp)(THIS_ UINT iSwapChain, D3DGAMMARAMP* pRamp) override;
	IFACEMETHOD(CreateTexture)(THIS_ UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture9** ppTexture, HANDLE* pSharedHandle) override;
	IFACEMETHOD(CreateVolumeTexture)(THIS_ UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DVolumeTexture9** ppVolumeTexture, HANDLE* pSharedHandle) override;
	IFACEMETHOD(CreateCubeTexture)(THIS_ UINT EdgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DCubeTexture9** ppCubeTexture, HANDLE* pSharedHandle) override;
	IFACEMETHOD(CreateVertexBuffer)(THIS_ UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool, IDirect3DVertexBuffer9** ppVertexBuffer, HANDLE* pSharedHandle) override;
	IFACEMETHOD(CreateIndexBuffer)(THIS_ UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DIndexBuffer9** ppIndexBuffer, HANDLE* pSharedHandle) override;
	IFACEMETHOD(CreateRenderTarget)(THIS_ UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle) override;
	IFACEMETHOD(CreateDepthStencilSurface)(THIS_ UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Discard, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle) override;
	IFACEMETHOD(UpdateSurface)(THIS_ IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestSurface, CONST POINT* pDestPoint) override;
	IFACEMETHOD(UpdateTexture)(THIS_ IDirect3DBaseTexture9* pSourceTexture, IDirect3DBaseTexture9* pDestinationTexture) override;
	IFACEMETHOD(GetRenderTargetData)(THIS_ IDirect3DSurface9* pRenderTarget, IDirect3DSurface9* pDestSurface) override;
	IFACEMETHOD(GetFrontBufferData)(THIS_ UINT iSwapChain, IDirect3DSurface9* pDestSurface) override;
	IFACEMETHOD(StretchRect)(THIS_ IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestSurface, CONST RECT* pDestRect, D3DTEXTUREFILTERTYPE Filter) override;
	IFACEMETHOD(ColorFill)(THIS_ IDirect3DSurface9* pSurface, CONST RECT* pRect, D3DCOLOR color) override;
	IFACEMETHOD(CreateOffscreenPlainSurface)(THIS_ UINT Width, UINT Height, D3DFORMAT Format, D3DPOOL Pool, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle) override;
	IFACEMETHOD(SetRenderTarget)(THIS_ DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget) override;
	IFACEMETHOD(GetRenderTarget)(THIS_ DWORD RenderTargetIndex, IDirect3DSurface9** ppRenderTarget) override;
	IFACEMETHOD(SetDepthStencilSurface)(THIS_ IDirect3DSurface9* pNewZStencil) override;
	IFACEMETHOD(GetDepthStencilSurface)(THIS_ IDirect3DSurface9** ppZStencilSurface) override;
	IFACEMETHOD(BeginScene)(THIS) override;
	IFACEMETHOD(EndScene)(THIS) override;
	IFACEMETHOD(Clear)(THIS_ DWORD Count, CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil) override;
	IFACEMETHOD(SetTransform)(THIS_ D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix) override;
	IFACEMETHOD(GetTransform)(THIS_ D3DTRANSFORMSTATETYPE State, D3DMATRIX* pMatrix) override;
	IFACEMETHOD(MultiplyTransform)(THIS_ D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX *pMatrix) override;
	IFACEMETHOD(SetViewport)(THIS_ CONST D3DVIEWPORT9* pViewport) override;
	IFACEMETHOD(GetViewport)(THIS_ D3DVIEWPORT9* pViewport) override;
	IFACEMETHOD(SetMaterial)(THIS_ CONST D3DMATERIAL9* pMaterial) override;
	IFACEMETHOD(GetMaterial)(THIS_ D3DMATERIAL9* pMaterial) override;
	IFACEMETHOD(SetLight)(THIS_ DWORD Index, CONST D3DLIGHT9* pLight) override;
	IFACEMETHOD(GetLight)(THIS_ DWORD Index, D3DLIGHT9* pLight) override;
	IFACEMETHOD(LightEnable)(THIS_ DWORD Index, BOOL Enable) override;
	IFACEMETHOD(GetLightEnable)(THIS_ DWORD Index, BOOL* pEnable) override;
	IFACEMETHOD(SetClipPlane)(THIS_ DWORD Index, CONST float* pPlane) override;
	IFACEMETHOD(GetClipPlane)(THIS_ DWORD Index, float* pPlane) override;
	IFACEMETHOD(SetRenderState)(THIS_ D3DRENDERSTATETYPE State, DWORD Value) override;
	IFACEMETHOD(GetRenderState)(THIS_ D3DRENDERSTATETYPE State, DWORD* pValue) override;
	IFACEMETHOD(CreateStateBlock)(THIS_ D3DSTATEBLOCKTYPE Type, IDirect3DStateBlock9** ppSB) override;
	IFACEMETHOD(BeginStateBlock)(THIS) override;
	IFACEMETHOD(EndStateBlock)(THIS_ IDirect3DStateBlock9** ppSB) override;
	IFACEMETHOD(SetClipStatus)(THIS_ CONST D3DCLIPSTATUS9* pClipStatus) override;
	IFACEMETHOD(GetClipStatus)(THIS_ D3DCLIPSTATUS9* pClipStatus) override;
	IFACEMETHOD(GetTexture)(THIS_ DWORD Stage, IDirect3DBaseTexture9** ppTexture) override;
	IFACEMETHOD(SetTexture)(THIS_ DWORD Stage, IDirect3DBaseTexture9* pTexture) override;
	IFACEMETHOD(GetTextureStageState)(THIS_ DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD* pValue) override;
	IFACEMETHOD(SetTextureStageState)(THIS_ DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value) override;
	IFACEMETHOD(GetSamplerState)(THIS_ DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD* pValue) override;
	IFACEMETHOD(SetSamplerState)(THIS_ DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value) override;
	IFACEMETHOD(ValidateDevice)(THIS_ DWORD* pNumPasses) override;
	IFACEMETHOD(SetPaletteEntries)(THIS_ UINT PaletteNumber, CONST PALETTEENTRY* pEntries) override;
	IFACEMETHOD(GetPaletteEntries)(THIS_ UINT PaletteNumber, PALETTEENTRY* pEntries) override;
	IFACEMETHOD(SetCurrentTexturePalette)(THIS_ UINT PaletteNumber) override;
	IFACEMETHOD(GetCurrentTexturePalette)(THIS_ UINT *PaletteNumber) override;
	IFACEMETHOD(SetScissorRect)(THIS_ CONST RECT* pRect) override;
	IFACEMETHOD(GetScissorRect)(THIS_ RECT* pRect) override;
	IFACEMETHOD(SetSoftwareVertexProcessing)(THIS_ BOOL bSoftware) override;
	IFACEMETHOD_(BOOL, GetSoftwareVertexProcessing)(THIS) override;
	IFACEMETHOD(SetNPatchMode)(THIS_ float nSegments) override;
	IFACEMETHOD_(float, GetNPatchMode)(THIS) override;
	IFACEMETHOD(DrawPrimitive)(THIS_ D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount) override;
	IFACEMETHOD(DrawIndexedPrimitive)(THIS_ D3DPRIMITIVETYPE, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount) override;
	IFACEMETHOD(DrawPrimitiveUP)(THIS_ D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride) override;
	IFACEMETHOD(DrawIndexedPrimitiveUP)(THIS_ D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex, UINT NumVertices, UINT PrimitiveCount, CONST void* pIndexData, D3DFORMAT IndexDataFormat, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride) override;
	IFACEMETHOD(ProcessVertices)(THIS_ UINT SrcStartIndex, UINT DestIndex, UINT VertexCount, IDirect3DVertexBuffer9* pDestBuffer, IDirect3DVertexDeclaration9* pVertexDecl, DWORD Flags) override;
	IFACEMETHOD(CreateVertexDeclaration)(THIS_ CONST D3DVERTEXELEMENT9* pVertexElements, IDirect3DVertexDeclaration9** ppDecl) override;
	IFACEMETHOD(SetVertexDeclaration)(THIS_ IDirect3DVertexDeclaration9* pDecl) override;
	IFACEMETHOD(GetVertexDeclaration)(THIS_ IDirect3DVertexDeclaration9** ppDecl) override;
	IFACEMETHOD(SetFVF)(THIS_ DWORD FVF) override;
	IFACEMETHOD(GetFVF)(THIS_ DWORD* pFVF) override;
	IFACEMETHOD(CreateVertexShader)(THIS_ CONST DWORD* pFunction, IDirect3DVertexShader9** ppShader) override;
	IFACEMETHOD(SetVertexShader)(THIS_ IDirect3DVertexShader9* pShader) override;
	IFACEMETHOD(GetVertexShader)(THIS_ IDirect3DVertexShader9** ppShader) override;
	IFACEMETHOD(SetVertexShaderConstantF)(THIS_ UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount) override;
	IFACEMETHOD(GetVertexShaderConstantF)(THIS_ UINT StartRegister, float* pConstantData, UINT Vector4fCount) override;
	IFACEMETHOD(SetVertexShaderConstantI)(THIS_ UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount) override;
	IFACEMETHOD(GetVertexShaderConstantI)(THIS_ UINT StartRegister, int* pConstantData, UINT Vector4iCount) override;
	IFACEMETHOD(SetVertexShaderConstantB)(THIS_ UINT StartRegister, CONST BOOL* pConstantData, UINT  BoolCount) override;
	IFACEMETHOD(GetVertexShaderConstantB)(THIS_ UINT StartRegister, BOOL* pConstantData, UINT BoolCount) override;
	IFACEMETHOD(SetStreamSource)(THIS_ UINT StreamNumber, IDirect3DVertexBuffer9* pStreamData, UINT OffsetInBytes, UINT Stride) override;
	IFACEMETHOD(GetStreamSource)(THIS_ UINT StreamNumber, IDirect3DVertexBuffer9** ppStreamData, UINT* pOffsetInBytes, UINT* pStride) override;
	IFACEMETHOD(SetStreamSourceFreq)(THIS_ UINT StreamNumber, UINT Setting) override;
	IFACEMETHOD(GetStreamSourceFreq)(THIS_ UINT StreamNumber, UINT* pSetting) override;
	IFACEMETHOD(SetIndices)(THIS_ IDirect3DIndexBuffer9* pIndexData) override;
	IFACEMETHOD(GetIndices)(THIS_ IDirect3DIndexBuffer9** ppIndexData) override;
	IFACEMETHOD(CreatePixelShader)(THIS_ CONST DWORD* pFunction, IDirect3DPixelShader9** ppShader) override;
	IFACEMETHOD(SetPixelShader)(THIS_ IDirect3DPixelShader9* pShader) override;
	IFACEMETHOD(GetPixelShader)(THIS_ IDirect3DPixelShader9** ppShader) override;
	IFACEMETHOD(SetPixelShaderConstantF)(THIS_ UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount) override;
	IFACEMETHOD(GetPixelShaderConstantF)(THIS_ UINT StartRegister, float* pConstantData, UINT Vector4fCount) override;
	IFACEMETHOD(SetPixelShaderConstantI)(THIS_ UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount) override;
	IFACEMETHOD(GetPixelShaderConstantI)(THIS_ UINT StartRegister, int* pConstantData, UINT Vector4iCount) override;
	IFACEMETHOD(SetPixelShaderConstantB)(THIS_ UINT StartRegister, CONST BOOL* pConstantData, UINT  BoolCount) override;
	IFACEMETHOD(GetPixelShaderConstantB)(THIS_ UINT StartRegister, BOOL* pConstantData, UINT BoolCount) override;
	IFACEMETHOD(DrawRectPatch)(THIS_ UINT Handle, CONST float* pNumSegs, CONST D3DRECTPATCH_INFO* pRectPatchInfo) override;
	IFACEMETHOD(DrawTriPatch)(THIS_ UINT Handle, CONST float* pNumSegs, CONST D3DTRIPATCH_INFO* pTriPatchInfo) override;
	IFACEMETHOD(DeletePatch)(THIS_ UINT Handle) override;
	IFACEMETHOD(CreateQuery)(THIS_ D3DQUERYTYPE Type, IDirect3DQuery9** ppQuery) override;

	/*** IDirect3DDevice9Ex methods ***/
	IFACEMETHOD(SetConvolutionMonoKernel)(THIS_ UINT width, UINT height, float* rows, float* columns) override;
	IFACEMETHOD(ComposeRects)(THIS_ IDirect3DSurface9* pSrc, IDirect3DSurface9* pDst, IDirect3DVertexBuffer9* pSrcRectDescs, UINT NumRects, IDirect3DVertexBuffer9* pDstRectDescs, D3DCOMPOSERECTSOP Operation, int Xoffset, int Yoffset) override;
	IFACEMETHOD(PresentEx)(THIS_ CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion, DWORD dwFlags) override;
	IFACEMETHOD(GetGPUThreadPriority)(THIS_ INT* pPriority) override;
	IFACEMETHOD(SetGPUThreadPriority)(THIS_ INT Priority) override;
	IFACEMETHOD(WaitForVBlank)(THIS_ UINT iSwapChain) override;
	IFACEMETHOD(CheckResourceResidency)(THIS_ IDirect3DResource9** pResourceArray, UINT32 NumResources) override;
	IFACEMETHOD(SetMaximumFrameLatency)(THIS_ UINT MaxLatency) override;
	IFACEMETHOD(GetMaximumFrameLatency)(THIS_ UINT* pMaxLatency) override;
	IFACEMETHOD(CheckDeviceState)(THIS_ HWND hDestinationWindow) override;
	IFACEMETHOD(CreateRenderTargetEx)(THIS_ UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle, DWORD Usage) override;
	IFACEMETHOD(CreateOffscreenPlainSurfaceEx)(THIS_ UINT Width, UINT Height, D3DFORMAT Format, D3DPOOL Pool, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle, DWORD Usage) override;
	IFACEMETHOD(CreateDepthStencilSurfaceEx)(THIS_ UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Discard, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle, DWORD Usage) override;
	IFACEMETHOD(ResetEx)(THIS_ D3DPRESENT_PARAMETERS* pPresentationParameters, D3DDISPLAYMODEEX *pFullscreenDisplayMode) override;
	IFACEMETHOD(GetDisplayModeEx)(THIS_ UINT iSwapChain, D3DDISPLAYMODEEX* pMode, D3DDISPLAYROTATION* pRotation) override;

	// Information functions
	LPDIRECT3DDEVICE9 GetProxyInterface() const { return ProxyInterface; }
	void InitInterface(void*, REFIID, UINT) {}	// Stub only
	AddressLookupTableD3d9* GetLookupTable() { return &ProxyAddressLookupTable9; }
	StateBlockCache* GetStateBlockTable() { return &StateBlockTable; }
	StateBlockCache* GetDeletedStateBlock() { return &DeletedStateBlocks; }
	void ClearDeletedStateBlock(m_IDirect3DStateBlock9* StateBlockX);
	bool GetDeviceMultiSampleFlag() const { return DeviceDetails.DeviceMultiSampleFlag; }
	D3DMULTISAMPLE_TYPE GetDeviceMultiSampleType() const { return DeviceDetails.DeviceMultiSampleType; }
	DWORD GetDeviceMultiSampleQuality() const { return DeviceDetails.DeviceMultiSampleQuality; }
	DWORD GetClientDXVersion() const { return DeviceDetails.ClientDirectXVersion; }
	REFIID GetIID() { return WrapperID; }
	void AddSurfaceToList(m_IDirect3DSurface9* pSurface) { EmulatedSurfaceList.insert(pSurface); }
	void RemoveSurfaceFromList(m_IDirect3DSurface9* pSurface) { EmulatedSurfaceList.erase(pSurface); }

	// Helper functions
	HRESULT GetFakeFrontBufferData(THIS_ UINT iSwapChain, IDirect3DSurface9* pDestSurface);
	HRESULT GetShadowFrontBufferData(THIS_ UINT iSwapChain, IDirect3DSurface9* pDestSurface);
	m_IDirect3DStateBlock9* GetCreateStateBlock(IDirect3DStateBlock9* pSB);

	// Static functions
	static void ModeExToMode(D3DDISPLAYMODEEX& ModeEx, D3DDISPLAYMODE& Mode);
};
