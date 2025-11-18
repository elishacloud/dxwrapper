#pragma once

#include <unordered_map>
#include <algorithm>

constexpr UINT MaxIndex = 43;

template <typename T>
struct AddressInterfaceList {
	static std::vector<T*> InterfaceList;
};

template <typename T>
T* InterfaceAddressCache(T* Interface)
{
	static std::vector<T*> InterfaceList;
	if (Interface)
	{
		if (!Config.DdrawKeepAllInterfaceCache)
		{
			while (!InterfaceList.empty())
			{
				T* InterfaceCache = InterfaceList.back();
				InterfaceCache->DeleteMe();
				InterfaceList.pop_back();
			}
		}
		InterfaceList.push_back(Interface);
		return nullptr;
	}
	else
	{
		T* InterfaceCache = nullptr;
		if (!InterfaceList.empty())
		{
			InterfaceCache = InterfaceList.back();
			InterfaceList.pop_back();
		}
		return InterfaceCache;
	}
}

template <typename T>
inline void SaveInterfaceAddress(T*& Interface)
{
	if (Interface)
	{
		if constexpr (std::is_same_v<T, m_IDirectDrawPalette>)
		{
			Interface->SetProxy(nullptr, nullptr, 0, nullptr);
		}
		else if constexpr (std::is_same_v<T, m_IDirect3DExecuteBuffer>)
		{
			Interface->SetProxy(nullptr, nullptr, nullptr);
		}
		else if constexpr (std::is_same_v<T, m_IDirectDrawClipper>)
		{
			Interface->SetProxy(nullptr, nullptr, 0);
		}
		else if constexpr (std::is_same_v<T, m_IDirect3DLight> || std::is_same_v<T, m_IDirectDrawColorControl> || std::is_same_v<T, m_IDirectDrawGammaControl>)
		{
			Interface->SetProxy(nullptr, nullptr);
		}
		else
		{
			Interface->SetProxy(nullptr);
		}
		InterfaceAddressCache(Interface);
		Interface = nullptr;
	}
}

template <typename T>
inline void SaveInterfaceAddress(const T* Interface)
{
	T* NewInterface = const_cast<T*>(Interface);
	SaveInterfaceAddress(NewInterface);
}

template <typename T, typename S, typename X>
inline T* GetInterfaceAddress(T*& Interface, S* ProxyInterface, X* InterfaceX)
{
	if (!Interface)
	{
		T* NewInterface = InterfaceAddressCache<T>(nullptr);
		if (NewInterface)
		{
			Interface = NewInterface;
			Interface->SetProxy(InterfaceX);
		}
		else
		{
			Interface = new T(ProxyInterface, InterfaceX);
		}
	}
	return Interface;
}

template <typename D>
class AddressLookupTableDdraw
{
private:
	bool ConstructorFlag = false;
	std::unordered_map<void*, class AddressLookupTableDdrawObject*> g_map[MaxIndex];
	std::unordered_map<class AddressLookupTableDdrawObject*, void*> reverse_map[MaxIndex];  // Reverse mapping

