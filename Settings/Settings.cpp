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
* Code in EraseCppComments, Parse, Read, ParseCallback and strippath functions taken from source code found in Aqrit's ddwrapper
* http://bitpatch.com/ddwrapper.html
*/

#include <stdlib.h>
#include "Settings.h"
#include "Dllmain\Dllmain.h"
#include "Logging\Logging.h"

typedef void(__stdcall* NV)(char* name, char* value);

CONFIG Config;

namespace Settings
{
	// Declare varables
	CRITICAL_SECTION CriticalSectionCfg;
	byte ExcludeCount;
	byte IncludeCount;
	char* szExclude[256];
	char* szInclude[256];

	// Function declarations
	void DeleteArrayMemory(char* [256], byte);
	void DeleteByteToWriteArrayMemory();
	void EraseCppComments(char*);
	void Parse(char*, NV);
	char* Read(char*);
	void SetConfig(char*, char*);
	void SetAddressPointerList(MEMORYINFO&, char*);
	void SetBytesList(MEMORYINFO&, char*);
	bool IsValueEnabled(char*);
	void LogSetting(char*, char*);
	void SetValue(char*, char*, DWORD*);
	void SetValue(char*, char*, bool*);
	void ParseConfigValue(char*, char*);
	void __stdcall ParseCallback(char*, char*);
	void strippath(char*);
	void ClearConfigSettings();
	void GetWrapperMode();
}

// Checks if a string value exists in a string array
bool Settings::IfStringExistsInList(char* szValue, char* szList[256], byte ListCount, bool CaseSensitive)
{
	for (UINT x = 1; x <= ListCount; ++x)
	{
		// Case sensitive check
		if (CaseSensitive)
		{
			if (strcmp(szValue, szList[x]) == 0)
			{
				return true;
			}
		}
		// Case insensitive check
		else if (_strcmpi(szValue, szList[x]) == 0)
		{
			return true;
		}
	}
	return false;
}

// Deletes all string values from an array
void Settings::DeleteArrayMemory(char* szList[256], byte ListCount)
{
	for (UINT x = 1; x <= ListCount; ++x)
	{
		delete szList[x];
	}
}

// Deletes all BytesToWrite values from the BytesToWrite array
void Settings::DeleteByteToWriteArrayMemory()
{
	for (UINT x = 1; x <= Config.BytesToWriteCount; ++x)
	{
		if (Config.MemoryInfo[x].SizeOfBytes > 0)
		{
			delete Config.MemoryInfo[x].Bytes;
		}
	}
}

// any C++ style (both inline and block) commented text is replaced with a space character 
// todo: skip comments inside double quotes
void Settings::EraseCppComments(char* str)
{
	while ((str = strchr(str, '/')) != 0)
	{
		if (str[1] == '/')
		{
			for (; ((*str != '\0') && (*str != '\n')); str++)
			{
				*str = '\x20';
			}
		}
		else if (str[1] == '*')
		{
			for (; ((*str != '\0') && ((str[0] != '*') || (str[1] != '/'))); str++)
			{
				*str = '\x20';
			}
			if (*str)
			{
				*str++ = '\x20';
				*str = '\x20';
			}
		}
		if (*str)
		{
			str++;
		}
		else
		{
			break;
		}
	}
}

// [sections] are ignored
// escape characters NOT support 
// double quotes NOT suppoted
// Name/value delimiter is an equal sign or colon 
// whitespace is removed from before and after both the name and value
// characters considered to be whitespace:
//  0x20 - space
//	0x09 - horizontal tab
//	0x0D - carriage return
void Settings::Parse(char* str, NV NameValueCallback)
{
	char *next_token = nullptr;
	EraseCppComments(str);
	for (str = strtok_s(str, "\n", &next_token); str; str = strtok_s(0, "\n", &next_token))
	{
		if (*str == ';' || *str == '#')
		{
			continue; // skip INI style comments ( must be at start of line )
		}
		char* rvalue = strchr(str, '=');
		if (!rvalue)
		{
			rvalue = strchr(str, ':');
		}
		if (rvalue)
		{
			*rvalue++ = '\0'; // split left and right values

			rvalue = &rvalue[strspn(rvalue, "\x20\t\r")]; // skip beginning whitespace
			for (char* end = strchr(rvalue, '\0'); (--end >= rvalue) && (*end == '\x20' || *end == '\t' || *end == '\r');)
			{
				*end = '\0';  // truncate ending whitespace
			}

			char* lvalue = &str[strspn(str, "\x20\t\r")]; // skip beginning whitespace
			for (char* end = strchr(lvalue, '\0'); (--end >= lvalue) && (*end == '\x20' || *end == '\t' || *end == '\r');)
			{
				*end = '\0';  // truncate ending whitespace
			}

			if (*lvalue && *rvalue)
			{
				NameValueCallback(lvalue, rvalue);
			}
		}
	}
}

