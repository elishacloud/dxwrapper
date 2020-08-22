#pragma once

#include "IDirectDrawX.h"

class m_IDirect3DDeviceX : public IUnknown, public AddressLookupTableDdrawObject
{
private:
	IDirect3DDevice7 *ProxyInterface = nullptr;
	DWORD ProxyDirectXVersion;
	REFCLSID ClassID;
	ULONG RefCount = 1;

	// Convert Device
	m_IDirectDrawX *ddrawParent = nullptr;
	m_IDirect3DViewportX *lpCurrentViewport = nullptr;
	LPDIRECT3DDEVICE9 *d3d9Device = nullptr;

	// Store d3d device version wrappers
	m_IDirect3DDevice *WrapperInterface;
	m_IDirect3DDevice2 *WrapperInterface2;
	m_IDirect3DDevice3 *WrapperInterface3;
	m_IDirect3DDevice7 *WrapperInterface7;

	// Wrapper interface functions
	REFIID GetWrapperType(DWORD DirectXVersion)
	{
		return (DirectXVersion == 1) ? IID_IDirect3DDevice :
			(DirectXVersion == 2) ? IID_IDirect3DDevice2 :
			(DirectXVersion == 3) ? IID_IDirect3DDevice3 :
			(DirectXVersion == 7) ? IID_IDirect3DDevice7 : IID_IUnknown;
	}
	bool CheckWrapperType(REFIID IID)
	{
		return (IID == IID_IDirect3DDevice ||
			IID == IID_IDirect3DDevice2 ||
			IID == IID_IDirect3DDevice3 ||
			IID == IID_IDirect3DDevice7) ? true : false;
	}
	IDirect3DDevice *GetProxyInterfaceV1() { return (IDirect3DDevice *)ProxyInterface; }
	IDirect3DDevice2 *GetProxyInterfaceV2() { return (IDirect3DDevice2 *)ProxyInterface; }
	IDirect3DDevice3 *GetProxyInterfaceV3() { return (IDirect3DDevice3 *)ProxyInterface; }
	IDirect3DDevice7 *GetProxyInterfaceV7() { return ProxyInterface; }

	// Interface initialization functions
	void InitDevice();
	void ReleaseDevice();

	// Check interfaces
	HRESULT CheckInterface(char *FunctionName, bool CheckD3DDevice);

public:
	m_IDirect3DDeviceX(IDirect3DDevice7 *aOriginal, DWORD DirectXVersion) : ProxyInterface(aOriginal), ClassID(IID_IUnknown)
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

		InitDevice();

