/**
* Copyright (C) 2019 Elisha Riedlinger
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
*/

#include <regex>
#include <algorithm>
#include "Settings.h"
#include "Dllmain\Dllmain.h"
#include "Wrappers\wrapper.h"
#include "Logging\Logging.h"

CONFIG Config;

namespace Settings
{
	// Declare variables
	size_t AddressPointerCount = 0;				// Count of addresses to hot patch
	size_t BytesToWriteCount = 0;				// Count of bytes to hot patch

	// Function declarations
	bool IsValueEnabled(char*);
	void ClearValue(void**);
	void ClearValue(std::vector<std::string>*);
	void ClearValue(std::string*);
	void ClearValue(DWORD*);
	void ClearValue(bool*);
	void SetValue(char*, char*, MEMORYINFO*);
	void SetValue(char*, char*, void**);
	void SetValue(char*, char*, std::string*);
	void SetValue(char*, char*, DWORD*);
	void SetValue(char*, char*, bool*);
	void __stdcall ParseCallback(char*, char*);
	void SetDefaultConfigSettings();
	UINT GetWrapperMode(std::string *name);
}

#define SET_VALUE(functionName) \
	if (!_strcmpi(name, #functionName)) \
	{ \
		SetValue(name, value, &Config.functionName); \
		return; \
	}

#define SET_APPCOMPATDATA_VALUE(functionName) \
	if (!_strcmpi(name, #functionName)) \
	{ \
		SetValue(name, value, &Config.DXPrimaryEmulation[AppCompatDataType.functionName]); \
		return; \
	}

#define CLEAR_VALUE(functionName) \
	ClearValue(&Config.functionName);

#define CLEAR_APPCOMPATDATA_VALUE(functionName) \
	ClearValue(&Config.DXPrimaryEmulation[AppCompatDataType.functionName]);

// Checks if a string value exists in a string array
bool Settings::IfStringExistsInList(const char* szValue, std::vector<std::string> szList, bool CaseSensitive)
{
	for (UINT x = 0; x < szList.size(); ++x)
	{
		// Case sensitive check
		if (CaseSensitive)
		{
			if (strcmp(szValue, szList[x].c_str()) == 0)
			{
				return true;
			}
		}
		// Case insensitive check
		else if (_strcmpi(szValue, szList[x].c_str()) == 0)
		{
			return true;
		}
	}
	return false;
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

// Set value for MEMORYINFO
void Settings::SetValue(char* name, char* value, MEMORYINFO* MemoryInfo)
{
	// Declare vars
	char charTemp[] = { '0', 'x', '0' , '0' };
	char *charEnd = &charTemp[3];
	DWORD len = strlen(value);

	// Check for valid bytes
	if ((len > 3) &&											// String has at least one byte
		(len % 2 == 0) &&										// String is even
		value[0] == '0' && (char)tolower(value[1]) == 'x')		// Check for leading "0x" to indicate hex number
	{
		// Get bytes size
		DWORD size = (len / 2) - 1;
		MemoryInfo->Bytes.resize(size, '\0');

		// Get byte data
		for (DWORD x = 1; x <= size; x++)
		{
			charTemp[2] = value[x * 2];
			charTemp[3] = value[(x * 2) + 1];
			MemoryInfo->Bytes[x - 1] = (byte)strtoul(charTemp, &charEnd, 16);
		}
#ifdef _DEBUG
		std::string buffer((size + 2) * 2, '\0');
		for (size_t j = 0; j < size; j++)
		{
			sprintf_s(&buffer[2 * j], 3, "%02X", (byte)MemoryInfo->Bytes[j]);
		}
		Logging::Log() << name << " set to '" << buffer.c_str() << "'";
#else
		UNREFERENCED_PARAMETER(name);
#endif
	}
}

// Set value for pointers
void Settings::SetValue(char* name, char* value, void** setting)
{
	// Set to zero
	*setting = nullptr;

	// Get address pointer
	if (strtoul(value, nullptr, 16) > 0 &&						// Verify pointer has a value higher than 0
		value[0] == '0' && (char)tolower(value[1]) == 'x')		// Check for leading "0x" to indicate hex number
	{
		*setting = (void*)strtoul(value, nullptr, 16);
	}
#ifdef _DEBUG
	Logging::Log() << name << " set to '" << *setting << "'";
#else
	UNREFERENCED_PARAMETER(name);
#endif
}

// Set value for vector of strings
void Settings::SetValue(char* name, char* value, std::vector<std::string>* setting)
{
	std::string newString;
	newString.assign(value);
	// Trim whitespaces
	newString = std::regex_replace(newString, std::regex("(^\\s*(.*\\S)\\s*$)|(^\\s*$)"), "$2");
	if (newString.size() != 0)
	{
		setting->push_back(newString);
	}
#ifdef _DEBUG
	Logging::Log() << name << " set to '" << setting->back().c_str() << "'";
#else
	UNREFERENCED_PARAMETER(name);
#endif
}

// Set value for string
void Settings::SetValue(char* name, char* value, std::string* setting)
{
	setting->assign(value);
	// Trim whitespaces
	*setting = std::regex_replace(*setting, std::regex("(^\\s*(.*\\S)\\s*$)|(^\\s*$)"), "$2");
#ifdef _DEBUG
	Logging::Log() << name << " set to '" << setting->c_str() << "'";
#else
	UNREFERENCED_PARAMETER(name);
#endif
}

// Set value for DWORD
void Settings::SetValue(char* name, char* value, DWORD* setting)
{
	DWORD NewValue = atoi(value);
	if (*setting != NewValue)
	{
		*setting = NewValue;
#ifdef _DEBUG
		Logging::Log() << name << " set to '" << *setting << "'";
#else
		UNREFERENCED_PARAMETER(name);
#endif
	}
}

// Set value for bool
void Settings::SetValue(char* name, char* value, bool* setting)
{
	bool NewValue = IsValueEnabled(value);
	if (*setting != NewValue)
	{
		*setting = NewValue;
#ifdef _DEBUG
		char* NewValueText = "false";
		if (*setting) NewValueText = "true";
		Logging::Log() << name << " set to '" << NewValueText << "'";
#else
		UNREFERENCED_PARAMETER(name);
#endif
	}
}

// Set config from string (file)
void __stdcall Settings::ParseCallback(char* name, char* value)
{
	// Check for the existance of certian values
	if (!_strcmpi(name, "DisableMaxWindowedMode"))
	{
		Config.DisableMaxWindowedModeNotSet = false;
	}
	if (!_strcmpi(name, "CacheClipPlane"))
	{
		Config.CacheClipPlaneNotSet = false;
	}
	if (!_strcmpi(name, "SingleProcAffinity"))
	{
		Config.SingleProcAffinityNotSet = false;
	}

	// Set Value of normal config settings
	VISIT_CONFIG_SETTINGS(SET_VALUE);

	// Set Value of AppCompatData LockColorkey setting
	if (!_strcmpi(name, "LockColorkey"))
	{
		SetValue(name, value, &Config.LockColorkey);
		Config.DXPrimaryEmulation[AppCompatDataType.LockColorkey] = true;
		return;
	}

	// Set Value of AppCompatData config settings
	VISIT_APPCOMPATDATA_SETTINGS(SET_APPCOMPATDATA_VALUE);

	// Set Value of Memory Hack config settings
	if (!_strcmpi(name, "VerificationAddress"))
	{
		SetValue(name, value, &Config.VerifyMemoryInfo.AddressPointer);
		return;
	}
	if (!_strcmpi(name, "VerificationBytes"))
	{
		SetValue(name, value, &Config.VerifyMemoryInfo);
		return;
	}
	if (!_strcmpi(name, "AddressPointer"))
	{
		if (Config.MemoryInfo.size() < AddressPointerCount + 1)
		{
			MEMORYINFO newMemoryInfo;
			Config.MemoryInfo.push_back(newMemoryInfo);
		}
		SetValue(name, value, &Config.MemoryInfo[AddressPointerCount++].AddressPointer);
		return;
	}
	if (!_strcmpi(name, "BytesToWrite"))
	{
		if (Config.MemoryInfo.size() < BytesToWriteCount + 1)
		{
			MEMORYINFO newMemoryInfo;
			Config.MemoryInfo.push_back(newMemoryInfo);
		}
		SetValue(name, value, &Config.MemoryInfo[BytesToWriteCount++]);
		return;
	}

	// Logging
	Logging::Log() << "Warning. Config setting not recognized: " << name;
}

// Clear pointers
void Settings::ClearValue(void** setting)
{
	*setting = nullptr;
}

// Clear vector of strings
void Settings::ClearValue(std::vector<std::string>* setting)
{
	setting->clear();
}

// Clear strings
void Settings::ClearValue(std::string* setting)
{
	setting->clear();
}

// Clear DWORD
void Settings::ClearValue(DWORD* setting)
{
	*setting = 0;
}

// Clear bool
void Settings::ClearValue(bool* setting)
{
	*setting = false;
}

// Clear all values
void Settings::ClearConfigSettings()
{
	// Set array size to zero
	AddressPointerCount = 0;
	BytesToWriteCount = 0;

	// Clear Verification memory bytes
	Config.VerifyMemoryInfo.AddressPointer = nullptr;
	Config.VerifyMemoryInfo.Bytes.clear();

	// Clear MemoryInfo vector
	Config.MemoryInfo.clear();

	// Clear normal config settings
	VISIT_CONFIG_SETTINGS(CLEAR_VALUE);

	// Set Value of AppCompatData config settings
	VISIT_APPCOMPATDATA_SETTINGS(CLEAR_APPCOMPATDATA_VALUE);

	// Default to 'true' until we know it is set
	Config.DisableMaxWindowedModeNotSet = true;
	Config.CacheClipPlaneNotSet = true;
	Config.SingleProcAffinityNotSet = true;
}

// Get wrapper mode based on dll name
UINT Settings::GetWrapperMode(std::string *name)
{
	// Check each wrapper library
	// Start at '1' to exclude '0' and dxwrapper.dll
	for (UINT x = 1; x < dtypeArraySize; ++x)
	{
		// Check dll name
		if (_strcmpi(name->c_str(), dtypename[x]) == 0)
		{
			return x;
		}
	}

	// Special for winmm.dll because sometimes it is changed to win32 or winnm or some other name
	if (name->size() > 8)
	{
		if (dtypename[dtype.winmm][0] == (char)tolower((*name)[0]) &&
			dtypename[dtype.winmm][1] == (char)tolower((*name)[1]) &&
			dtypename[dtype.winmm][2] == (char)tolower((*name)[2]) &&
			dtypename[dtype.winmm][5] == (char)tolower((*name)[5]) &&
			dtypename[dtype.winmm][6] == (char)tolower((*name)[6]) &&
			dtypename[dtype.winmm][7] == (char)tolower((*name)[7]) &&
			dtypename[dtype.winmm][8] == (char)tolower((*name)[8]))
		{
			return dtype.winmm;
		}
	}
	return (UINT)-1;
}

// Set default values
void Settings::SetDefaultConfigSettings()
{
	// Set defaults
	Config.DisableHighDPIScaling = true;
	Config.DxWnd = true;
	Config.ResetScreenRes = true;
	Config.DDrawResolutionHack = true;

	// Set other default values
	Config.LoopSleepTime = 120;
	Config.WindowSleepTime = 500;
	Config.PrimaryBufferBits = 16;
	Config.PrimaryBufferSamples = 44100;
	Config.PrimaryBufferChannels = 2;
	Config.SpeakerConfig = 6;
	SetValue("ExcludeProcess", "dxwnd.exe", &Config.ExcludeProcess);
	SetValue("ExcludeProcess", "dgVoodooSetup.exe", &Config.ExcludeProcess);
}

void CONFIG::Init()
{
	using namespace Settings;

	// Reset all values
	ClearConfigSettings();

	// Get module handle
	HMODULE hModule = NULL;
	GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCTSTR)Settings::ClearConfigSettings, &hModule);

	// Get module name
	char wrappername[MAX_PATH] = { 0 };
	GetModuleFileName(hModule, wrappername, MAX_PATH);
	char* p_wName = strrchr(wrappername, '\\') + 1;

	// Get process name
	char processname[MAX_PATH] = { 0 };
	GetModuleFileName(nullptr, processname, MAX_PATH);
	char* p_pName = strrchr(processname, '\\') + 1;

	// Set default settings
	SetDefaultConfigSettings();

	// Check for memory loading
	if (_strcmpi(p_wName, p_pName) == 0)
	{
		strcpy_s(wrappername, MAX_PATH, processname);
		strcpy_s(strrchr(wrappername, '\\'), MAX_PATH - strlen(wrappername), "\\dxwrapper.dll");
	}

	// Set lower case
	for (char* p = wrappername; *p != '\0'; p++) { *p = (char)tolower(*p); }

	// Get config path to include process name
	char buffer[MAX_PATH] = { 0 };
	strcpy_s(buffer, MAX_PATH, wrappername);
	strcpy_s(strrchr(buffer, '.'), MAX_PATH - strlen(buffer), "-");
	strcat_s(buffer, MAX_PATH, p_pName);
	strcpy_s(strrchr(buffer, '.'), MAX_PATH - strlen(buffer), ".ini");

	// Read defualt config file
	char* szCfg = Read(buffer);

	// Parce config file
	if (szCfg)
	{
		Logging::Log() << "Reading config file: " << strrchr(buffer, '\\') + 1;
		Parse(szCfg, ParseCallback);
		free(szCfg);
	}

	// If cannot load config file check for default config
	if (!szCfg)
	{
		// Get config file path
		strcpy_s(buffer, MAX_PATH, wrappername);
		strcpy_s(strrchr(buffer, '.'), MAX_PATH - strlen(buffer), ".ini");

		// Open config file
		szCfg = Read(buffer);

		// Parce config file
		if (szCfg)
		{
			Logging::Log() << "Reading config file: " << strrchr(buffer, '\\') + 1;
			Parse(szCfg, ParseCallback);
			free(szCfg);
		}
	}

	// If config file cannot be read
	if (!szCfg)
	{
		Logging::Log() << "Could not load config file using defaults";
	}

	// Set module name
	if (_strcmpi(p_wName, p_pName) == 0)
	{
		WrapperName.assign("dxwrapper.dll");
	}
	else
	{
		WrapperName.assign(p_wName);
		std::transform(WrapperName.begin(), WrapperName.end(), WrapperName.begin(),
			[](char c) {return static_cast<char>(::tolower(c)); });
	}

	// Check wrapper mode
	if (WrapperMode.size())
	{
		std::transform(WrapperMode.begin(), WrapperMode.end(), WrapperMode.begin(),
			[](char c) {return static_cast<char>(::tolower(c)); });
		if (!Wrapper::CheckWrapperName(WrapperMode.c_str()))
		{
			Logging::Log() << "Error: Wrapper mode setting incorrect!";
			WrapperName.clear();
		}
	}

	// Set RealWrapperMode
	if (!WrapperMode.size() && !Wrapper::CheckWrapperName(WrapperName.c_str()))
	{
		RealWrapperMode = dtype.dxwrapper;
	}
	else
	{
		RealWrapperMode = GetWrapperMode((GetWrapperMode(&WrapperMode) != (UINT)-1) ? &WrapperMode : &WrapperName);
	}

	// Check if process should be excluded or not included
	// if so, then clear all settings (disable everything)
	ProcessExcluded = false;
	if ((ExcludeProcess.size() != 0 && IfStringExistsInList(p_pName, ExcludeProcess, false)) ||
		(IncludeProcess.size() != 0 && !IfStringExistsInList(p_pName, IncludeProcess, false)))
	{
		ProcessExcluded = true;
		Logging::Log() << "Clearing config and disabling dxwrapper!";
		ClearConfigSettings();
	}

	// Verify sleep time to make sure it is not set too low (can be perf issues if it is too low)
	if (LoopSleepTime < 30)
	{
		Logging::Log() << "Cannot set LoopSleepTime to less than 30ms!  Resetting to 30ms.";
		LoopSleepTime = 30;
	}

	// Verify DSoundCtrl options
	if (DSoundCtrl)
	{
		if (ForceSoftwareMixing && ForceHardwareMixing)
		{
			Logging::Log() << "Cannot set both ForceSoftwareMixing and ForceHardwareMixing!  Disabling ForceSoftwareMixing.";
			ForceSoftwareMixing = false;
		}
		if (ForceVoiceManagement && (ForceSoftwareMixing || ForceHardwareMixing))
		{
			if (ForceSoftwareMixing)
			{
				Logging::Log() << "Cannot set both ForceVoiceManagement and ForceSoftwareMixing!  Disabling ForceVoiceManagement.";
			}
			else
			{
				Logging::Log() << "Cannot set both ForceVoiceManagement and ForceSoftwareMixing!  Disabling ForceVoiceManagement.";
			}
			ForceVoiceManagement = false;
		}
		if (ForceVoiceManagement && !ForceNonStaticBuffers)
		{
			Logging::Log() << "ForceNonStaticBuffers should always be enabled with ForceVoiceManagement!  Enabling ForceNonStaticBuffers.";
			ForceNonStaticBuffers = true;
		}
		if (ForceHQ3DSoftMixing && ForceHardwareMixing)
		{
			Logging::Log() << "ForceHQ3DSoftMixing has no effect whenForceHardwareMixing is enabled!  Disabling ForceHQ3DSoftMixing.";
			ForceHQ3DSoftMixing = false;
		}
	}

	// Enable wrapper settings
	if (Dd7to9)
	{
		ConvertToDirectDraw7 = true;
		ConvertToDirect3D7 = true;
	}

	DDrawCompat = (DDrawCompat || DDrawCompat20 || DDrawCompat21 || DDrawCompatExperimental);
	isDdrawWrapperEnabled = (EnableDdrawWrapper || ConvertToDirectDraw7 || ConvertToDirect3D7);
	isD3d9WrapperEnabled = (AntiAliasing || CacheClipPlane || EnableVSync || EnableWindowMode);

	// Enable clip plane caching by default
	if (CacheClipPlaneNotSet)
	{
		CacheClipPlane = true;
	}
}
