#pragma once

class m_IDirectInputDevice8 : public IDirectInputDevice8A, public IDirectInputDevice8W, public AddressLookupTableDinput8Object
{
private:
	IDirectInputDevice8W *ProxyInterface;
	const IID WrapperID;

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
	DWORD LastSentSequenceCounter = 0;
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

	template <class T>
	inline auto* GetProxyInterface() { return (T*)ProxyInterface; }

	template <class T, class V>
	inline HRESULT EnumObjectsT(V lpCallback, LPVOID pvRef, DWORD dwFlags);

	template <class T, class V>
	inline HRESULT GetObjectInfoT(V pdidoi, DWORD dwObj, DWORD dwHow);

	template <class T, class V>
	inline HRESULT GetDeviceInfoT(V pdidi);

	template <class T, class V>
	inline HRESULT EnumEffectsT(V lpCallback, LPVOID pvRef, DWORD dwEffType);

	template <class T, class V>
	inline HRESULT GetEffectInfoT(V pdei, REFGUID rguid);

	template <class T, class V>
	inline HRESULT EnumEffectsInFileT(V lpszFileName, LPDIENUMEFFECTSINFILECALLBACK pec, LPVOID pvRef, DWORD dwFlags);

	template <class T, class V>
	inline HRESULT WriteEffectToFileT(V lpszFileName, DWORD dwEntries, LPDIFILEEFFECT rgDiFileEft, DWORD dwFlags);

	template <class T, class V, class W>
	inline HRESULT BuildActionMapT(V lpdiaf, W lpszUserName, DWORD dwFlags);

	template <class T, class V, class W>
	inline HRESULT SetActionMapT(V lpdiActionFormat, W lptszUserName, DWORD dwFlags);

	template <class T, class V>
	inline HRESULT GetImageInfoT(V lpdiDevImageInfoHeader);

public:
	m_IDirectInputDevice8(IDirectInputDevice8W *aOriginal, REFIID riid) : ProxyInterface(aOriginal), WrapperID(riid)
	{
		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ")");

		if (IsEqualIID(riid, IID_IUnknown))
		{
			Logging::Log() << __FUNCTION__ << " Error: could not get riid when creating interface!";
		}

		ProcessID = GetCurrentProcessId();

		InitializeCriticalSection(&dics);

