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
*/

#include "cfg.h"
#include "dllmain.h"

CONFIG Config;
DLLTYPE dtype;
APPCOMPATDATATYPE AppCompatDataType;

uint8_t ExcludeCount;
uint8_t IncludeCount;
char* szExclude[256];
char* szInclude[256];

// Checks if a string value exists in a string array
inline bool IfStringExistsInList(char* szValue, char* szList[256], uint8_t ListCount, bool CaseSensitive)
{
	for (UINT x = 1; x <= ListCount; ++x)
	{
		// Case sensitive check
		if (CaseSensitive)
		{
			if (strcmp(szValue, szList[x]) == 0)
				return true;
		}
		// Case insensitive check
		else
		{
			if (_strcmpi(szValue, szList[x]) == 0)
				return true;
		}
	}
	return false;
}

// Deletes all string values from an array
void DeleteArrayMemory(char* szList[256], uint8_t ListCount)
{
	for (UINT x = 1; x <= ListCount; ++x)
	{
		delete szList[x];
	}
}

// Deletes all BytesToWrite values from the BytesToWrite array
void DeleteByteToWriteArrayMemory()
{
	for (UINT x = 1; x <= Config.BytesToWriteCount; ++x)
	{
		if (Config.MemoryInfo[x].SizeOfBytes > 0) delete Config.MemoryInfo[x].Bytes;
	}
}

// any C++ style (both inline and block) commented text is replaced with a space character 
// todo: skip comments inside double quotes
#pragma warning (disable : 4706)
void EraseCppComments(char* str)
{
	while (str = strchr(str, '/'))
	{
		if (str[1] == '/') for (; ((*str != '\0') && (*str != '\n')); str++) *str = '\x20';
		else if (str[1] == '*')
		{
			for (; ((*str != '\0') && ((str[0] != '*') || (str[1] != '/'))); str++) *str = '\x20';
			if (*str) { *str++ = '\x20'; *str = '\x20'; }
		}
		if (*str) str++;
		else break;
	}
}
#pragma warning (default : 4706)

// [sections] are ignored
// escape characters NOT support 
// double quotes NOT suppoted
// Name/value delimiter is an equal sign or colon 
// whitespace is removed from before and after both the name and value
// characters considered to be whitespace:
//  0x20 - space
//	0x09 - horizontal tab
//	0x0D - carriage return
#pragma warning (disable : 4996)
typedef void(__stdcall* NV)(char* name, char* value);
void Parse(char* str, NV NameValueCallback)
{
	EraseCppComments(str);
	for (str = strtok(str, "\n"); str; str = strtok(0, "\n"))
	{
		if (*str == ';' || *str == '#') continue; // skip INI style comments ( must be at start of line )
		char* rvalue = strchr(str, '=');
		if (!rvalue) rvalue = strchr(str, ':');
		if (rvalue)
		{
			*rvalue++ = '\0'; // split left and right values

			rvalue = &rvalue[strspn(rvalue, "\x20\t\r")]; // skip beginning whitespace
			for (char* end = strchr(rvalue, '\0'); (--end >= rvalue) && (*end == '\x20' || *end == '\t' || *end == '\r');) *end = '\0';  // truncate ending whitespace

			char* lvalue = &str[strspn(str, "\x20\t\r")]; // skip beginning whitespace
			for (char* end = strchr(lvalue, '\0'); (--end >= lvalue) && (*end == '\x20' || *end == '\t' || *end == '\r');) *end = '\0';  // truncate ending whitespace

			if (*lvalue && *rvalue) NameValueCallback(lvalue, rvalue);
		}
	}
}
#pragma warning (default : 4996)

