/**
* Copyright (C) 2018 Elisha Riedlinger
*
* This software is  provided 'as-is', without any express  or implied  warranty. In no event will the
* authors be held liable for any damages arising from the use of this software.
* Permission  is granted  to anyone  to use  this software  for  any  purpose,  including  commercial
* applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*   1. The origin of this software must not be misrepresented; you must not claim that you  wrote the
*      original  software. If you use this  software  in a product, an  acknowledgment in the product
*      documentation would be appreciated but is not required.
*   2. Altered source versions must  be plainly  marked as such, and  must not be  misrepresented  as
*      being the original software.
*   3. This notice may not be removed or altered from any source distribution.
*
* Code in GetOSVersion and GetVersionReg functions taken from source code found in DirectSoundControl
* https://github.com/nRaecheR/DirectSoundControl
*
* Code in GetVersionFile function taken from source code found on stackoverflow.com
* https://stackoverflow.com/questions/940707/how-do-i-programmatically-get-the-version-of-a-dll-or-exe-file
*
* Code in LogComputerManufacturer function taken from source code found on rohitab.com
* http://www.rohitab.com/discuss/topic/35915-win32-api-to-get-system-information/
*/

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <VersionHelpers.h>
#include "Logging.h"

namespace Logging
{
	void GetOsVersion(RTL_OSVERSIONINFOEXW *);
	void GetVersionReg(OSVERSIONINFO *);
	void GetVersionFile(OSVERSIONINFO *);
}

void Logging::LogFormat(char * fmt, ...)
{
	// Format arg list
	va_list ap;
	va_start(ap, fmt);
	auto size = vsnprintf(nullptr, 0, fmt, ap);
	std::string output(size + 1, '\0');
	vsprintf_s(&output[0], size + 1, fmt, ap);
	va_end(ap);

	// Log formated text
	Log() << output.c_str();
	return;
}

// Logs the process name and PID
void Logging::LogProcessNameAndPID()
{
	// Get process name
	char exepath[MAX_PATH] = { 0 };
	GetModuleFileName(nullptr, exepath, MAX_PATH);

	// Remove path and add process name
	char *pdest = strrchr(exepath, '\\');

	// Log process name and ID
	Log() << (++pdest) << " (PID:" << GetCurrentProcessId() << ")";
}

// Get Windows Operating System version number from RtlGetVersion
void Logging::GetOsVersion(RTL_OSVERSIONINFOEXW* pk_OsVer)
{
	// Initualize variables
	pk_OsVer->dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOEXW);
	ZeroMemory(&(pk_OsVer->szCSDVersion), 128 * sizeof(WCHAR));
	pk_OsVer->dwMajorVersion = 0;
	pk_OsVer->dwMinorVersion = 0;
	pk_OsVer->dwBuildNumber = 0;

	// Load ntdll.dll
	HMODULE Module = LoadLibrary("ntdll.dll");
	if (!Module)
	{
		Log() << "Failed to load ntdll.dll!";
		return;
	}

	// Call RtlGetVersion API
	typedef LONG(WINAPI* tRtlGetVersion)(RTL_OSVERSIONINFOEXW*);
	tRtlGetVersion f_RtlGetVersion = (tRtlGetVersion)GetProcAddress(Module, "RtlGetVersion");

	// Get version data
	if (f_RtlGetVersion)
	{
		f_RtlGetVersion(pk_OsVer);
	}
}

// Get Windows Operating System version number from the registry
void Logging::GetVersionReg(OSVERSIONINFO *oOS_version)
{
	// Initualize variables
	oOS_version->dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	oOS_version->dwMajorVersion = 0;
	oOS_version->dwMinorVersion = 0;
	oOS_version->dwBuildNumber = 0;

	// Define registry keys
	HKEY			RegKey = nullptr;
	DWORD			dwDataMajor = 0;
	DWORD			dwDataMinor = 0;
	unsigned long	iSize = sizeof(DWORD);
	DWORD			dwType = 0;

	// Get version
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", 0, KEY_ALL_ACCESS, &RegKey) == ERROR_SUCCESS)
	{
		if (RegQueryValueEx(RegKey, "CurrentMajorVersionNumber", nullptr, &dwType, (LPBYTE)&dwDataMajor, &iSize) == ERROR_SUCCESS &&
			RegQueryValueEx(RegKey, "CurrentMinorVersionNumber", nullptr, &dwType, (LPBYTE)&dwDataMinor, &iSize) == ERROR_SUCCESS)
		{
			oOS_version->dwMajorVersion = dwDataMajor;
			oOS_version->dwMinorVersion = dwDataMinor;
		}
		RegCloseKey(RegKey);
	}
}

