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
* Some functions taken from source code found in Aqrit's ddwrapper
* http://bitpatch.com/ddwrapper.html
*
* Some functions taken from source code found in DirectSoundControl
* https://github.com/nRaecheR/DirectSoundControl
*/

#include "cfg.h"
#include "wrappers\wrapper.h"
#include <VersionHelpers.h>

// Get Windows Operating System version number from the registry
void GetVersionReg(OSVERSIONINFO *oOS_version)
{
	// Initualize variables
	oOS_version->dwMajorVersion = 0;
	oOS_version->dwMinorVersion = 0;
	oOS_version->dwBuildNumber = 0;

	// Define registry keys
	HKEY			RegKey = NULL;
	DWORD			dwDataMajor = 0;
	DWORD			dwDataMinor = 0;
	unsigned long	iSize = sizeof(DWORD);
	DWORD			dwType = 0;

	// Get version
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", 0, KEY_ALL_ACCESS, &RegKey) == ERROR_SUCCESS)
	{
		if (RegQueryValueEx(RegKey, "CurrentMajorVersionNumber", NULL, &dwType, (LPBYTE)&dwDataMajor, &iSize) == ERROR_SUCCESS &&
			RegQueryValueEx(RegKey, "CurrentMinorVersionNumber", NULL, &dwType, (LPBYTE)&dwDataMinor, &iSize) == ERROR_SUCCESS)
		{
			oOS_version->dwMajorVersion = dwDataMajor;
			oOS_version->dwMinorVersion = dwDataMinor;
		}
		RegCloseKey(RegKey);
	}
}

// Get Windows Operating System version number from kernel32.dll
void GetVersionFile(OSVERSIONINFO *oOS_version)
{
	// Initualize variables
	oOS_version->dwMajorVersion = 0;
	oOS_version->dwMinorVersion = 0;
	oOS_version->dwBuildNumber = 0;

	// Load version.dll
	HMODULE Module = LoadLibrary("version.dll");
	if (!Module)
	{
		Compat::Log() << "Failed to load version.dll!";
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
		if (!GetFileVersionInfoSize) Compat::Log() << "Failed to get 'GetFileVersionInfoSize' ProcAddress of version.dll!";
		if (!GetFileVersionInfo) Compat::Log() << "Failed to get 'GetFileVersionInfo' ProcAddress of version.dll!";
		if (!VerQueryValue) Compat::Log() << "Failed to get 'VerQueryValue' ProcAddress of version.dll!";
		return;
	}

	// Get kernel32.dll path
	char buffer[MAX_PATH];
	GetSystemDirectory(buffer, MAX_PATH);
	strcat_s(buffer, MAX_PATH, "\\kernel32.dll");

	// Define registry keys
	DWORD  verHandle = 0;
	UINT   size = 0;
	LPBYTE lpBuffer = NULL;
	LPCSTR szVersionFile = buffer;
	DWORD  verSize = GetFileVersionInfoSize(szVersionFile, &verHandle);

	// GetVersion from a file
	if (verSize != NULL)
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
void GetOSVersion()
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
			case 0: sOSName = "Windows 2000 Server"; break;
			case 2:
				if (GetSystemMetrics(SM_SERVERR2) == 0)
				{
					sOSName = "Windows Server 2003";
				}
				else
				{
					sOSName = "Windows Server 2003 R2";
				}
				break;
			}
			break;
		case 6:
			switch (oOS_version.dwMinorVersion)
			{
			case 0: sOSName = "Windows Server 2008"; break;
			case 1: sOSName = "Windows Server 2008 R2"; break;
			case 2: sOSName = "Windows Server 2012"; break;
			case 3: sOSName = "Windows Server 2012 R2"; break;
			}
			break;
		case 10:
			switch (oOS_version.dwMinorVersion)
			{
			case 0: sOSName = "Windows Server 2016"; break;
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
			case 0: sOSName = "Windows 2000"; break;
			case 1: sOSName = "Windows XP"; break;
			case 2: sOSName = "Windows XP Professional"; break; // Windows XP Professional x64
			}
			break;
		case 6:
			switch (oOS_version.dwMinorVersion)
			{
			case 0: sOSName = "Windows Vista"; break;
			case 1: sOSName = "Windows 7"; break;
			case 2: sOSName = "Windows 8"; break;
			case 3: sOSName = "Windows 8.1"; break;
			}
			break;
		case 10:
			switch (oOS_version.dwMinorVersion)
			{
			case 0: sOSName = "Windows 10"; break;
			}
			break;
		}
	}

	// Get bitness (32bit vs 64bit)
	char *bitness = "";
	SYSTEM_INFO SystemInfo;
	GetNativeSystemInfo(&SystemInfo);
	if (SystemInfo.wProcessorArchitecture == 9) bitness = " 64-bit";

	// Log operating system version and type
	Compat::Log() << sOSName << bitness << " (" << oOS_version.dwMajorVersion << "." << oOS_version.dwMinorVersion << "." << oOS_version.dwBuildNumber << ")";
}

