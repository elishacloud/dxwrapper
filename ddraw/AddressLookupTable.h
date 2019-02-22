#pragma once

#include <unordered_map>
#include <algorithm>

constexpr UINT MaxIndex = 37;

template <typename D>
class AddressLookupTableDdraw
{
public:
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
	struct AddressCacheIndex { static constexpr UINT CacheIndex = 0; };
	template <>
	struct AddressCacheIndex<IClassFactory> { static constexpr UINT CacheIndex = 1; };
	template <>
	struct AddressCacheIndex<IDirect3DExecuteBuffer> { static constexpr UINT CacheIndex = 2; };
	template <>
	struct AddressCacheIndex<IDirect3DLight> { static constexpr UINT CacheIndex = 3; };
	template <>
	struct AddressCacheIndex<IDirectDrawClipper> { static constexpr UINT CacheIndex = 4; };
	template <>
	struct AddressCacheIndex<IDirectDrawColorControl> { static constexpr UINT CacheIndex = 5; };
	template <>
	struct AddressCacheIndex<IDirectDrawFactory> { static constexpr UINT CacheIndex = 6; };
	template <>
	struct AddressCacheIndex<IDirectDrawGammaControl> { static constexpr UINT CacheIndex = 7; };
	template <>
	struct AddressCacheIndex<IDirectDrawPalette> { static constexpr UINT CacheIndex = 8; };
	template <>
	struct AddressCacheIndex<m_IDirect3D> { static constexpr UINT CacheIndex = 9; };
	template <>
	struct AddressCacheIndex<m_IDirect3D2> { static constexpr UINT CacheIndex = 10; };
	template <>
	struct AddressCacheIndex<m_IDirect3D3> { static constexpr UINT CacheIndex = 11; };
	template <>
	struct AddressCacheIndex<m_IDirect3D7> { static constexpr UINT CacheIndex = 12;
		using Type1 = m_IDirect3D;
		using Type2 = m_IDirect3D2;
		using Type3 = m_IDirect3D3;
		using Type4 = m_IDirect3D3;
		using Type7 = m_IDirect3D7;
	};
	template <>
	struct AddressCacheIndex<m_IDirect3DDevice> { static constexpr UINT CacheIndex = 13; };
	template <>
	struct AddressCacheIndex<m_IDirect3DDevice2> { static constexpr UINT CacheIndex = 14; };
	template <>
	struct AddressCacheIndex<m_IDirect3DDevice3> { static constexpr UINT CacheIndex = 15; };
	template <>
	struct AddressCacheIndex<m_IDirect3DDevice7> { static constexpr UINT CacheIndex = 16;
		using Type1 = m_IDirect3DDevice;
		using Type2 = m_IDirect3DDevice2;
		using Type3 = m_IDirect3DDevice3;
		using Type4 = m_IDirect3DDevice3;
		using Type7 = m_IDirect3DDevice7;
	};
	template <>
	struct AddressCacheIndex<m_IDirect3DMaterial> { static constexpr UINT CacheIndex = 17; };
	template <>
	struct AddressCacheIndex<m_IDirect3DMaterial2> { static constexpr UINT CacheIndex = 18; };
	template <>
	struct AddressCacheIndex<m_IDirect3DMaterial3> { static constexpr UINT CacheIndex = 19;
		using Type1 = m_IDirect3DMaterial;
		using Type2 = m_IDirect3DMaterial2;
		using Type3 = m_IDirect3DMaterial3;
		using Type4 = m_IDirect3DMaterial3;
		using Type7 = m_IDirect3DMaterial3;
	};
	template <>
	struct AddressCacheIndex<m_IDirect3DTexture> { static constexpr UINT CacheIndex = 20; };
	template <>
	struct AddressCacheIndex<m_IDirect3DTexture2> { static constexpr UINT CacheIndex = 21;
		using Type1 = m_IDirect3DTexture;
		using Type2 = m_IDirect3DTexture2;
		using Type3 = m_IDirect3DTexture2;
		using Type4 = m_IDirect3DTexture2;
		using Type7 = m_IDirect3DTexture2;
	};
	template <>
	struct AddressCacheIndex<m_IDirect3DVertexBuffer> { static constexpr UINT CacheIndex = 22; };
	template <>
	struct AddressCacheIndex<m_IDirect3DVertexBuffer7> { static constexpr UINT CacheIndex = 23;
		using Type1 = m_IDirect3DVertexBuffer;
		using Type2 = m_IDirect3DVertexBuffer;
		using Type3 = m_IDirect3DVertexBuffer;
		using Type4 = m_IDirect3DVertexBuffer;
		using Type7 = m_IDirect3DVertexBuffer7;
	};
	template <>
	struct AddressCacheIndex<m_IDirect3DViewport> { static constexpr UINT CacheIndex = 24; };
	template <>
	struct AddressCacheIndex<m_IDirect3DViewport2> { static constexpr UINT CacheIndex = 25; };
	template <>
	struct AddressCacheIndex<m_IDirect3DViewport3> { static constexpr UINT CacheIndex = 26;
		using Type1 = m_IDirect3DViewport;
		using Type2 = m_IDirect3DViewport2;
		using Type3 = m_IDirect3DViewport3;
		using Type4 = m_IDirect3DViewport3;
		using Type7 = m_IDirect3DViewport3;
	};
	template <>
	struct AddressCacheIndex<m_IDirectDraw> { static constexpr UINT CacheIndex = 27; };
	template <>
	struct AddressCacheIndex<m_IDirectDraw2> { static constexpr UINT CacheIndex = 28; };
	template <>
	struct AddressCacheIndex<m_IDirectDraw3> { static constexpr UINT CacheIndex = 29; };
	template <>
	struct AddressCacheIndex<m_IDirectDraw4> { static constexpr UINT CacheIndex = 30; };
	template <>
	struct AddressCacheIndex<m_IDirectDraw7> { static constexpr UINT CacheIndex = 31;
		using Type1 = m_IDirectDraw;
		using Type2 = m_IDirectDraw2;
		using Type3 = m_IDirectDraw3;
		using Type4 = m_IDirectDraw4;
		using Type7 = m_IDirectDraw7;
	};
	template <>
	struct AddressCacheIndex<m_IDirectDrawSurface> { static constexpr UINT CacheIndex = 32; };
	template <>
	struct AddressCacheIndex<m_IDirectDrawSurface2> { static constexpr UINT CacheIndex = 33; };
	template <>
	struct AddressCacheIndex<m_IDirectDrawSurface3> { static constexpr UINT CacheIndex = 34;  };
	template <>
	struct AddressCacheIndex<m_IDirectDrawSurface4> { static constexpr UINT CacheIndex = 35; };
	template <>
	struct AddressCacheIndex<m_IDirectDrawSurface7> { static constexpr UINT CacheIndex = 36;
		using Type1 = m_IDirectDrawSurface;
		using Type2 = m_IDirectDrawSurface2;
		using Type3 = m_IDirectDrawSurface3;
		using Type4 = m_IDirectDrawSurface4;
		using Type7 = m_IDirectDrawSurface7;
	};

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
	bool IsValidAddress(T *Wrapper)
	{
		if (!Wrapper)
		{
			return false;
		}

		for (UINT CacheIndex = 0 ; CacheIndex < MaxIndex; CacheIndex++)
		{
			auto it = std::find_if(g_map[CacheIndex].begin(), g_map[CacheIndex].end(),
				[=](auto Map) -> bool { return Map.second == Wrapper; });

			if (it != std::end(g_map[CacheIndex]))
			{
				return true;
			}
		}

		return false;
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