		ProxyAddressLookupTable.SaveAddress(this, (ProxyInterface) ? ProxyInterface : (void*)this);
	}
	m_IDirect3DDeviceX(m_IDirectDrawX *lpDdraw, REFCLSID rclsid, DWORD DirectXVersion) : ddrawParent(lpDdraw), ClassID(rclsid)
	{
		ProxyDirectXVersion = 9;

		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << "(" << this << ")" << " converting interface from v" << DirectXVersion << " to v" << ProxyDirectXVersion);

		InitDevice();

		ProxyAddressLookupTable.SaveAddress(this, (ProxyInterface) ? ProxyInterface : (void*)this);
	}
	~m_IDirect3DDeviceX()
	{
		LOG_LIMIT(3, __FUNCTION__ << "(" << this << ")" << " deleting interface!");

		ReleaseDevice();

		ProxyAddressLookupTable.DeleteAddress(this);
	}

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * ppvObj, DWORD DirectXVersion);
	STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * ppvObj) { return QueryInterface(riid, ppvObj, GetGUIDVersion(riid)); }
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

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
	STDMETHOD(SwapTextureHandles)(THIS_ LPDIRECT3DTEXTURE2, LPDIRECT3DTEXTURE2);
	STDMETHOD(EnumTextureFormats)(THIS_ LPD3DENUMTEXTUREFORMATSCALLBACK, LPVOID);
	STDMETHOD(EnumTextureFormats)(THIS_ LPD3DENUMPIXELFORMATSCALLBACK, LPVOID);
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
	STDMETHOD(GetLightState)(THIS_ D3DLIGHTSTATETYPE, LPDWORD);
	STDMETHOD(SetLightState)(THIS_ D3DLIGHTSTATETYPE, DWORD);
	STDMETHOD(GetTexture)(THIS_ DWORD, LPDIRECT3DTEXTURE2 *, DWORD);
	STDMETHOD(SetTexture)(THIS_ DWORD, LPDIRECT3DTEXTURE2);
	STDMETHOD(BeginScene)(THIS);
	STDMETHOD(EndScene)(THIS);
	STDMETHOD(GetDirect3D)(THIS_ LPDIRECT3D7*, DWORD);
	STDMETHOD(SetRenderTarget)(THIS_ LPDIRECTDRAWSURFACE7, DWORD);
	STDMETHOD(GetRenderTarget)(THIS_ LPDIRECTDRAWSURFACE7 *, DWORD);
	STDMETHOD(Clear)(THIS_ DWORD, LPD3DRECT, DWORD, D3DCOLOR, D3DVALUE, DWORD);
	STDMETHOD(SetTransform)(THIS_ D3DTRANSFORMSTATETYPE, LPD3DMATRIX);
	STDMETHOD(GetTransform)(THIS_ D3DTRANSFORMSTATETYPE, LPD3DMATRIX);
	STDMETHOD(MultiplyTransform)(THIS_ D3DTRANSFORMSTATETYPE, LPD3DMATRIX);
	STDMETHOD(SetMaterial)(THIS_ LPD3DMATERIAL7);
	STDMETHOD(GetMaterial)(THIS_ LPD3DMATERIAL7);
	STDMETHOD(SetLight)(THIS_ DWORD, LPD3DLIGHT7);
	STDMETHOD(GetLight)(THIS_ DWORD, LPD3DLIGHT7);
	STDMETHOD(SetRenderState)(THIS_ D3DRENDERSTATETYPE, DWORD);
	STDMETHOD(GetRenderState)(THIS_ D3DRENDERSTATETYPE, LPDWORD);
	STDMETHOD(BeginStateBlock)(THIS);
	STDMETHOD(EndStateBlock)(THIS_ LPDWORD);
	STDMETHOD(PreLoad)(THIS_ LPDIRECTDRAWSURFACE7);
	STDMETHOD(DrawPrimitive)(THIS_ D3DPRIMITIVETYPE, DWORD, LPVOID, DWORD, DWORD);
	STDMETHOD(DrawIndexedPrimitive)(THIS_ D3DPRIMITIVETYPE, DWORD, LPVOID, DWORD, LPWORD, DWORD, DWORD);
	STDMETHOD(SetClipStatus)(THIS_ LPD3DCLIPSTATUS);
	STDMETHOD(GetClipStatus)(THIS_ LPD3DCLIPSTATUS);
	STDMETHOD(DrawPrimitiveStrided)(THIS_ D3DPRIMITIVETYPE, DWORD, LPD3DDRAWPRIMITIVESTRIDEDDATA, DWORD, DWORD);
	STDMETHOD(DrawIndexedPrimitiveStrided)(THIS_ D3DPRIMITIVETYPE, DWORD, LPD3DDRAWPRIMITIVESTRIDEDDATA, DWORD, LPWORD, DWORD, DWORD);
	STDMETHOD(DrawPrimitiveVB)(THIS_ D3DPRIMITIVETYPE, LPDIRECT3DVERTEXBUFFER7, DWORD, DWORD, DWORD);
	STDMETHOD(DrawIndexedPrimitiveVB)(THIS_ D3DPRIMITIVETYPE, LPDIRECT3DVERTEXBUFFER7, DWORD, DWORD, LPWORD, DWORD, DWORD);
	STDMETHOD(ComputeSphereVisibility)(THIS_ LPD3DVECTOR, LPD3DVALUE, DWORD, DWORD, LPDWORD);
	STDMETHOD(GetTexture)(THIS_ DWORD, LPDIRECTDRAWSURFACE7 *, DWORD);
	STDMETHOD(SetTexture)(THIS_ DWORD, LPDIRECTDRAWSURFACE7);
	STDMETHOD(GetTextureStageState)(THIS_ DWORD, D3DTEXTURESTAGESTATETYPE, LPDWORD);
	STDMETHOD(SetTextureStageState)(THIS_ DWORD, D3DTEXTURESTAGESTATETYPE, DWORD);
	STDMETHOD(ValidateDevice)(THIS_ LPDWORD);
	STDMETHOD(ApplyStateBlock)(THIS_ DWORD);
	STDMETHOD(CaptureStateBlock)(THIS_ DWORD);
	STDMETHOD(DeleteStateBlock)(THIS_ DWORD);
	STDMETHOD(CreateStateBlock)(THIS_ D3DSTATEBLOCKTYPE, LPDWORD);
	STDMETHOD(Load)(THIS_ LPDIRECTDRAWSURFACE7, LPPOINT, LPDIRECTDRAWSURFACE7, LPRECT, DWORD);
	STDMETHOD(LightEnable)(THIS_ DWORD, BOOL);
	STDMETHOD(GetLightEnable)(THIS_ DWORD, BOOL*);
	STDMETHOD(SetClipPlane)(THIS_ DWORD, D3DVALUE*);
	STDMETHOD(GetClipPlane)(THIS_ DWORD, D3DVALUE*);
	STDMETHOD(GetInfo)(THIS_ DWORD, LPVOID, DWORD);

	// Helper functions
	void *GetWrapperInterfaceX(DWORD DirectXVersion);

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
};
