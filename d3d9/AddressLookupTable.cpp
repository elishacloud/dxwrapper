#include "d3d9.h"
#include "Libraries\ComPtr.h"

// Destructor for AddressLookupTableD3d9Object
AddressLookupTableD3d9Object::~AddressLookupTableD3d9Object() {}

// DeleteMe method
void AddressLookupTableD3d9Object::DeleteMe()
{
	delete this;
}

// Constructor for AddressLookupTableD3d9
AddressLookupTableD3d9::AddressLookupTableD3d9() {}

// Destructor for AddressLookupTableD3d9
AddressLookupTableD3d9::~AddressLookupTableD3d9()
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

// Specializations for CreateInterface methods
template <>
m_IDirect3D9Ex* AddressLookupTableD3d9::CreateInterface<class m_IDirect3D9Ex, void, LPVOID>(
	m_IDirect3D9Ex* Proxy, void*, REFIID riid, void*)
{
	return new m_IDirect3D9Ex(static_cast<m_IDirect3D9Ex*>(Proxy), riid);
}

template <>
m_IDirect3DDevice9Ex* AddressLookupTableD3d9::CreateInterface<m_IDirect3DDevice9Ex, m_IDirect3D9Ex, UINT>(
	m_IDirect3DDevice9Ex* Proxy, m_IDirect3D9Ex* Device, REFIID riid, UINT Data)
{
	return new m_IDirect3DDevice9Ex(static_cast<m_IDirect3DDevice9Ex*>(Proxy), Device, riid, Data);
}

template <>
m_IDirect3DCubeTexture9* AddressLookupTableD3d9::CreateInterface<m_IDirect3DCubeTexture9, m_IDirect3DDevice9Ex, LPVOID>(
	m_IDirect3DCubeTexture9* Proxy, m_IDirect3DDevice9Ex* Device, REFIID, void*)
{
	return new m_IDirect3DCubeTexture9(static_cast<m_IDirect3DCubeTexture9*>(Proxy), Device);
}

template <>
m_IDirect3DIndexBuffer9* AddressLookupTableD3d9::CreateInterface<m_IDirect3DIndexBuffer9, m_IDirect3DDevice9Ex, LPVOID>(
	m_IDirect3DIndexBuffer9* Proxy, m_IDirect3DDevice9Ex* Device, REFIID, void*)
{
	return new m_IDirect3DIndexBuffer9(static_cast<m_IDirect3DIndexBuffer9*>(Proxy), Device);
}

template <>
m_IDirect3DPixelShader9* AddressLookupTableD3d9::CreateInterface<m_IDirect3DPixelShader9, m_IDirect3DDevice9Ex, LPVOID>(
	m_IDirect3DPixelShader9* Proxy, m_IDirect3DDevice9Ex* Device, REFIID, void*)
{
	return new m_IDirect3DPixelShader9(static_cast<m_IDirect3DPixelShader9*>(Proxy), Device);
}

template <>
m_IDirect3DQuery9* AddressLookupTableD3d9::CreateInterface<m_IDirect3DQuery9, m_IDirect3DDevice9Ex, LPVOID>(
	m_IDirect3DQuery9* Proxy, m_IDirect3DDevice9Ex* Device, REFIID, void*)
{
	return new m_IDirect3DQuery9(static_cast<m_IDirect3DQuery9*>(Proxy), Device);
}

template <>
m_IDirect3DStateBlock9* AddressLookupTableD3d9::CreateInterface<m_IDirect3DStateBlock9, m_IDirect3DDevice9Ex, LPVOID>(
	m_IDirect3DStateBlock9* Proxy, m_IDirect3DDevice9Ex* Device, REFIID, void*)
{
	return new m_IDirect3DStateBlock9(static_cast<m_IDirect3DStateBlock9*>(Proxy), Device);
}

template <>
m_IDirect3DSurface9* AddressLookupTableD3d9::CreateInterface<m_IDirect3DSurface9, m_IDirect3DDevice9Ex, LPVOID>(
	m_IDirect3DSurface9* Proxy, m_IDirect3DDevice9Ex* Device, REFIID, void*)
{
	return new m_IDirect3DSurface9(static_cast<m_IDirect3DSurface9*>(Proxy), Device);
}

