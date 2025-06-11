#pragma once

#include <deque>
#include <unordered_map>
#include <algorithm>

constexpr UINT MaxIndex = 16;

class AddressLookupTableD3d9Object
{
public:
	virtual ~AddressLookupTableD3d9Object();
	void DeleteMe();
};

class AddressLookupTableD3d9
{
public:
	explicit AddressLookupTableD3d9();
	~AddressLookupTableD3d9();

	template <typename T>
	struct AddressCacheIndex { static constexpr UINT CacheIndex = 0; };

	// Specializations for AddressCacheIndex
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

	// General template function for CreateInterface
	template <typename T, typename D, typename L>
	T* CreateInterface(T* Proxy, D* Device, REFIID riid, L Data);

	template <typename T, typename D, typename L>
	T* FindAddress(void* Proxy, D* Device, REFIID riid, L Data);

	template <typename T>
	void SaveAddress(T* Wrapper, void* Proxy);

	template <typename T>
	void DeleteAddress(T* Wrapper);

private:
	bool ConstructorFlag = false;
	std::unordered_map<void*, class AddressLookupTableD3d9Object*> g_map[MaxIndex];
};

class StateBlockCache
{
private:
	std::vector<m_IDirect3DStateBlock9*> stateBlocks;

public:
	~StateBlockCache();

	void AddStateBlock(m_IDirect3DStateBlock9* stateBlock);
	void RemoveStateBlock(m_IDirect3DStateBlock9* stateBlock);
};
