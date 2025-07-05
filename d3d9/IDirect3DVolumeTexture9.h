#pragma once

class m_IDirect3DVolumeTexture9 : public IDirect3DVolumeTexture9, public AddressLookupTableD3d9Object
{
private:
	LPDIRECT3DVOLUMETEXTURE9 ProxyInterface;
	m_IDirect3DDevice9Ex* m_pDeviceEx;
	const IID WrapperID = IID_IDirect3DVolumeTexture9;

public:
	m_IDirect3DVolumeTexture9(LPDIRECT3DVOLUMETEXTURE9 pTexture8, m_IDirect3DDevice9Ex* pDevice) : ProxyInterface(pTexture8), m_pDeviceEx(pDevice)
	{
		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ")");

		m_pDeviceEx->GetLookupTable()->SaveAddress(this, ProxyInterface);
	}
	~m_IDirect3DVolumeTexture9()
	{
		LOG_LIMIT(3, __FUNCTION__ << " (" << this << ")" << " deleting interface!");
	}

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	/*** IDirect3DBaseTexture9 methods ***/
	STDMETHOD(GetDevice)(THIS_ IDirect3DDevice9** ppDevice);
	STDMETHOD(SetPrivateData)(THIS_ REFGUID refguid, CONST void* pData, DWORD SizeOfData, DWORD Flags);
	STDMETHOD(GetPrivateData)(THIS_ REFGUID refguid, void* pData, DWORD* pSizeOfData);
	STDMETHOD(FreePrivateData)(THIS_ REFGUID refguid);
	STDMETHOD_(DWORD, SetPriority)(THIS_ DWORD PriorityNew);
	STDMETHOD_(DWORD, GetPriority)(THIS);
	STDMETHOD_(void, PreLoad)(THIS);
	STDMETHOD_(D3DRESOURCETYPE, GetType)(THIS);
	STDMETHOD_(DWORD, SetLOD)(THIS_ DWORD LODNew);
	STDMETHOD_(DWORD, GetLOD)(THIS);
	STDMETHOD_(DWORD, GetLevelCount)(THIS);
	STDMETHOD(SetAutoGenFilterType)(THIS_ D3DTEXTUREFILTERTYPE FilterType);
	STDMETHOD_(D3DTEXTUREFILTERTYPE, GetAutoGenFilterType)(THIS);
	STDMETHOD_(void, GenerateMipSubLevels)(THIS);
	STDMETHOD(GetLevelDesc)(THIS_ UINT Level, D3DVOLUME_DESC *pDesc);
	STDMETHOD(GetVolumeLevel)(THIS_ UINT Level, IDirect3DVolume9** ppVolumeLevel);
	STDMETHOD(LockBox)(THIS_ UINT Level, D3DLOCKED_BOX* pLockedVolume, CONST D3DBOX* pBox, DWORD Flags);
	STDMETHOD(UnlockBox)(THIS_ UINT Level);
	STDMETHOD(AddDirtyBox)(THIS_ CONST D3DBOX* pDirtyBox);

	// Helper functions
	LPDIRECT3DVOLUMETEXTURE9 GetProxyInterface() const { return ProxyInterface; }
	void SetInterfaceValues(m_IDirect3DDevice9Ex* Device, REFIID, void*) { m_pDeviceEx = Device; }
};