template <>
m_IDirect3DSwapChain9Ex* AddressLookupTableD3d9::CreateInterface<m_IDirect3DSwapChain9Ex, m_IDirect3DDevice9Ex, LPVOID>(
	m_IDirect3DSwapChain9Ex* Proxy, m_IDirect3DDevice9Ex* Device, REFIID riid, void*)
{
	return new m_IDirect3DSwapChain9Ex(static_cast<m_IDirect3DSwapChain9Ex*>(Proxy), Device, riid);
}

template <>
m_IDirect3DTexture9* AddressLookupTableD3d9::CreateInterface<m_IDirect3DTexture9, m_IDirect3DDevice9Ex, LPVOID>(
	m_IDirect3DTexture9* Proxy, m_IDirect3DDevice9Ex* Device, REFIID, void*)
{
	return new m_IDirect3DTexture9(static_cast<m_IDirect3DTexture9*>(Proxy), Device);
}

template <>
m_IDirect3DVertexBuffer9* AddressLookupTableD3d9::CreateInterface<m_IDirect3DVertexBuffer9, m_IDirect3DDevice9Ex, LPVOID>(
	m_IDirect3DVertexBuffer9* Proxy, m_IDirect3DDevice9Ex* Device, REFIID, void*)
{
	return new m_IDirect3DVertexBuffer9(static_cast<m_IDirect3DVertexBuffer9*>(Proxy), Device);
}

template <>
m_IDirect3DVertexDeclaration9* AddressLookupTableD3d9::CreateInterface<m_IDirect3DVertexDeclaration9, m_IDirect3DDevice9Ex, LPVOID>(
	m_IDirect3DVertexDeclaration9* Proxy, m_IDirect3DDevice9Ex* Device, REFIID, void*)
{
	return new m_IDirect3DVertexDeclaration9(static_cast<m_IDirect3DVertexDeclaration9*>(Proxy), Device);
}

template <>
m_IDirect3DVertexShader9* AddressLookupTableD3d9::CreateInterface<m_IDirect3DVertexShader9, m_IDirect3DDevice9Ex, LPVOID>(
	m_IDirect3DVertexShader9* Proxy, m_IDirect3DDevice9Ex* Device, REFIID, void*)
{
	return new m_IDirect3DVertexShader9(static_cast<m_IDirect3DVertexShader9*>(Proxy), Device);
}

template <>
m_IDirect3DVolume9* AddressLookupTableD3d9::CreateInterface<m_IDirect3DVolume9, m_IDirect3DDevice9Ex, LPVOID>(
	m_IDirect3DVolume9* Proxy, m_IDirect3DDevice9Ex* Device, REFIID, void*)
{
	return new m_IDirect3DVolume9(static_cast<m_IDirect3DVolume9*>(Proxy), Device);
}

template <>
m_IDirect3DVolumeTexture9* AddressLookupTableD3d9::CreateInterface<m_IDirect3DVolumeTexture9, m_IDirect3DDevice9Ex, LPVOID>(
	m_IDirect3DVolumeTexture9* Proxy, m_IDirect3DDevice9Ex* Device, REFIID, void*)
{
	return new m_IDirect3DVolumeTexture9(static_cast<m_IDirect3DVolumeTexture9*>(Proxy), Device);
}

template <>
m_IDirect3DVideoDevice9* AddressLookupTableD3d9::CreateInterface<m_IDirect3DVideoDevice9, m_IDirect3DDevice9Ex, LPVOID>(
	m_IDirect3DVideoDevice9* Proxy, m_IDirect3DDevice9Ex* Device, REFIID, void*)
{
	return new m_IDirect3DVideoDevice9(static_cast<m_IDirect3DVideoDevice9*>(Proxy), Device);
}

template <>
m_IDirect3DDXVADevice9* AddressLookupTableD3d9::CreateInterface<m_IDirect3DDXVADevice9, m_IDirect3DDevice9Ex, LPVOID>(
	m_IDirect3DDXVADevice9* Proxy, m_IDirect3DDevice9Ex* Device, REFIID, void*)
{
	return new m_IDirect3DDXVADevice9(static_cast<m_IDirect3DDXVADevice9*>(Proxy), Device);
}

template <typename T>
IUnknown* AddressLookupTableD3d9::GetIndentityInterface(IUnknown* Proxy)
{
	constexpr UINT CacheIndex = AddressCacheIndex<T>::CacheIndex;

	if constexpr (CacheIndex != AddressCacheIndex<m_IDirect3D9Ex>::CacheIndex &&
		CacheIndex != AddressCacheIndex<m_IDirect3DDevice9Ex>::CacheIndex &&
		CacheIndex != AddressCacheIndex<m_IDirect3DSwapChain9Ex>::CacheIndex)
	{
		ComPtr<IUnknown> pAddress;
		if (SUCCEEDED(Proxy->QueryInterface(IID_IUnknown, (void**)pAddress.GetAddressOf())) && pAddress.Get())
		{
			return pAddress.Get();
		}
	}

	return Proxy;
}

