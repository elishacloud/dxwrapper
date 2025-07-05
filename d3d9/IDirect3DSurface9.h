#pragma once

class m_IDirect3DSurface9 : public IDirect3DSurface9, public AddressLookupTableD3d9Object
{
private:
	LPDIRECT3DSURFACE9 ProxyInterface;
	m_IDirect3DDevice9Ex* m_pDeviceEx;
	const IID WrapperID = IID_IDirect3DSurface9;

	// For fake emulated locking
	D3DSURFACE_DESC Desc = {};
	struct {
		bool ReadOnly = false;
		RECT* pRect = nullptr;
		RECT Rect = {};
		m_IDirect3DSurface9* pSurface = nullptr;
	} Emu;

	m_IDirect3DSurface9* m_GetNonMultiSampledSurface(const RECT* pSurfaceRect, DWORD Flags);

public:
	m_IDirect3DSurface9(LPDIRECT3DSURFACE9 pSurface9, m_IDirect3DDevice9Ex* pDevice) : ProxyInterface(pSurface9), m_pDeviceEx(pDevice)
	{
		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ")");

		if (FAILED(GetDesc(&Desc)))
		{
			LOG_LIMIT(3, __FUNCTION__ << " Failed to GetDesc()!" << this << ")");
		}

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

	/*** IDirect3DResource9 methods ***/
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

	// Helper functions
	LPDIRECT3DSURFACE9 GetProxyInterface() const { return ProxyInterface; }
	LPDIRECT3DSURFACE9 GetNonMultiSampledSurface(const RECT* pSurfaceRect, DWORD Flags)
	{
		if (Desc.MultiSampleType && !(Desc.Usage & D3DUSAGE_DEPTHSTENCIL))
		{
			m_IDirect3DSurface9* pSurface = m_GetNonMultiSampledSurface(pSurfaceRect, Flags);
			if (pSurface)
			{
				return pSurface->GetProxyInterface();
			}
			LOG_LIMIT(100, __FUNCTION__ << " Error: getting non-multi-sampled surface!");
		}
		return ProxyInterface;
	}
	HRESULT RestoreMultiSampleData();
	void SetInterfaceValues(m_IDirect3DDevice9Ex* Device, REFIID, void*) { m_pDeviceEx = Device; }
};
