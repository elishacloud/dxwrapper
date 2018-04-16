#pragma once

#include <unordered_map>
#include <algorithm>

constexpr UINT MaxIndex = 8;

template <typename D>
class AddressLookupTableDdraw
{
public:
	template <typename T>
	struct AddressCacheIndex { static constexpr UINT CacheIndex = 0; };
	template <>
	struct AddressCacheIndex<m_IDirectDraw> { static constexpr UINT CacheIndex = 1; };
	template <>
	struct AddressCacheIndex<m_IDirectDraw2> { static constexpr UINT CacheIndex = 2; };
	template <>
	struct AddressCacheIndex<m_IDirectDraw3> { static constexpr UINT CacheIndex = 3; };
	template <>
	struct AddressCacheIndex<m_IDirectDraw4> { static constexpr UINT CacheIndex = 4; };
	template <>
	struct AddressCacheIndex<m_IDirectDraw7> { static constexpr UINT CacheIndex = 7;
		using Type1 = m_IDirectDraw;
		using Type2 = m_IDirectDraw2;
		using Type3 = m_IDirectDraw3;
		using Type4 = m_IDirectDraw4;
		using Type7 = m_IDirectDraw7;
	};
	template <>
	struct AddressCacheIndex<m_IDirectDrawSurface> { static constexpr UINT CacheIndex = 1; };
	template <>
	struct AddressCacheIndex<m_IDirectDrawSurface2> { static constexpr UINT CacheIndex = 2; };
	template <>
	struct AddressCacheIndex<m_IDirectDrawSurface3> { static constexpr UINT CacheIndex = 3;  };
	template <>
	struct AddressCacheIndex<m_IDirectDrawSurface4> { static constexpr UINT CacheIndex = 4; };
	template <>
	struct AddressCacheIndex<m_IDirectDrawSurface7> { static constexpr UINT CacheIndex = 7;
		using Type1 = m_IDirectDrawSurface;
		using Type2 = m_IDirectDrawSurface2;
		using Type3 = m_IDirectDrawSurface3;
		using Type4 = m_IDirectDrawSurface4;
		using Type7 = m_IDirectDrawSurface7;
	};

	explicit AddressLookupTableDdraw() {}
	~AddressLookupTableDdraw()
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
	T *FindAddress(void *Proxy, DWORD Version)
	{
		switch (Version)
		{
		case 1:
			return (T*)FindAddress<AddressCacheIndex<T>::Type1>(Proxy);
		case 2:
			return (T*)FindAddress<AddressCacheIndex<T>::Type2>(Proxy);
		case 3:
			return (T*)FindAddress<AddressCacheIndex<T>::Type3>(Proxy);
		case 4:
			return (T*)FindAddress<AddressCacheIndex<T>::Type4>(Proxy);
		case 7:
			return (T*)FindAddress<AddressCacheIndex<T>::Type7>(Proxy);
		default:
			return nullptr;
		}
	}

	template <typename T>
	T *FindAddress(void *Proxy)
	{
		if (Proxy == nullptr)
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
		if (Wrapper != nullptr && Proxy != nullptr)
		{
			g_map[CacheIndex][Proxy] = Wrapper;
		}
	}

	template <typename T>
	void DeleteAddress(T *Wrapper)
	{
		if (Wrapper != nullptr && !ConstructorFlag)
		{
			constexpr UINT CacheIndex = AddressCacheIndex<T>::CacheIndex;
			auto it = std::find_if(g_map[CacheIndex].begin(), g_map[CacheIndex].end(),
				[Wrapper](std::pair<void*, class AddressLookupTableDdrawObject*> Map) -> bool { return Map.second == Wrapper; });

			if (it != std::end(g_map[CacheIndex]))
			{
				it = g_map[CacheIndex].erase(it);
			}
		}
	}

private:
	bool ConstructorFlag = false;
	D * unused = nullptr;
	std::unordered_map<void*, class AddressLookupTableDdrawObject*> g_map[MaxIndex];
};

class AddressLookupTableDdrawObject
{
public:
	virtual ~AddressLookupTableDdrawObject() { }

	void DeleteMe()
	{
		delete this;
	}
};
