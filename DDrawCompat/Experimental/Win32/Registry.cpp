#include <cassert>
#include <cstring>
#include <map>
#include <set>

#include <atlstr.h>

#include "Common/Hook.h"
#include "DDrawLog.h"
#include "Win32/Registry.h"

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

	std::map<RegistryKey, DWORD> g_dwordValues;
	std::set<RegistryKey> g_unsetValues;

	CStringW getKeyName(HKEY key)
	{
		enum KEY_INFORMATION_CLASS
		{
			KeyBasicInformation = 0,
			KeyNodeInformation = 1,
			KeyFullInformation = 2,
			KeyNameInformation = 3,
			KeyCachedInformation = 4,
			KeyFlagsInformation = 5,
			KeyVirtualizationInformation = 6,
			KeyHandleTagsInformation = 7,
			MaxKeyInfoClass = 8
		};

		typedef NTSTATUS(WINAPI *NtQueryKeyFuncPtr)(
			HANDLE KeyHandle,
			KEY_INFORMATION_CLASS KeyInformationClass,
			PVOID KeyInformation,
			ULONG Length,
			PULONG ResultLength);

		static NtQueryKeyFuncPtr ntQueryKey = reinterpret_cast<NtQueryKeyFuncPtr>(
			GetProcAddress(GetModuleHandle("ntdll"), "NtQueryKey"));
		
		if (ntQueryKey)
		{
			struct KEY_NAME_INFORMATION
			{
				ULONG NameLength;
				WCHAR Name[256];
			};

			KEY_NAME_INFORMATION keyName = {};
			ULONG resultSize = 0;
			if (SUCCEEDED(ntQueryKey(key, KeyNameInformation, &keyName, sizeof(keyName), &resultSize)))
			{
				return CStringW(keyName.Name, keyName.NameLength / 2);
			}
		}

		return CStringW();
	}

	LONG WINAPI regGetValueW(HKEY hkey, LPCWSTR lpSubKey, LPCWSTR lpValue, 
		DWORD dwFlags, LPDWORD pdwType, PVOID pvData, LPDWORD pcbData)
	{
		Compat::LogEnter("regGetValueW", hkey, lpSubKey, lpValue, dwFlags, pdwType, pvData, pcbData);
		LONG result = ERROR_SUCCESS;

		const auto it = hkey && lpSubKey && lpValue && (dwFlags & RRF_RT_REG_DWORD)
			? g_dwordValues.find(RegistryKey(hkey, lpSubKey, lpValue))
			: g_dwordValues.end();

		if (it != g_dwordValues.end())
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

	LONG WINAPI regQueryValueExA(HKEY hKey, LPCSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType,
		LPBYTE lpData, LPDWORD lpcbData)
	{
		Compat::LogEnter("regQueryValueExA", hKey, lpValueName, lpReserved, lpType,
			static_cast<void*>(lpData), lpcbData);

		if (hKey && lpValueName)
		{
			const CStringW keyName = getKeyName(hKey);
			const CStringW localMachinePrefix = "\\REGISTRY\\MACHINE\\";
			if (localMachinePrefix == keyName.Mid(0, localMachinePrefix.GetLength()))
			{
				auto it = g_unsetValues.find(RegistryKey(HKEY_LOCAL_MACHINE,
					keyName.Mid(localMachinePrefix.GetLength()), lpValueName));
				if (it != g_unsetValues.end())
				{
					return ERROR_FILE_NOT_FOUND;
				}
			}
		}

		LONG result = CALL_ORIG_FUNC(RegQueryValueExA)(hKey, lpValueName, lpReserved, lpType,
			lpData, lpcbData);

		Compat::LogLeave("regQueryValueExA", hKey, lpValueName, lpReserved, lpType,
			static_cast<void*>(lpData), lpcbData) << result;
		return result;
	}
}

namespace Win32
{
	namespace Registry
	{
		void installHooks()
		{
			HOOK_FUNCTION(KernelBase, RegGetValueW, regGetValueW);
			HOOK_SHIM_FUNCTION(RegQueryValueExA, regQueryValueExA);
		}

		void setValue(HKEY key, const char* subKey, const char* valueName, DWORD value)
		{
			assert(key && subKey && valueName);
			g_dwordValues[RegistryKey(key, subKey, valueName)] = value;
		}

		void unsetValue(HKEY key, const char* subKey, const char* valueName)
		{
			assert(key && subKey && valueName);
			g_unsetValues.insert(RegistryKey(key, subKey, valueName));
		}
	}
}