// Reads szFileName from disk
char* Settings::Read(char* szFileName)
{
	HANDLE hFile;
	DWORD dwBytesToRead;
	DWORD dwBytesRead;

	char* szCfg = nullptr;
	hFile = CreateFile(szFileName, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		dwBytesToRead = GetFileSize(hFile, nullptr);
		if ((dwBytesToRead != 0) && (dwBytesToRead != 0xFFFFFFFF))
		{
			szCfg = (char*)malloc(dwBytesToRead + 1); // +1 so a NULL terminator can be added
			if (szCfg)
			{
				if (ReadFile(hFile, szCfg, dwBytesToRead, &dwBytesRead, nullptr))
				{
					if (dwBytesRead != 0)
					{
						szCfg[dwBytesRead] = '\0'; // make txt file easy to deal with 
					}
				}
				else
				{
					free(szCfg);
					szCfg = nullptr;
				}
			}
		}
		CloseHandle(hFile);
	}
	return szCfg;
}

// Set config from string (file)
void Settings::SetConfig(char* name, char* value)
{
	if (!_strcmpi(value, "AUTO") != 0)
	{
		name[0] = '\0';
	}
	else if (strlen(value) <= MAX_PATH)
	{
		strcpy_s(name, MAX_PATH, value);
	}
}

// Set config from string (file)
void Settings::SetConfigList(char* name[], byte& count, char* value)
{
	if (strlen(value) <= MAX_PATH)
	{
		count++;
		name[count] = new char[strlen(value) + 1];
		strcpy_s(name[count], MAX_PATH, value);
	}
}

// Set AddressPointer array from string (file)
void Settings::SetAddressPointerList(MEMORYINFO& MemoryInfo, char* value)
{
	// Get address pointer
	if (strtoul(value, nullptr, 16) > 0 &&						// Verify pointer has a value higher than 0
		value[0] == '0' && (char)tolower(value[1]) == 'x')		// Check for leading "0x" to indicate hex number
	{
		MemoryInfo.AddressPointer = strtoul(value, nullptr, 16);
	}
	// Set to 0 if invalid address pointer
	else
	{
		MemoryInfo.AddressPointer = 0;
	}
}

// Set BytesToWrite to memory from string (file)
void Settings::SetBytesList(MEMORYINFO& MemoryInfo, char* value)
{
	// Declare vars
	char charTemp[] = { '0', 'x', '0' , '0' };
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
		for (DWORD x = 1; x <= size; x++)
		{
			charTemp[2] = value[x * 2];
			charTemp[3] = value[(x * 2) + 1];
			MemoryInfo.Bytes[x - 1] = (byte)strtoul(charTemp, nullptr, 16);
		}
	}
	// Set to 0 if invalid Bytes are set
	else
	{
		MemoryInfo.SizeOfBytes = 0;
	}
}

// Set booloean value from string (file)
bool Settings::IsValueEnabled(char* name)
{
	return (atoi(name) > 0 ||
		_strcmpi("on", name) == 0 ||
		_strcmpi("yes", name) == 0 ||
		_strcmpi("true", name) == 0 ||
		_strcmpi("enabled", name) == 0);
}

// Log setting value for bool
void Settings::LogSetting(char* name, char* value)
{
#ifdef _DEBUG
	Logging::Log() << name << " set to '" << value << "'";
#else
	UNREFERENCED_PARAMETER(name);
	UNREFERENCED_PARAMETER(value);
#endif
}

