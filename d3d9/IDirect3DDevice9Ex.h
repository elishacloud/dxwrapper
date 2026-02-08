#pragma once

static constexpr size_t MAX_CLIP_PLANES = 6;
static constexpr size_t MAX_TEXTURE_STAGES = 8;
const std::chrono::seconds FPS_CALCULATION_WINDOW(1);	// Define a constant for the desired duration of FPS calculation

struct DEVICEDETAILS
{
	DEVICEDETAILS()
	{
		if (!InitializeCriticalSectionAndSpinCount(&d9cs, 4000))
		{
			Logging::Log() << __FUNCTION__ << " Warning: failed to initialize CriticalSectionAndSpinCount for d9cs.  Failing over to CriticalSection!";
			InitializeCriticalSection(&d9cs);
		}
	}
	~DEVICEDETAILS()
	{
		DeleteCriticalSection(&d9cs);
	}

	// Window handle and size
	DWORD ClientDirectXVersion = 0;
	bool IsWindowMode = false;
	bool AppRequestedWindowMode = false;
	bool IsDirectDrawDevice = false;
	UINT Adapter = D3DADAPTER_DEFAULT;
	D3DDEVTYPE DeviceType = D3DDEVTYPE_HAL;
	D3DFORMAT BackBufferFormat = D3DFMT_UNKNOWN;
	DWORD BackBufferCount = 0;
	HMONITOR hMonitor = nullptr;
	HWND DeviceWindow = nullptr;
	LONG BufferWidth = 0, BufferHeight = 0;
	LONG screenWidth = 0, screenHeight = 0;

	CRITICAL_SECTION d9cs = {};

	std::unordered_map<m_IDirect3DDevice9Ex*, BOOL> DeviceMap;

	AddressLookupTableD3d9 ProxyAddressLookupTable9;

	StateBlockCache StateBlockTable;

	StateBlockCache DeletedStateBlocks;

	// Limit frame rate
	struct {
		DWORD FrameCounter = 0;
		LARGE_INTEGER LastPresentTime = {};
	} Counter;

	// Frame counter
	double AverageFPSCounter = 0.0;
	std::deque<std::pair<std::chrono::steady_clock::time_point, std::chrono::duration<double>>> frameTimes;	// Store frame times in a deque
	std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();	// Store start time for PFS counter

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

class m_IDirect3DDevice9Ex : public IDirect3DDevice9Ex, public AddressLookupTableD3d9Object
{
private:
	LPDIRECT3DDEVICE9 ProxyInterface;
	LPDIRECT3DDEVICE9EX ProxyInterfaceEx = nullptr;
	m_IDirect3D9Ex* m_pD3DEx;
	const IID WrapperID;
	std::unique_ptr<ShadowSurfaceStorage> ShadowBackbuffer = std::make_unique<ShadowSurfaceStorage>();
	std::vector<IDirect3DSurface9*> BackBufferList;

	UINT DDKey;

	D3DCAPS9 Caps = {};

	// Begin/End Scene
	bool IsInScene = false;
	bool BeginSceneCalled = false;

	// FPS display
	LPD3DXFONT pFont = nullptr;
	DWORD FontRefCount = 0;
	LPD3DXSPRITE pSprite = nullptr;
	DWORD SprintRefCount = 0;

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
	bool isAnisotropySet = false;
	bool AnisotropyDisabledFlag = false;	// Tracks when Anisotropic Fintering was disabled becasue lack of multi-stage texture support

	// Antialiasing
	bool RenderTargetNonMultiSampled = false;
	bool DepthStencilNonMultiSampled = false;
	bool NullDepthStencil = false;
	m_IDirect3DSurface9* CurrentRenderTarget = nullptr;

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
	void LimitFrameRate() const;

	// Frame counter
	void CalculateFPS() const;
	void DrawFPS(float fps, const RECT& presentRect, DWORD position);

