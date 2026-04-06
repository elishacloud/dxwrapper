#pragma once

class m_IDirectInputDevice8 final : public IDirectInputDevice8A, public IDirectInputDevice8W, AddressLookupTableDinput8Object<m_IDirectInputDevice8>, ModuleObjectCount::CountedObject
{
public:
	// Factory traits
	static inline const CLSID wrapper_clsid = CLSID_DirectInputDevice8;

	static inline const CLSID proxy_clsid = CLSID_DirectInputDevice8;
	static inline const IID proxy_iid = IID_IDirectInputDevice8W;
	using proxy_type = IDirectInputDevice8W;

private:
	proxy_type* ProxyInterface;
	IDirectInputDevice8A* ProxyInterfaceA; // Non-owning alias

	volatile LONG RefCount = 1;

	CRITICAL_SECTION dics = {};

	DWORD ProcessID;

	struct MOUSECACHEDATA_DX3 {
		DWORD dwOfs;
		LONG lData;
		DWORD dwTimeStamp;
		DWORD dwSequence;
	};

	struct MOUSECACHEDATA {
		DWORD dwOfs;
		LONG lData;
		DWORD dwTimeStamp;
		DWORD dwSequence;
		UINT_PTR uAppData;
	};

	bool IsMouse = false;
	bool SentBufferOverflow = false;
	DWORD MouseBufferSize = 0;
	DWORD RequestedMouseBufferSize = 0;
	DWORD SequenceCounter = 0;
	DWORD LastObjectSize = 0;
	struct {
		DWORD x = DIMOFS_X;
		DWORD y = DIMOFS_Y;
		DWORD z = DIMOFS_Z;
	} Ofs;
	std::vector<DWORD> cachedAxisOffsets;
	std::vector<MOUSECACHEDATA_DX3> dod_dx3;
	std::vector<MOUSECACHEDATA> dod_dx8;
	std::vector<BYTE> tmp_dod;

	template <class T, class V>
	inline HRESULT EnumObjectsT(T* ProxyInterfaceT, V lpCallback, LPVOID pvRef, DWORD dwFlags);

	template <class T, class V>
	inline HRESULT GetObjectInfoT(T* ProxyInterfaceT, V pdidoi, DWORD dwObj, DWORD dwHow);

	template <class T, class V>
	inline HRESULT GetDeviceInfoT(T* ProxyInterfaceT, V pdidi);

	template <class T, class V>
	inline HRESULT EnumEffectsT(T* ProxyInterfaceT, V lpCallback, LPVOID pvRef, DWORD dwEffType);

	template <class T, class V>
	inline HRESULT GetEffectInfoT(T* ProxyInterfaceT, V pdei, REFGUID rguid);

	template <class T, class V>
	inline HRESULT EnumEffectsInFileT(T* ProxyInterfaceT, V lpszFileName, LPDIENUMEFFECTSINFILECALLBACK pec, LPVOID pvRef, DWORD dwFlags);

	template <class T, class V>
	inline HRESULT WriteEffectToFileT(T* ProxyInterfaceT, V lpszFileName, DWORD dwEntries, LPDIFILEEFFECT rgDiFileEft, DWORD dwFlags);

	template <class T, class V, class W>
	inline HRESULT BuildActionMapT(T* ProxyInterfaceT, V lpdiaf, W lpszUserName, DWORD dwFlags);

	template <class T, class V, class W>
	inline HRESULT SetActionMapT(T* ProxyInterfaceT, V lpdiActionFormat, W lptszUserName, DWORD dwFlags);

	template <class T, class V>
	inline HRESULT GetImageInfoT(T* ProxyInterfaceT, V lpdiDevImageInfoHeader);

public:
	m_IDirectInputDevice8(IUnknown* aOriginal) : AddressLookupTableDinput8Object(aOriginal)
	{
		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ")");

		aOriginal->QueryInterface(IID_IDirectInputDevice8A, reinterpret_cast<LPVOID*>(&ProxyInterfaceA));
		aOriginal->QueryInterface(IID_IDirectInputDevice8W, reinterpret_cast<LPVOID*>(&ProxyInterface));
		if (aOriginal == ProxyInterface || aOriginal == ProxyInterfaceA)
		{
			aOriginal->Release();
		}
		else
		{
			LOG_LIMIT(3, __FUNCTION__ << " Warning: passed interface does not match either ProxyInterface values!");
		}

		ProcessID = GetCurrentProcessId();

