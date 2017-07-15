/**
 * Copyright (C) 2015 Patrick Mours. All rights reserved.
 * License: https://github.com/crosire/d3d8to9#license
 */

#pragma once

#include <vector>
#include <iostream>

class AddressLookupTable
{
	struct AddressStruct
	{
		class AddressLookupTableObject *Address8 = nullptr;
		void *Address9 = nullptr;
	};

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

		T *pAddress8 = nullptr;
		constexpr UINT CacheIndex = AddressCacheIndex<T>::CacheIndex;

		for (size_t i = 0; i < AddressCache[CacheIndex].size(); i++)
		{
			if (AddressCache[CacheIndex][i].Address9 == pAddress9)
			{
				pAddress8 = static_cast<T *>(AddressCache[CacheIndex][i].Address8);
			}
		}

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

		AddressStruct CacheData;
		CacheData.Address8 = pAddress8;
		CacheData.Address9 = pAddress9;

		AddressCache[AddressCacheIndex<T>::CacheIndex].push_back(std::move(CacheData));
	}
	template <typename T>
	void DeleteAddress(T *pAddress8)
	{
		if (pAddress8 == nullptr)
		{
			return;
		}

		constexpr UINT CacheIndex = AddressCacheIndex<T>::CacheIndex;

		for (size_t i = 0; i < AddressCache[CacheIndex].size(); i++)
		{
			if (AddressCache[CacheIndex][i].Address8 == pAddress8)
			{
				std::swap(AddressCache[CacheIndex][i], AddressCache[CacheIndex].back());
				AddressCache[CacheIndex].pop_back();
				return;
			}
		}
	}

private:
	Direct3DDevice8 *const Device;
	std::vector<AddressStruct> AddressCache[8];
};

class AddressLookupTableObject
{
public:
	virtual ~AddressLookupTableObject() { }

	void DeleteMe(bool CleanUp = true)
	{
		CleanUpFlag = CleanUp;

		delete this;
	}

protected:
	bool Active = true;
	bool CleanUpFlag = true;
};