// Get Windows Operating System version number from kernel32.dll
void Logging::GetVersionFile(OSVERSIONINFO *oOS_version)
{
	// Initualize variables
	oOS_version->dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	oOS_version->dwMajorVersion = 0;
	oOS_version->dwMinorVersion = 0;
	oOS_version->dwBuildNumber = 0;

	// Load version.dll
	HMODULE Module = LoadLibrary("version.dll");
	if (!Module)
	{
		Log() << "Failed to load version.dll!";
		return;
	}

	// Declare functions
	typedef DWORD(WINAPI *PFN_GetFileVersionInfoSize)(LPCTSTR lptstrFilename, LPDWORD lpdwHandle);
	typedef BOOL(WINAPI *PFN_GetFileVersionInfo)(LPCTSTR lptstrFilename, DWORD dwHandle, DWORD dwLen, LPVOID lpData);
	typedef BOOL(WINAPI *PFN_VerQueryValue)(LPCVOID pBlock, LPCTSTR lpSubBlock, LPVOID *lplpBuffer, PUINT puLen);

	// Get functions ProcAddress
	PFN_GetFileVersionInfoSize GetFileVersionInfoSize = reinterpret_cast<PFN_GetFileVersionInfoSize>(GetProcAddress(Module, "GetFileVersionInfoSizeA"));
	PFN_GetFileVersionInfo GetFileVersionInfo = reinterpret_cast<PFN_GetFileVersionInfo>(GetProcAddress(Module, "GetFileVersionInfoA"));
	PFN_VerQueryValue VerQueryValue = reinterpret_cast<PFN_VerQueryValue>(GetProcAddress(Module, "VerQueryValueA"));
	if (!GetFileVersionInfoSize || !GetFileVersionInfo || !VerQueryValue)
	{
		if (!GetFileVersionInfoSize)
		{
			Log() << "Failed to get 'GetFileVersionInfoSize' ProcAddress of version.dll!";
		}
		if (!GetFileVersionInfo)
		{
			Log() << "Failed to get 'GetFileVersionInfo' ProcAddress of version.dll!";
		}
		if (!VerQueryValue)
		{
			Log() << "Failed to get 'VerQueryValue' ProcAddress of version.dll!";
		}
		return;
	}

	// Get kernel32.dll path
	char buffer[MAX_PATH] = { 0 };
	GetSystemDirectory(buffer, MAX_PATH);
	strcat_s(buffer, MAX_PATH, "\\kernel32.dll");

	// Define registry keys
	DWORD  verHandle = 0;
	UINT   size = 0;
	LPBYTE lpBuffer = nullptr;
	LPCSTR szVersionFile = buffer;
	DWORD  verSize = GetFileVersionInfoSize(szVersionFile, &verHandle);

	// GetVersion from a file
	if (verSize != 0)
	{
		std::string verData(verSize + 1, '\0');

		if (GetFileVersionInfo(szVersionFile, verHandle, verSize, &verData[0]))
		{
			if (VerQueryValue(&verData[0], "\\", (VOID FAR* FAR*)&lpBuffer, &size))
			{
				if (size)
				{
					VS_FIXEDFILEINFO *verInfo = (VS_FIXEDFILEINFO *)lpBuffer;
					if (verInfo->dwSignature == 0xfeef04bd)
					{
						oOS_version->dwMajorVersion = (verInfo->dwFileVersionMS >> 16) & 0xffff;
						oOS_version->dwMinorVersion = (verInfo->dwFileVersionMS >> 0) & 0xffff;
						oOS_version->dwBuildNumber = (verInfo->dwFileVersionLS >> 16) & 0xffff;
						//(verInfo->dwFileVersionLS >> 0) & 0xffff		//  <-- Other data not used
					}
				}
			}
		}
	}
}

