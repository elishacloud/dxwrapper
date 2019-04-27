#include <cassert>
#include <cstring>
#include <map>

#include <atlstr.h>

#include "CompatRegistry.h"
#include "DDrawCompat\DDrawLog.h"
#include "Hook.h"

namespace Compat21
{
	namespace
	{
		struct RegistryKey
		{
			HKEY key;
			CStringW subKey;
			CStringW value;

			RegistryKey(HKEY key, CStringW subKey, CStringW value) : key(key), subKey(subKey), value(value) {}

			bool operator<(const RegistryKey& rhs) const
			{
				if (key < rhs.key) { return true; }
				if (key > rhs.key) { return false; }
				const int subKeyComp = subKey.CompareNoCase(rhs.subKey);
				if (subKeyComp < 0) { return true; }
				if (subKeyComp > 0) { return false; }
				return value.CompareNoCase(rhs.value) < 0;
			}

			bool operator==(const RegistryKey& rhs) const
			{
				return key == rhs.key &&
					0 == subKey.CompareNoCase(rhs.subKey) &&
					0 == value.CompareNoCase(rhs.value);
			}
		};

		std::map<RegistryKey, DWORD> g_registryOverride;

		LSTATUS WINAPI regGetValueW(HKEY hkey, LPCWSTR lpSubKey, LPCWSTR lpValue,
			DWORD dwFlags, LPDWORD pdwType, PVOID pvData, LPDWORD pcbData)
		{
			Compat::LogEnter("regGetValueW", hkey, lpSubKey, lpValue, dwFlags, pdwType, pvData, pcbData);
			LSTATUS result = ERROR_SUCCESS;

			const auto it = hkey && lpSubKey && lpValue && (dwFlags & RRF_RT_REG_DWORD)
				? g_registryOverride.find(RegistryKey(hkey, lpSubKey, lpValue))
				: g_registryOverride.end();

			if (it != g_registryOverride.end())
			{
				if (pdwType)
				{
					*pdwType = REG_DWORD;
				}

				if (pvData)
				{
					if (!pcbData)
					{
						result = ERROR_INVALID_PARAMETER;
					}
					else if (*pcbData >= sizeof(DWORD))
					{
						std::memcpy(pvData, &it->second, sizeof(DWORD));
					}
					else
					{
						result = ERROR_MORE_DATA;
					}
				}

				if (pcbData)
				{
					*pcbData = sizeof(DWORD);
				}
			}
			else
			{
				result = CALL_ORIG_FUNC(RegGetValueW)(hkey, lpSubKey, lpValue, dwFlags, pdwType, pvData, pcbData);
			}

			Compat::LogLeave("regGetValueW", hkey, lpSubKey, lpValue, dwFlags, pdwType, pvData, pcbData) << result;
			return result;
		}
	}

	namespace CompatRegistry
	{
		void installHooks()
		{
			HOOK_FUNCTION(KernelBase, RegGetValueW, regGetValueW);
		}

		void setValue(HKEY key, const char* subKey, const char* valueName, DWORD value)
		{
			assert(key && subKey && valueName);
			g_registryOverride[RegistryKey(key, subKey, valueName)] = value;
		}
	}
}
