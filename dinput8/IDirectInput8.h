#pragma once

#include <chrono>
#include <vector>

class m_IDirectInput8 : public IDirectInput8A, public IDirectInput8W, public AddressLookupTableDinput8Object
{
private:
	IDirectInput8W *ProxyInterface;
	REFIID WrapperID;
	REFIID WrapperDeviceID;

	const std::chrono::seconds cacheDuration = std::chrono::seconds(5); // Cache duration in seconds

	// Define a template structure to hold cached device data
	template <class T, class V>
	struct CachedDeviceDataT
	{
		std::chrono::steady_clock::time_point lastUpdate;
		DWORD dwDevType = 0;
		DWORD dwFlags = 0;
		std::vector<T> devices;
	};
	std::vector<int> devices;
	
	// Global or class member variable to store the cache for each template instantiation
	CachedDeviceDataT<DIDEVICEINSTANCEA, LPDIENUMDEVICESCALLBACKA> cachedDataA;
	CachedDeviceDataT<DIDEVICEINSTANCEW, LPDIENUMDEVICESCALLBACKW> cachedDataW;

	auto& GetEnumCache(IDirectInput8A*) { return cachedDataA; }
	auto& GetEnumCache(IDirectInput8W*) { return cachedDataW; }

	template <class T>
	inline auto* GetProxyInterface() { return (T*)ProxyInterface; }

	template <class T, class V>
	inline HRESULT CreateDeviceT(REFGUID rguid, V lplpDirectInputDevice, LPUNKNOWN pUnkOuter);

	template <class T, class V, class D>
	inline HRESULT EnumDevicesT(DWORD dwDevType, V lpCallback, LPVOID pvRef, DWORD dwFlags);

	template <class T, class V>
	inline HRESULT FindDeviceT(REFGUID rguidClass, V ptszName, LPGUID pguidInstance);

	template <class T, class V, class W, class X, class C, class D>
	inline HRESULT EnumDevicesBySemanticsT(V ptszUserName, W lpdiActionFormat, X lpCallback, LPVOID pvRef, DWORD dwFlags);

	template <class T, class V>
	inline HRESULT ConfigureDevicesT(LPDICONFIGUREDEVICESCALLBACK lpdiCallback, V lpdiCDParams, DWORD dwFlags, LPVOID pvRefData);

public:
	m_IDirectInput8(IDirectInput8W *aOriginal, REFIID riid) : ProxyInterface(aOriginal), WrapperID(riid),
		WrapperDeviceID(WrapperID == IID_IDirectInput8A ? IID_IDirectInputDevice8A : WrapperID == IID_IDirectInput8W ? IID_IDirectInputDevice8W : IID_IUnknown)
	{
		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ")");

		if (IsEqualIID(riid, IID_IUnknown))
		{
			Logging::Log() << __FUNCTION__ << " Error: could not get riid when creating interface!";
		}