// Reads szFileName from disk
char* Read(char* szFileName)
{
	HANDLE hFile;
	DWORD dwBytesToRead;
	DWORD dwBytesRead;

	char* szCfg = NULL;
	hFile = CreateFile(szFileName, GENERIC_READ, FILE_SHARE_READ, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		dwBytesToRead = GetFileSize(hFile, NULL);
		if ((dwBytesToRead != 0) && (dwBytesToRead != 0xFFFFFFFF))
		{
			szCfg = (char*)malloc(dwBytesToRead + 1); // +1 so a NULL terminator can be added
			if (szCfg != NULL)
			{
				if (ReadFile(hFile, szCfg, dwBytesToRead, &dwBytesRead, NULL))
				{
					if (dwBytesRead != 0) szCfg[dwBytesRead] = '\0'; // make txt file easy to deal with 
				}
				else {
					free(szCfg);
					szCfg = NULL;
				}
			}
		}
		CloseHandle(hFile);
	}
	return szCfg;
}

// Set config from string (file)
void SetConfig(char* name, char* value)
{
	if (!_strcmpi(value, "AUTO") != 0)
	{
		name[0] = '\0';
	}
	else {
		if (strlen(value) <= MAX_PATH) strcpy_s(name, MAX_PATH, value);
	}
}

// Set config from string (file)
void SetConfigList(char* name[], uint8_t& count, char* value)
{
	if (strlen(value) <= MAX_PATH)
	{
		count++;
		name[count] = new char[strlen(value) + 1];
		strcpy_s(name[count], MAX_PATH, value);
	}
}

// Set AddressPointer array from string (file)
void SetAddressPointerList(MEMORYINFO& MemoryInfo, char* value)
{
	// Get address pointer
	if (strtoul(value, NULL, 16) > 0 &&							// Verify pointer has a value higher than 0
		value[0] == '0' && (char)tolower(value[1]) == 'x')		// Check for leading "0x" to indicate hex number
	{
		MemoryInfo.AddressPointer = strtoul(value, NULL, 16);
	}
	// Set to 0 if invalid address pointer
	else
	{
		MemoryInfo.AddressPointer = 0;
	}
}

// Set BytesToWrite to memory from string (file)
void SetBytesList(MEMORYINFO& MemoryInfo, char* value)
{
	// Declare vars
	char charTemp[] = {'0', 'x', '0' , '0'};
	DWORD len = strlen(value);

	// Check for valid bytes
	if ((len > 3) &&											// String has at least one byte
		(len % 2 == 0) &&										// String is even
		value[0] == '0' && (char)tolower(value[1]) == 'x')		// Check for leading "0x" to indicate hex number
	{
		// Get bytes size
		DWORD size = (len / 2) - 1;
		MemoryInfo.Bytes = new byte[size];
		MemoryInfo.SizeOfBytes = size;

		// Get byte data
		for (UINT x = 1; x <= size; x++)
		{
			charTemp[2] = value[x * 2];
			charTemp[3] = value[(x * 2) + 1];
			MemoryInfo.Bytes[x - 1] = (byte)strtoul(charTemp, NULL, 16);
		}
	}
	// Set to 0 if invalid Bytes are set
	else
	{
		MemoryInfo.SizeOfBytes = 0;
	}
}

// Set booloean value from string (file)
bool IsValueEnabled(char* name)
{
	return (atoi(name) > 0 ||
		_strcmpi("on", name) == 0 ||
		_strcmpi("yes", name) == 0 ||
		_strcmpi("true", name) == 0 ||
		_strcmpi("enabled", name) == 0);
}