	// Anisotropic Filtering
	void DisableAnisotropicSamplerState(bool AnisotropyMin, bool AnisotropyMag);
	void ReeableAnisotropicSamplerState();

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

public:
	m_IDirect3DDevice9Ex(LPDIRECT3DDEVICE9EX pDevice, m_IDirect3D9Ex* pD3D, REFIID DeviceID, UINT Key) : ProxyInterface(pDevice), m_pD3DEx(pD3D), WrapperID(DeviceID), DDKey(Key)
	{
		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ") " << WrapperID << " game interface v" << SHARED.ClientDirectXVersion);

		if (WrapperID == IID_IDirect3DDevice9Ex)
		{
			ProxyInterfaceEx = pDevice;
		}

		D3d9Wrapper::TestAllDeviceRefs(ProxyInterface);

		// Check for SSAA
		if (SHARED.DeviceMultiSampleType && m_pD3DEx)
		{
			if (m_pD3DEx->CheckDeviceFormat(SHARED.Adapter, SHARED.DeviceType, D3DFMT_X8R8G8B8, 0, D3DRTYPE_SURFACE, (D3DFORMAT)MAKEFOURCC('S', 'S', 'A', 'A')) == S_OK)
			{
				SHARED.SetSSAA = true;
			}
			if (Config.EnableMultisamplingATOC &&
				m_pD3DEx->CheckDeviceFormat(SHARED.Adapter, SHARED.DeviceType, D3DFMT_X8R8G8B8, 0, D3DRTYPE_SURFACE, (D3DFORMAT)MAKEFOURCC('A', 'T', 'O', 'C')) == S_OK)
			{
				SHARED.SetATOC = true;
			}
		}

		ReInitInterface();

		SHARED.DeviceMap[this] = TRUE;