		ProxyAddressLookupTableDinput8.SaveAddress(this, ProxyInterface);
	}
	~m_IDirectInput8()
	{
		LOG_LIMIT(3, __FUNCTION__ << " (" << this << ")" << " deleting interface!");

		ProxyAddressLookupTableDinput8.DeleteAddress(this);
	}

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	/*** IDirectInput8W methods ***/
	STDMETHOD(CreateDevice)(THIS_ REFGUID rguid, LPDIRECTINPUTDEVICE8A* lplpDirectInputDevice, LPUNKNOWN pUnkOuter)
	{
		return CreateDeviceT<IDirectInput8A, LPDIRECTINPUTDEVICE8A*>(rguid, lplpDirectInputDevice, pUnkOuter);
	}
	STDMETHOD(CreateDevice)(THIS_ REFGUID rguid, LPDIRECTINPUTDEVICE8W* lplpDirectInputDevice, LPUNKNOWN pUnkOuter)
	{
		return CreateDeviceT<IDirectInput8W, LPDIRECTINPUTDEVICE8W*>(rguid, lplpDirectInputDevice, pUnkOuter);
	}
	STDMETHOD(EnumDevices)(THIS_ DWORD dwDevType, LPDIENUMDEVICESCALLBACKA lpCallback, LPVOID pvRef, DWORD dwFlags)
	{
		return EnumDevicesT<IDirectInput8A, LPDIENUMDEVICESCALLBACKA, DIDEVICEINSTANCEA>(dwDevType, lpCallback, pvRef, dwFlags);
	}
	STDMETHOD(EnumDevices)(THIS_ DWORD dwDevType, LPDIENUMDEVICESCALLBACKW lpCallback, LPVOID pvRef, DWORD dwFlags)
	{
		return EnumDevicesT<IDirectInput8W, LPDIENUMDEVICESCALLBACKW, DIDEVICEINSTANCEW>(dwDevType, lpCallback, pvRef, dwFlags);
	}
	STDMETHOD(GetDeviceStatus)(THIS_ REFGUID);
	STDMETHOD(RunControlPanel)(THIS_ HWND, DWORD);
	STDMETHOD(Initialize)(THIS_ HINSTANCE, DWORD);
	STDMETHOD(FindDevice)(THIS_ REFGUID rguidClass, LPCSTR ptszName, LPGUID pguidInstance)
	{
		return FindDeviceT<IDirectInput8A, LPCSTR>(rguidClass, ptszName, pguidInstance);
	}
	STDMETHOD(FindDevice)(THIS_ REFGUID rguidClass, LPCWSTR ptszName, LPGUID pguidInstance)
	{
		return FindDeviceT<IDirectInput8W, LPCWSTR>(rguidClass, ptszName, pguidInstance);
	}
	STDMETHOD(EnumDevicesBySemantics)(THIS_ LPCSTR ptszUserName, LPDIACTIONFORMATA lpdiActionFormat, LPDIENUMDEVICESBYSEMANTICSCBA lpCallback, LPVOID pvRef, DWORD dwFlags)
	{
		return EnumDevicesBySemanticsT<IDirectInput8A, LPCSTR, LPDIACTIONFORMATA, LPDIENUMDEVICESBYSEMANTICSCBA, LPCDIDEVICEINSTANCEA, LPDIRECTINPUTDEVICE8A>(ptszUserName, lpdiActionFormat, lpCallback, pvRef, dwFlags);
	}
	STDMETHOD(EnumDevicesBySemantics)(THIS_ LPCWSTR ptszUserName, LPDIACTIONFORMATW lpdiActionFormat, LPDIENUMDEVICESBYSEMANTICSCBW lpCallback, LPVOID pvRef, DWORD dwFlags)
	{
		return EnumDevicesBySemanticsT<IDirectInput8W, LPCWSTR, LPDIACTIONFORMATW, LPDIENUMDEVICESBYSEMANTICSCBW, LPCDIDEVICEINSTANCEW, LPDIRECTINPUTDEVICE8W>(ptszUserName, lpdiActionFormat, lpCallback, pvRef, dwFlags);
	}
	STDMETHOD(ConfigureDevices)(THIS_ LPDICONFIGUREDEVICESCALLBACK lpdiCallback, LPDICONFIGUREDEVICESPARAMSA lpdiCDParams, DWORD dwFlags, LPVOID pvRefData)
	{
		return ConfigureDevicesT<IDirectInput8A, LPDICONFIGUREDEVICESPARAMSA>(lpdiCallback, lpdiCDParams, dwFlags, pvRefData);
	}
	STDMETHOD(ConfigureDevices)(THIS_ LPDICONFIGUREDEVICESCALLBACK lpdiCallback, LPDICONFIGUREDEVICESPARAMSW lpdiCDParams, DWORD dwFlags, LPVOID pvRefData)
	{
		return ConfigureDevicesT<IDirectInput8W, LPDICONFIGUREDEVICESPARAMSW>(lpdiCallback, lpdiCDParams, dwFlags, pvRefData);
	}
};
