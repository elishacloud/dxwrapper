#pragma once

#include <unordered_map>
#include <algorithm>

constexpr UINT MaxIndex = 16;

template <typename D>
class AddressLookupTableD3d9
{
public:
	explicit AddressLookupTableD3d9(D*) {}
	~AddressLookupTableD3d9()
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
	struct AddressCacheIndex<m_IDirect3D9Ex> { static constexpr UINT CacheIndex = 1; };
	template <>
	struct AddressCacheIndex<m_IDirect3DDevice9Ex> { static constexpr UINT CacheIndex = 2; };
	template <>
	struct AddressCacheIndex<m_IDirect3DCubeTexture9> { static constexpr UINT CacheIndex = 3; };
	template <>
	struct AddressCacheIndex<m_IDirect3DIndexBuffer9> { static constexpr UINT CacheIndex = 4; };
	template <>
	struct AddressCacheIndex<m_IDirect3DPixelShader9> { static constexpr UINT CacheIndex = 5; };
	template <>
	struct AddressCacheIndex<m_IDirect3DQuery9> { static constexpr UINT CacheIndex = 6; };
	template <>
	struct AddressCacheIndex<m_IDirect3DStateBlock9> { static constexpr UINT CacheIndex = 7; };
	template <>
	struct AddressCacheIndex<m_IDirect3DSurface9> { static constexpr UINT CacheIndex = 8; };
	template <>
	struct AddressCacheIndex<m_IDirect3DSwapChain9Ex> { static constexpr UINT CacheIndex = 9; };
	template <>
	struct AddressCacheIndex<m_IDirect3DTexture9> { static constexpr UINT CacheIndex = 10; };
	template <>
	struct AddressCacheIndex<m_IDirect3DVertexBuffer9> { static constexpr UINT CacheIndex = 11; };
	template <>
	struct AddressCacheIndex<m_IDirect3DVertexDeclaration9> { static constexpr UINT CacheIndex = 12; };
	template <>
	struct AddressCacheIndex<m_IDirect3DVertexShader9> { static constexpr UINT CacheIndex = 13; };
	template <>
	struct AddressCacheIndex<m_IDirect3DVolume9> { static constexpr UINT CacheIndex = 14; };
	template <>
	struct AddressCacheIndex<m_IDirect3DVolumeTexture9> { static constexpr UINT CacheIndex = 15; };