		InitializeCriticalSection(&dics);
	}
	~m_IDirectInputDevice8()
	{
		LOG_LIMIT(3, __FUNCTION__ << " (" << this << ")" << " deleting interface!");

		DeleteCriticalSection(&dics);

		ProxyInterfaceA->Release();
		ProxyInterface->Release();
	}

	/*** IUnknown methods ***/
	IFACEMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) override;
	IFACEMETHOD_(ULONG, AddRef)(THIS) override;
	IFACEMETHOD_(ULONG, Release)(THIS) override;

	/*** IDirectInputDevice8W methods ***/
	IFACEMETHOD(GetCapabilities)(THIS_ LPDIDEVCAPS) override;
	IFACEMETHOD(EnumObjects)(THIS_ LPDIENUMDEVICEOBJECTSCALLBACKA lpCallback, LPVOID pvRef, DWORD dwFlags) override
	{
		return EnumObjectsT<IDirectInputDevice8A, LPDIENUMDEVICEOBJECTSCALLBACKA>(ProxyInterfaceA, lpCallback, pvRef, dwFlags);
	}
	IFACEMETHOD(EnumObjects)(THIS_ LPDIENUMDEVICEOBJECTSCALLBACKW lpCallback, LPVOID pvRef, DWORD dwFlags) override
	{
		return EnumObjectsT<IDirectInputDevice8W, LPDIENUMDEVICEOBJECTSCALLBACKW>(ProxyInterface, lpCallback, pvRef, dwFlags);
	}
	IFACEMETHOD(GetProperty)(THIS_ REFGUID, LPDIPROPHEADER) override;
	IFACEMETHOD(SetProperty)(THIS_ REFGUID, LPCDIPROPHEADER) override;
	IFACEMETHOD(Acquire)(THIS) override;
	IFACEMETHOD(Unacquire)(THIS) override;
	IFACEMETHOD(GetDeviceState)(THIS_ DWORD, LPVOID) override;
	IFACEMETHOD(GetDeviceData)(THIS_ DWORD, LPDIDEVICEOBJECTDATA, LPDWORD, DWORD) override;
	IFACEMETHOD(SetDataFormat)(THIS_ LPCDIDATAFORMAT) override;
	IFACEMETHOD(SetEventNotification)(THIS_ HANDLE) override;
	IFACEMETHOD(SetCooperativeLevel)(THIS_ HWND, DWORD) override;
	IFACEMETHOD(GetObjectInfo)(THIS_ LPDIDEVICEOBJECTINSTANCEA pdidoi, DWORD dwObj, DWORD dwHow) override
	{
		return GetObjectInfoT<IDirectInputDevice8A, LPDIDEVICEOBJECTINSTANCEA>(ProxyInterfaceA, pdidoi, dwObj, dwHow);
	}
	IFACEMETHOD(GetObjectInfo)(THIS_ LPDIDEVICEOBJECTINSTANCEW pdidoi, DWORD dwObj, DWORD dwHow) override
	{
		return GetObjectInfoT<IDirectInputDevice8W, LPDIDEVICEOBJECTINSTANCEW>(ProxyInterface, pdidoi, dwObj, dwHow);
	}
	IFACEMETHOD(GetDeviceInfo)(THIS_ LPDIDEVICEINSTANCEA pdidi) override
	{
		return GetDeviceInfoT<IDirectInputDevice8A, LPDIDEVICEINSTANCEA>(ProxyInterfaceA, pdidi);
	}
	IFACEMETHOD(GetDeviceInfo)(THIS_ LPDIDEVICEINSTANCEW pdidi) override
	{
		return GetDeviceInfoT<IDirectInputDevice8W, LPDIDEVICEINSTANCEW>(ProxyInterface, pdidi);
	}
	IFACEMETHOD(RunControlPanel)(THIS_ HWND, DWORD) override;
	IFACEMETHOD(Initialize)(THIS_ HINSTANCE, DWORD, REFGUID) override;
	IFACEMETHOD(CreateEffect)(THIS_ REFGUID, LPCDIEFFECT, LPDIRECTINPUTEFFECT *, LPUNKNOWN) override;
	IFACEMETHOD(EnumEffects)(THIS_ LPDIENUMEFFECTSCALLBACKA lpCallback, LPVOID pvRef, DWORD dwEffType) override
	{
		return EnumEffectsT<IDirectInputDevice8A, LPDIENUMEFFECTSCALLBACKA>(ProxyInterfaceA, lpCallback, pvRef, dwEffType);
	}
	IFACEMETHOD(EnumEffects)(THIS_ LPDIENUMEFFECTSCALLBACKW lpCallback, LPVOID pvRef, DWORD dwEffType) override
	{
		return EnumEffectsT<IDirectInputDevice8W, LPDIENUMEFFECTSCALLBACKW>(ProxyInterface, lpCallback, pvRef, dwEffType);
	}
	IFACEMETHOD(GetEffectInfo)(THIS_ LPDIEFFECTINFOA pdei, REFGUID rguid) override
	{
		return GetEffectInfoT<IDirectInputDevice8A, LPDIEFFECTINFOA>(ProxyInterfaceA, pdei, rguid);
	}
	IFACEMETHOD(GetEffectInfo)(THIS_ LPDIEFFECTINFOW pdei, REFGUID rguid) override
	{
		return GetEffectInfoT<IDirectInputDevice8W, LPDIEFFECTINFOW>(ProxyInterface, pdei, rguid);
	}
	IFACEMETHOD(GetForceFeedbackState)(THIS_ LPDWORD) override;
	IFACEMETHOD(SendForceFeedbackCommand)(THIS_ DWORD) override;
	IFACEMETHOD(EnumCreatedEffectObjects)(THIS_ LPDIENUMCREATEDEFFECTOBJECTSCALLBACK, LPVOID, DWORD) override;
	IFACEMETHOD(Escape)(THIS_ LPDIEFFESCAPE) override;
	IFACEMETHOD(Poll)(THIS) override;
	IFACEMETHOD(SendDeviceData)(THIS_ DWORD, LPCDIDEVICEOBJECTDATA, LPDWORD, DWORD) override;
	IFACEMETHOD(EnumEffectsInFile)(THIS_ LPCSTR lpszFileName, LPDIENUMEFFECTSINFILECALLBACK pec, LPVOID pvRef, DWORD dwFlags) override
	{
		return EnumEffectsInFileT<IDirectInputDevice8A, LPCSTR>(ProxyInterfaceA, lpszFileName, pec, pvRef, dwFlags);
	}
	IFACEMETHOD(EnumEffectsInFile)(THIS_ LPCWSTR lpszFileName, LPDIENUMEFFECTSINFILECALLBACK pec, LPVOID pvRef, DWORD dwFlags) override
	{
		return EnumEffectsInFileT<IDirectInputDevice8W, LPCWSTR>(ProxyInterface, lpszFileName, pec, pvRef, dwFlags);
	}
	IFACEMETHOD(WriteEffectToFile)(THIS_ LPCSTR lpszFileName, DWORD dwEntries, LPDIFILEEFFECT rgDiFileEft, DWORD dwFlags) override
	{
		return WriteEffectToFileT<IDirectInputDevice8A, LPCSTR>(ProxyInterfaceA, lpszFileName, dwEntries, rgDiFileEft, dwFlags);
	}
	IFACEMETHOD(WriteEffectToFile)(THIS_ LPCWSTR lpszFileName, DWORD dwEntries, LPDIFILEEFFECT rgDiFileEft, DWORD dwFlags) override
	{
		return WriteEffectToFileT<IDirectInputDevice8W, LPCWSTR>(ProxyInterface, lpszFileName, dwEntries, rgDiFileEft, dwFlags);
	}
	IFACEMETHOD(BuildActionMap)(THIS_ LPDIACTIONFORMATA lpdiaf, LPCSTR lpszUserName, DWORD dwFlags) override
	{
		return BuildActionMapT<IDirectInputDevice8A, LPDIACTIONFORMATA, LPCSTR>(ProxyInterfaceA, lpdiaf, lpszUserName, dwFlags);
	}
	IFACEMETHOD(BuildActionMap)(THIS_ LPDIACTIONFORMATW lpdiaf, LPCWSTR lpszUserName, DWORD dwFlags) override
	{
		return BuildActionMapT<IDirectInputDevice8W, LPDIACTIONFORMATW, LPCWSTR>(ProxyInterface, lpdiaf, lpszUserName, dwFlags);
	}
	IFACEMETHOD(SetActionMap)(THIS_ LPDIACTIONFORMATA lpdiActionFormat, LPCSTR lptszUserName, DWORD dwFlags) override
	{
		return SetActionMapT<IDirectInputDevice8A, LPDIACTIONFORMATA, LPCSTR>(ProxyInterfaceA, lpdiActionFormat, lptszUserName, dwFlags);
	}
	IFACEMETHOD(SetActionMap)(THIS_ LPDIACTIONFORMATW lpdiActionFormat, LPCWSTR lptszUserName, DWORD dwFlags) override
	{
		return SetActionMapT<IDirectInputDevice8W, LPDIACTIONFORMATW, LPCWSTR>(ProxyInterface, lpdiActionFormat, lptszUserName, dwFlags);
	}
	IFACEMETHOD(GetImageInfo)(THIS_ LPDIDEVICEIMAGEINFOHEADERA lpdiDevImageInfoHeader) override
	{
		return GetImageInfoT<IDirectInputDevice8A, LPDIDEVICEIMAGEINFOHEADERA>(ProxyInterfaceA, lpdiDevImageInfoHeader);
	}
	IFACEMETHOD(GetImageInfo)(THIS_ LPDIDEVICEIMAGEINFOHEADERW lpdiDevImageInfoHeader) override
	{
		return GetImageInfoT<IDirectInputDevice8W, LPDIDEVICEIMAGEINFOHEADERW>(ProxyInterface, lpdiDevImageInfoHeader);
	}

	// Helper functions
	template <class T>
	HRESULT GetMouseDeviceData(DWORD cbObjectData, LPDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD dwFlags, std::vector<T>& dod);
	void SetAsMouse() { IsMouse = true; }
	void AdjustMouseAxis(LONG& value, bool isY);
};
