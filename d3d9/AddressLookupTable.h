#pragma once

#include "Settings\Settings.h"
#include "Logging\Logging.h"

class AddressLookupTableD3d9Object
{
public:
	virtual ~AddressLookupTableD3d9Object() {}
};

class AddressLookupTableD3d9
{
private:
	static constexpr size_t MaxCacheIndex = 15;

	bool ConstructorFlag = false;
	std::unordered_map<void*, class AddressLookupTableD3d9Object*> g_map[MaxCacheIndex];

	template <typename T>
	struct AddressCacheIndex {};
	template <>
	struct AddressCacheIndex<m_IDirect3DCubeTexture9> { static constexpr size_t CacheIndex = 0; };
	template <>
	struct AddressCacheIndex<m_IDirect3DIndexBuffer9> { static constexpr size_t CacheIndex = 1; };
	template <>
	struct AddressCacheIndex<m_IDirect3DPixelShader9> { static constexpr size_t CacheIndex = 2; };
	template <>
	struct AddressCacheIndex<m_IDirect3DQuery9> { static constexpr size_t CacheIndex = 3; };
	template <>
	struct AddressCacheIndex<m_IDirect3DStateBlock9> { static constexpr size_t CacheIndex = 4; };
	template <>
	struct AddressCacheIndex<m_IDirect3DSurface9> { static constexpr size_t CacheIndex = 5; };
	template <>
	struct AddressCacheIndex<m_IDirect3DSwapChain9Ex> { static constexpr size_t CacheIndex = 6; };
	template <>
	struct AddressCacheIndex<m_IDirect3DTexture9> { static constexpr size_t CacheIndex = 7; };
	template <>
	struct AddressCacheIndex<m_IDirect3DVertexBuffer9> { static constexpr size_t CacheIndex = 8; };
	template <>
	struct AddressCacheIndex<m_IDirect3DVertexDeclaration9> { static constexpr size_t CacheIndex = 9; };
	template <>
	struct AddressCacheIndex<m_IDirect3DVertexShader9> { static constexpr size_t CacheIndex = 10; };
	template <>
	struct AddressCacheIndex<m_IDirect3DVolume9> { static constexpr size_t CacheIndex = 11; };
	template <>
	struct AddressCacheIndex<m_IDirect3DVolumeTexture9> { static constexpr size_t CacheIndex = 12; };
	template <>
	struct AddressCacheIndex<m_IDirect3DVideoDevice9> { static constexpr size_t CacheIndex = 13; };
	template <>
	struct AddressCacheIndex<m_IDirect3DDXVADevice9> { static constexpr size_t CacheIndex = 14; };

	// General template function for CreateInterface
	template <typename T, typename D, typename L>
	inline T* CreateInterface(T* Proxy, D* Device, REFIID riid, L Data);

	template <typename T>
	inline IUnknown* GetIndentityInterface(IUnknown* Proxy)
	{
		ComPtr<IUnknown> pAddress;
		if (SUCCEEDED(Proxy->QueryInterface(IID_IUnknown, (void**)pAddress.GetAddressOf())) && pAddress.Get())
		{
			return pAddress.Get();
		}

		return Proxy;
	}

	template <typename T>
	inline T* FindInterface(IUnknown* Proxy)
	{
		constexpr size_t CacheIndex = AddressCacheIndex<T>::CacheIndex;

		auto it = g_map[CacheIndex].find(GetIndentityInterface<T>(Proxy));
		if (it != std::end(g_map[CacheIndex]))
		{
			T* addr = static_cast<T*>(it->second);
			addr->AddRef();
			Proxy->Release();
			return addr;
		}

		return nullptr;
	}

	void DeleteAll()
	{
		for (const auto& map : g_map)
		{
			for (const auto& entry : map)
			{
				delete entry.second;
			}
		}
	}

public:
	explicit AddressLookupTableD3d9() {}
	~AddressLookupTableD3d9()
	{
		ConstructorFlag = true;
		DeleteAll();
	}

	template <typename T, typename D, typename L>
	T* FindCreateAddress(IUnknown* Proxy, D* Device, REFIID riid, L Data)
	{
		if (!Proxy)
		{
			return nullptr;
		}

		T* addr = FindInterface<T>(Proxy);
		if (addr)
		{
			addr->InitInterface(Device, riid, Data);
			return addr;
		}

		IUnknown* object = nullptr;
		if (SUCCEEDED(Proxy->QueryInterface(riid, (void**)&object)) && object)
		{
			Proxy->Release();
			return CreateInterface((T*)object, Device, riid, Data);
		}

		return CreateInterface((T*)Proxy, Device, riid, Data);
	}

	template <typename T>
	T* FindAddress(IUnknown* Proxy)
	{
		if (!Proxy)
		{
			return nullptr;
		}

		T* addr = FindInterface<T>(Proxy);
		if (addr)
		{
			return addr;
		}

		LOG_LIMIT(100, __FUNCTION__ << " Error: could not find interface for index: " << AddressCacheIndex<T>::CacheIndex);
		return nullptr;
	}

	template <typename T, typename M>
	M* GetSafeProxyInterface(T* WrapperInterface)
	{
		constexpr size_t CacheIndex = AddressCacheIndex<T>::CacheIndex;

		for (const auto& pair : g_map[CacheIndex])
		{
			if (pair.second == WrapperInterface)
			{
				return WrapperInterface->GetProxyInterface();
			}
		}

		return nullptr;
	}

	template <typename T>
	void SaveAddress(T* Wrapper, IUnknown* Proxy)
	{
		if (!Wrapper || !Proxy)
		{
			return;
		}

		constexpr UINT CacheIndex = AddressCacheIndex<T>::CacheIndex;

		IUnknown* identity = GetIndentityInterface<T>(Proxy);

		// Check if the entry already exists in the map
		auto it = g_map[CacheIndex].find(identity);
		if (it != g_map[CacheIndex].end())
		{
			// If the entry exists, delete the existing object
			if (it->second)
			{
				delete it->second;
				DeleteAddress((T*)it->second);
			}
		}

		// Now save the new entry in the map
		g_map[CacheIndex][identity] = Wrapper;
	}

	template <typename T>
	void DeleteAddress(T* Wrapper)
	{
		if (!Wrapper || ConstructorFlag)
		{
			return;
		}

		constexpr size_t CacheIndex = AddressCacheIndex<T>::CacheIndex;

		auto it = std::find_if(g_map[CacheIndex].begin(), g_map[CacheIndex].end(),
			[=](auto& Map) -> bool { return Map.second == Wrapper; });
		if (it != std::end(g_map[CacheIndex]))
		{
			g_map[CacheIndex].erase(it);
		}
	}
};

class StateBlockCache
{
private:
	std::vector<m_IDirect3DStateBlock9*> stateBlocks;

public:
	~StateBlockCache();

	size_t size() { return stateBlocks.size(); }
	m_IDirect3DStateBlock9* back() { return stateBlocks.back(); }
	void AddStateBlock(m_IDirect3DStateBlock9* stateBlock);
	void RemoveStateBlock(m_IDirect3DStateBlock9* stateBlock);
};