// Set value for DWORD
void Settings::SetValue(char* name, char* value, DWORD* setting)
{
	DWORD NewValue = atoi(value);
	if (*setting != NewValue)
	{
#ifdef _DEBUG
		Logging::Log() << name << " set to '" << NewValue << "'";
#else
		UNREFERENCED_PARAMETER(name);
#endif
		*setting = NewValue;
	}
}

// Set value for bool
void Settings::SetValue(char* name, char* value, bool* setting)
{
	bool NewValue = IsValueEnabled(value);
	if (*setting != NewValue)
	{
#ifdef _DEBUG
		char* NewValueText = "false";
		if (NewValue) NewValueText = "true";
		Logging::Log() << name << " set to '" << NewValueText << "'";
#else
		UNREFERENCED_PARAMETER(name);
#endif
		*setting = NewValue;
	}
}

// Set config from string (file)
void Settings::ParseConfigValue(char* name, char* value)
{
	// Boolean values
	if (!_strcmpi(name, "SingleProcAffinity"))
	{
		// Sets Affinity and AffinityNotSet flags
		SetValue(name, value, &Config.Affinity);
		Config.AffinityNotSet = false; return;
	}
	if (!_strcmpi(name, "D3d8to9"))
	{
		SetValue(name, value, &Config.D3d8to9);
		return;
	}
	if (!_strcmpi(name, "DDrawCompat"))
	{
		SetValue(name, value, &Config.DDrawCompat);
		return;
	}
	if (!_strcmpi(name, "DDrawCompatDisableGDIHook"))
	{
		SetValue(name, value, &Config.DDrawCompatDisableGDIHook);
		return;
	}
	if (!_strcmpi(name, "DisableHighDpiScaling"))
	{
		SetValue(name, value, &Config.DpiAware);
		return;
	}
	if (!_strcmpi(name, "DxWnd"))
	{
		SetValue(name, value, &Config.DxWnd);
		return;
	}
	if (!_strcmpi(name, "FullScreen"))
	{
		SetValue(name, value, &Config.FullScreen);
		return;
	}
	if (!_strcmpi(name, "ForceTermination"))
	{
		SetValue(name, value, &Config.ForceTermination);
		return;
	}
	if (!_strcmpi(name, "ForceWindowResize"))
	{
		SetValue(name, value, &Config.ForceWindowResize);
		return;
	}
	if (!_strcmpi(name, "HandleExceptions"))
	{
		SetValue(name, value, &Config.HandleExceptions);
		return;
	}
	if (!_strcmpi(name, "DSoundCtrl"))
	{
		SetValue(name, value, &Config.DSoundCtrl);
		return;
	}
	if (!_strcmpi(name, "ResetScreenRes"))
	{
		SetValue(name, value, &Config.ResetScreenRes);
		return;
	}
	if (!_strcmpi(name, "SendAltEnter"))
	{
		SetValue(name, value, &Config.SendAltEnter);
		return;
	}
	if (!_strcmpi(name, "WaitForProcess"))
	{
		SetValue(name, value, &Config.WaitForProcess);
		return;
	}
	if (!_strcmpi(name, "WaitForWindowChanges"))
	{
		SetValue(name, value, &Config.WaitForWindowChanges);
		return;
	}
	// DSoundCtrl
	if (!_strcmpi(name, "Num2DBuffers"))
	{
		SetValue(name, value, &Config.Num2DBuffers);
		return;
	}
	if (!_strcmpi(name, "Num3DBuffers"))
	{
		SetValue(name, value, &Config.Num3DBuffers);
		return;
	}
	if (!_strcmpi(name, "ForceCertification"))
	{
		SetValue(name, value, &Config.ForceCertification);
		return;
	}
	if (!_strcmpi(name, "ForceExclusiveMode"))
	{
		SetValue(name, value, &Config.ForceExclusiveMode);
		return;
	}
	if (!_strcmpi(name, "ForceSoftwareMixing"))
	{
		SetValue(name, value, &Config.ForceSoftwareMixing);
		return;
	}
	if (!_strcmpi(name, "ForceHardwareMixing"))
	{
		SetValue(name, value, &Config.ForceHardwareMixing);
		return;
	}
	if (!_strcmpi(name, "PreventSpeakerSetup"))
	{
		SetValue(name, value, &Config.PreventSpeakerSetup);
		return;
	}
	if (!_strcmpi(name, "ForceHQ3DSoftMixing"))
	{
		SetValue(name, value, &Config.ForceHQ3DSoftMixing);
		return;
	}
	if (!_strcmpi(name, "ForceNonStaticBuffers"))
	{
		SetValue(name, value, &Config.ForceNonStaticBuffers);
		return;
	}
	if (!_strcmpi(name, "ForceVoiceManagement"))
	{
		SetValue(name, value, &Config.ForceVoiceManagement);
		return;
	}
	if (!_strcmpi(name, "ForcePrimaryBufferFormat"))
	{
		SetValue(name, value, &Config.ForcePrimaryBufferFormat);
		return;
	}
	if (!_strcmpi(name, "PrimaryBufferBits"))
	{
		SetValue(name, value, &Config.PrimaryBufferBits);
		return;
	}
	if (!_strcmpi(name, "PrimaryBufferSamples"))
	{
		SetValue(name, value, &Config.PrimaryBufferSamples);
		return;
	}
	if (!_strcmpi(name, "PrimaryBufferChannels"))
	{
		SetValue(name, value, &Config.PrimaryBufferChannels);
		return;
	}
	if (!_strcmpi(name, "ForceSpeakerConfig"))
	{
		SetValue(name, value, &Config.ForceSpeakerConfig);
		return;
	}
	if (!_strcmpi(name, "SpeakerConfig"))
	{
		SetValue(name, value, &Config.SpeakerConfig);
		return;
	}
	if (!_strcmpi(name, "StoppedDriverWorkaround"))
	{
		SetValue(name, value, &Config.StoppedDriverWorkaround);
		return;
	}
	// AppCompatData
	if (!_strcmpi(name, "LockEmulation"))
	{
		SetValue(name, value, &Config.DXPrimaryEmulation[AppCompatDataType.LockEmulation]);
		return;
	}
	if (!_strcmpi(name, "BltEmulation"))
	{
		SetValue(name, value, &Config.DXPrimaryEmulation[AppCompatDataType.BltEmulation]);
		return;
	}
	if (!_strcmpi(name, "ForceLockNoWindow"))
	{
		SetValue(name, value, &Config.DXPrimaryEmulation[AppCompatDataType.ForceLockNoWindow]);
		return;
	}
	if (!_strcmpi(name, "ForceBltNoWindow"))
	{
		SetValue(name, value, &Config.DXPrimaryEmulation[AppCompatDataType.ForceBltNoWindow]);
		return;
	}
	if (!_strcmpi(name, "LockColorkey"))
	{
		// Sets DXPrimaryEmulation and LockColorkey
		SetValue(name, value, &Config.LockColorkey);
		Config.DXPrimaryEmulation[AppCompatDataType.LockColorkey] = true;
		return;
	}
	if (!_strcmpi(name, "FullscreenWithDWM"))
	{
		SetValue(name, value, &Config.DXPrimaryEmulation[AppCompatDataType.FullscreenWithDWM]);
		return;
	}
	if (!_strcmpi(name, "DisableLockEmulation"))
	{
		SetValue(name, value, &Config.DXPrimaryEmulation[AppCompatDataType.DisableLockEmulation]);
		return;
	}
	if (!_strcmpi(name, "EnableOverlays"))
	{
		SetValue(name, value, &Config.DXPrimaryEmulation[AppCompatDataType.EnableOverlays]);
		return;
	}
	if (!_strcmpi(name, "DisableSurfaceLocks"))
	{
		SetValue(name, value, &Config.DXPrimaryEmulation[AppCompatDataType.DisableSurfaceLocks]);
		return;
	}
	if (!_strcmpi(name, "RedirectPrimarySurfBlts"))
	{
		SetValue(name, value, &Config.DXPrimaryEmulation[AppCompatDataType.RedirectPrimarySurfBlts]);
		return;
	}
	if (!_strcmpi(name, "StripBorderStyle"))
	{
		SetValue(name, value, &Config.DXPrimaryEmulation[AppCompatDataType.StripBorderStyle]);
		return;
	}
	if (!_strcmpi(name, "DisableMaxWindowedMode"))
	{
		// Sets DisableMaxWindowedMode and DisableMaxWindowedModeNotSet flags
		SetValue(name, value, &Config.DXPrimaryEmulation[AppCompatDataType.DisableMaxWindowedMode]);
		Config.DisableMaxWindowedModeNotSet = false;
		return;
	}
	// Numeric values
	if (!_strcmpi(name, "LoopSleepTime"))
	{
		SetValue(name, value, &Config.LoopSleepTime);
		return;
	}
	if (!_strcmpi(name, "ResetMemoryAfter"))
	{
		SetValue(name, value, &Config.ResetMemoryAfter);
		return;
	}
	if (!_strcmpi(name, "WindowSleepTime"))
	{
		SetValue(name, value, &Config.WindowSleepTime);
		return;
	}
	if (!_strcmpi(name, "SetFullScreenLayer"))
	{
		SetValue(name, value, &Config.SetFullScreenLayer);
		return;
	}
	if (!_strcmpi(name, "WrapperMode"))
	{
		SetValue(name, value, &Config.WrapperMode);
		return;
	}
	// Char values
	if (!_strcmpi(name, "RealDllPath"))
	{
		SetConfig(Config.szDllPath, value);
		LogSetting(name, value);
		return;
	}
	if (!_strcmpi(name, "RunProcess"))
	{
		SetConfig(Config.szShellPath, value);
		LogSetting(name, value);
		return;
	}
	// Memory Hack items
	if (!_strcmpi(name, "VerificationAddress"))
	{
		SetAddressPointerList(Config.VerifyMemoryInfo, value);
		LogSetting(name, value);
		return;
	}
	if (!_strcmpi(name, "VerificationBytes"))
	{
		SetBytesList(Config.VerifyMemoryInfo, value);
		LogSetting(name, value);
		return;
	}
	if (!_strcmpi(name, "AddressPointer"))
	{
		SetAddressPointerList(Config.MemoryInfo[++Config.AddressPointerCount], value);
		LogSetting(name, value);
		return;
	}
	if (!_strcmpi(name, "BytesToWrite"))
	{
		SetBytesList(Config.MemoryInfo[++Config.BytesToWriteCount], value);
		LogSetting(name, value);
		return;
	}
	// Lists of values
	if (!_strcmpi(name, "LoadCustomDllPath"))
	{
		SetConfigList(Config.szCustomDllPath, Config.CustomDllCount, value);
		LogSetting(name, value);
		return;
	}
	if (!_strcmpi(name, "SetNamedLayer"))
	{
		SetConfigList(Config.szSetNamedLayer, Config.NamedLayerCount, value);
		LogSetting(name, value);
		return;
	}
	if (!_strcmpi(name, "IgnoreWindowName"))
	{
		SetConfigList(Config.szIgnoreWindowName, Config.IgnoreWindowCount, value);
		LogSetting(name, value);
		return;
	}
	if (!_strcmpi(name, "ExcludeProcess"))
	{
		SetConfigList(szExclude, ExcludeCount, value);
		LogSetting(name, value);
		return;
	}
	if (!_strcmpi(name, "IncludeProcess"))
	{
		SetConfigList(szInclude, IncludeCount, value);
		LogSetting(name, value);
		return;
	}
	// Logging
	Logging::Log() << "Warning. Config setting not recognized: " << name;
}