	m_IDirect3D9Ex* CreateInterface(m_IDirect3D9Ex* Proxy, void*, REFIID riid)
	{
		return new m_IDirect3D9Ex(static_cast<m_IDirect3D9Ex*>(Proxy), riid);
	}
	m_IDirect3DDevice9Ex* CreateInterface(m_IDirect3DDevice9Ex* Proxy, m_IDirect3D9Ex* Device, REFIID riid)
	{
		return new m_IDirect3DDevice9Ex(static_cast<m_IDirect3DDevice9Ex*>(Proxy), Device, riid);
	}
	m_IDirect3DCubeTexture9* CreateInterface(m_IDirect3DCubeTexture9* Proxy, m_IDirect3DDevice9Ex* Device, REFIID)
	{
		return new m_IDirect3DCubeTexture9(static_cast<m_IDirect3DCubeTexture9*>(Proxy), Device);
	}
	m_IDirect3DIndexBuffer9* CreateInterface(m_IDirect3DIndexBuffer9* Proxy, m_IDirect3DDevice9Ex* Device, REFIID)
	{
		return new m_IDirect3DIndexBuffer9(static_cast<m_IDirect3DIndexBuffer9*>(Proxy), Device);
	}
	m_IDirect3DPixelShader9* CreateInterface(m_IDirect3DPixelShader9* Proxy, m_IDirect3DDevice9Ex* Device, REFIID)
	{
		return new m_IDirect3DPixelShader9(static_cast<m_IDirect3DPixelShader9*>(Proxy), Device);
	}
	m_IDirect3DQuery9* CreateInterface(m_IDirect3DQuery9* Proxy, m_IDirect3DDevice9Ex* Device, REFIID)
	{
		return new m_IDirect3DQuery9(static_cast<m_IDirect3DQuery9*>(Proxy), Device);
	}
	m_IDirect3DStateBlock9* CreateInterface(m_IDirect3DStateBlock9* Proxy, m_IDirect3DDevice9Ex* Device, REFIID)
	{
		return new m_IDirect3DStateBlock9(static_cast<m_IDirect3DStateBlock9*>(Proxy), Device);
	}
	m_IDirect3DSurface9* CreateInterface(m_IDirect3DSurface9* Proxy, m_IDirect3DDevice9Ex* Device, REFIID)
	{
		return new m_IDirect3DSurface9(static_cast<m_IDirect3DSurface9*>(Proxy), Device);
	}
	m_IDirect3DSwapChain9Ex* CreateInterface(m_IDirect3DSwapChain9Ex* Proxy, m_IDirect3DDevice9Ex* Device, REFIID riid)
	{
		return new m_IDirect3DSwapChain9Ex(static_cast<m_IDirect3DSwapChain9Ex*>(Proxy), Device, riid);
	}
	m_IDirect3DTexture9* CreateInterface(m_IDirect3DTexture9* Proxy, m_IDirect3DDevice9Ex* Device, REFIID)
	{
		return new m_IDirect3DTexture9(static_cast<m_IDirect3DTexture9*>(Proxy), Device);
	}
	m_IDirect3DVertexBuffer9* CreateInterface(m_IDirect3DVertexBuffer9* Proxy, m_IDirect3DDevice9Ex* Device, REFIID)
	{
		return new m_IDirect3DVertexBuffer9(static_cast<m_IDirect3DVertexBuffer9*>(Proxy), Device);
	}
	m_IDirect3DVertexDeclaration9* CreateInterface(m_IDirect3DVertexDeclaration9* Proxy, m_IDirect3DDevice9Ex* Device, REFIID)
	{
		return new m_IDirect3DVertexDeclaration9(static_cast<m_IDirect3DVertexDeclaration9*>(Proxy), Device);
	}
	m_IDirect3DVertexShader9* CreateInterface(m_IDirect3DVertexShader9* Proxy, m_IDirect3DDevice9Ex* Device, REFIID)
	{
		return new m_IDirect3DVertexShader9(static_cast<m_IDirect3DVertexShader9*>(Proxy), Device);
	}
	m_IDirect3DVolume9* CreateInterface(m_IDirect3DVolume9* Proxy, m_IDirect3DDevice9Ex* Device, REFIID)
	{
		return new m_IDirect3DVolume9(static_cast<m_IDirect3DVolume9*>(Proxy), Device);
	}
	m_IDirect3DVolumeTexture9* CreateInterface(m_IDirect3DVolumeTexture9* Proxy, m_IDirect3DDevice9Ex* Device, REFIID)
	{
		return new m_IDirect3DVolumeTexture9(static_cast<m_IDirect3DVolumeTexture9*>(Proxy), Device);
	}

	template <typename T, typename D>
	T *FindAddress(void* Proxy, D* Device, REFIID riid)
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

		return CreateInterface((T*)Proxy, Device, riid);
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

	UINT GetDeviceCount()
	{
		if (ConstructorFlag)
		{
			return TRUE;
		}
		return g_map[AddressCacheIndex<m_IDirect3DDevice9Ex>::CacheIndex].size();
	}

private:
	bool ConstructorFlag = false;
	std::unordered_map<void*, class AddressLookupTableD3d9Object*> g_map[MaxIndex];
};

class AddressLookupTableD3d9Object
{
public:
	virtual ~AddressLookupTableD3d9Object() {}

	void DeleteMe()
	{
		delete this;
	}
};
