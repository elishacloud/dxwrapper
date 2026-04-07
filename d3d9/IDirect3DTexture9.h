#pragma once

class m_IDirect3DTexture9 final : public IDirect3DTexture9, public AddressLookupTableD3d9Object
{
private:
	LPDIRECT3DTEXTURE9 ProxyInterface;
	m_IDirect3DDevice9Ex* m_pDeviceEx;
	const IID WrapperID = IID_IDirect3DTexture9;

	DWORD TextureUSN = 0;
	std::unordered_set<m_IDirect3DSurface9*> SurfaceLevelList;

	inline void IncrementTextureUSN() { TextureUSN++; }

public:
	m_IDirect3DTexture9(LPDIRECT3DTEXTURE9 pTexture9, m_IDirect3DDevice9Ex* pDevice) : ProxyInterface(pTexture9), m_pDeviceEx(pDevice)
	{
		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ")");

		InitInterface(pDevice, WrapperID, nullptr);

		m_pDeviceEx->GetLookupTable()->SaveAddress(this, ProxyInterface);
	}
	~m_IDirect3DTexture9()
	{
		LOG_LIMIT(3, __FUNCTION__ << " (" << this << ")" << " deleting interface!");
	}

	/*** IUnknown methods ***/
	IFACEMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj) override;
	IFACEMETHOD_(ULONG, AddRef)(THIS) override;
	IFACEMETHOD_(ULONG, Release)(THIS) override;

	/*** IDirect3DTexture9 methods ***/
	IFACEMETHOD(GetDevice)(THIS_ IDirect3DDevice9** ppDevice) override;
	IFACEMETHOD(SetPrivateData)(THIS_ REFGUID refguid, CONST void* pData, DWORD SizeOfData, DWORD Flags) override;
	IFACEMETHOD(GetPrivateData)(THIS_ REFGUID refguid, void* pData, DWORD* pSizeOfData) override;
	IFACEMETHOD(FreePrivateData)(THIS_ REFGUID refguid) override;
	IFACEMETHOD_(DWORD, SetPriority)(THIS_ DWORD PriorityNew) override;
	IFACEMETHOD_(DWORD, GetPriority)(THIS) override;
	IFACEMETHOD_(void, PreLoad)(THIS) override;
	IFACEMETHOD_(D3DRESOURCETYPE, GetType)(THIS) override;
	IFACEMETHOD_(DWORD, SetLOD)(THIS_ DWORD LODNew) override;
	IFACEMETHOD_(DWORD, GetLOD)(THIS) override;
	IFACEMETHOD_(DWORD, GetLevelCount)(THIS) override;
	IFACEMETHOD(SetAutoGenFilterType)(THIS_ D3DTEXTUREFILTERTYPE FilterType) override;
	IFACEMETHOD_(D3DTEXTUREFILTERTYPE, GetAutoGenFilterType)(THIS) override;
	IFACEMETHOD_(void, GenerateMipSubLevels)(THIS) override;
	IFACEMETHOD(GetLevelDesc)(THIS_ UINT Level, D3DSURFACE_DESC *pDesc) override;
	IFACEMETHOD(GetSurfaceLevel)(THIS_ UINT Level, IDirect3DSurface9** ppSurfaceLevel) override;
	IFACEMETHOD(LockRect)(THIS_ UINT Level, D3DLOCKED_RECT* pLockedRect, CONST RECT* pRect, DWORD Flags) override;
	IFACEMETHOD(UnlockRect)(THIS_ UINT Level) override;
	IFACEMETHOD(AddDirtyRect)(THIS_ CONST RECT* pDirtyRect) override;

	// Helper functions
	LPDIRECT3DTEXTURE9 GetProxyInterface() const { return ProxyInterface; }
	void InitInterface(m_IDirect3DDevice9Ex* Device, REFIID, void*) { m_pDeviceEx = Device; }
	DWORD GetTextureUSN() const { return TextureUSN; }
	void AddSurfaceToList(m_IDirect3DSurface9* pSurface) { SurfaceLevelList.insert(pSurface); }
	void RemoveSurfaceFromList(m_IDirect3DSurface9* pSurface) { SurfaceLevelList.erase(pSurface); }
	void PrepareReadingFromTexture();
	void PrepareWritingToTexture(bool IncreamentUSN);
};