// Log Windows Operating System type
void Logging::LogOSVersion()
{
	// Declare vars
	RTL_OSVERSIONINFOEXW oOS_version;
	OSVERSIONINFO fOS_version, rOS_version;

	// GetVersion from RtlGetVersion which is needed for some cases (Need for Speed III)
	GetOsVersion(&oOS_version);
	std::wstring ws(L" " + std::wstring(oOS_version.szCSDVersion));
	std::string str(ws.begin(), ws.end());
	char *ServicePack = &str[0];

	// GetVersion from registry which is more relayable for Windows 10
	GetVersionReg(&rOS_version);

	// GetVersion from a file which is needed to get the build number
	GetVersionFile(&fOS_version);

	// Choose whichever version is higher
	// Newer OS's report older version numbers for compatibility
	// This allows dxwrapper to get the proper OS version number
	if (rOS_version.dwMajorVersion > oOS_version.dwMajorVersion)
	{
		oOS_version.dwMajorVersion = rOS_version.dwMajorVersion;
		oOS_version.dwMinorVersion = rOS_version.dwMinorVersion;
		ServicePack = "";
	}
	if (fOS_version.dwMajorVersion > oOS_version.dwMajorVersion)
	{
		oOS_version.dwMajorVersion = fOS_version.dwMajorVersion;
		oOS_version.dwMinorVersion = fOS_version.dwMinorVersion;
		ServicePack = "";
	}
	// The file almost always has the right build number
	if (fOS_version.dwBuildNumber != 0)
	{
		oOS_version.dwBuildNumber = fOS_version.dwBuildNumber;
	}

	// Get OS string name
	char *sOSName = "Unknown platform";
	if (IsWindowsServer())
	{
		sOSName = "Unknown Windows Server";
		switch (oOS_version.dwMajorVersion)
		{
		case 5:
			switch (oOS_version.dwMinorVersion)
			{
			case 0:
				sOSName = "Windows 2000 Server";
				break;
			case 2:
				sOSName = (GetSystemMetrics(SM_SERVERR2) == 0) ? "Windows Server 2003" : "Windows Server 2003 R2";
				break;
			}
			break;
		case 6:
			switch (oOS_version.dwMinorVersion)
			{
			case 0:
				sOSName = "Windows Server 2008";
				break;
			case 1:
				sOSName = "Windows Server 2008 R2";
				break;
			case 2:
				sOSName = "Windows Server 2012";
				break;
			case 3:
				sOSName = "Windows Server 2012 R2";
				break;
			}
			break;
		case 10:
			switch (oOS_version.dwMinorVersion)
			{
			case 0:
				sOSName = "Windows Server 2016";
				break;
			}
			break;
		}
	}
	else
	{
		sOSName = "Unknown Windows Desktop";
		switch (oOS_version.dwMajorVersion)
		{
		case 5:
			switch (oOS_version.dwMinorVersion)
			{
			case 0:
				sOSName = "Windows 2000";
				break;
			case 1:
				sOSName = "Windows XP";
				break;
			case 2:
				sOSName = "Windows XP Professional"; // Windows XP Professional x64
				break;
			}
			break;
		case 6:
			switch (oOS_version.dwMinorVersion)
			{
			case 0:
				sOSName = "Windows Vista";
				break;
			case 1:
				sOSName = "Windows 7";
				break;
			case 2:
				sOSName = "Windows 8";
				break;
			case 3:
				sOSName = "Windows 8.1";
				break;
			}
			break;
		case 10:
			switch (oOS_version.dwMinorVersion)
			{
			case 0:
				sOSName = "Windows 10";
				break;
			}
			break;
		}
	}

	// Get bitness (32bit vs 64bit)
	SYSTEM_INFO SystemInfo;
	GetNativeSystemInfo(&SystemInfo);

	// Log operating system version and type
	Log() << sOSName << ((SystemInfo.wProcessorArchitecture == 9) ? " 64-bit" : "") << " (" << oOS_version.dwMajorVersion << "." << oOS_version.dwMinorVersion << "." << oOS_version.dwBuildNumber << ")" << ServicePack;
}

