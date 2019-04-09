#pragma once

#include <unordered_map>
#include <algorithm>

constexpr UINT MaxIndex = 6;

template <typename D>
class AddressLookupTableDinput8
{
public:
	explicit AddressLookupTableDinput8() {}
	~AddressLookupTableDinput8()
	{
		ConstructorFlag = true;

		for (const auto& cache : g_map)
		{
			for (const auto& entry : cache)
			{
				entry.second->DeleteMe();
			}
		}
	}

	template <typename T>
	struct AddressCacheIndex { static constexpr UINT CacheIndex = 0; };
	template <>
	struct AddressCacheIndex<m_IDirectInput8A> { static constexpr UINT CacheIndex = 1; };
	template <>
	struct AddressCacheIndex<m_IDirectInput8W> { static constexpr UINT CacheIndex = 2; };
	template <>
	struct AddressCacheIndex<m_IDirectInputDevice8A> { static constexpr UINT CacheIndex = 3; };
	template <>
	struct AddressCacheIndex<m_IDirectInputDevice8W> { static constexpr UINT CacheIndex = 4; };
	template <>
	struct AddressCacheIndex<m_IDirectInputEffect8> { static constexpr UINT CacheIndex = 5; };

	template <typename T>
	T *FindAddress(void *Proxy)
	{
		if (!Proxy)
		{
			return nullptr;
		}

		constexpr UINT CacheIndex = AddressCacheIndex<T>::CacheIndex;
		auto it = g_map[CacheIndex].find(Proxy);

		if (it != std::end(g_map[CacheIndex]))
		{
			return static_cast<T *>(it->second);
		}

		return new T(static_cast<T *>(Proxy));
	}

	template <typename T>
	void SaveAddress(T *Wrapper, void *Proxy)
	{
		constexpr UINT CacheIndex = AddressCacheIndex<T>::CacheIndex;
		if (Wrapper && Proxy)
		{
			g_map[CacheIndex][Proxy] = Wrapper;
		}
	}

	template <typename T>
	void DeleteAddress(T *Wrapper)
	{
		if (!Wrapper || ConstructorFlag)
		{
			return;
		}

		constexpr UINT CacheIndex = AddressCacheIndex<T>::CacheIndex;
		auto it = std::find_if(g_map[CacheIndex].begin(), g_map[CacheIndex].end(),
			[=](auto Map) -> bool { return Map.second == Wrapper; });

		if (it != std::end(g_map[CacheIndex]))
		{
			it = g_map[CacheIndex].erase(it);
		}
	}

private:
	bool ConstructorFlag = false;
	D *unused = nullptr;
	std::unordered_map<void*, class AddressLookupTableDinput8Object*> g_map[MaxIndex];
};

class AddressLookupTableDinput8Object
{
public:
	virtual ~AddressLookupTableDinput8Object() { }

	void DeleteMe()
	{
		delete this;
	}
};