		SHARED.ProxyAddressLookupTable9.SaveAddress(this, ProxyInterface);
	}
	~m_IDirect3DDevice9Ex()
	{
		LOG_LIMIT(3, __FUNCTION__ << " (" << this << ")" << " deleting interface!");
	}

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	/*** IDirect3DDevice9 methods ***/
	STDMETHOD(TestCooperativeLevel)(THIS);
	STDMETHOD_(UINT, GetAvailableTextureMem)(THIS);
	STDMETHOD(EvictManagedResources)(THIS);
	STDMETHOD(GetDirect3D)(THIS_ IDirect3D9** ppD3D9);
	STDMETHOD(GetDeviceCaps)(THIS_ D3DCAPS9* pCaps);
	STDMETHOD(GetDisplayMode)(THIS_ UINT iSwapChain, D3DDISPLAYMODE* pMode);
	STDMETHOD(GetCreationParameters)(THIS_ D3DDEVICE_CREATION_PARAMETERS *pParameters);
	STDMETHOD(SetCursorProperties)(THIS_ UINT XHotSpot, UINT YHotSpot, IDirect3DSurface9* pCursorBitmap);
	STDMETHOD_(void, SetCursorPosition)(THIS_ int X, int Y, DWORD Flags);
	STDMETHOD_(BOOL, ShowCursor)(THIS_ BOOL bShow);
	STDMETHOD(CreateAdditionalSwapChain)(THIS_ D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DSwapChain9** pSwapChain);
	STDMETHOD(GetSwapChain)(THIS_ UINT iSwapChain, IDirect3DSwapChain9** pSwapChain);
	STDMETHOD_(UINT, GetNumberOfSwapChains)(THIS);
	STDMETHOD(Reset)(THIS_ D3DPRESENT_PARAMETERS* pPresentationParameters);
	STDMETHOD(Present)(THIS_ CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion);
	STDMETHOD(GetBackBuffer)(THIS_ UINT iSwapChain, UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface9** ppBackBuffer);
	STDMETHOD(GetRasterStatus)(THIS_ UINT iSwapChain, D3DRASTER_STATUS* pRasterStatus);
	STDMETHOD(SetDialogBoxMode)(THIS_ BOOL bEnableDialogs);
	STDMETHOD_(void, SetGammaRamp)(THIS_ UINT iSwapChain, DWORD Flags, CONST D3DGAMMARAMP* pRamp);
	STDMETHOD_(void, GetGammaRamp)(THIS_ UINT iSwapChain, D3DGAMMARAMP* pRamp);
	STDMETHOD(CreateTexture)(THIS_ UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture9** ppTexture, HANDLE* pSharedHandle);
	STDMETHOD(CreateVolumeTexture)(THIS_ UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DVolumeTexture9** ppVolumeTexture, HANDLE* pSharedHandle);
	STDMETHOD(CreateCubeTexture)(THIS_ UINT EdgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DCubeTexture9** ppCubeTexture, HANDLE* pSharedHandle);
	STDMETHOD(CreateVertexBuffer)(THIS_ UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool, IDirect3DVertexBuffer9** ppVertexBuffer, HANDLE* pSharedHandle);
	STDMETHOD(CreateIndexBuffer)(THIS_ UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DIndexBuffer9** ppIndexBuffer, HANDLE* pSharedHandle);
	STDMETHOD(CreateRenderTarget)(THIS_ UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle);
	STDMETHOD(CreateDepthStencilSurface)(THIS_ UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Discard, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle);
	STDMETHOD(UpdateSurface)(THIS_ IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestSurface, CONST POINT* pDestPoint);
	STDMETHOD(UpdateTexture)(THIS_ IDirect3DBaseTexture9* pSourceTexture, IDirect3DBaseTexture9* pDestinationTexture);
	STDMETHOD(GetRenderTargetData)(THIS_ IDirect3DSurface9* pRenderTarget, IDirect3DSurface9* pDestSurface);
	STDMETHOD(GetFrontBufferData)(THIS_ UINT iSwapChain, IDirect3DSurface9* pDestSurface);
	STDMETHOD(StretchRect)(THIS_ IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestSurface, CONST RECT* pDestRect, D3DTEXTUREFILTERTYPE Filter);
	STDMETHOD(ColorFill)(THIS_ IDirect3DSurface9* pSurface, CONST RECT* pRect, D3DCOLOR color);
	STDMETHOD(CreateOffscreenPlainSurface)(THIS_ UINT Width, UINT Height, D3DFORMAT Format, D3DPOOL Pool, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle);
	STDMETHOD(SetRenderTarget)(THIS_ DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget);
	STDMETHOD(GetRenderTarget)(THIS_ DWORD RenderTargetIndex, IDirect3DSurface9** ppRenderTarget);
	STDMETHOD(SetDepthStencilSurface)(THIS_ IDirect3DSurface9* pNewZStencil);
	STDMETHOD(GetDepthStencilSurface)(THIS_ IDirect3DSurface9** ppZStencilSurface);
	STDMETHOD(BeginScene)(THIS);
	STDMETHOD(EndScene)(THIS);
	STDMETHOD(Clear)(THIS_ DWORD Count, CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil);
	STDMETHOD(SetTransform)(THIS_ D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix);
	STDMETHOD(GetTransform)(THIS_ D3DTRANSFORMSTATETYPE State, D3DMATRIX* pMatrix);
	STDMETHOD(MultiplyTransform)(THIS_ D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX *pMatrix);
	STDMETHOD(SetViewport)(THIS_ CONST D3DVIEWPORT9* pViewport);
	STDMETHOD(GetViewport)(THIS_ D3DVIEWPORT9* pViewport);
	STDMETHOD(SetMaterial)(THIS_ CONST D3DMATERIAL9* pMaterial);
	STDMETHOD(GetMaterial)(THIS_ D3DMATERIAL9* pMaterial);
	STDMETHOD(SetLight)(THIS_ DWORD Index, CONST D3DLIGHT9* pLight);
	STDMETHOD(GetLight)(THIS_ DWORD Index, D3DLIGHT9* pLight);
	STDMETHOD(LightEnable)(THIS_ DWORD Index, BOOL Enable);
	STDMETHOD(GetLightEnable)(THIS_ DWORD Index, BOOL* pEnable);
	STDMETHOD(SetClipPlane)(THIS_ DWORD Index, CONST float* pPlane);
	STDMETHOD(GetClipPlane)(THIS_ DWORD Index, float* pPlane);
	STDMETHOD(SetRenderState)(THIS_ D3DRENDERSTATETYPE State, DWORD Value);
	STDMETHOD(GetRenderState)(THIS_ D3DRENDERSTATETYPE State, DWORD* pValue);
	STDMETHOD(CreateStateBlock)(THIS_ D3DSTATEBLOCKTYPE Type, IDirect3DStateBlock9** ppSB);
	STDMETHOD(BeginStateBlock)(THIS);
	STDMETHOD(EndStateBlock)(THIS_ IDirect3DStateBlock9** ppSB);
	STDMETHOD(SetClipStatus)(THIS_ CONST D3DCLIPSTATUS9* pClipStatus);
	STDMETHOD(GetClipStatus)(THIS_ D3DCLIPSTATUS9* pClipStatus);
	STDMETHOD(GetTexture)(THIS_ DWORD Stage, IDirect3DBaseTexture9** ppTexture);
	STDMETHOD(SetTexture)(THIS_ DWORD Stage, IDirect3DBaseTexture9* pTexture);
	STDMETHOD(GetTextureStageState)(THIS_ DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD* pValue);
	STDMETHOD(SetTextureStageState)(THIS_ DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value);
	STDMETHOD(GetSamplerState)(THIS_ DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD* pValue);
	STDMETHOD(SetSamplerState)(THIS_ DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value);
	STDMETHOD(ValidateDevice)(THIS_ DWORD* pNumPasses);
	STDMETHOD(SetPaletteEntries)(THIS_ UINT PaletteNumber, CONST PALETTEENTRY* pEntries);
	STDMETHOD(GetPaletteEntries)(THIS_ UINT PaletteNumber, PALETTEENTRY* pEntries);
	STDMETHOD(SetCurrentTexturePalette)(THIS_ UINT PaletteNumber);
	STDMETHOD(GetCurrentTexturePalette)(THIS_ UINT *PaletteNumber);
	STDMETHOD(SetScissorRect)(THIS_ CONST RECT* pRect);
	STDMETHOD(GetScissorRect)(THIS_ RECT* pRect);
	STDMETHOD(SetSoftwareVertexProcessing)(THIS_ BOOL bSoftware);
	STDMETHOD_(BOOL, GetSoftwareVertexProcessing)(THIS);
	STDMETHOD(SetNPatchMode)(THIS_ float nSegments);
	STDMETHOD_(float, GetNPatchMode)(THIS);
	STDMETHOD(DrawPrimitive)(THIS_ D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount);
	STDMETHOD(DrawIndexedPrimitive)(THIS_ D3DPRIMITIVETYPE, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount);
	STDMETHOD(DrawPrimitiveUP)(THIS_ D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride);
	STDMETHOD(DrawIndexedPrimitiveUP)(THIS_ D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex, UINT NumVertices, UINT PrimitiveCount, CONST void* pIndexData, D3DFORMAT IndexDataFormat, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride);
	STDMETHOD(ProcessVertices)(THIS_ UINT SrcStartIndex, UINT DestIndex, UINT VertexCount, IDirect3DVertexBuffer9* pDestBuffer, IDirect3DVertexDeclaration9* pVertexDecl, DWORD Flags);
	STDMETHOD(CreateVertexDeclaration)(THIS_ CONST D3DVERTEXELEMENT9* pVertexElements, IDirect3DVertexDeclaration9** ppDecl);
	STDMETHOD(SetVertexDeclaration)(THIS_ IDirect3DVertexDeclaration9* pDecl);
	STDMETHOD(GetVertexDeclaration)(THIS_ IDirect3DVertexDeclaration9** ppDecl);
	STDMETHOD(SetFVF)(THIS_ DWORD FVF);
	STDMETHOD(GetFVF)(THIS_ DWORD* pFVF);
	STDMETHOD(CreateVertexShader)(THIS_ CONST DWORD* pFunction, IDirect3DVertexShader9** ppShader);
	STDMETHOD(SetVertexShader)(THIS_ IDirect3DVertexShader9* pShader);
	STDMETHOD(GetVertexShader)(THIS_ IDirect3DVertexShader9** ppShader);
	STDMETHOD(SetVertexShaderConstantF)(THIS_ UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount);
	STDMETHOD(GetVertexShaderConstantF)(THIS_ UINT StartRegister, float* pConstantData, UINT Vector4fCount);
	STDMETHOD(SetVertexShaderConstantI)(THIS_ UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount);
	STDMETHOD(GetVertexShaderConstantI)(THIS_ UINT StartRegister, int* pConstantData, UINT Vector4iCount);
	STDMETHOD(SetVertexShaderConstantB)(THIS_ UINT StartRegister, CONST BOOL* pConstantData, UINT  BoolCount);
	STDMETHOD(GetVertexShaderConstantB)(THIS_ UINT StartRegister, BOOL* pConstantData, UINT BoolCount);
	STDMETHOD(SetStreamSource)(THIS_ UINT StreamNumber, IDirect3DVertexBuffer9* pStreamData, UINT OffsetInBytes, UINT Stride);
	STDMETHOD(GetStreamSource)(THIS_ UINT StreamNumber, IDirect3DVertexBuffer9** ppStreamData, UINT* pOffsetInBytes, UINT* pStride);
	STDMETHOD(SetStreamSourceFreq)(THIS_ UINT StreamNumber, UINT Setting);
	STDMETHOD(GetStreamSourceFreq)(THIS_ UINT StreamNumber, UINT* pSetting);
	STDMETHOD(SetIndices)(THIS_ IDirect3DIndexBuffer9* pIndexData);
	STDMETHOD(GetIndices)(THIS_ IDirect3DIndexBuffer9** ppIndexData);
	STDMETHOD(CreatePixelShader)(THIS_ CONST DWORD* pFunction, IDirect3DPixelShader9** ppShader);
	STDMETHOD(SetPixelShader)(THIS_ IDirect3DPixelShader9* pShader);
	STDMETHOD(GetPixelShader)(THIS_ IDirect3DPixelShader9** ppShader);
	STDMETHOD(SetPixelShaderConstantF)(THIS_ UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount);
	STDMETHOD(GetPixelShaderConstantF)(THIS_ UINT StartRegister, float* pConstantData, UINT Vector4fCount);
	STDMETHOD(SetPixelShaderConstantI)(THIS_ UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount);
	STDMETHOD(GetPixelShaderConstantI)(THIS_ UINT StartRegister, int* pConstantData, UINT Vector4iCount);
	STDMETHOD(SetPixelShaderConstantB)(THIS_ UINT StartRegister, CONST BOOL* pConstantData, UINT  BoolCount);
	STDMETHOD(GetPixelShaderConstantB)(THIS_ UINT StartRegister, BOOL* pConstantData, UINT BoolCount);
	STDMETHOD(DrawRectPatch)(THIS_ UINT Handle, CONST float* pNumSegs, CONST D3DRECTPATCH_INFO* pRectPatchInfo);
	STDMETHOD(DrawTriPatch)(THIS_ UINT Handle, CONST float* pNumSegs, CONST D3DTRIPATCH_INFO* pTriPatchInfo);
	STDMETHOD(DeletePatch)(THIS_ UINT Handle);
	STDMETHOD(CreateQuery)(THIS_ D3DQUERYTYPE Type, IDirect3DQuery9** ppQuery);