// Set config from string (file)
void __stdcall ParseCallback(char* name, char* value)
{
	// Boolean values
	if (!_strcmpi(name, "SingleProcAffinity")) { Config.Affinity = IsValueEnabled(value); Config.AffinityNotSet = false; return; }
	if (!_strcmpi(name, "D3d8to9")) { Config.D3d8to9 = IsValueEnabled(value); return; }
	if (!_strcmpi(name, "DDrawCompat")) { Config.DDrawCompat = IsValueEnabled(value); return; }
	if (!_strcmpi(name, "DisableHighDpiScaling")) { Config.DpiAware = IsValueEnabled(value); return; }
	if (!_strcmpi(name, "DxWnd")) { Config.DxWnd = IsValueEnabled(value); return; }
	if (!_strcmpi(name, "FullScreen")) { Config.FullScreen = IsValueEnabled(value); return; }
	if (!_strcmpi(name, "ForceTermination")) { Config.ForceTermination = IsValueEnabled(value); return; }
	if (!_strcmpi(name, "ForceWindowResize")) { Config.ForceWindowResize = IsValueEnabled(value); return; }
	if (!_strcmpi(name, "HandleExceptions")) { Config.HandleExceptions = IsValueEnabled(value); return; }
	if (!_strcmpi(name, "DSoundCtrl")) { Config.DSoundCtrl = IsValueEnabled(value); return; }
	if (!_strcmpi(name, "ResetScreenRes")) { Config.ResetScreenRes = IsValueEnabled(value); return; }
	if (!_strcmpi(name, "SendAltEnter")) { Config.SendAltEnter = IsValueEnabled(value); return; }
	if (!_strcmpi(name, "WaitForProcess")) { Config.WaitForProcess = IsValueEnabled(value); return; }
	if (!_strcmpi(name, "WaitForWindowChanges")) { Config.WaitForWindowChanges = IsValueEnabled(value); return; }
	// DSoundCtrl
	if (!_strcmpi(name, "Num2DBuffers")) { Config.Num2DBuffers = atoi(value); return; }
	if (!_strcmpi(name, "Num3DBuffers")) { Config.Num3DBuffers = atoi(value); return; }
	if (!_strcmpi(name, "ForceCertification")) { Config.ForceCertification = IsValueEnabled(value); return; }
	if (!_strcmpi(name, "ForceExclusiveMode")) { Config.ForceExclusiveMode = IsValueEnabled(value); return; }
	if (!_strcmpi(name, "ForceSoftwareMixing")) { Config.ForceSoftwareMixing = IsValueEnabled(value); return; }
	if (!_strcmpi(name, "ForceHardwareMixing")) { Config.ForceHardwareMixing = IsValueEnabled(value); return; }
	if (!_strcmpi(name, "PreventSpeakerSetup")) { Config.PreventSpeakerSetup = IsValueEnabled(value); return; }
	if (!_strcmpi(name, "ForceHQ3DSoftMixing")) { Config.ForceHQ3DSoftMixing = IsValueEnabled(value); return; }
	if (!_strcmpi(name, "ForceNonStaticBuffers")) { Config.ForceNonStaticBuffers = IsValueEnabled(value); return; }
	if (!_strcmpi(name, "ForceVoiceManagement")) { Config.ForceVoiceManagement = IsValueEnabled(value); return; }
	if (!_strcmpi(name, "ForcePrimaryBufferFormat")) { Config.ForcePrimaryBufferFormat = IsValueEnabled(value); return; }
	if (!_strcmpi(name, "PrimaryBufferBits")) { Config.PrimaryBufferBits = atoi(value); return; }
	if (!_strcmpi(name, "PrimaryBufferSamples")) { Config.PrimaryBufferSamples = atoi(value); return; }
	if (!_strcmpi(name, "PrimaryBufferChannels")) { Config.PrimaryBufferChannels = atoi(value); return; }
	if (!_strcmpi(name, "ForceSpeakerConfig")) { Config.ForceSpeakerConfig = IsValueEnabled(value); return; }
	if (!_strcmpi(name, "SpeakerConfig")) { Config.SpeakerConfig = atoi(value); return; }
	if (!_strcmpi(name, "EnableStoppedDriverWorkaround")) { Config.EnableStoppedDriverWorkaround = IsValueEnabled(value); return; }
	// AppCompatData
	if (!_strcmpi(name, "LockEmulation")) { Config.DXPrimaryEmulation[AppCompatDataType.LockEmulation] = IsValueEnabled(value); return; }
	if (!_strcmpi(name, "BltEmulation")) { Config.DXPrimaryEmulation[AppCompatDataType.BltEmulation] = IsValueEnabled(value); return; }
	if (!_strcmpi(name, "ForceLockNoWindow")) { Config.DXPrimaryEmulation[AppCompatDataType.ForceLockNoWindow] = IsValueEnabled(value); return; }
	if (!_strcmpi(name, "ForceBltNoWindow")) { Config.DXPrimaryEmulation[AppCompatDataType.ForceBltNoWindow] = IsValueEnabled(value); return; }
	if (!_strcmpi(name, "LockColorkey")) { Config.DXPrimaryEmulation[AppCompatDataType.LockColorkey] = true; Config.LockColorkey = atoi(value); return; }
	if (!_strcmpi(name, "FullscreenWithDWM")) { Config.DXPrimaryEmulation[AppCompatDataType.FullscreenWithDWM] = IsValueEnabled(value); return; }
	if (!_strcmpi(name, "DisableLockEmulation")) { Config.DXPrimaryEmulation[AppCompatDataType.DisableLockEmulation] = IsValueEnabled(value); return; }
	if (!_strcmpi(name, "EnableOverlays")) { Config.DXPrimaryEmulation[AppCompatDataType.EnableOverlays] = IsValueEnabled(value); return; }
	if (!_strcmpi(name, "DisableSurfaceLocks")) { Config.DXPrimaryEmulation[AppCompatDataType.DisableSurfaceLocks] = IsValueEnabled(value); return; }
	if (!_strcmpi(name, "RedirectPrimarySurfBlts")) { Config.DXPrimaryEmulation[AppCompatDataType.RedirectPrimarySurfBlts] = IsValueEnabled(value); return; }
	if (!_strcmpi(name, "StripBorderStyle")) { Config.DXPrimaryEmulation[AppCompatDataType.StripBorderStyle] = IsValueEnabled(value); return; }
	if (!_strcmpi(name, "DisableMaxWindowedMode")) { Config.DXPrimaryEmulation[AppCompatDataType.DisableMaxWindowedMode] = IsValueEnabled(value); Config.DisableMaxWindowedModeNotSet = false; return; }
	// Numeric values
	if (!_strcmpi(name, "LoopSleepTime")) { Config.LoopSleepTime = atoi(value); return; }
	if (!_strcmpi(name, "ResetMemoryAfter")) { Config.ResetMemoryAfter = atoi(value); return; }
	if (!_strcmpi(name, "WindowSleepTime")) { Config.WindowSleepTime = atoi(value); return; }
	if (!_strcmpi(name, "SetFullScreenLayer")) { Config.SetFullScreenLayer = atoi(value); return; }
	if (!_strcmpi(name, "WrapperMode")) { Config.WrapperMode = atoi(value); return; }
	// Char values
	if (!_strcmpi(name, "RealDllPath")) { SetConfig(Config.szDllPath, value); return; }
	if (!_strcmpi(name, "RunProcess")) { SetConfig(Config.szShellPath, value); return; }
	// Memory Hack items
	if (!_strcmpi(name, "VerificationAddress")) { SetAddressPointerList(Config.VerifyMemoryInfo, value); return; }
	if (!_strcmpi(name, "VerificationBytes")) { SetBytesList(Config.VerifyMemoryInfo, value); return; }
	if (!_strcmpi(name, "AddressPointer")) { SetAddressPointerList(Config.MemoryInfo[++Config.AddressPointerCount], value); return; }
	if (!_strcmpi(name, "BytesToWrite")) { SetBytesList(Config.MemoryInfo[++Config.BytesToWriteCount], value); return; }
	// Lists of values
	if (!_strcmpi(name, "LoadCustomDllPath")) { SetConfigList(Config.szCustomDllPath, Config.CustomDllCount, value); return; }
	if (!_strcmpi(name, "SetNamedLayer")) { SetConfigList(Config.szSetNamedLayer, Config.NamedLayerCount, value); return; }
	if (!_strcmpi(name, "IgnoreWindowName")) { SetConfigList(Config.szIgnoreWindowName, Config.IgnoreWindowCount, value); return; }
	if (!_strcmpi(name, "ExcludeProcess")) { SetConfigList(szExclude, ExcludeCount, value); return; }
	if (!_strcmpi(name, "IncludeProcess")) { SetConfigList(szInclude, IncludeCount, value); return; }
	// Logging
	Compat::Log() << "Warning. Config setting not recognized: " << name;
}

