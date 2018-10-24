#pragma once

#include <unordered_map>
#include <algorithm>

template <typename D>
class AddressLookupTableD3d9
{
public:
	explicit AddressLookupTableD3d9(D *pDevice) : pDevice(pDevice) {}
	~AddressLookupTableD3d9()
	{
		ConstructorFlag = true;
		for (const auto& entry : g_map)
		{
			entry.second->DeleteMe();
		}
	}

	template <typename T>
	T *FindAddress(void *Proxy)
	{
		if (!Proxy)
		{
			return nullptr;
		}

		auto it = g_map.find(Proxy);

		if (it != std::end(g_map))
		{
			return static_cast<T *>(it->second);
		}

		return new T(static_cast<T *>(Proxy), pDevice);
	}

	template <typename T>
	void SaveAddress(T *Wrapper, void *Proxy)
	{
		if (Wrapper && Proxy)
		{
			g_map[Proxy] = Wrapper;
		}
	}

	template <typename T>
	void DeleteAddress(T *Wrapper)
	{
		if (!Wrapper || ConstructorFlag)
		{
			return;
		}

		auto it = std::find_if(g_map.begin(), g_map.end(),
			[=](auto Map) -> bool { return Map.second == Wrapper; });

		if (it != std::end(g_map))
		{
			it = g_map.erase(it);
		}
	}

private:
	bool ConstructorFlag = false;
	D *const pDevice;
	std::unordered_map<void*, class AddressLookupTableD3d9Object*> g_map;
};

class AddressLookupTableD3d9Object
{
public:
	virtual ~AddressLookupTableD3d9Object() { }

	void DeleteMe()
	{
		delete this;
	}
};