	template <typename T>
	struct AddressCacheIndex { static constexpr UINT CacheIndex = 0; };
	template <>
	struct AddressCacheIndex<m_IDirect3D> { static constexpr UINT CacheIndex = 1; };
	template <>
	struct AddressCacheIndex<m_IDirect3D2> { static constexpr UINT CacheIndex = 2; };
	template <>
	struct AddressCacheIndex<m_IDirect3D3> { static constexpr UINT CacheIndex = 3; };
	template <>
	struct AddressCacheIndex<m_IDirect3D7> { static constexpr UINT CacheIndex = 4;
		using Type1 = m_IDirect3D;
		using Type2 = m_IDirect3D2;
		using Type3 = m_IDirect3D3;
		using Type4 = m_IDirect3D3;
		using Type7 = m_IDirect3D7;
	};
	template <>
	struct AddressCacheIndex<m_IDirect3DDevice> { static constexpr UINT CacheIndex = 5; };
	template <>
	struct AddressCacheIndex<m_IDirect3DDevice2> { static constexpr UINT CacheIndex = 6; };
	template <>
	struct AddressCacheIndex<m_IDirect3DDevice3> { static constexpr UINT CacheIndex = 7; };
	template <>
	struct AddressCacheIndex<m_IDirect3DDevice7> { static constexpr UINT CacheIndex = 8;
		using Type1 = m_IDirect3DDevice;
		using Type2 = m_IDirect3DDevice2;
		using Type3 = m_IDirect3DDevice3;
		using Type4 = m_IDirect3DDevice3;
		using Type7 = m_IDirect3DDevice7;
	};
	template <>
	struct AddressCacheIndex<m_IDirect3DMaterial> { static constexpr UINT CacheIndex = 9; };
	template <>
	struct AddressCacheIndex<m_IDirect3DMaterial2> { static constexpr UINT CacheIndex = 10; };
	template <>
	struct AddressCacheIndex<m_IDirect3DMaterial3> { static constexpr UINT CacheIndex = 11;
		using Type1 = m_IDirect3DMaterial;
		using Type2 = m_IDirect3DMaterial2;
		using Type3 = m_IDirect3DMaterial3;
		using Type4 = m_IDirect3DMaterial3;
		using Type7 = m_IDirect3DMaterial3;
	};
	template <>
	struct AddressCacheIndex<m_IDirect3DTexture> { static constexpr UINT CacheIndex = 12; };
	template <>
	struct AddressCacheIndex<m_IDirect3DTexture2> { static constexpr UINT CacheIndex = 13;
		using Type1 = m_IDirect3DTexture;
		using Type2 = m_IDirect3DTexture2;
		using Type3 = m_IDirect3DTexture2;
		using Type4 = m_IDirect3DTexture2;
		using Type7 = m_IDirect3DTexture2;
	};
	template <>
	struct AddressCacheIndex<m_IDirect3DVertexBuffer> { static constexpr UINT CacheIndex = 14; };
	template <>
	struct AddressCacheIndex<m_IDirect3DVertexBuffer7> { static constexpr UINT CacheIndex = 15;
		using Type1 = m_IDirect3DVertexBuffer;
		using Type2 = m_IDirect3DVertexBuffer;
		using Type3 = m_IDirect3DVertexBuffer;
		using Type4 = m_IDirect3DVertexBuffer;
		using Type7 = m_IDirect3DVertexBuffer7;
	};
	template <>
	struct AddressCacheIndex<m_IDirect3DViewport> { static constexpr UINT CacheIndex = 16; };
	template <>
	struct AddressCacheIndex<m_IDirect3DViewport2> { static constexpr UINT CacheIndex = 17; };
	template <>
	struct AddressCacheIndex<m_IDirect3DViewport3> { static constexpr UINT CacheIndex = 18;
		using Type1 = m_IDirect3DViewport;
		using Type2 = m_IDirect3DViewport2;
		using Type3 = m_IDirect3DViewport3;
		using Type4 = m_IDirect3DViewport3;
		using Type7 = m_IDirect3DViewport3;
	};
	template <>
	struct AddressCacheIndex<m_IDirectDraw> { static constexpr UINT CacheIndex = 19; };
	template <>
	struct AddressCacheIndex<m_IDirectDraw2> { static constexpr UINT CacheIndex = 20; };
	template <>
	struct AddressCacheIndex<m_IDirectDraw3> { static constexpr UINT CacheIndex = 21; };
	template <>
	struct AddressCacheIndex<m_IDirectDraw4> { static constexpr UINT CacheIndex = 22; };
	template <>
	struct AddressCacheIndex<m_IDirectDraw7> { static constexpr UINT CacheIndex = 23;
		using Type1 = m_IDirectDraw;
		using Type2 = m_IDirectDraw2;
		using Type3 = m_IDirectDraw3;
		using Type4 = m_IDirectDraw4;
		using Type7 = m_IDirectDraw7;
	};
	template <>
	struct AddressCacheIndex<m_IDirectDrawSurface> { static constexpr UINT CacheIndex = 24; };
	template <>
	struct AddressCacheIndex<m_IDirectDrawSurface2> { static constexpr UINT CacheIndex = 25; };
	template <>
	struct AddressCacheIndex<m_IDirectDrawSurface3> { static constexpr UINT CacheIndex = 26;  };
	template <>
	struct AddressCacheIndex<m_IDirectDrawSurface4> { static constexpr UINT CacheIndex = 27; };
	template <>
	struct AddressCacheIndex<m_IDirectDrawSurface7> { static constexpr UINT CacheIndex = 28;
		using Type1 = m_IDirectDrawSurface;
		using Type2 = m_IDirectDrawSurface2;
		using Type3 = m_IDirectDrawSurface3;
		using Type4 = m_IDirectDrawSurface4;
		using Type7 = m_IDirectDrawSurface7;
	};
	template <>
	struct AddressCacheIndex<IDirect3DExecuteBuffer> { static constexpr UINT CacheIndex = 29; };
	template <>
	struct AddressCacheIndex<IDirect3DLight> { static constexpr UINT CacheIndex = 30; };
	template <>
	struct AddressCacheIndex<IDirectDrawClipper> { static constexpr UINT CacheIndex = 31; };
	template <>
	struct AddressCacheIndex<IDirectDrawColorControl> { static constexpr UINT CacheIndex = 32; };
	template <>
	struct AddressCacheIndex<IDirectDrawGammaControl> { static constexpr UINT CacheIndex = 33; };
	template <>
	struct AddressCacheIndex<IDirectDrawPalette> { static constexpr UINT CacheIndex = 34; };
	template <>
	struct AddressCacheIndex<m_IDirect3DX> { static constexpr UINT CacheIndex = 35; };
	template <>
	struct AddressCacheIndex<m_IDirect3DDeviceX> { static constexpr UINT CacheIndex = 36; };
	template <>
	struct AddressCacheIndex<m_IDirect3DMaterialX> { static constexpr UINT CacheIndex = 37; };
	template <>
	struct AddressCacheIndex<m_IDirect3DTextureX> { static constexpr UINT CacheIndex = 38; };
	template <>
	struct AddressCacheIndex<m_IDirect3DVertexBufferX> { static constexpr UINT CacheIndex = 39; };
	template <>
	struct AddressCacheIndex<m_IDirect3DViewportX> { static constexpr UINT CacheIndex = 40; };
	template <>
	struct AddressCacheIndex<m_IDirectDrawX> { static constexpr UINT CacheIndex = 41; };
	template <>
	struct AddressCacheIndex<m_IDirectDrawSurfaceX> { static constexpr UINT CacheIndex = 42; };

