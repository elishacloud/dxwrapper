#pragma once

class m_IDirect3DSurface9 final : public IDirect3DSurface9, public AddressLookupTableD3d9Object
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
	bool IsSurfaceTexture = false;
	m_IDirect3DTexture9* pTextureContainer = nullptr;
	struct {
		bool UsingEmulatedSurface = false;
		bool ReadOnly = false;
		DWORD SurfaceUSN = 0;
		LPDIRECT3DSURFACE9 pSurface = nullptr;
	} Emu;

	inline bool ShouldEmulateMultiSampledSurface() const;
	inline bool ShouldEmulateNonMultiSampledSurface() const;
	inline bool IsEmulatedSurfaceOutofDate() const;

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
	IFACEMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj) override;
	IFACEMETHOD_(ULONG, AddRef)(THIS) override;
	IFACEMETHOD_(ULONG, Release)(THIS) override;

	/*** IDirect3DSurface9 methods ***/
	IFACEMETHOD(GetDevice)(THIS_ IDirect3DDevice9** ppDevice) override;
	IFACEMETHOD(SetPrivateData)(THIS_ REFGUID refguid, CONST void* pData, DWORD SizeOfData, DWORD Flags) override;
	IFACEMETHOD(GetPrivateData)(THIS_ REFGUID refguid, void* pData, DWORD* pSizeOfData) override;
	IFACEMETHOD(FreePrivateData)(THIS_ REFGUID refguid) override;
	IFACEMETHOD_(DWORD, SetPriority)(THIS_ DWORD PriorityNew) override;
	IFACEMETHOD_(DWORD, GetPriority)(THIS) override;
	IFACEMETHOD_(void, PreLoad)(THIS) override;
	IFACEMETHOD_(D3DRESOURCETYPE, GetType)(THIS) override;
	IFACEMETHOD(GetContainer)(THIS_ REFIID riid, void** ppContainer) override;
	IFACEMETHOD(GetDesc)(THIS_ D3DSURFACE_DESC *pDesc) override;
	IFACEMETHOD(LockRect)(THIS_ D3DLOCKED_RECT* pLockedRect, CONST RECT* pRect, DWORD Flags) override;
	IFACEMETHOD(UnlockRect)(THIS) override;
	IFACEMETHOD(GetDC)(THIS_ HDC *phdc) override;
	IFACEMETHOD(ReleaseDC)(THIS_ HDC hdc) override;

	// Information functions
	LPDIRECT3DSURFACE9 GetProxyInterface() const { return ProxyInterface; }
	ULONG GetEmulatedSurfaceCount() const { return Emu.pSurface ? 1 : 0; }

	// Helper functions
	void InitInterface(m_IDirect3DDevice9Ex* Device, REFIID, void*);
	void SetTextureContainer(m_IDirect3DTexture9* pTexture);
	void ClearTextureContainer() { pTextureContainer = nullptr; }
	void ReleaseEmulatedSurface();
	void PrepareReadingFromSurface();
	void PrepareWritingToSurface(bool IncreamentUSN);
	LPDIRECT3DSURFACE9 GetNonMultiSampledSurface(DWORD Flags);
	LPDIRECT3DSURFACE9 GetMultiSampledSurface();
	HRESULT CopyToEmulatedSurface();
	HRESULT CopyToRealSurface();
	HRESULT RestoreMultiSampleData();
};