// Logs the process name and PID
void GetProcessNameAndPID()
{
	// Get process name
	char exepath[MAX_PATH];
	GetModuleFileName(NULL, exepath, MAX_PATH);

	// Remove path and add process name
	char *pdest = strrchr(exepath, '\\');

	// Log process name and ID
	Compat::Log() << (++pdest) << " (PID:" << GetCurrentProcessId() << ")";
}

// Execute a specified string
void Shell(char* fileName)
{
	Compat::Log() << "Running process: " << fileName;

	// Get StartupInfo and ProcessInfo memory size and set process window to hidden
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	si.wShowWindow = SW_HIDE;
	ZeroMemory(&pi, sizeof(pi));

	// Start the child process
	if (!CreateProcess(NULL, fileName, NULL, NULL, true, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
	{
		// Failed to launch process!
		Compat::Log() << "Failed to launch process!";
	}
	else
	{
		// Wait for process to exit
		if (Config.WaitForProcess) WaitForSingleObjectEx(pi.hThread, INFINITE, true);

		// Close thread handle
		CloseHandle(pi.hThread);

		// Close process handle
		CloseHandle(pi.hProcess);
	}
	// Quit function
	return;
}

// DPI virtualization causes:
// Text Clipping, Blurring, or Inconsistent font sizes.
// "Rendering of full-screen DX applications partially off screen" - Mircosoft
// ...drawing(writting) to someplace that doesn't exist may cause crashes... 
// if your going to Disable DPI Scaling then do it as soon as possible
void DisableHighDPIScaling()
{
	Compat::Log() << "Disabling High DPI Scaling...";
	// use GetProcAddress because SetProcessDPIAware exists only on win6+
	// and "High" dpi scaling only exits on win6+?
	HMODULE hUser32 = GetModuleHandle("user32.dll");
	typedef bool(__stdcall* SetProcessDPIAwareFunc)();
	if (hUser32)
	{
		SetProcessDPIAwareFunc setDPIAware = (SetProcessDPIAwareFunc)GetProcAddress(hUser32, "SetProcessDPIAware");
		if (setDPIAware) setDPIAware();
	}
}

void SetAppCompat()
{
	// Check if any DXPrimaryEmulation flags is set
	bool appCompatFlag = false;
	for (int x = 1; x <= 12; x++) if (Config.DXPrimaryEmulation[x]) appCompatFlag = true;

	// SetAppCompatData see: http://www.blitzbasic.com/Community/post.php?topic=99477&post=1202996
	if (appCompatFlag)
	{
		typedef HRESULT(__stdcall *SetAppCompatDataFunc)(DWORD, DWORD);
		HMODULE module = LoadDll(dtype.ddraw);
		if (module)
		{
			FARPROC SetAppCompatDataPtr = GetProcAddress(module, "SetAppCompatData");
			SetAppCompatDataFunc SetAppCompatData = (SetAppCompatDataFunc)SetAppCompatDataPtr;
			for (int x = 1; x <= 12; x++)
			{
				if (Config.DXPrimaryEmulation[x])
				{
					if (SetAppCompatData)
					{
						Compat::Log() << "SetAppCompatData: " << x;
						// For LockColorkey, this one uses the second parameter
						if (x == AppCompatDataType.LockColorkey)
						{
							(SetAppCompatData)(x, Config.LockColorkey);
						}
						// For all the other items
						else
						{
							(SetAppCompatData)(x, 0);
						}
					}
				}
			}
		}
		else
		{
			Compat::Log() << "Cannnot open ddraw.dll to SetAppCompatData";
		}
	}
}