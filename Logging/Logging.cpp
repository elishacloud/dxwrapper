/**
* Copyright (C) 2017 Elisha Riedlinger
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
*/

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <VersionHelpers.h>
#include "Logging.h"

namespace Logging
{
	void GetVersionReg(OSVERSIONINFO *);
	void GetVersionFile(OSVERSIONINFO *);
}

#define CHECK_TYPE(myChar, myType) \
		case myChar: \
		{ \
			sprintf_s(mybuffer, buffer, va_arg(ap, myType)); \
			break; \
		}

#define VISIT_TYPE(visit) \
	visit('c', char) \
	visit('C', char) \
	visit('s', void*) \
	visit('S', void*) \
	visit('d', int) \
	visit('i', int) \
	visit('o', UINT) \
	visit('x', UINT) \
	visit('X', UINT) \
	visit('u', UINT) \
	visit('f', float) \
	visit('F', float) \
	visit('e', float) \
	visit('E', float) \
	visit('a', float) \
	visit('A', float) \
	visit('g', float) \
	visit('G', float) \
	visit('n', void*) \
	visit('p', void*)

void Logging::LogFormat(char * fmt, ...)
{
	// Get arg list
	va_list ap;
	va_start(ap, fmt);

	// Declare vars
	static constexpr DWORD BuffSize = 1024;
	static char buffer[BuffSize];
	static char mybuffer[BuffSize];

	// Setup string
	strcpy_s(buffer, fmt);
	char *loc = strchr(buffer, '%');

	// Loop through string
	while (loc)
	{
		// Get format type
		char myChar = buffer[loc - buffer + 1];

		// Update format string
		switch (myChar)
		{
			VISIT_TYPE(CHECK_TYPE);
		default:
			Log() << "Error in LogFormat type '" << myChar << "'";
			va_end(ap);
			return;
		}

		// Copy format string back
		strcpy_s(buffer, mybuffer);

		// Check for next format
		char *myloc = strchr(buffer, '%');
		if (myloc == loc || !myloc)
		{
			loc = nullptr;
		}
		else
		{
			loc = myloc;
		}
	}

	// End arg list
	va_end(ap);

	// Log results
	Log() << buffer;
}

// Logs the process name and PID
void Logging::LogProcessNameAndPID()
{
	// Get process name
	char exepath[MAX_PATH];
	GetModuleFileName(nullptr, exepath, MAX_PATH);

	// Remove path and add process name
	char *pdest = strrchr(exepath, '\\');

	// Log process name and ID
	Log() << (++pdest) << " (PID:" << GetCurrentProcessId() << ")";
}

// Get Windows Operating System version number from the registry
void Logging::GetVersionReg(OSVERSIONINFO *oOS_version)
{
	// Initualize variables
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
	char buffer[MAX_PATH];
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
		LPSTR verData = new char[verSize];

		if (GetFileVersionInfo(szVersionFile, verHandle, verSize, verData))
		{
			if (VerQueryValue(verData, "\\", (VOID FAR* FAR*)&lpBuffer, &size))
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
		delete[] verData;
	}
	FreeLibrary(Module);
}

// Log Windows Operating System type
void Logging::LogOSVersion()
{
	// Declare vars
	OSVERSIONINFO oOS_version, rOS_version;
	oOS_version.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	rOS_version.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	// GetVersion from registry which is more relayable for Windows 10
	GetVersionReg(&rOS_version);

	// GetVersion from a file which is needed to get the build number
	GetVersionFile(&oOS_version);

	// Choose whichever version is higher
	// Newer OS's report older version numbers for compatibility
	// This allows dxwrapper to get the proper OS version number
	if (rOS_version.dwMajorVersion > oOS_version.dwMajorVersion)
	{
		oOS_version.dwMajorVersion = rOS_version.dwMajorVersion;
		oOS_version.dwMinorVersion = rOS_version.dwMinorVersion;
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
	char *bitness = "";
	SYSTEM_INFO SystemInfo;
	GetNativeSystemInfo(&SystemInfo);
	if (SystemInfo.wProcessorArchitecture == 9)
	{
		bitness = " 64-bit";
	}

	// Log operating system version and type
	Log() << sOSName << bitness << " (" << oOS_version.dwMajorVersion << "." << oOS_version.dwMinorVersion << "." << oOS_version.dwBuildNumber << ")";
}
