#define WIN32_LEAN_AND_MEAN
#define CINTERFACE

#include <cassert>
#include <cstring>
#include <map>
#include <set>
#include <sstream>

#include <DDrawCompat/v0.3.1/Common/Hook.h>
#include <DDrawCompat/DDrawLog.h>
#include <DDrawCompat/v0.3.1/Win32/Registry.h>

typedef long NTSTATUS;

namespace
{
	struct RegistryKey
	{
		HKEY key;
		std::wstring subKey;
		std::wstring value;

		RegistryKey(HKEY key, const std::wstring& subKey, const std::wstring& value)
			: key(key), subKey(subKey), value(value)
		{
		}

		bool operator<(const RegistryKey& rhs) const
		{
			if (key < rhs.key) { return true; }
			if (key > rhs.key) { return false; }
			const int subKeyComp = lstrcmpiW(subKey.c_str(), rhs.subKey.c_str());
			if (subKeyComp < 0) { return true; }
			if (subKeyComp > 0) { return false; }
			return lstrcmpiW(value.c_str(), rhs.value.c_str()) < 0;
		}

		bool operator==(const RegistryKey& rhs) const
		{
			return key == rhs.key &&
				0 == lstrcmpiW(subKey.c_str(), rhs.subKey.c_str()) &&
				0 == lstrcmpiW(value.c_str(), rhs.value.c_str());
		}
	};

	std::map<RegistryKey, DWORD> g_dwordValues;
	std::set<RegistryKey> g_unsetValues;

	std::wstring getKeyName(HKEY key)
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
				return std::wstring(keyName.Name, keyName.NameLength / 2);
			}
		}

		return std::wstring();
	}

	LONG WINAPI regGetValueW(HKEY hkey, LPCWSTR lpSubKey, LPCWSTR lpValue, 
		DWORD dwFlags, LPDWORD pdwType, PVOID pvData, LPDWORD pcbData)
	{
		LOG_FUNC("regGetValueW", hkey, lpSubKey, lpValue, dwFlags, pdwType, pvData, pcbData);
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

		return LOG_RESULT(result);
	}

	LONG WINAPI regQueryValueExA(HKEY hKey, LPCSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType,
		LPBYTE lpData, LPDWORD lpcbData)
	{
		LOG_FUNC("regQueryValueExA", hKey, lpValueName, lpReserved, lpType, static_cast<void*>(lpData), lpcbData);

		if (hKey && lpValueName)
		{
			const std::wstring keyName = getKeyName(hKey);
			const std::wstring localMachinePrefix = L"\\REGISTRY\\MACHINE\\";
			if (localMachinePrefix == keyName.substr(0, localMachinePrefix.size()))
			{
				std::wostringstream oss;
				oss << lpValueName;
				auto it = g_unsetValues.find(RegistryKey(HKEY_LOCAL_MACHINE,
					keyName.substr(localMachinePrefix.size()), oss.str()));
				if (it != g_unsetValues.end())
				{
					return LOG_RESULT(ERROR_FILE_NOT_FOUND);
				}
			}
		}

		return LOG_RESULT(CALL_ORIG_FUNC(RegQueryValueExA)(hKey, lpValueName, lpReserved, lpType, lpData, lpcbData));
	}
}

namespace Win32
{
	namespace Registry
	{
		void installHooks()
		{
			HOOK_SHIM_FUNCTION(RegGetValueW, regGetValueW);
			HOOK_SHIM_FUNCTION(RegQueryValueExA, regQueryValueExA);
		}

		void setValue(HKEY key, const char* subKey, const char* valueName, DWORD value)
		{
			assert(key && subKey && valueName);
			std::wostringstream subKeyW;
			subKeyW << subKey;
			std::wostringstream valueNameW;
			valueNameW << valueName;
			g_dwordValues[RegistryKey(key, subKeyW.str(), valueNameW.str())] = value;
		}

		void unsetValue(HKEY key, const char* subKey, const char* valueName)
		{
			assert(key && subKey && valueName);
			std::wostringstream subKeyW;
			subKeyW << subKey;
			std::wostringstream valueNameW;
			valueNameW << valueName;
			g_unsetValues.insert(RegistryKey(key, subKeyW.str(), valueNameW.str()));
		}
	}
}