void Logging::LogComputerManufacturer()
{
	std::string Buffer1, Buffer2;
	HKEY   hkData;
	LPSTR  lpString1 = nullptr, lpString2 = nullptr, lpString3 = nullptr;
	LPBYTE lpData = nullptr;
	DWORD  dwType = 0, dwSize = 0;
	UINT   uIndex, uStart, uEnd, uString, uState = 0;
	LONG   lErr;

	if ((lErr = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Services\\mssmbios\\Data", 0, KEY_QUERY_VALUE, &hkData)) != ERROR_SUCCESS)
	{
		SetLastError(lErr);
		return;
	}

	if ((lErr = RegQueryValueEx(hkData, "SMBiosData", nullptr, &dwType, nullptr, &dwSize)) == ERROR_SUCCESS)
	{
		if (dwSize < 8 || dwType != REG_BINARY)
		{
			lErr = ERROR_BADKEY;
		}
		else
		{
			lpData = new BYTE[dwSize];
			if (!lpData)
			{
				lErr = ERROR_NOT_ENOUGH_MEMORY;
			}
			else
			{
				lErr = RegQueryValueEx(hkData, "SMBiosData", nullptr, nullptr, lpData, &dwSize);
			}
		}
	}

	RegCloseKey(hkData);

	if (lErr == ERROR_SUCCESS)
	{
		uIndex = 8 + *(WORD *)(lpData + 6);
		uEnd = 8 + *(WORD *)(lpData + 4);

		while (uIndex < uEnd && lpData[uIndex] != 0x7F)
		{
			UINT tmp = ((uStart = uIndex) + 1);
			if (tmp < uEnd)
			{
				uIndex += lpData[tmp];
			}
			else
			{
				while (++uIndex && uIndex < uEnd && (lpData[uIndex - 1] || lpData[uIndex]));
			}
			uString = 1;
			if (uIndex < uEnd && uStart + 6 < uEnd)
			{
				do
				{
					if (lpData[uStart] == 0x01 && uState == 0)
					{
						if (lpData[uStart + 4] == uString ||
							lpData[uStart + 5] == uString ||
							lpData[uStart + 6] == uString)
						{
							lpString1 = (LPSTR)(lpData + uIndex);
							if (!_strcmpi(lpString1, "System manufacturer"))
							{
								lpString1 = nullptr;
								uState++;
							}
						}
					}
					else if (lpData[uStart] == 0x02 && uState == 1)
					{
						if (lpData[uStart + 4] == uString ||
							lpData[uStart + 5] == uString ||
							lpData[uStart + 6] == uString)
						{
							lpString1 = (LPSTR)(lpData + uIndex);
						}
					}
					else if (lpData[uStart] == 0x02 && uState == 0)
					{
						if (lpData[uStart + 4] == uString ||
							lpData[uStart + 5] == uString ||
							lpData[uStart + 6] == uString)
						{
							lpString2 = (LPSTR)(lpData + uIndex);
						}
					}
					else if (lpData[uStart] == 0x03 && uString == 1)
					{
						switch (lpData[uStart + 5])
						{
						default:   lpString3 = "(Other)";               break;
						case 0x02: lpString3 = "(Unknown)";             break;
						case 0x03: lpString3 = "(Desktop)";             break;
						case 0x04: lpString3 = "(Low Profile Desktop)"; break;
						case 0x06: lpString3 = "(Mini Tower)";          break;
						case 0x07: lpString3 = "(Tower)";               break;
						case 0x08: lpString3 = "(Portable)";            break;
						case 0x09: lpString3 = "(Laptop)";              break;
						case 0x0A: lpString3 = "(Notebook)";            break;
						case 0x0E: lpString3 = "(Sub Notebook)";        break;
						}
					}
					if (lpString1 != nullptr)
					{
						if (Buffer1.size() != 0)
						{
							Buffer1.append(" ");
						}
						Buffer1.append(lpString1);
						lpString1 = nullptr;
					}
					else if (lpString2 != nullptr)
					{
						if (Buffer2.size() != 0)
						{
							Buffer2.append(" ");
						}
						Buffer2.append(lpString2);
						lpString2 = nullptr;
					}
					else if (lpString3 != nullptr)
					{
						if (Buffer1.size() != 0)
						{
							Buffer1.append(" ");
						}
						Buffer1.append(lpString3);
						if (Buffer2.size() != 0)
						{
							Buffer2.append(" ");
						}
						Buffer2.append(lpString3);
						break;
					}
					uString++;
					while (++uIndex && uIndex < uEnd && lpData[uIndex]);
					uIndex++;
				} while (uIndex < uEnd && lpData[uIndex]);
				if (lpString3 != nullptr)
				{
					break;
				}
			}
			uIndex++;
		}
	}

	if (lpData)
	{
		delete[] lpData;
	}

	if (Buffer1.size() != 0)
	{
		Log() << Buffer1.c_str();
	}
	if (Buffer2.size() != 0)
	{
		Log() << Buffer2.c_str();
	}
}

void Logging::LogVideoCard()
{
	DISPLAY_DEVICE DispDev;
	ZeroMemory(&DispDev, sizeof(DispDev));
	DispDev.cb = sizeof(DispDev);
	DWORD nDeviceIndex = 0;
	EnumDisplayDevices(NULL, nDeviceIndex, &DispDev, 0);
	Log() << DispDev.DeviceString;
}
