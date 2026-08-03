#pragma once

#include <unordered_map>
#include <algorithm>
#include "dinput8.h"

class AddressLookupTableDinput8ObjectBase
{
public:
	virtual ~AddressLookupTableDinput8ObjectBase() = default;
};

template<typename T>
class AddressLookupTableDinput8Object;

class AddressLookupTableDinput8
{
private:
	static constexpr size_t MaxCacheIndex = 3;

	bool ConstructorFlag = false;
	std::unordered_map<void*, class AddressLookupTableDinput8ObjectBase*> g_map[MaxCacheIndex];

	template <typename T>
	struct AddressCacheIndex {};
	template <>
	struct AddressCacheIndex<m_IDirectInput8> { static constexpr size_t CacheIndex = 0; };
	template <>
	struct AddressCacheIndex<m_IDirectInputDevice8> { static constexpr size_t CacheIndex = 1; };
	template <>
	struct AddressCacheIndex<m_IDirectInputEffect8> { static constexpr size_t CacheIndex = 2; };

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
	explicit AddressLookupTableDinput8() {}
	~AddressLookupTableDinput8()
	{
		ConstructorFlag = true;
		DeleteAll();
	}

	template <typename T>
	T *FindAddress(void *Proxy)
	{
		if (!Proxy)
		{
			return nullptr;
		}

		constexpr size_t CacheIndex = AddressCacheIndex<T>::CacheIndex;

		auto it = g_map[CacheIndex].find(Proxy);
		if (it != std::end(g_map[CacheIndex]))
		{
			return static_cast<T*>(it->second);
		}

		return nullptr;
	}

	template <typename T>
	void SaveAddress(AddressLookupTableDinput8Object<T>* Wrapper, void *Proxy)
	{
		constexpr size_t CacheIndex = AddressCacheIndex<T>::CacheIndex;
		if (Wrapper && Proxy)
		{
			g_map[CacheIndex][Proxy] = Wrapper;
		}
	}

	template <typename T>
	void DeleteAddress(AddressLookupTableDinput8Object<T>*Wrapper)
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

template<typename T>
class AddressLookupTableDinput8Object : public AddressLookupTableDinput8ObjectBase
{
public:
	AddressLookupTableDinput8Object(void* Proxy)
	{
		ProxyAddressLookupTableDinput8.SaveAddress(this, Proxy);
	}

	~AddressLookupTableDinput8Object()
	{
		ProxyAddressLookupTableDinput8.DeleteAddress(this);
	}
};
