#pragma once

#include <unordered_map>
#include <algorithm>

constexpr UINT MaxIndex = 21;

template <typename D>
class AddressLookupTableDsound
{
public:
	explicit AddressLookupTableDsound() {}
	~AddressLookupTableDsound()
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
	struct AddressCacheIndex<m_IDirectSound8> { static constexpr UINT CacheIndex = 1; };
	template <>
	struct AddressCacheIndex<m_IDirectSound3DBuffer8> { static constexpr UINT CacheIndex = 2; };
	template <>
	struct AddressCacheIndex<m_IDirectSound3DListener8> { static constexpr UINT CacheIndex = 3; };
	template <>
	struct AddressCacheIndex<m_IDirectSoundBuffer8> { static constexpr UINT CacheIndex = 4; };
	template <>
	struct AddressCacheIndex<m_IDirectSoundCapture8> { static constexpr UINT CacheIndex = 5; };
	template <>
	struct AddressCacheIndex<m_IDirectSoundCaptureBuffer8> { static constexpr UINT CacheIndex = 6; };
	template <>
	struct AddressCacheIndex<m_IDirectSoundCaptureFXAec8> { static constexpr UINT CacheIndex = 7; };
	template <>
	struct AddressCacheIndex<m_IDirectSoundCaptureFXNoiseSuppress8> { static constexpr UINT CacheIndex = 8; };
	template <>
	struct AddressCacheIndex<m_IDirectSoundFullDuplex8> { static constexpr UINT CacheIndex = 9; };
	template <>
	struct AddressCacheIndex<m_IDirectSoundFXChorus8> { static constexpr UINT CacheIndex = 10; };
	template <>
	struct AddressCacheIndex<m_IDirectSoundFXCompressor8> { static constexpr UINT CacheIndex = 11; };
	template <>
	struct AddressCacheIndex<m_IDirectSoundFXDistortion8> { static constexpr UINT CacheIndex = 12; };
	template <>
	struct AddressCacheIndex<m_IDirectSoundFXEcho8> { static constexpr UINT CacheIndex = 13; };
	template <>
	struct AddressCacheIndex<m_IDirectSoundFXFlanger8> { static constexpr UINT CacheIndex = 14; };
	template <>
	struct AddressCacheIndex<m_IDirectSoundFXGargle8> { static constexpr UINT CacheIndex = 15; };
	template <>
	struct AddressCacheIndex<m_IDirectSoundFXI3DL2Reverb8> { static constexpr UINT CacheIndex = 16; };
	template <>
	struct AddressCacheIndex<m_IDirectSoundFXParamEq8> { static constexpr UINT CacheIndex = 17; };
	template <>
	struct AddressCacheIndex<m_IDirectSoundFXWavesReverb8> { static constexpr UINT CacheIndex = 18; };
	template <>
	struct AddressCacheIndex<m_IDirectSoundNotify8> { static constexpr UINT CacheIndex = 19; };
	template <>
	struct AddressCacheIndex<m_IKsPropertySet> { static constexpr UINT CacheIndex = 20; };

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
	std::unordered_map<void*, class AddressLookupTableDsoundObject*> g_map[MaxIndex];
};

class AddressLookupTableDsoundObject
{
public:
	virtual ~AddressLookupTableDsoundObject() {}

	void DeleteMe()
	{
		delete this;
	}
};