// Set config from string (file)
void __stdcall Settings::ParseCallback(char* name, char* value)
{
	// Critical section
	EnterCriticalSection(&CriticalSectionCfg);
	// Parce config value
	ParseConfigValue(name, value);
	// Critical section
	LeaveCriticalSection(&CriticalSectionCfg);
}

// Strip path from a string
void Settings::strippath(char* str)
{
	int ch = '\\';
	size_t len;
	char* pdest;
	char* inpfile = nullptr;

	// Search backwards for last backslash in filepath 
	pdest = strrchr(str, ch);

	// if backslash not found in filepath
	if (!pdest)
	{
		return;
	}
	else
	{
		pdest++; // Skip the backslash itself.
	}

	// extract filename from file path
	len = strlen(pdest) + 1;
	inpfile = (char*)malloc(len);			// Make space for the zero.
	strcpy_s(inpfile, MAX_PATH, pdest);		// Copy. 
	strcpy_s(str, MAX_PATH, inpfile);		// Copy back. 
	return;
}

// Set default values
void Settings::ClearConfigSettings()
{
	// Cleanup memory (needs to be done first)
	Config.CleanUp();
	// Clear Config values
	Config.Affinity = false;
	Config.AffinityNotSet = true;  // Default to 'true' until we know it is set
	Config.D3d8to9 = false;
	Config.DDrawCompat = false;
	Config.DDrawCompatDisableGDIHook = false;
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
	Config.StoppedDriverWorkaround = false;
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
	Config.DisableMaxWindowedModeNotSet = true;  // Default to 'true' until we know it is set
	for (UINT x = 1; x <= 12; x++)
	{
		Config.DXPrimaryEmulation[x] = false;
	}
	Config.LockColorkey = 0;
	// Set local default values
	ExcludeCount = 0;
	IncludeCount = 0;
}

