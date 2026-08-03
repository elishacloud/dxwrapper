#pragma once

#include <unordered_map>
#include <algorithm>
#include "dsound.h"

class AddressLookupTableDsoundObjectBase
{
public:
	virtual ~AddressLookupTableDsoundObjectBase() = default;
};

template<typename T>
class AddressLookupTableDsoundObject;

class AddressLookupTableDsound
{
private:
	static constexpr size_t MaxCacheIndex = 20;

	bool ConstructorFlag = false;
	std::unordered_map<void*, class AddressLookupTableDsoundObjectBase*> g_map[MaxCacheIndex];

	void DeleteAll()
	{
		for (const auto& map : g_map)
		{
			for (const auto& entry : map)
			{
				delete entry.second;
			}
		}
	}

public:
	explicit AddressLookupTableDsound() {}
	~AddressLookupTableDsound()
	{
		ConstructorFlag = true;
		DeleteAll();
	}

	template <typename T>
	struct AddressCacheIndex {};
	template <>
	struct AddressCacheIndex<m_IKsPropertySet> { static constexpr size_t CacheIndex = 0; };
	template <>
	struct AddressCacheIndex<m_IDirectSound8> { static constexpr size_t CacheIndex = 1; };
	template <>
	struct AddressCacheIndex<m_IDirectSound3DBuffer8> { static constexpr size_t CacheIndex = 2; };
	template <>
	struct AddressCacheIndex<m_IDirectSound3DListener8> { static constexpr size_t CacheIndex = 3; };
	template <>
	struct AddressCacheIndex<m_IDirectSoundBuffer8> { static constexpr size_t CacheIndex = 4; };
	template <>
	struct AddressCacheIndex<m_IDirectSoundCapture8> { static constexpr size_t CacheIndex = 5; };
	template <>
	struct AddressCacheIndex<m_IDirectSoundCaptureBuffer8> { static constexpr size_t CacheIndex = 6; };
	template <>
	struct AddressCacheIndex<m_IDirectSoundCaptureFXAec8> { static constexpr size_t CacheIndex = 7; };
	template <>
	struct AddressCacheIndex<m_IDirectSoundCaptureFXNoiseSuppress8> { static constexpr size_t CacheIndex = 8; };
	template <>
	struct AddressCacheIndex<m_IDirectSoundFullDuplex8> { static constexpr size_t CacheIndex = 9; };
	template <>
	struct AddressCacheIndex<m_IDirectSoundFXChorus8> { static constexpr size_t CacheIndex = 10; };
	template <>
	struct AddressCacheIndex<m_IDirectSoundFXCompressor8> { static constexpr size_t CacheIndex = 11; };
	template <>
	struct AddressCacheIndex<m_IDirectSoundFXDistortion8> { static constexpr size_t CacheIndex = 12; };
	template <>
	struct AddressCacheIndex<m_IDirectSoundFXEcho8> { static constexpr size_t CacheIndex = 13; };
	template <>
	struct AddressCacheIndex<m_IDirectSoundFXFlanger8> { static constexpr size_t CacheIndex = 14; };
	template <>
	struct AddressCacheIndex<m_IDirectSoundFXGargle8> { static constexpr size_t CacheIndex = 15; };
	template <>
	struct AddressCacheIndex<m_IDirectSoundFXI3DL2Reverb8> { static constexpr size_t CacheIndex = 16; };
	template <>
	struct AddressCacheIndex<m_IDirectSoundFXParamEq8> { static constexpr size_t CacheIndex = 17; };
	template <>
	struct AddressCacheIndex<m_IDirectSoundFXWavesReverb8> { static constexpr size_t CacheIndex = 18; };
	template <>
	struct AddressCacheIndex<m_IDirectSoundNotify8> { static constexpr size_t CacheIndex = 19; };

	template <typename T>
	T *FindAddress(void *Proxy)
	{
		if (!Proxy)
		{
			return nullptr;
		}

		constexpr size_t CacheIndex = AddressCacheIndex<T>::CacheIndex;

		auto it = g_map[CacheIndex].find(Proxy);
		if (it != std::end(g_map[CacheIndex]))
		{
			return static_cast<T *>(it->second);
		}

		return new T(static_cast<T *>(Proxy));
	}

	template <typename T>
	void SaveAddress(AddressLookupTableDsoundObject<T> *Wrapper, void *Proxy)
	{
		constexpr size_t CacheIndex = AddressCacheIndex<T>::CacheIndex;
		if (Wrapper && Proxy)
		{
			g_map[CacheIndex][Proxy] = Wrapper;
		}
	}

	template <typename T>
	void DeleteAddress(AddressLookupTableDsoundObject<T>*Wrapper)
	{
		if (!Wrapper || ConstructorFlag)
		{
			return;
		}

		constexpr size_t CacheIndex = AddressCacheIndex<T>::CacheIndex;

		auto it = std::find_if(g_map[CacheIndex].begin(), g_map[CacheIndex].end(),
			[=](auto& Map) -> bool { return Map.second == Wrapper; });
		if (it != std::end(g_map[CacheIndex]))
		{
			g_map[CacheIndex].erase(it);
		}
	}
};

template<typename T>
class AddressLookupTableDsoundObject : public AddressLookupTableDsoundObjectBase
{
public:
	AddressLookupTableDsoundObject(void* Proxy)
	{
		ProxyAddressLookupTableDsound.SaveAddress(this, Proxy);
	}

	~AddressLookupTableDsoundObject()
	{
		ProxyAddressLookupTableDsound.DeleteAddress(this);
	}
};
