#pragma once

#include "IDirectDrawX.h"

class m_IDirect3DX : public IUnknown, public AddressLookupTableDdrawObject
{
private:
	IDirect3D7 *ProxyInterface = nullptr;
	DWORD ProxyDirectXVersion;
	ULONG RefCount1 = 0;
	ULONG RefCount2 = 0;
	ULONG RefCount3 = 0;
	ULONG RefCount7 = 0;
	m_IDirectDrawX *ddrawParent = nullptr;

	// Store d3d version wrappers
	m_IDirect3D *WrapperInterface;
	m_IDirect3D2 *WrapperInterface2;
	m_IDirect3D3 *WrapperInterface3;
	m_IDirect3D7 *WrapperInterface7;

	// Cache Cap9
	struct DUALCAP9 {
		D3DCAPS9 REF = {};
		D3DCAPS9 HAL = {};
	};
	std::vector<DUALCAP9> Cap9Cache;

	// Wrapper interface functions
	inline REFIID GetWrapperType(DWORD DirectXVersion)
	{
		return (DirectXVersion == 1) ? IID_IDirect3D :
			(DirectXVersion == 2) ? IID_IDirect3D2 :
			(DirectXVersion == 3) ? IID_IDirect3D3 :
			(DirectXVersion == 7) ? IID_IDirect3D7 : IID_IUnknown;
	}
	inline bool CheckWrapperType(REFIID IID)
	{
		return (IID == IID_IDirect3D ||
			IID == IID_IDirect3D2 ||
			IID == IID_IDirect3D3 ||
			IID == IID_IDirect3D7) ? true : false;
	}
	inline IDirect3D *GetProxyInterfaceV1() { return (IDirect3D *)ProxyInterface; }
	inline IDirect3D2 *GetProxyInterfaceV2() { return (IDirect3D2 *)ProxyInterface; }
	inline IDirect3D3 *GetProxyInterfaceV3() { return (IDirect3D3 *)ProxyInterface; }
	inline IDirect3D7 *GetProxyInterfaceV7() { return ProxyInterface; }

	// Interface initialization functions
	void InitDirect3D(DWORD DirectXVersion);
	void ReleaseDirect3D();

	// Helper functions
	void GetCap9Cache();
	void ResolutionHack();

public:
	m_IDirect3DX(IDirect3D7 *aOriginal, DWORD DirectXVersion) : ProxyInterface(aOriginal)
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

		InitDirect3D(DirectXVersion);
	}
	m_IDirect3DX(m_IDirectDrawX *lpDdraw, DWORD DirectXVersion) : ddrawParent(lpDdraw)
	{
		ProxyDirectXVersion = 9;

		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ")" << " converting interface from v" << DirectXVersion << " to v" << ProxyDirectXVersion);

		InitDirect3D(DirectXVersion);
	}
	~m_IDirect3DX()
	{
		LOG_LIMIT(3, __FUNCTION__ << " (" << this << ")" << " deleting interface!");

		ReleaseDirect3D();
	}

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * ppvObj) { return QueryInterface(riid, ppvObj, 0); }
	STDMETHOD_(ULONG, AddRef) (THIS) { return AddRef(0); }
	STDMETHOD_(ULONG, Release) (THIS) { return Release(0); }

	/*** IDirect3D methods ***/
	STDMETHOD(Initialize)(THIS_ REFCLSID);
	HRESULT EnumDevices(LPD3DENUMDEVICESCALLBACK, LPVOID, DWORD);
	HRESULT EnumDevices7(LPD3DENUMDEVICESCALLBACK7, LPVOID, DWORD);
	STDMETHOD(CreateLight)(THIS_ LPDIRECT3DLIGHT*, LPUNKNOWN);
	STDMETHOD(CreateMaterial)(THIS_ LPDIRECT3DMATERIAL3*, LPUNKNOWN, DWORD);
	STDMETHOD(CreateViewport)(THIS_ LPDIRECT3DVIEWPORT3*, LPUNKNOWN, DWORD);
	STDMETHOD(FindDevice)(THIS_ LPD3DFINDDEVICESEARCH, LPD3DFINDDEVICERESULT);
	STDMETHOD(CreateDevice)(THIS_ REFCLSID, LPDIRECTDRAWSURFACE7, LPDIRECT3DDEVICE7*, LPUNKNOWN, DWORD);
	STDMETHOD(CreateVertexBuffer)(THIS_ LPD3DVERTEXBUFFERDESC, LPDIRECT3DVERTEXBUFFER7*, DWORD, LPUNKNOWN, DWORD);
	STDMETHOD(EnumZBufferFormats)(THIS_ REFCLSID, LPD3DENUMPIXELFORMATSCALLBACK, LPVOID);
	STDMETHOD(EvictManagedTextures)(THIS);

	// Helper functions
	HRESULT QueryInterface(REFIID riid, LPVOID FAR * ppvObj, DWORD DirectXVersion);
	void *GetWrapperInterfaceX(DWORD DirectXVersion);
	ULONG AddRef(DWORD DirectXVersion);
	ULONG Release(DWORD DirectXVersion);

	// Functions handling the ddraw parent interface
	void SetDdrawParent(m_IDirectDrawX* ddraw) { ddrawParent = ddraw; GetCap9Cache(); ddrawParent->SetD3D(this); }
	void ClearDdraw() { ddrawParent = nullptr; }
};