	/*** IDirect3DDevice9Ex methods ***/
	STDMETHOD(SetConvolutionMonoKernel)(THIS_ UINT width, UINT height, float* rows, float* columns);
	STDMETHOD(ComposeRects)(THIS_ IDirect3DSurface9* pSrc, IDirect3DSurface9* pDst, IDirect3DVertexBuffer9* pSrcRectDescs, UINT NumRects, IDirect3DVertexBuffer9* pDstRectDescs, D3DCOMPOSERECTSOP Operation, int Xoffset, int Yoffset);
	STDMETHOD(PresentEx)(THIS_ CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion, DWORD dwFlags);
	STDMETHOD(GetGPUThreadPriority)(THIS_ INT* pPriority);
	STDMETHOD(SetGPUThreadPriority)(THIS_ INT Priority);
	STDMETHOD(WaitForVBlank)(THIS_ UINT iSwapChain);
	STDMETHOD(CheckResourceResidency)(THIS_ IDirect3DResource9** pResourceArray, UINT32 NumResources);
	STDMETHOD(SetMaximumFrameLatency)(THIS_ UINT MaxLatency);
	STDMETHOD(GetMaximumFrameLatency)(THIS_ UINT* pMaxLatency);
	STDMETHOD(CheckDeviceState)(THIS_ HWND hDestinationWindow);
	STDMETHOD(CreateRenderTargetEx)(THIS_ UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle, DWORD Usage);
	STDMETHOD(CreateOffscreenPlainSurfaceEx)(THIS_ UINT Width, UINT Height, D3DFORMAT Format, D3DPOOL Pool, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle, DWORD Usage);
	STDMETHOD(CreateDepthStencilSurfaceEx)(THIS_ UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Discard, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle, DWORD Usage);
	STDMETHOD(ResetEx)(THIS_ D3DPRESENT_PARAMETERS* pPresentationParameters, D3DDISPLAYMODEEX *pFullscreenDisplayMode);
	STDMETHOD(GetDisplayModeEx)(THIS_ UINT iSwapChain, D3DDISPLAYMODEEX* pMode, D3DDISPLAYROTATION* pRotation);