		ProxyAddressLookupTableDinput8.SaveAddress(this, ProxyInterface);
	}
	~m_IDirectInputDevice8()
	{
		LOG_LIMIT(3, __FUNCTION__ << " (" << this << ")" << " deleting interface!");

		DeleteCriticalSection(&dics);

		ProxyAddressLookupTableDinput8.DeleteAddress(this);
	}

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	/*** IDirectInputDevice8W methods ***/
	STDMETHOD(GetCapabilities)(THIS_ LPDIDEVCAPS);
	STDMETHOD(EnumObjects)(THIS_ LPDIENUMDEVICEOBJECTSCALLBACKA lpCallback, LPVOID pvRef, DWORD dwFlags)
	{
		return EnumObjectsT<IDirectInputDevice8A, LPDIENUMDEVICEOBJECTSCALLBACKA>(lpCallback, pvRef, dwFlags);
	}
	STDMETHOD(EnumObjects)(THIS_ LPDIENUMDEVICEOBJECTSCALLBACKW lpCallback, LPVOID pvRef, DWORD dwFlags)
	{
		return EnumObjectsT<IDirectInputDevice8W, LPDIENUMDEVICEOBJECTSCALLBACKW>(lpCallback, pvRef, dwFlags);
	}
	STDMETHOD(GetProperty)(THIS_ REFGUID, LPDIPROPHEADER);
	STDMETHOD(SetProperty)(THIS_ REFGUID, LPCDIPROPHEADER);
	STDMETHOD(Acquire)(THIS);
	STDMETHOD(Unacquire)(THIS);
	STDMETHOD(GetDeviceState)(THIS_ DWORD, LPVOID);
	template <class T>
	HRESULT GetMouseDeviceData(DWORD cbObjectData, LPDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD dwFlags, std::vector<T>& dod);
	STDMETHOD(GetDeviceData)(THIS_ DWORD, LPDIDEVICEOBJECTDATA, LPDWORD, DWORD);
	STDMETHOD(SetDataFormat)(THIS_ LPCDIDATAFORMAT);
	STDMETHOD(SetEventNotification)(THIS_ HANDLE);
	STDMETHOD(SetCooperativeLevel)(THIS_ HWND, DWORD);
	STDMETHOD(GetObjectInfo)(THIS_ LPDIDEVICEOBJECTINSTANCEA pdidoi, DWORD dwObj, DWORD dwHow)
	{
		return GetObjectInfoT<IDirectInputDevice8A, LPDIDEVICEOBJECTINSTANCEA>(pdidoi, dwObj, dwHow);
	}
	STDMETHOD(GetObjectInfo)(THIS_ LPDIDEVICEOBJECTINSTANCEW pdidoi, DWORD dwObj, DWORD dwHow)
	{
		return GetObjectInfoT<IDirectInputDevice8W, LPDIDEVICEOBJECTINSTANCEW>(pdidoi, dwObj, dwHow);
	}
	STDMETHOD(GetDeviceInfo)(THIS_ LPDIDEVICEINSTANCEA pdidi)
	{
		return GetDeviceInfoT<IDirectInputDevice8A, LPDIDEVICEINSTANCEA>(pdidi);
	}
	STDMETHOD(GetDeviceInfo)(THIS_ LPDIDEVICEINSTANCEW pdidi)
	{
		return GetDeviceInfoT<IDirectInputDevice8W, LPDIDEVICEINSTANCEW>(pdidi);
	}
	STDMETHOD(RunControlPanel)(THIS_ HWND, DWORD);
	STDMETHOD(Initialize)(THIS_ HINSTANCE, DWORD, REFGUID);
	STDMETHOD(CreateEffect)(THIS_ REFGUID, LPCDIEFFECT, LPDIRECTINPUTEFFECT *, LPUNKNOWN);
	STDMETHOD(EnumEffects)(THIS_ LPDIENUMEFFECTSCALLBACKA lpCallback, LPVOID pvRef, DWORD dwEffType)
	{
		return EnumEffectsT<IDirectInputDevice8A, LPDIENUMEFFECTSCALLBACKA>(lpCallback, pvRef, dwEffType);
	}
	STDMETHOD(EnumEffects)(THIS_ LPDIENUMEFFECTSCALLBACKW lpCallback, LPVOID pvRef, DWORD dwEffType)
	{
		return EnumEffectsT<IDirectInputDevice8W, LPDIENUMEFFECTSCALLBACKW>(lpCallback, pvRef, dwEffType);
	}
	STDMETHOD(GetEffectInfo)(THIS_ LPDIEFFECTINFOA pdei, REFGUID rguid)
	{
		return GetEffectInfoT<IDirectInputDevice8A, LPDIEFFECTINFOA>(pdei, rguid);
	}
	STDMETHOD(GetEffectInfo)(THIS_ LPDIEFFECTINFOW pdei, REFGUID rguid)
	{
		return GetEffectInfoT<IDirectInputDevice8W, LPDIEFFECTINFOW>(pdei, rguid);
	}
	STDMETHOD(GetForceFeedbackState)(THIS_ LPDWORD);
	STDMETHOD(SendForceFeedbackCommand)(THIS_ DWORD);
	STDMETHOD(EnumCreatedEffectObjects)(THIS_ LPDIENUMCREATEDEFFECTOBJECTSCALLBACK, LPVOID, DWORD);
	STDMETHOD(Escape)(THIS_ LPDIEFFESCAPE);
	STDMETHOD(Poll)(THIS);
	STDMETHOD(SendDeviceData)(THIS_ DWORD, LPCDIDEVICEOBJECTDATA, LPDWORD, DWORD);
	STDMETHOD(EnumEffectsInFile)(THIS_ LPCSTR lpszFileName, LPDIENUMEFFECTSINFILECALLBACK pec, LPVOID pvRef, DWORD dwFlags)
	{
		return EnumEffectsInFileT<IDirectInputDevice8A, LPCSTR>(lpszFileName, pec, pvRef, dwFlags);
	}
	STDMETHOD(EnumEffectsInFile)(THIS_ LPCWSTR lpszFileName, LPDIENUMEFFECTSINFILECALLBACK pec, LPVOID pvRef, DWORD dwFlags)
	{
		return EnumEffectsInFileT<IDirectInputDevice8W, LPCWSTR>(lpszFileName, pec, pvRef, dwFlags);
	}
	STDMETHOD(WriteEffectToFile)(THIS_ LPCSTR lpszFileName, DWORD dwEntries, LPDIFILEEFFECT rgDiFileEft, DWORD dwFlags)
	{
		return WriteEffectToFileT<IDirectInputDevice8A, LPCSTR>(lpszFileName, dwEntries, rgDiFileEft, dwFlags);
	}
	STDMETHOD(WriteEffectToFile)(THIS_ LPCWSTR lpszFileName, DWORD dwEntries, LPDIFILEEFFECT rgDiFileEft, DWORD dwFlags)
	{
		return WriteEffectToFileT<IDirectInputDevice8W, LPCWSTR>(lpszFileName, dwEntries, rgDiFileEft, dwFlags);
	}
	STDMETHOD(BuildActionMap)(THIS_ LPDIACTIONFORMATA lpdiaf, LPCSTR lpszUserName, DWORD dwFlags)
	{
		return BuildActionMapT<IDirectInputDevice8A, LPDIACTIONFORMATA, LPCSTR>(lpdiaf, lpszUserName, dwFlags);
	}
	STDMETHOD(BuildActionMap)(THIS_ LPDIACTIONFORMATW lpdiaf, LPCWSTR lpszUserName, DWORD dwFlags)
	{
		return BuildActionMapT<IDirectInputDevice8W, LPDIACTIONFORMATW, LPCWSTR>(lpdiaf, lpszUserName, dwFlags);
	}
	STDMETHOD(SetActionMap)(THIS_ LPDIACTIONFORMATA lpdiActionFormat, LPCSTR lptszUserName, DWORD dwFlags)
	{
		return SetActionMapT<IDirectInputDevice8A, LPDIACTIONFORMATA, LPCSTR>(lpdiActionFormat, lptszUserName, dwFlags);
	}
	STDMETHOD(SetActionMap)(THIS_ LPDIACTIONFORMATW lpdiActionFormat, LPCWSTR lptszUserName, DWORD dwFlags)
	{
		return SetActionMapT<IDirectInputDevice8W, LPDIACTIONFORMATW, LPCWSTR>(lpdiActionFormat, lptszUserName, dwFlags);
	}
	STDMETHOD(GetImageInfo)(THIS_ LPDIDEVICEIMAGEINFOHEADERA lpdiDevImageInfoHeader)
	{
		return GetImageInfoT<IDirectInputDevice8A, LPDIDEVICEIMAGEINFOHEADERA>(lpdiDevImageInfoHeader);
	}
	STDMETHOD(GetImageInfo)(THIS_ LPDIDEVICEIMAGEINFOHEADERW lpdiDevImageInfoHeader)
	{
		return GetImageInfoT<IDirectInputDevice8W, LPDIDEVICEIMAGEINFOHEADERW>(lpdiDevImageInfoHeader);
	}

	// Helper functions
	void SetAsMouse() { IsMouse = true; }
};
