#pragma once

#include <chrono>
#include <vector>

class m_IDirectInput8 final : public IDirectInput8A, public IDirectInput8W, AddressLookupTableDinput8Object<m_IDirectInput8>, ModuleObjectCount::CountedObject
{
public:
	// Factory traits
	static inline const CLSID wrapper_clsid = CLSID_DirectInput8;

	static inline const CLSID proxy_clsid = CLSID_DirectInput8;
	static inline const IID proxy_iid = IID_IDirectInput8W;
	using proxy_type = IDirectInput8W;

private:
	proxy_type* ProxyInterface;
	IDirectInput8A* ProxyInterfaceA; // Non-owning alias

	volatile LONG RefCount = 1;

	const std::chrono::seconds cacheDuration = std::chrono::seconds(Config.DeviceLookupCacheTime); // Cache duration in seconds

	// Define a template structure to hold cached device data
	template <class T, class V>
	struct CachedDeviceDataT
	{
		std::chrono::steady_clock::time_point lastUpdate = std::chrono::steady_clock::now() -
			std::chrono::seconds(Config.DeviceLookupCacheTime ? Config.DeviceLookupCacheTime - 1 : 0);
		DWORD dwDevType = (DWORD)-1;
		DWORD dwFlags = (DWORD)-1;
		std::vector<T> devices;
	};
	
	// Global or class member variable to store the cache for each template instantiation
	CachedDeviceDataT<DIDEVICEINSTANCEA, LPDIENUMDEVICESCALLBACKA> cachedDataA;
	CachedDeviceDataT<DIDEVICEINSTANCEW, LPDIENUMDEVICESCALLBACKW> cachedDataW;

	auto& GetEnumCache(IDirectInput8A*) { return cachedDataA; }
	auto& GetEnumCache(IDirectInput8W*) { return cachedDataW; }

	template <class T, class V>
	inline HRESULT CreateDeviceT(T* ProxyInterfaceT, REFGUID rguid, V lplpDirectInputDevice, LPUNKNOWN pUnkOuter);

	template <class T, class V, class D>
	inline HRESULT EnumDevicesT(T* ProxyInterfaceT, DWORD dwDevType, V lpCallback, LPVOID pvRef, DWORD dwFlags);

	template <class T, class V>
	inline HRESULT FindDeviceT(T* ProxyInterfaceT, REFGUID rguidClass, V ptszName, LPGUID pguidInstance);

	template <class T, class V, class W, class X, class C, class D>
	inline HRESULT EnumDevicesBySemanticsT(T* ProxyInterfaceT, V ptszUserName, W lpdiActionFormat, X lpCallback, LPVOID pvRef, DWORD dwFlags);

	template <class T, class V>
	inline HRESULT ConfigureDevicesT(T* ProxyInterfaceT, LPDICONFIGUREDEVICESCALLBACK lpdiCallback, V lpdiCDParams, DWORD dwFlags, LPVOID pvRefData);

public:
	m_IDirectInput8(IUnknown* aOriginal) : AddressLookupTableDinput8Object(aOriginal)
	{
		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ")");