// Get wrapper mode based on dll name
void Settings::GetWrapperMode()
{
	char buffer[MAX_PATH];
	Config.RealWrapperMode = 0;
	GetModuleFileNameA(hModule_dll, buffer, sizeof(buffer));
	strippath(buffer);

	// Check each wrapper library
	for (UINT x = 1; x < dtypeArraySize; ++x)
	{
		// Check dll name
		if (_strcmpi(buffer, dtypename[x]) == 0)
		{
			// Set RealWrapperMode
			Config.RealWrapperMode = x;
			return;
		}
	}

	// Special for winmm.dll because sometimes it is changed to win32 or winnm or some other name
	if (strlen(buffer) > 8)
	{
		buffer[3] = 'm';
		buffer[4] = 'm';
	}
	if (_strcmpi(buffer, dtypename[dtype.winmm]) == 0)
	{
		// Set RealWrapperMode
		Config.RealWrapperMode = dtype.winmm;
	}
}

void CONFIG::CleanUp()
{
	using namespace Settings;
	DeleteArrayMemory(szExclude, ExcludeCount);
	DeleteArrayMemory(szInclude, IncludeCount);
	DeleteArrayMemory(Config.szCustomDllPath, Config.CustomDllCount);
	DeleteArrayMemory(Config.szIgnoreWindowName, Config.IgnoreWindowCount);
	DeleteByteToWriteArrayMemory();
}