// Strip path from a string
void strippath(char* str)
{
	int ch = '\\';
	size_t len;
	char* pdest;
	char* inpfile = NULL;

	// Search backwards for last backslash in filepath 
	pdest = strrchr(str, ch);

	// if backslash not found in filepath
	if (pdest == NULL)
	{
		return;
	}
	else {
		pdest++; // Skip the backslash itself.
	}

	// extract filename from file path
	len = strlen(pdest) + 1;
	inpfile = (char*)malloc(len);  // Make space for the zero.
	strcpy_s(inpfile, MAX_PATH, pdest);  // Copy. 
	strcpy_s(str, MAX_PATH, inpfile);  // Copy back. 
	return;
}

// Set default values
void ClearConfigSettings()
{
	// Cleanup memory (needs to be done first)
	Config.CleanUp();
	// Clear Config values
	Config.Affinity = false;
	Config.AffinityNotSet = true;  // Default to 'true' until we know it is set
	Config.D3d8to9 = false;
	Config.DDrawCompat = false;
	Config.DpiAware = false;
	Config.DxWnd = false;
	Config.FullScreen = false;
	Config.ForceTermination = false;
	Config.ForceWindowResize = false;
	Config.HandleExceptions = false;
	Config.DSoundCtrl = false;
	Config.ResetScreenRes = false;
	Config.SendAltEnter = false;
	Config.WaitForProcess = false;
	Config.WaitForWindowChanges = false;
	// Numeric values
	Config.LoopSleepTime = 0;
	Config.ResetMemoryAfter = 0;
	Config.WindowSleepTime = 0;
	Config.SetFullScreenLayer = 0;
	Config.WrapperMode = 0;
	// DSoundCtrl
	Config.Num2DBuffers = 0;
	Config.Num3DBuffers = 0;
	Config.ForceCertification = false;
	Config.ForceExclusiveMode = false;
	Config.ForceSoftwareMixing = false;
	Config.ForceHardwareMixing = false;
	Config.PreventSpeakerSetup = false;
	Config.ForceHQ3DSoftMixing = false;
	Config.ForceNonStaticBuffers = false;
	Config.ForceVoiceManagement = false;
	Config.ForcePrimaryBufferFormat = false;
	Config.PrimaryBufferBits = 16;
	Config.PrimaryBufferSamples = 44100;
	Config.PrimaryBufferChannels = 2;
	Config.ForceSpeakerConfig = false;
	Config.SpeakerConfig = 6;
	Config.EnableStoppedDriverWorkaround = false;
	// Arrary counters
	Config.AddressPointerCount = 0;
	Config.BytesToWriteCount = 0;
	Config.CustomDllCount = 0;
	Config.NamedLayerCount = 0;
	Config.IgnoreWindowCount = 0;
	// Char of values
	Config.szShellPath[0] = '\0';
	Config.szDllPath[0] = '\0';
	Config.szSetNamedLayer[0] = '\0';
	// AppCompatData
	for (int x = 1; x <= 12; x++)
	{
		Config.DXPrimaryEmulation[x] = false;
	}
	Config.LockColorkey = 0;
	Config.DisableMaxWindowedModeNotSet = true;  // Default to 'true' until we know it is set
	// Set local default values
	ExcludeCount = 0;
	IncludeCount = 0;
}