		aOriginal->QueryInterface(IID_IDirectInput8A, reinterpret_cast<LPVOID*>(&ProxyInterfaceA));
		aOriginal->QueryInterface(IID_IDirectInput8W, reinterpret_cast<LPVOID*>(&ProxyInterface));
		if (aOriginal == ProxyInterface || aOriginal == ProxyInterfaceA)
		{
			aOriginal->Release();
		}
		else
		{
			LOG_LIMIT(3, __FUNCTION__ << " Warning: passed interface does not match either ProxyInterface values!");
		}
	}
	~m_IDirectInput8()
	{
		LOG_LIMIT(3, __FUNCTION__ << " (" << this << ")" << " deleting interface!");

		ProxyInterfaceA->Release();
		ProxyInterface->Release();
	}

	/*** IUnknown methods ***/
	IFACEMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) override;
	IFACEMETHOD_(ULONG, AddRef)(THIS) override;
	IFACEMETHOD_(ULONG, Release)(THIS) override;

	/*** IDirectInput8W methods ***/
	IFACEMETHOD(CreateDevice)(THIS_ REFGUID rguid, LPDIRECTINPUTDEVICE8A* lplpDirectInputDevice, LPUNKNOWN pUnkOuter) override
	{
		return CreateDeviceT<IDirectInput8A, LPDIRECTINPUTDEVICE8A*>(ProxyInterfaceA, rguid, lplpDirectInputDevice, pUnkOuter);
	}
	IFACEMETHOD(CreateDevice)(THIS_ REFGUID rguid, LPDIRECTINPUTDEVICE8W* lplpDirectInputDevice, LPUNKNOWN pUnkOuter) override
	{
		return CreateDeviceT<IDirectInput8W, LPDIRECTINPUTDEVICE8W*>(ProxyInterface, rguid, lplpDirectInputDevice, pUnkOuter);
	}
	IFACEMETHOD(EnumDevices)(THIS_ DWORD dwDevType, LPDIENUMDEVICESCALLBACKA lpCallback, LPVOID pvRef, DWORD dwFlags) override
	{
		return EnumDevicesT<IDirectInput8A, LPDIENUMDEVICESCALLBACKA, DIDEVICEINSTANCEA>(ProxyInterfaceA, dwDevType, lpCallback, pvRef, dwFlags);
	}
	IFACEMETHOD(EnumDevices)(THIS_ DWORD dwDevType, LPDIENUMDEVICESCALLBACKW lpCallback, LPVOID pvRef, DWORD dwFlags) override
	{
		return EnumDevicesT<IDirectInput8W, LPDIENUMDEVICESCALLBACKW, DIDEVICEINSTANCEW>(ProxyInterface, dwDevType, lpCallback, pvRef, dwFlags);
	}
	IFACEMETHOD(GetDeviceStatus)(THIS_ REFGUID) override;
	IFACEMETHOD(RunControlPanel)(THIS_ HWND, DWORD) override;
	IFACEMETHOD(Initialize)(THIS_ HINSTANCE, DWORD) override;
	IFACEMETHOD(FindDevice)(THIS_ REFGUID rguidClass, LPCSTR ptszName, LPGUID pguidInstance) override
	{
		return FindDeviceT<IDirectInput8A, LPCSTR>(ProxyInterfaceA, rguidClass, ptszName, pguidInstance);
	}
	IFACEMETHOD(FindDevice)(THIS_ REFGUID rguidClass, LPCWSTR ptszName, LPGUID pguidInstance) override
	{
		return FindDeviceT<IDirectInput8W, LPCWSTR>(ProxyInterface, rguidClass, ptszName, pguidInstance);
	}
	IFACEMETHOD(EnumDevicesBySemantics)(THIS_ LPCSTR ptszUserName, LPDIACTIONFORMATA lpdiActionFormat, LPDIENUMDEVICESBYSEMANTICSCBA lpCallback, LPVOID pvRef, DWORD dwFlags) override
	{
		return EnumDevicesBySemanticsT<IDirectInput8A, LPCSTR, LPDIACTIONFORMATA, LPDIENUMDEVICESBYSEMANTICSCBA, LPCDIDEVICEINSTANCEA, LPDIRECTINPUTDEVICE8A>(ProxyInterfaceA, ptszUserName, lpdiActionFormat, lpCallback, pvRef, dwFlags);
	}
	IFACEMETHOD(EnumDevicesBySemantics)(THIS_ LPCWSTR ptszUserName, LPDIACTIONFORMATW lpdiActionFormat, LPDIENUMDEVICESBYSEMANTICSCBW lpCallback, LPVOID pvRef, DWORD dwFlags) override
	{
		return EnumDevicesBySemanticsT<IDirectInput8W, LPCWSTR, LPDIACTIONFORMATW, LPDIENUMDEVICESBYSEMANTICSCBW, LPCDIDEVICEINSTANCEW, LPDIRECTINPUTDEVICE8W>(ProxyInterface, ptszUserName, lpdiActionFormat, lpCallback, pvRef, dwFlags);
	}
	IFACEMETHOD(ConfigureDevices)(THIS_ LPDICONFIGUREDEVICESCALLBACK lpdiCallback, LPDICONFIGUREDEVICESPARAMSA lpdiCDParams, DWORD dwFlags, LPVOID pvRefData) override
	{
		return ConfigureDevicesT<IDirectInput8A, LPDICONFIGUREDEVICESPARAMSA>(ProxyInterfaceA, lpdiCallback, lpdiCDParams, dwFlags, pvRefData);
	}
	IFACEMETHOD(ConfigureDevices)(THIS_ LPDICONFIGUREDEVICESCALLBACK lpdiCallback, LPDICONFIGUREDEVICESPARAMSW lpdiCDParams, DWORD dwFlags, LPVOID pvRefData) override
	{
		return ConfigureDevicesT<IDirectInput8W, LPDICONFIGUREDEVICESPARAMSW>(ProxyInterface, lpdiCallback, lpdiCDParams, dwFlags, pvRefData);
	}
};