	void DeleteAll()
	{
		for (DWORD x = 29; x < MaxIndex; x++)
		{
			for (const auto& entry : g_map[x])
			{
				entry.second->DeleteMe();
			}
		}
	}

	template <typename T, typename X, typename I>
	T *FindAddressVersion(void *Proxy, DWORD DxVersion)
	{
		T *Interface = FindAddressAllInterfaces<T>(Proxy);

		if (!Interface)
		{
			X *InterfaceX = new X((I*)Proxy, DxVersion);

			Interface = (T*)InterfaceX->GetWrapperInterfaceX(DxVersion);
		}

		return Interface;
	}

	template <typename T>
	T *FindAddressPrivate(void *Proxy)
	{
		T *Interface = FindAddressAllInterfaces<T>(Proxy);

		if (!Interface)
		{
			Interface = new T(static_cast<T *>(Proxy));
		}

		return Interface;
	}
	template <>
	m_IDirectDraw *FindAddressPrivate(void *Proxy) { return FindAddressVersion<m_IDirectDraw, m_IDirectDrawX, IDirectDraw7>(Proxy, 1); }
	template <>
	m_IDirectDraw2 *FindAddressPrivate(void *Proxy) { return FindAddressVersion<m_IDirectDraw2, m_IDirectDrawX, IDirectDraw7>(Proxy, 2); }
	template <>
	m_IDirectDraw3 *FindAddressPrivate(void *Proxy) { return FindAddressVersion<m_IDirectDraw3, m_IDirectDrawX, IDirectDraw7>(Proxy, 3); }
	template <>
	m_IDirectDraw4 *FindAddressPrivate(void *Proxy) { return FindAddressVersion<m_IDirectDraw4, m_IDirectDrawX, IDirectDraw7>(Proxy, 4); }
	template <>
	m_IDirectDraw7 *FindAddressPrivate(void *Proxy) { return FindAddressVersion<m_IDirectDraw7, m_IDirectDrawX, IDirectDraw7>(Proxy, 7); }
	template <>
	m_IDirectDrawSurface *FindAddressPrivate(void *Proxy) { return FindAddressVersion<m_IDirectDrawSurface, m_IDirectDrawSurfaceX, IDirectDrawSurface7>(Proxy, 1); }
	template <>
	m_IDirectDrawSurface2 *FindAddressPrivate(void *Proxy) { return FindAddressVersion<m_IDirectDrawSurface2, m_IDirectDrawSurfaceX, IDirectDrawSurface7>(Proxy, 2); }
	template <>
	m_IDirectDrawSurface3 *FindAddressPrivate(void *Proxy) { return FindAddressVersion<m_IDirectDrawSurface3, m_IDirectDrawSurfaceX, IDirectDrawSurface7>(Proxy, 3); }
	template <>
	m_IDirectDrawSurface4 *FindAddressPrivate(void *Proxy) { return FindAddressVersion<m_IDirectDrawSurface4, m_IDirectDrawSurfaceX, IDirectDrawSurface7>(Proxy, 4); }
	template <>
	m_IDirectDrawSurface7 *FindAddressPrivate(void *Proxy) { return FindAddressVersion<m_IDirectDrawSurface7, m_IDirectDrawSurfaceX, IDirectDrawSurface7>(Proxy, 7); }
	template <>
	m_IDirect3D *FindAddressPrivate(void *Proxy) { return FindAddressVersion<m_IDirect3D, m_IDirect3DX, IDirect3D7>(Proxy, 1); }
	template <>
	m_IDirect3D2 *FindAddressPrivate(void *Proxy) { return FindAddressVersion<m_IDirect3D2, m_IDirect3DX, IDirect3D7>(Proxy, 2); }
	template <>
	m_IDirect3D3 *FindAddressPrivate(void *Proxy) { return FindAddressVersion<m_IDirect3D3, m_IDirect3DX, IDirect3D7>(Proxy, 3); }
	template <>
	m_IDirect3D7 *FindAddressPrivate(void *Proxy) { return FindAddressVersion<m_IDirect3D7, m_IDirect3DX, IDirect3D7>(Proxy, 7); }
	template <>
	m_IDirect3DDevice *FindAddressPrivate(void *Proxy) { return FindAddressVersion<m_IDirect3DDevice, m_IDirect3DDeviceX, IDirect3DDevice7>(Proxy, 1); }
	template <>
	m_IDirect3DDevice2 *FindAddressPrivate(void *Proxy) { return FindAddressVersion<m_IDirect3DDevice2, m_IDirect3DDeviceX, IDirect3DDevice7>(Proxy, 2); }
	template <>
	m_IDirect3DDevice3 *FindAddressPrivate(void *Proxy) { return FindAddressVersion<m_IDirect3DDevice3, m_IDirect3DDeviceX, IDirect3DDevice7>(Proxy, 3); }
	template <>
	m_IDirect3DDevice7 *FindAddressPrivate(void *Proxy) { return FindAddressVersion<m_IDirect3DDevice7, m_IDirect3DDeviceX, IDirect3DDevice7>(Proxy, 7); }
	template <>
	m_IDirect3DMaterial *FindAddressPrivate(void *Proxy) { return FindAddressVersion<m_IDirect3DMaterial, m_IDirect3DMaterialX, IDirect3DMaterial3>(Proxy, 1); }
	template <>
	m_IDirect3DMaterial2 *FindAddressPrivate(void *Proxy) { return FindAddressVersion<m_IDirect3DMaterial2, m_IDirect3DMaterialX, IDirect3DMaterial3>(Proxy, 2); }
	template <>
	m_IDirect3DMaterial3 *FindAddressPrivate(void *Proxy) { return FindAddressVersion<m_IDirect3DMaterial3, m_IDirect3DMaterialX, IDirect3DMaterial3>(Proxy, 3); }
	template <>
	m_IDirect3DViewport *FindAddressPrivate(void *Proxy) { return FindAddressVersion<m_IDirect3DViewport, m_IDirect3DViewportX, IDirect3DViewport3>(Proxy, 1); }
	template <>
	m_IDirect3DViewport2 *FindAddressPrivate(void *Proxy) { return FindAddressVersion<m_IDirect3DViewport2, m_IDirect3DViewportX, IDirect3DViewport3>(Proxy, 2); }
	template <>
	m_IDirect3DViewport3 *FindAddressPrivate(void *Proxy) { return FindAddressVersion<m_IDirect3DViewport3, m_IDirect3DViewportX, IDirect3DViewport3>(Proxy, 3); }
	template <>
	m_IDirect3DTexture *FindAddressPrivate(void *Proxy) { return FindAddressVersion<m_IDirect3DTexture, m_IDirect3DTextureX, IDirect3DTexture2>(Proxy, 1); }
	template <>
	m_IDirect3DTexture2 *FindAddressPrivate(void *Proxy) { return FindAddressVersion<m_IDirect3DTexture2, m_IDirect3DTextureX, IDirect3DTexture2>(Proxy, 2); }
	template <>
	m_IDirect3DVertexBuffer *FindAddressPrivate(void *Proxy) { return FindAddressVersion<m_IDirect3DVertexBuffer, m_IDirect3DVertexBufferX, IDirect3DVertexBuffer7>(Proxy, 1); }
	template <>
	m_IDirect3DVertexBuffer7 *FindAddressPrivate(void *Proxy) { return FindAddressVersion<m_IDirect3DVertexBuffer7, m_IDirect3DVertexBufferX, IDirect3DVertexBuffer7>(Proxy, 7); }