// Get wrapper mode based on dll name
void GetWrapperMode()
{
	char buffer[MAX_PATH];
	Config.RealWrapperMode = 0;
	GetModuleFileNameA(hModule_dll, buffer, sizeof(buffer));
	strippath(buffer);
	if (_strcmpi(buffer, "ddraw.dll") == 0) { Config.RealWrapperMode = dtype.ddraw; return; }
	if (_strcmpi(buffer, "d3d9.dll") == 0) { Config.RealWrapperMode = dtype.d3d9; return; }
	if (_strcmpi(buffer, "d3d8.dll") == 0) { Config.RealWrapperMode = dtype.d3d8; return; }
	if (_strcmpi(buffer, "dsound.dll") == 0) { Config.RealWrapperMode = dtype.dsound; return; }
	if (_strcmpi(buffer, "dxgi.dll") == 0) { Config.RealWrapperMode = dtype.dxgi; return; }
	if (_strcmpi(buffer, "dplayx.dll") == 0) { Config.RealWrapperMode = dtype.dplayx; return; }
	if (_strcmpi(buffer, "winspool.drv") == 0) { Config.RealWrapperMode = dtype.winspool; return; }

	// Special for winmm.dll because sometimes it is changed to win32 or winnm or some other name
	if (strlen(buffer) > 8)
	{
		buffer[3] = 'm';
		buffer[4] = 'm';
	}
	if (_strcmpi(buffer, "winmm.dll") == 0) { Config.RealWrapperMode = dtype.winmm; return; }
}

