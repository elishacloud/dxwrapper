#pragma once

class m_IDirect3DSurface9 : public IDirect3DSurface9, public AddressLookupTableD3d9Object
{
private:
	LPDIRECT3DSURFACE9 ProxyInterface;
	m_IDirect3DDevice9Ex* m_pDeviceEx;
	const IID WrapperID = IID_IDirect3DSurface9;
	D3DSURFACE_DESC Desc = {};

	// For multi-sampling
	bool DeviceMultiSampleFlag = false;
	D3DMULTISAMPLE_TYPE DeviceMultiSampleType = D3DMULTISAMPLE_NONE;
	DWORD DeviceMultiSampleQuality = 0;

	// For fake emulated locking
	struct {
		bool ReadOnly = false;
		bool IsSurfaceTexture = false;
		m_IDirect3DTexture9* pTextureContainer = nullptr;
		m_IDirect3DSurface9* pSurface = nullptr;
	} Emu;

	inline bool ShouldEmulateMultiSampledSurface() const;
	inline bool ShouldEmulateNonMultiSampledSurface() const;

public:
	m_IDirect3DSurface9(LPDIRECT3DSURFACE9 pSurface9, m_IDirect3DDevice9Ex* pDevice) : ProxyInterface(pSurface9), m_pDeviceEx(pDevice)
	{
		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ")");

		InitInterface(pDevice, WrapperID, nullptr);

		m_pDeviceEx->GetLookupTable()->SaveAddress(this, ProxyInterface);
	}
	~m_IDirect3DSurface9()
	{
		LOG_LIMIT(3, __FUNCTION__ << " (" << this << ")" << " deleting interface!");

		if (Emu.pSurface)
		{
			ULONG eref = Emu.pSurface->Release();
			if (eref)
			{
				Logging::Log() << __FUNCTION__ << " Error: there is still a reference to 'Emu.pSurface' " << eref;
			}
			Emu.pSurface = nullptr;
		}
	}

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	/*** IDirect3DSurface9 methods ***/
	STDMETHOD(GetDevice)(THIS_ IDirect3DDevice9** ppDevice);
	STDMETHOD(SetPrivateData)(THIS_ REFGUID refguid, CONST void* pData, DWORD SizeOfData, DWORD Flags);
	STDMETHOD(GetPrivateData)(THIS_ REFGUID refguid, void* pData, DWORD* pSizeOfData);
	STDMETHOD(FreePrivateData)(THIS_ REFGUID refguid);
	STDMETHOD_(DWORD, SetPriority)(THIS_ DWORD PriorityNew);
	STDMETHOD_(DWORD, GetPriority)(THIS);
	STDMETHOD_(void, PreLoad)(THIS);
	STDMETHOD_(D3DRESOURCETYPE, GetType)(THIS);
	STDMETHOD(GetContainer)(THIS_ REFIID riid, void** ppContainer);
	STDMETHOD(GetDesc)(THIS_ D3DSURFACE_DESC *pDesc);
	STDMETHOD(LockRect)(THIS_ D3DLOCKED_RECT* pLockedRect, CONST RECT* pRect, DWORD Flags);
	STDMETHOD(UnlockRect)(THIS);
	STDMETHOD(GetDC)(THIS_ HDC *phdc);
	STDMETHOD(ReleaseDC)(THIS_ HDC hdc);

	// Information functions
	LPDIRECT3DSURFACE9 GetProxyInterface() const { return ProxyInterface; }
	ULONG GetSurfaceCount() const { return Emu.pSurface ? 2 : 1; }

	// Helper functions
	void InitInterface(m_IDirect3DDevice9Ex* Device, REFIID, void*);
	void SetTextureContainer(m_IDirect3DTexture9* pTexture);
	void ClearTextureContainer() { Emu.pTextureContainer = nullptr; }
	LPDIRECT3DSURFACE9 GetNonMultiSampledSurface(DWORD Flags);
	LPDIRECT3DSURFACE9 GetMultiSampledSurface();
	HRESULT CopyMultiSampleData();
	HRESULT RestoreMultiSampleData();
};