	// Information functions
	LPDIRECT3DDEVICE9 GetProxyInterface() const { return ProxyInterface; }
	void InitInterface(void*, REFIID, UINT) {}	// Stub only
	AddressLookupTableD3d9* GetLookupTable() const { return &SHARED.ProxyAddressLookupTable9; }
	StateBlockCache* GetStateBlockTable() const { return &SHARED.StateBlockTable; }
	StateBlockCache* GetDeletedStateBlock() const { return &SHARED.DeletedStateBlocks; }
	bool GetDeviceMultiSampleFlag() const { return SHARED.DeviceMultiSampleFlag; }
	D3DMULTISAMPLE_TYPE GetDeviceMultiSampleType() const { return SHARED.DeviceMultiSampleType; }
	DWORD GetDeviceMultiSampleQuality() const { return SHARED.DeviceMultiSampleQuality; }
	DWORD GetClientDXVersion() const { return SHARED.ClientDirectXVersion; }
	REFIID GetIID() { return WrapperID; }

	// Helper functions
	HRESULT GetFakeFrontBufferData(THIS_ UINT iSwapChain, IDirect3DSurface9* pDestSurface);
	HRESULT GetShadowFrontBufferData(THIS_ UINT iSwapChain, IDirect3DSurface9* pDestSurface);
	m_IDirect3DStateBlock9* GetCreateStateBlock(IDirect3DStateBlock9* pSB);

	// Static functions
	static void ModeExToMode(D3DDISPLAYMODEEX& ModeEx, D3DDISPLAYMODE& Mode);
};