void CONFIG::CleanUp()
{
	DeleteArrayMemory(szExclude, ExcludeCount);
	DeleteArrayMemory(szInclude, IncludeCount);
	DeleteArrayMemory(Config.szCustomDllPath, Config.CustomDllCount);
	DeleteArrayMemory(Config.szIgnoreWindowName, Config.IgnoreWindowCount);
	DeleteByteToWriteArrayMemory();
}

void CONFIG::Init()
{
	// Reset all values
	ClearConfigSettings();

	// Get wrapper mode from dll name
	GetWrapperMode();
	Config.WrapperMode = Config.RealWrapperMode;

	// Set defaults
	Config.DpiAware = true;
	Config.DxWnd = true;
	Config.HandleExceptions = true;
	Config.ResetScreenRes = true;
	Config.LoopSleepTime = 120;
	Config.WindowSleepTime = 500;
	SetConfigList(szExclude, ExcludeCount, "dxwnd.exe");
	SetConfigList(szExclude, ExcludeCount, "dgVoodooSetup.exe");

	// Get config file path
	char* szCfg;
	char path[MAX_PATH];
	GetModuleFileNameA(hModule_dll, path, sizeof(path));
	char* pdest = strrchr(path, '.');
	strcpy_s(pdest, MAX_PATH, ".ini");

	// Get config file name for log
	pdest = strrchr(path, '\\')+1;
	for (char* p = pdest; *p != '\0'; p++) *p = (char)tolower(*p);
	Compat::Log() << "Reading config file: " << pdest;

	// Read config file
	szCfg = Read(path);

	// Read from dgame.ini config file if config does not already exist
	if (szCfg == NULL) {
		strcpy_s(pdest, MAX_PATH, "dgame.ini");
		Compat::Log() << "Reading config file: dgame.ini";
		szCfg = Read(path);
	}

	// Parce config file
	if (szCfg)
	{
		Parse(szCfg, ParseCallback);
		free(szCfg);
	}
	else {
		Compat::Log() << "Could not load config file using defaults";
	}

	// Verify sleep time to make sure it is not be set too low (can be perf issues if it is too low)
	if (Config.LoopSleepTime < 30) Config.LoopSleepTime = 30;

	// Get porcess name
	char szFileName[MAX_PATH];
	GetModuleFileName(NULL, szFileName, MAX_PATH);
	strippath(szFileName);

	// Check if process should be excluded or not included
	// if so, then clear all settings (disable everything)
	if ((ExcludeCount > 0 && IfStringExistsInList(szFileName, szExclude, ExcludeCount, false)) ||
		(IncludeCount > 0 && !IfStringExistsInList(szFileName, szInclude, IncludeCount, false)))
	{
		ClearConfigSettings();
	}

	// Update wrapper mode
	if (Config.WrapperMode == 0) Config.WrapperMode = Config.RealWrapperMode;
}