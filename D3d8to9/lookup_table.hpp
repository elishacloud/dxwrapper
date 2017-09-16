/**
 * Copyright (C) 2015 Patrick Mours. All rights reserved.
 * License: https://github.com/crosire/d3d8to9#license
 */

#pragma once

#include <unordered_map>

class AddressLookupTable
{
	template <typename T>
	struct AddressCacheIndex;
	template <>
	struct AddressCacheIndex<Direct3DSurface8>
	{ static constexpr UINT CacheIndex = 0; using Type9 = IDirect3DSurface9; };
	template <>
	struct AddressCacheIndex<Direct3DTexture8>
	{ static constexpr UINT CacheIndex = 1; using Type9 = IDirect3DTexture9; };
	template <>
	struct AddressCacheIndex<Direct3DVolumeTexture8>
	{ static constexpr UINT CacheIndex = 2; using Type9 = IDirect3DVolumeTexture9; };
	template <>
	struct AddressCacheIndex<Direct3DCubeTexture8>
	{ static constexpr UINT CacheIndex = 3; using Type9 = IDirect3DCubeTexture9; };
	template <>
	struct AddressCacheIndex<Direct3DVolume8>
	{ static constexpr UINT CacheIndex = 4; using Type9 = IDirect3DVolume9; };
	template <>
	struct AddressCacheIndex<Direct3DVertexBuffer8>
	{ static constexpr UINT CacheIndex = 5; using Type9 = IDirect3DVertexBuffer9; };
	template <>
	struct AddressCacheIndex<Direct3DIndexBuffer8>
	{ static constexpr UINT CacheIndex = 6; using Type9 = IDirect3DIndexBuffer9; };
	template <>
	struct AddressCacheIndex<Direct3DSwapChain8>
	{ static constexpr UINT CacheIndex = 7; using Type9 = IDirect3DSwapChain9; };


public:
	explicit AddressLookupTable(Direct3DDevice8 *Device);
	~AddressLookupTable();

	template <typename T>
	T *FindAddress(void *pAddress9)
	{
		if (pAddress9 == nullptr)
		{
			return nullptr;
		}

		T *pAddress8 = static_cast<T *>(AddressCache[AddressCacheIndex<T>::CacheIndex][pAddress9]);

		if (pAddress8 == nullptr)
		{
			pAddress8 = new T(Device, static_cast<AddressCacheIndex<T>::Type9 *>(pAddress9));
		}

		return pAddress8;
	}

	template <typename T>
	void SaveAddress(T *pAddress8, void *pAddress9)
	{
		if (pAddress8 == nullptr || pAddress9 == nullptr)
		{
			return;
		}

		AddressCache[AddressCacheIndex<T>::CacheIndex][pAddress9] = pAddress8;
	}

	template <typename T>
	void DeleteAddress(T *pAddress8)
	{
		if (pAddress8 == nullptr)
		{
			return;
		}

		constexpr UINT CacheIndex = AddressCacheIndex<T>::CacheIndex;
		for (auto it = AddressCache[CacheIndex].begin(); it != AddressCache[CacheIndex].end(); ++it)
		{
			if (it->second == pAddress8)
			{
				it = AddressCache[CacheIndex].erase(it);
				return;
			}
		}
	}

private:
	Direct3DDevice8 *const Device;
	std::unordered_map<void*, class AddressLookupTableObject*> AddressCache[8];
};

class AddressLookupTableObject
{
public:
	virtual ~AddressLookupTableObject() { }

	void DeleteMe()
	{
		delete this;
	}
};
