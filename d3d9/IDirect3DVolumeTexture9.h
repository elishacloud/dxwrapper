#pragma once

class m_IDirect3DVolumeTexture9  final : public IDirect3DVolumeTexture9, public AddressLookupTableD3d9Object
{
private:
	LPDIRECT3DVOLUMETEXTURE9 ProxyInterface;
	m_IDirect3DDevice9Ex* m_pDeviceEx;
	const IID WrapperID = IID_IDirect3DVolumeTexture9;

public:
	m_IDirect3DVolumeTexture9(LPDIRECT3DVOLUMETEXTURE9 pTexture8, m_IDirect3DDevice9Ex* pDevice) : ProxyInterface(pTexture8), m_pDeviceEx(pDevice)
	{
		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ")");

		InitInterface(pDevice, WrapperID, nullptr);

		m_pDeviceEx->GetLookupTable()->SaveAddress(this, ProxyInterface);
	}
	~m_IDirect3DVolumeTexture9()
	{
		LOG_LIMIT(3, __FUNCTION__ << " (" << this << ")" << " deleting interface!");
	}

	/*** IUnknown methods ***/
	IFACEMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj) override;
	IFACEMETHOD_(ULONG, AddRef)(THIS) override;
	IFACEMETHOD_(ULONG, Release)(THIS) override;

	/*** IDirect3DVolumeTexture9 methods ***/
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
	IFACEMETHOD(GetLevelDesc)(THIS_ UINT Level, D3DVOLUME_DESC *pDesc) override;
	IFACEMETHOD(GetVolumeLevel)(THIS_ UINT Level, IDirect3DVolume9** ppVolumeLevel) override;
	IFACEMETHOD(LockBox)(THIS_ UINT Level, D3DLOCKED_BOX* pLockedVolume, CONST D3DBOX* pBox, DWORD Flags) override;
	IFACEMETHOD(UnlockBox)(THIS_ UINT Level) override;
	IFACEMETHOD(AddDirtyBox)(THIS_ CONST D3DBOX* pDirtyBox) override;

	// Helper functions
	LPDIRECT3DVOLUMETEXTURE9 GetProxyInterface() const { return ProxyInterface; }
	void InitInterface(m_IDirect3DDevice9Ex* Device, REFIID, void*) { m_pDeviceEx = Device; }
};