	template <typename T>
	T *FindAddressAllInterfaces(void *Proxy)
	{
		if (Config.Dd7to9)
		{
			Logging::Log() << __FUNCTION__ " Error: Dd7to9 should not be calling FindAddress<>!";
		}

		constexpr UINT CacheIndex = AddressCacheIndex<T>::CacheIndex;
		auto it = g_map[CacheIndex].find(Proxy);

		if (it != std::end(g_map[CacheIndex]))
		{
			return static_cast<T *>(it->second);
		}

		return nullptr;
	}

public:
	explicit AddressLookupTableDdraw() {}
	~AddressLookupTableDdraw()
	{
		ConstructorFlag = true;
		DeleteAll();
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
		if (!Proxy)
		{
			return nullptr;
		}

		return FindAddressPrivate<T>(Proxy);
	}

	template <typename T>
	bool IsValidWrapperAddress(T *Wrapper)
	{
		if (!Wrapper)
		{
			return false;
		}

		constexpr UINT CacheIndex = AddressCacheIndex<T>::CacheIndex;
		auto it = reverse_map[CacheIndex].find(Wrapper);

		if (it != reverse_map[CacheIndex].end())
		{
			return true;
		}

		return false;
	}

	template <typename T>
	bool IsValidProxyAddress(void *Proxy)
	{
		if (!Proxy)
		{
			return false;
		}

		constexpr UINT CacheIndex = AddressCacheIndex<T>::CacheIndex;
		auto it = g_map[CacheIndex].find(Proxy);

		if (it != std::end(g_map[CacheIndex]))
		{
			return true;
		}

		return false;
	}