template <typename T>
T* AddressLookupTableD3d9::FindInterface(IUnknown* Proxy)
{
	constexpr UINT CacheIndex = AddressCacheIndex<T>::CacheIndex;

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

template m_IDirect3D9Ex* AddressLookupTableD3d9::FindCreateAddress<m_IDirect3D9Ex, void, LPVOID>(IUnknown*, void*, REFIID, LPVOID);
template m_IDirect3DDevice9Ex* AddressLookupTableD3d9::FindCreateAddress<m_IDirect3DDevice9Ex, m_IDirect3D9Ex, UINT>(IUnknown*, m_IDirect3D9Ex*, REFIID, UINT);
template m_IDirect3DCubeTexture9* AddressLookupTableD3d9::FindCreateAddress<m_IDirect3DCubeTexture9, m_IDirect3DDevice9Ex, LPVOID>(IUnknown*, m_IDirect3DDevice9Ex*, REFIID, LPVOID);
template m_IDirect3DIndexBuffer9* AddressLookupTableD3d9::FindCreateAddress<m_IDirect3DIndexBuffer9, m_IDirect3DDevice9Ex, LPVOID>(IUnknown*, m_IDirect3DDevice9Ex*, REFIID, LPVOID);
template m_IDirect3DPixelShader9* AddressLookupTableD3d9::FindCreateAddress<m_IDirect3DPixelShader9, m_IDirect3DDevice9Ex, LPVOID>(IUnknown*, m_IDirect3DDevice9Ex*, REFIID, LPVOID);
template m_IDirect3DQuery9* AddressLookupTableD3d9::FindCreateAddress<m_IDirect3DQuery9, m_IDirect3DDevice9Ex, LPVOID>(IUnknown*, m_IDirect3DDevice9Ex*, REFIID, LPVOID);
template m_IDirect3DStateBlock9* AddressLookupTableD3d9::FindCreateAddress<m_IDirect3DStateBlock9, m_IDirect3DDevice9Ex, LPVOID>(IUnknown*, m_IDirect3DDevice9Ex*, REFIID, LPVOID);
template m_IDirect3DSurface9* AddressLookupTableD3d9::FindCreateAddress<m_IDirect3DSurface9, m_IDirect3DDevice9Ex, LPVOID>(IUnknown*, m_IDirect3DDevice9Ex*, REFIID, LPVOID);
template m_IDirect3DSwapChain9Ex* AddressLookupTableD3d9::FindCreateAddress<m_IDirect3DSwapChain9Ex, m_IDirect3DDevice9Ex, LPVOID>(IUnknown*, m_IDirect3DDevice9Ex*, REFIID, LPVOID);
template m_IDirect3DTexture9* AddressLookupTableD3d9::FindCreateAddress<m_IDirect3DTexture9, m_IDirect3DDevice9Ex, LPVOID>(IUnknown*, m_IDirect3DDevice9Ex*, REFIID, LPVOID);
template m_IDirect3DVertexBuffer9* AddressLookupTableD3d9::FindCreateAddress<m_IDirect3DVertexBuffer9, m_IDirect3DDevice9Ex, LPVOID>(IUnknown*, m_IDirect3DDevice9Ex*, REFIID, LPVOID);
template m_IDirect3DVertexDeclaration9* AddressLookupTableD3d9::FindCreateAddress<m_IDirect3DVertexDeclaration9, m_IDirect3DDevice9Ex, LPVOID>(IUnknown*, m_IDirect3DDevice9Ex*, REFIID, LPVOID);
template m_IDirect3DVertexShader9* AddressLookupTableD3d9::FindCreateAddress<m_IDirect3DVertexShader9, m_IDirect3DDevice9Ex, LPVOID>(IUnknown*, m_IDirect3DDevice9Ex*, REFIID, LPVOID);
template m_IDirect3DVolume9* AddressLookupTableD3d9::FindCreateAddress<m_IDirect3DVolume9, m_IDirect3DDevice9Ex, LPVOID>(IUnknown*, m_IDirect3DDevice9Ex*, REFIID, LPVOID);
template m_IDirect3DVolumeTexture9* AddressLookupTableD3d9::FindCreateAddress<m_IDirect3DVolumeTexture9, m_IDirect3DDevice9Ex, LPVOID>(IUnknown*, m_IDirect3DDevice9Ex*, REFIID, LPVOID);
template m_IDirect3DVideoDevice9* AddressLookupTableD3d9::FindCreateAddress<m_IDirect3DVideoDevice9, m_IDirect3DDevice9Ex, LPVOID>(IUnknown*, m_IDirect3DDevice9Ex*, REFIID, LPVOID);
template m_IDirect3DDXVADevice9* AddressLookupTableD3d9::FindCreateAddress<m_IDirect3DDXVADevice9, m_IDirect3DDevice9Ex, LPVOID>(IUnknown*, m_IDirect3DDevice9Ex*, REFIID, LPVOID);
template <typename T, typename D, typename L>
T* AddressLookupTableD3d9::FindCreateAddress(IUnknown* Proxy, D* Device, REFIID riid, L Data)
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

template m_IDirect3D9Ex* AddressLookupTableD3d9::FindAddress<m_IDirect3D9Ex>(IUnknown*);
template m_IDirect3DDevice9Ex* AddressLookupTableD3d9::FindAddress<m_IDirect3DDevice9Ex>(IUnknown*);
template m_IDirect3DCubeTexture9* AddressLookupTableD3d9::FindAddress<m_IDirect3DCubeTexture9>(IUnknown*);
template m_IDirect3DIndexBuffer9* AddressLookupTableD3d9::FindAddress<m_IDirect3DIndexBuffer9>(IUnknown*);
template m_IDirect3DPixelShader9* AddressLookupTableD3d9::FindAddress<m_IDirect3DPixelShader9>(IUnknown*);
template m_IDirect3DQuery9* AddressLookupTableD3d9::FindAddress<m_IDirect3DQuery9>(IUnknown*);
template m_IDirect3DStateBlock9* AddressLookupTableD3d9::FindAddress<m_IDirect3DStateBlock9>(IUnknown*);
template m_IDirect3DSurface9* AddressLookupTableD3d9::FindAddress<m_IDirect3DSurface9>(IUnknown*);
template m_IDirect3DSwapChain9Ex* AddressLookupTableD3d9::FindAddress<m_IDirect3DSwapChain9Ex>(IUnknown*);
template m_IDirect3DTexture9* AddressLookupTableD3d9::FindAddress<m_IDirect3DTexture9>(IUnknown*);
template m_IDirect3DVertexBuffer9* AddressLookupTableD3d9::FindAddress<m_IDirect3DVertexBuffer9>(IUnknown*);
template m_IDirect3DVertexDeclaration9* AddressLookupTableD3d9::FindAddress<m_IDirect3DVertexDeclaration9>(IUnknown*);
template m_IDirect3DVertexShader9* AddressLookupTableD3d9::FindAddress<m_IDirect3DVertexShader9>(IUnknown*);
template m_IDirect3DVolume9* AddressLookupTableD3d9::FindAddress<m_IDirect3DVolume9>(IUnknown*);
template m_IDirect3DVolumeTexture9* AddressLookupTableD3d9::FindAddress<m_IDirect3DVolumeTexture9>(IUnknown*);
template m_IDirect3DVideoDevice9* AddressLookupTableD3d9::FindAddress<m_IDirect3DVideoDevice9>(IUnknown*);
template m_IDirect3DDXVADevice9* AddressLookupTableD3d9::FindAddress<m_IDirect3DDXVADevice9>(IUnknown*);
template <typename T>
T* AddressLookupTableD3d9::FindAddress(IUnknown* Proxy)
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

template IDirect3DSurface9* AddressLookupTableD3d9::GetSafeProxyInterface<m_IDirect3DSurface9, IDirect3DSurface9>(m_IDirect3DSurface9*);
template <typename T, typename M>
M* AddressLookupTableD3d9::GetSafeProxyInterface(T* WrapperInterface)
{
	constexpr UINT CacheIndex = AddressCacheIndex<T>::CacheIndex;

	for (const auto& pair : g_map[CacheIndex])
	{
		if (pair.second == WrapperInterface)
		{
			return WrapperInterface->GetProxyInterface();
		}
	}

	return nullptr;
}

template void AddressLookupTableD3d9::SaveAddress<m_IDirect3D9Ex>(m_IDirect3D9Ex*, IUnknown*);
template <typename T>
void AddressLookupTableD3d9::SaveAddress(T* Wrapper, IUnknown* Proxy)
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
		// If the entry exists, call DeleteMe() on the existing object
		if (it->second)
		{
			it->second->DeleteMe();
		}
	}

	// Now save the new entry in the map
	g_map[CacheIndex][identity] = Wrapper;
}

