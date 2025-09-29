#include <cassert>
#include <cstring>
#include <map>

#include <string>

#include "CompatRegistry.h"
#include "DDrawCompat\DDrawLog.h"
#include "Hook.h"

namespace Compat21
{
	namespace
	{
		inline std::wstring ToWString(const char* s)
		{
			if (!s) return L"";
			size_t len = strlen(s);
			std::wstring ws(len, L'\0');
			size_t converted = 0;
			mbstowcs_s(&converted, &ws[0], len + 1, s, len);

			return ws;
		}

		inline int CompareNoCase(const std::wstring& a, const std::wstring& b)
		{
			// LOCALE_INVARIANT avoids locale-specific differences
			return CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE,
				a.c_str(), (int)a.length(),
				b.c_str(), (int)b.length()) - CSTR_EQUAL;
		}

		struct RegistryKey
		{
			HKEY key;
			std::wstring subKey;
			std::wstring value;

			RegistryKey(HKEY key, const std::wstring& subKey, const std::wstring& value) : key(key), subKey(subKey), value(value) {}

			RegistryKey(HKEY key, const char* subKey, const char* value) : key(key), subKey(ToWString(subKey)), value(ToWString(value)) {}

			bool operator<(const RegistryKey& rhs) const
			{
				if (key < rhs.key) { return true; }
				if (key > rhs.key) { return false; }
				const int subKeyComp = CompareNoCase(subKey, rhs.subKey);
				if (subKeyComp < 0) { return true; }
				if (subKeyComp > 0) { return false; }
				return CompareNoCase(value, rhs.value) < 0;
			}

			bool operator==(const RegistryKey& rhs) const
			{
				return key == rhs.key &&
					0 == CompareNoCase(subKey, rhs.subKey) &&
					0 == CompareNoCase(value, rhs.value);
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