	bool CheckSurfaceExists(LPDIRECTDRAWSURFACE7 lpDDSrcSurface) {
		return
			(IsValidWrapperAddress((m_IDirectDrawSurface*)lpDDSrcSurface) ||
				IsValidWrapperAddress((m_IDirectDrawSurface2*)lpDDSrcSurface) ||
				IsValidWrapperAddress((m_IDirectDrawSurface3*)lpDDSrcSurface) ||
				IsValidWrapperAddress((m_IDirectDrawSurface4*)lpDDSrcSurface) ||
				IsValidWrapperAddress((m_IDirectDrawSurface7*)lpDDSrcSurface));
	}

	template <typename T>
	void SaveAddress(T *Wrapper, void *Proxy)
	{
		constexpr UINT CacheIndex = AddressCacheIndex<T>::CacheIndex;
		if (Wrapper && Proxy)
		{
			g_map[CacheIndex][Proxy] = Wrapper;
			reverse_map[CacheIndex][Wrapper] = Proxy;  // Update reverse map
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

		// Remove from g_map
		for (auto it = g_map[CacheIndex].begin(); it != g_map[CacheIndex].end(); ++it)
		{
			if (it->second == Wrapper)
			{
				g_map[CacheIndex].erase(it);
				break;
			}
		}

		// Remove from reverse_map
		reverse_map[CacheIndex].erase(Wrapper);

#pragma warning (push)
#pragma warning (disable : 4127)
		if (CacheIndex == AddressCacheIndex<m_IDirectDrawX>::CacheIndex &&
			g_map[AddressCacheIndex<m_IDirectDrawX>::CacheIndex].size() == 0)
		{
			DeleteAll();
		}
#pragma warning (pop)
	}
};

class AddressLookupTableDdrawObject
{
public:
	virtual ~AddressLookupTableDdrawObject() {}

	void DeleteMe()
	{
		delete this;
	}
};