template void AddressLookupTableD3d9::DeleteAddress<m_IDirect3D9Ex>(m_IDirect3D9Ex*);
template void AddressLookupTableD3d9::DeleteAddress<m_IDirect3DCubeTexture9>(m_IDirect3DCubeTexture9*);
template void AddressLookupTableD3d9::DeleteAddress<m_IDirect3DIndexBuffer9>(m_IDirect3DIndexBuffer9*);
template void AddressLookupTableD3d9::DeleteAddress<m_IDirect3DPixelShader9>(m_IDirect3DPixelShader9*);
template void AddressLookupTableD3d9::DeleteAddress<m_IDirect3DQuery9>(m_IDirect3DQuery9*);
template void AddressLookupTableD3d9::DeleteAddress<m_IDirect3DStateBlock9>(m_IDirect3DStateBlock9*);
template void AddressLookupTableD3d9::DeleteAddress<m_IDirect3DSurface9>(m_IDirect3DSurface9*);
template void AddressLookupTableD3d9::DeleteAddress<m_IDirect3DSwapChain9Ex>(m_IDirect3DSwapChain9Ex*);
template void AddressLookupTableD3d9::DeleteAddress<m_IDirect3DTexture9>(m_IDirect3DTexture9*);
template void AddressLookupTableD3d9::DeleteAddress<m_IDirect3DVertexBuffer9>(m_IDirect3DVertexBuffer9*);
template void AddressLookupTableD3d9::DeleteAddress<m_IDirect3DVertexDeclaration9>(m_IDirect3DVertexDeclaration9*);
template void AddressLookupTableD3d9::DeleteAddress<m_IDirect3DVertexShader9>(m_IDirect3DVertexShader9*);
template void AddressLookupTableD3d9::DeleteAddress<m_IDirect3DVolume9>(m_IDirect3DVolume9*);
template void AddressLookupTableD3d9::DeleteAddress<m_IDirect3DVolumeTexture9>(m_IDirect3DVolumeTexture9*);
template void AddressLookupTableD3d9::DeleteAddress<m_IDirect3DVideoDevice9>(m_IDirect3DVideoDevice9*);
template void AddressLookupTableD3d9::DeleteAddress<m_IDirect3DDXVADevice9>(m_IDirect3DDXVADevice9*);
template <typename T>
void AddressLookupTableD3d9::DeleteAddress(T* Wrapper)
{
	if (!Wrapper || ConstructorFlag)
	{
		return;
	}

	constexpr UINT CacheIndex = AddressCacheIndex<T>::CacheIndex;

	auto it = std::find_if(g_map[CacheIndex].begin(), g_map[CacheIndex].end(),
		[=](auto& Map) -> bool { return Map.second == Wrapper; });
	if (it != std::end(g_map[CacheIndex]))
	{
		g_map[CacheIndex].erase(it);
	}
}

StateBlockCache::~StateBlockCache()
{
	while (stateBlocks.size())
	{
		m_IDirect3DStateBlock9* Interface = stateBlocks.back();
		RemoveStateBlock(Interface);
		Interface->DeleteMe();
	}
}

void StateBlockCache::AddStateBlock(m_IDirect3DStateBlock9* stateBlock)
{
	if (stateBlock == nullptr) return;

	// Check if the state block already exists
	auto it = std::find(stateBlocks.begin(), stateBlocks.end(), stateBlock);
	if (it == stateBlocks.end())
	{
		stateBlocks.push_back(stateBlock);

		// If we exceed the max allowed state blocks, remove the oldest ones
		if (Config.LimitStateBlocks && stateBlocks.size() > MAX_STATE_BLOCKS)
		{
			m_IDirect3DStateBlock9* StateBlockX = stateBlocks.front();
			StateBlockX->Release();
		}
	}
}

void StateBlockCache::RemoveStateBlock(m_IDirect3DStateBlock9* stateBlock)
{
	auto it = std::find(stateBlocks.begin(), stateBlocks.end(), stateBlock);
	if (it != stateBlocks.end())
	{
		stateBlocks.erase(it);
	}
}
