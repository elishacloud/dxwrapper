#pragma once

#include "IDirectDrawX.h"
#include "RenderData.h"

class m_IDirect3DDeviceX : public IUnknown, public AddressLookupTableDdrawObject
{
private:
	IDirect3DDevice7 *ProxyInterface = nullptr;
	DWORD ProxyDirectXVersion;
	ULONG RefCount1 = 0;
	ULONG RefCount2 = 0;
	ULONG RefCount3 = 0;
	ULONG RefCount7 = 0;
	REFCLSID ClassID;

	// Convert Device
	m_IDirectDrawX *ddrawParent = nullptr;
	m_IDirect3DViewportX *lpCurrentViewport = nullptr;
	LPDIRECT3DDEVICE9 *d3d9Device = nullptr;

	// Store d3d device version wrappers
	m_IDirect3DDevice *WrapperInterface;
	m_IDirect3DDevice2 *WrapperInterface2;
	m_IDirect3DDevice3 *WrapperInterface3;
	m_IDirect3DDevice7 *WrapperInterface7;

	// SetTexture array
	LPDIRECTDRAWSURFACE7 AttachedTexture[8] = {};

	// The data used for rendering
	RenderData RenderData;

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
	inline bool CheckSurfaceExists(LPDIRECTDRAWSURFACE7 lpDDSrcSurface) { return
		(ProxyAddressLookupTable.IsValidWrapperAddress((m_IDirectDrawSurface*)lpDDSrcSurface) ||
		ProxyAddressLookupTable.IsValidWrapperAddress((m_IDirectDrawSurface2*)lpDDSrcSurface) ||
		ProxyAddressLookupTable.IsValidWrapperAddress((m_IDirectDrawSurface3*)lpDDSrcSurface) ||
		ProxyAddressLookupTable.IsValidWrapperAddress((m_IDirectDrawSurface4*)lpDDSrcSurface) ||
		ProxyAddressLookupTable.IsValidWrapperAddress((m_IDirectDrawSurface7*)lpDDSrcSurface));
	}

	// Interface initialization functions
	void InitDevice(DWORD DirectXVersion);
	void ReleaseDevice();

	// Check interfaces
	HRESULT CheckInterface(char *FunctionName, bool CheckD3DDevice);

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

		InitDevice(DirectXVersion);
	}
	m_IDirect3DDeviceX(m_IDirectDrawX *lpDdraw, REFCLSID rclsid, DWORD DirectXVersion) : ddrawParent(lpDdraw), ClassID(rclsid)
	{
		ProxyDirectXVersion = 9;

		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ")" << " converting interface from v" << DirectXVersion << " to v" << ProxyDirectXVersion);

		InitDevice(DirectXVersion);
	}
	~m_IDirect3DDeviceX()
	{
		LOG_LIMIT(3, __FUNCTION__ << " (" << this << ")" << " deleting interface!");

		ReleaseDevice();
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
	STDMETHOD(GetStats)(THIS_ LPD3DSTATS);
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
	STDMETHOD(SetLight)(THIS_ DWORD, LPD3DLIGHT7);
	STDMETHOD(GetLight)(THIS_ DWORD, LPD3DLIGHT7);
	STDMETHOD(LightEnable)(THIS_ DWORD, BOOL);
	STDMETHOD(GetLightEnable)(THIS_ DWORD, BOOL*);
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

	// Functions handling the ddraw parent interface
	void SetDdrawParent(m_IDirectDrawX *ddraw)
	{
		ddrawParent = ddraw;

		// Store D3DDevice
		if (ddrawParent)
		{
			ddrawParent->SetD3DDevice(this);
		}
	}
	void ClearDdraw() { ddrawParent = nullptr; }
	void ResetDevice();
	void SetDrawFlags(DWORD &rsClipping, DWORD &rsLighting, DWORD &rsExtents, DWORD dwVertexTypeDesc, DWORD dwFlags, DWORD DirectXVersion);
	void UnSetDrawFlags(DWORD rsClipping, DWORD rsLighting, DWORD rsExtents, DWORD dwVertexTypeDesc, DWORD dwFlags, DWORD DirectXVersion);
	void CopyScaleVertex(LPVOID lpVertices, std::vector<D3DTLVERTEX> &pVert, DWORD dwVertexCount);
	UINT GetNumberOfPrimitives(D3DPRIMITIVETYPE dptPrimitiveType, DWORD dwVertexCount);
	UINT GetVertexStride(DWORD dwVertexTypeDesc);
};