void CONFIG::Init()
{
	using namespace Settings;

	// Initialize the critical section one time only.
	InitializeCriticalSectionAndSpinCount(&CriticalSectionCfg, 0);

	// Reset all values
	ClearConfigSettings();

	// Get wrapper mode from dll name
	GetWrapperMode();
	Config.WrapperMode = Config.RealWrapperMode;

	// Set defaults
	Config.DpiAware = true;
	Config.DxWnd = true;
	Config.HandleExceptions = true;
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
	pdest = strrchr(path, '\\') + 1;
	for (char* p = pdest; *p != '\0'; p++)
	{
		*p = (char)tolower(*p);
	}
	Logging::Log() << "Reading config file: " << pdest;

	// Read config file
	szCfg = Read(path);

	// Parce config file
	if (szCfg)
	{
		Parse(szCfg, ParseCallback);
		free(szCfg);
	}
	else
	{
		Logging::Log() << "Could not load config file using defaults";
	}

	// Verify sleep time to make sure it is not be set too low (can be perf issues if it is too low)
	if (Config.LoopSleepTime < 30) Config.LoopSleepTime = 30;

	// Get porcess name
	char szFileName[MAX_PATH];
	GetModuleFileName(nullptr, szFileName, MAX_PATH);
	strippath(szFileName);

	// Check if process should be excluded or not included
	// if so, then clear all settings (disable everything)
	if ((ExcludeCount > 0 && IfStringExistsInList(szFileName, szExclude, ExcludeCount, false)) ||
		(IncludeCount > 0 && !IfStringExistsInList(szFileName, szInclude, IncludeCount, false)))
	{
		Logging::Log() << "Clearing config and disabling dxwrapper!";
		ClearConfigSettings();
	}

	// Update wrapper mode
	if (Config.WrapperMode == 0)
	{
		Config.WrapperMode = Config.RealWrapperMode;
	}

	// Release resources used by the critical section object.
	DeleteCriticalSection(&CriticalSectionCfg);
}
