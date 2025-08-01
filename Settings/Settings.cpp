/**
* Copyright (C) 2025 Elisha Riedlinger
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
#include "Libraries\ScopeGuard.h"
#include "External\Hooking.Patterns\Hooking.Patterns.h"
#include "Dllmain\Dllmain.h"
#include "Wrappers\wrapper.h"
#include "Logging\Logging.h"

CONFIG Config;

namespace Settings
{
	// Config
	bool ConfigLoaded = false;
	char configpath[MAX_PATH] = {};

	// Declare variables
	bool WrapperModeIncorrect = false;
	size_t AddressPointerCount = 0;				// Count of addresses to hot patch
	size_t BytesToWriteCount = 0;				// Count of bytes to hot patch
	bool Force16bitColor;						// Forces DirectX to use 16bit color
	bool Force32bitColor;						// Forces DirectX to use 32bit color

	// Legacy settings
	DWORD AutoFrameSkip = 0;
	DWORD DdrawOverrideRefreshRate = 0;
	bool DSoundCtrl = false;
	bool EnvironmentMapCubeFix = false;
	bool DDrawCompatExperimental = false;
	bool DDrawCompat30 = false;
	bool DDrawCompat31 = false;

	// Function declarations
	bool IsValueEnabled(char*);
	void ClearValue(void**);
	void ClearValue(std::vector<std::string>*);
	void ClearValue(std::string*);
	void ClearValue(DWORD*);
	void ClearValue(float*);
	void ClearValue(double*);
	void ClearValue(bool*);
	void SetValue(char*, char*, MEMORYINFO*);
	void SetValue(char*, char*, void**);
	void SetValue(char*, char*, std::string*);
	void SetValue(char*, char*, DWORD*);
	void SetValue(char*, char*, float*);
	void SetValue(char*, char*, double*);
	void SetValue(char*, char*, bool*);
	void __stdcall ParseCallback(char*, char*);
	void SetDefaultConfigSettings();
	UINT GetWrapperMode(std::string *name);
}

#define VISIT_LOCAL_SETTINGS(visit) \
	visit(Force16bitColor) \
	visit(Force32bitColor)

#define SET_LOCAL_VALUE(functionName) \
	if (!_stricmp(name, #functionName)) \
	{ \
		SetValue(name, value, &functionName); \
		return; \
	}

#define SET_VALUE(functionName) \
	if (!_stricmp(name, #functionName)) \
	{ \
		SetValue(name, value, &Config.functionName); \
		return; \
	}

#define SET_APPCOMPATDATA_VALUE(functionName) \
	if (!_stricmp(name, #functionName)) \
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
		else if (_stricmp(szValue, szList[x].c_str()) == 0)
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
		_stricmp("on", name) == 0 ||
		_stricmp("yes", name) == 0 ||
		_stricmp("true", name) == 0 ||
		_stricmp("enable", name) == 0 ||
		_stricmp("enabled", name) == 0);
}

// Set value for MEMORYINFO
void Settings::SetValue(char* name, char* value, MEMORYINFO* MemoryInfo)
{
	// Declare vars
	char charTemp[] = { '0', 'x', '0' , '0', '\0' };
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
			MemoryInfo->Bytes[x - 1] = (byte)strtoul(charTemp, nullptr, 16);
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
	newString = std::regex_replace(newString, std::regex("^\\s+|\\s+$"), "");

	if (!newString.empty() && _stricmp(newString.c_str(), "auto") != 0)
	{
		setting->push_back(newString);
#ifdef _DEBUG
		Logging::Log() << name << " set to '" << setting->back().c_str() << "'";
#else
		UNREFERENCED_PARAMETER(name);
#endif
	}
}

// Set value for strings
void Settings::SetValue(char* name, char* value, std::string* setting)
{
	std::string newString;
	newString.assign(value);
	// Trim whitespaces
	newString = std::regex_replace(newString, std::regex("^\\s+|\\s+$"), "");

	if (!newString.empty() && _stricmp(newString.c_str(), "auto") != 0)
	{
		setting->assign(newString);
#ifdef _DEBUG
		Logging::Log() << name << " set to '" << setting->c_str() << "'";
#else
		UNREFERENCED_PARAMETER(name);
#endif
	}
}

// Set value for DWORD
void Settings::SetValue(char* name, char* value, DWORD* setting)
{
	DWORD NewValue = atoi(value);
	if (*setting != NewValue)
	{
		*setting = (NewValue) ? NewValue : IsValueEnabled(value);
#ifdef _DEBUG
		Logging::Log() << name << " set to '" << *setting << "'";
#else
		UNREFERENCED_PARAMETER(name);
#endif
	}
}

// Set value for float
void Settings::SetValue(char* name, char* value, float* setting)
{
	float NewValue = std::stof(value);
	if ( std::abs(*setting - NewValue) > 0.0001f )
	{
		*setting = NewValue;
#ifdef _DEBUG
		Logging::Log() << name << " set to '" << *setting << "'";
#else
		UNREFERENCED_PARAMETER(name);
#endif
	}
}

// Set value for double
void Settings::SetValue(char* name, char* value, double* setting)
{
	double NewValue = std::stod(value);
	if ( std::abs(*setting - NewValue) > 0.0001 )
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
	if (!_stricmp(name, "DisableMaxWindowedMode"))
	{
		Config.DisableMaxWindowedModeNotSet = false;
	}

	// For legacy settings
	SET_LOCAL_VALUE(AutoFrameSkip);
	SET_LOCAL_VALUE(DdrawOverrideRefreshRate);
	SET_LOCAL_VALUE(DSoundCtrl);
	SET_LOCAL_VALUE(EnvironmentMapCubeFix);
	SET_LOCAL_VALUE(DDrawCompatExperimental);
	SET_LOCAL_VALUE(DDrawCompat30);
	SET_LOCAL_VALUE(DDrawCompat31);

	// Set Value of local settings
	VISIT_LOCAL_SETTINGS(SET_LOCAL_VALUE);

	// Set Value of normal config settings
	VISIT_CONFIG_SETTINGS(SET_VALUE);

	// Set Value of AppCompatData LockColorkey setting
	if (!_stricmp(name, "LockColorkey"))
	{
		SetValue(name, value, &Config.LockColorkey);
		Config.DXPrimaryEmulation[AppCompatDataType.LockColorkey] = true;
		return;
	}

	// Set Value of AppCompatData config settings
	VISIT_APPCOMPATDATA_SETTINGS(SET_APPCOMPATDATA_VALUE);

	// Set Value of Memory Hack config settings
	if (!_stricmp(name, "VerificationAddress"))
	{
		SetValue(name, value, &Config.VerifyMemoryInfo.AddressPointer);
		return;
	}
	if (!_stricmp(name, "VerificationBytes"))
	{
		SetValue(name, value, &Config.VerifyMemoryInfo);
		return;
	}
	if (!_stricmp(name, "AddressPointer"))
	{
		if (Config.MemoryInfo.size() < AddressPointerCount + 1)
		{
			MEMORYINFO newMemoryInfo;
			Config.MemoryInfo.push_back(newMemoryInfo);
		}
		SetValue(name, value, &Config.MemoryInfo[AddressPointerCount++].AddressPointer);
		return;
	}
	if (!_stricmp(name, "PatternString"))
	{
		std::string PatterString(value);
		auto pattern = hook::pattern(value);
		if (Config.MemoryInfo.size() < AddressPointerCount + 1)
		{
			MEMORYINFO newMemoryInfo;
			Config.MemoryInfo.push_back(newMemoryInfo);
		}
		Config.MemoryInfo[AddressPointerCount].PatternString.assign(value);
		Config.MemoryInfo[AddressPointerCount++].AddressPointer = pattern.size() > 0UL ? pattern.get(0).get<uint32_t*>(0) : nullptr;
		return;
	}
	if (!_stricmp(name, "BytesToWrite"))
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

// Clear float
void Settings::ClearValue(float* setting)
{
	*setting = 0.0f;
}

// Clear double
void Settings::ClearValue(double* setting)
{
	*setting = 0.0;
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
}

// Get wrapper mode based on dll name
UINT Settings::GetWrapperMode(std::string *name)
{
	// Check each wrapper library
	// Start at '1' to exclude '0' and dxwrapper.dll
	for (UINT x = 1; x < dtypeArraySize; ++x)
	{
		// Check dll name
		if (_stricmp(name->c_str(), dtypename[x]) == 0)
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
	// Set value to check if it exists in the ini file
	Config.EnableD3d9Wrapper = NOT_EXIST;
	Config.DdrawHookSystem32 = NOT_EXIST;
	Config.D3d8HookSystem32 = NOT_EXIST;
	Config.D3d9HookSystem32 = NOT_EXIST;
	Config.DinputHookSystem32 = NOT_EXIST;
	Config.Dinput8HookSystem32 = NOT_EXIST;
	Config.DsoundHookSystem32 = NOT_EXIST;
	Config.DdrawResolutionHack = NOT_EXIST;
	Config.CacheClipPlane = NOT_EXIST;
	Config.LimitStateBlocks = NOT_EXIST;
	Config.WindowModeGammaShader = NOT_EXIST;

	// Other values that may not exist in ini file
	Config.DisableMaxWindowedModeNotSet = true;

	// Set defaults
	Config.DisableHighDPIScaling = true;
	Config.FixSpeakerConfigType = true;

	// Set other default values
	Config.LoopSleepTime = 120;
	Config.WindowSleepTime = 500;
	Config.PrimaryBufferBits = 16;
	Config.PrimaryBufferSamples = 44100;
	Config.PrimaryBufferChannels = 2;
	Config.AudioFadeOutDelayMS = 20;
	SetValue("ExcludeProcess", "dxwnd.exe", &Config.ExcludeProcess);
	SetValue("ExcludeProcess", "dgVoodooSetup.exe", &Config.ExcludeProcess);
}

bool CONFIG::IsSet(DWORD Value)
{
	return (Value && Value != NOT_EXIST);
}

void CONFIG::Init()
{
	using namespace Settings;

	// Reset all values
	ClearConfigSettings();

	// Get module handle
	HMODULE hModule = NULL;
	GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCTSTR)Settings::ClearConfigSettings, &hModule);

	// Declare variables
	HeapBuffer<char> p_wName(MAX_PATH);
	HeapBuffer<char> p_pName(MAX_PATH);

	// Get module name
	HeapBuffer<char> wrappername(MAX_PATH);
	GetModuleFileName(hModule, wrappername.data(), MAX_PATH);
	if (strrchr(wrappername.data(), '\\'))
	{
		strcpy_s(p_wName.data(), MAX_PATH, strrchr(wrappername.data(), '\\') + 1);
	}

	// Get process name
	HeapBuffer<char> processname(MAX_PATH);
	GetModuleFileName(nullptr, processname.data(), MAX_PATH);
	if (strrchr(processname.data(), '\\'))
	{
		strcpy_s(p_pName.data(), MAX_PATH, strrchr(processname.data(), '\\') + 1);
	}

	// Set default settings
	SetDefaultConfigSettings();

	// Set settings from environment variables
	if (LPCH p_envStrings = GetEnvironmentStrings(); p_envStrings)
	{
		const char* szEnvPrefix = "DXWRAPPER_";
		char* szEnvVar = p_envStrings;
		HeapBuffer<char> szSetting(MAX_ENV_VAR);

		while (*szEnvVar)
		{
			if (!_strnicmp(szEnvVar, szEnvPrefix, strlen(szEnvPrefix)))
			{
				szEnvVar += strlen(szEnvPrefix);
				strcpy_s(szSetting.data(), MAX_ENV_VAR, szEnvVar);
				Parse(szSetting.data(), ParseCallback);
			}
			szEnvVar += strlen(szEnvVar) + 1;
		}

		FreeEnvironmentStrings(p_envStrings);
	}

	// Check for memory loading
	if (_stricmp(p_wName.data(), p_pName.data()) == 0)
	{
		strcpy_s(wrappername.data(), MAX_PATH, processname.data());
		strcpy_s(strrchr(wrappername.data(), '\\'), MAX_PATH - strlen(wrappername.data()), "\\dxwrapper.dll");
	}

	// Get config path to include process name
	strcpy_s(configpath, MAX_PATH, wrappername.data());
	strcpy_s(strrchr(configpath, '.'), MAX_PATH - strlen(configpath), "-");
	strcat_s(configpath, MAX_PATH, p_pName.data());
	strcpy_s(strrchr(configpath, '.'), MAX_PATH - strlen(configpath), ".ini");

	// Read defualt config file
	char* szCfg = ReadFileContent(configpath);

	// Parce config file
	if (szCfg)
	{
		ConfigLoaded = true;
		Parse(szCfg, ParseCallback);
		free(szCfg);
	}
	// If cannot load config file check for default config
	else
	{
		// Get config file path
		strcpy_s(configpath, MAX_PATH, wrappername.data());
		strcpy_s(strrchr(configpath, '.'), MAX_PATH - strlen(configpath), ".ini");

		// Open config file
		szCfg = ReadFileContent(configpath);

		// Parce config file
		if (szCfg)
		{
			ConfigLoaded = true;
			Parse(szCfg, ParseCallback);
			free(szCfg);
		}
	}

	// Set module name
	if (_stricmp(p_wName.data(), p_pName.data()) == 0)
	{
		WrapperName.assign("dxwrapper.dll");
	}
	else
	{
		WrapperName.assign(p_wName.data());
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
			WrapperModeIncorrect = true;
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
	if ((ExcludeProcess.size() != 0 && IfStringExistsInList(p_pName.data(), ExcludeProcess, false)) ||
		(IncludeProcess.size() != 0 && !IfStringExistsInList(p_pName.data(), IncludeProcess, false)))
	{
		ProcessExcluded = true;
	}
}

void CONFIG::SetConfig()
{
	using namespace Settings;

	// If config file was read
	if (ConfigLoaded)
	{
		Logging::Log() << "Reading config file: " << configpath;
	}
	else
	{
		Logging::Log() << "Could not load config file using defaults";
	}

	// Check wrapper mode
	if (WrapperModeIncorrect)
	{
		Logging::Log() << "Error: Wrapper mode setting incorrect!";
	}

	// Verify sleep time to make sure it is not set too low (can be perf issues if it is too low)
	if (LoopSleepTime < 30)
	{
		Logging::Log() << "Cannot set 'LoopSleepTime' to less than 30ms!  Resetting to 30ms.";
		LoopSleepTime = 30;
	}

	// Verify DirectSoundControl options
	EnableDsoundWrapper = (EnableDsoundWrapper || DSoundCtrl || IsSet(DsoundHookSystem32));
	if (EnableDsoundWrapper)
	{
		if (ForceSoftwareMixing && ForceHardwareMixing)
		{
			Logging::Log() << "Cannot set both 'ForceSoftwareMixing' and 'ForceHardwareMixing'!  Disabling 'ForceSoftwareMixing'.";
			ForceSoftwareMixing = false;
		}
		if (ForceVoiceManagement && (ForceSoftwareMixing || ForceHardwareMixing))
		{
			if (ForceSoftwareMixing)
			{
				Logging::Log() << "Cannot set both 'ForceVoiceManagement' and 'ForceSoftwareMixing'!  Disabling 'ForceVoiceManagement'.";
			}
			else
			{
				Logging::Log() << "Cannot set both 'ForceVoiceManagement' and 'ForceHardwareMixing'!  Disabling 'ForceVoiceManagement'.";
			}
			ForceVoiceManagement = false;
		}
		if (ForceVoiceManagement && !ForceNonStaticBuffers)
		{
			Logging::Log() << "'ForceNonStaticBuffers' should always be enabled with 'ForceVoiceManagement'!  Enabling 'ForceNonStaticBuffers'.";
			ForceNonStaticBuffers = true;
		}
		if (ForceHQ3DSoftMixing && ForceHardwareMixing)
		{
			Logging::Log() << "'ForceHQ3DSoftMixing' has no effect when 'ForceHardwareMixing' is enabled!  Disabling 'ForceHQ3DSoftMixing'.";
			ForceHQ3DSoftMixing = false;
		}
	}

	// Default to 8 samples as some games have issues with more samples
	AntiAliasing = (AntiAliasing == 1 || AntiAliasing > 16) ? 8 : AntiAliasing;

	// Enable refresh rate
	OverrideRefreshRate = (DdrawOverrideRefreshRate && !OverrideRefreshRate) ? DdrawOverrideRefreshRate : OverrideRefreshRate;
	DdrawOverrideRefreshRate = 0;

	// Enable wrapper settings
	Dinputto8 = (Dinputto8 || IsSet(Dinput8HookSystem32));
	EnableDinput8Wrapper = (EnableDinput8Wrapper || IsSet(Dinput8HookSystem32));

	DDrawCompat32 = (DDrawCompat30 || DDrawCompat31 || DDrawCompat32 || DDrawCompatExperimental ||
		(DDrawCompat && !DDrawCompat20 && !DDrawCompat21));
	DDrawCompat = (DDrawCompat || DDrawCompat20 || DDrawCompat21 || DDrawCompat32);
	if (DDrawCompat32)
	{
		DDrawCompat20 = false;
		DDrawCompat21 = false;
	}
	else if (DDrawCompat21)
	{
		DDrawCompat20 = false;
	}
	EnableDdrawWrapper = (EnableDdrawWrapper || IsSet(DdrawHookSystem32) || IsSet(DdrawResolutionHack) || DdrawUseDirect3D9Caps || Dd7to9);
	D3d8to9 = (D3d8to9 || IsSet(D3d8HookSystem32));
	DdrawAutoFrameSkip = (AutoFrameSkip || DdrawAutoFrameSkip);																	// For legacy purposes
	EnableWindowMode = (FullscreenWindowMode) ? true : EnableWindowMode;
	EnableD3d9Wrapper = (IsSet(EnableD3d9Wrapper) || IsSet(D3d9HookSystem32) || D3d9to9Ex ||
		(EnableD3d9Wrapper == NOT_EXIST && (AnisotropicFiltering || AntiAliasing || IsSet(CacheClipPlane) || EnableVSync ||		// For legacy purposes
			ForceMixedVertexProcessing || ForceSystemMemVertexCache || ForceVsyncMode || EnableWindowMode)));					// For legacy purposes
	EnvironmentCubeMapFix = EnvironmentCubeMapFix || EnvironmentMapCubeFix;														// For legacy purposes

	// Set ddraw color bit mode
	DdrawOverrideBitMode = (DdrawOverrideBitMode) ? DdrawOverrideBitMode : (Force32bitColor) ? 32 : (Force16bitColor) ? 16 : 0;
	switch (DdrawOverrideBitMode)
	{
	case 0:
		break;
	case 1:
	case 8:
		DdrawOverrideBitMode = 8;
		break;
	case 2:
	case 16:
		DdrawOverrideBitMode = 16;
		break;
	case 3:
	case 24:
		DdrawOverrideBitMode = 24;
		break;
	case 4:
	case 32:
		DdrawOverrideBitMode = 32;
		break;
	default:
		Logging::Log() << "Invalid 'DdrawOverrideBitMode'!  Disabling...";
		DdrawOverrideBitMode = 0;
		break;
	}

	// Check stencil format
	if (DdrawOverrideStencilFormat && DdrawOverrideStencilFormat < 70 || DdrawOverrideStencilFormat > 80)
	{
		Logging::Log() << __FUNCTION__ << " Invalid 'DdrawOverrideStencilFormat'!  Disabling...";
		DdrawOverrideStencilFormat = 0;
	}

	// Check if any DXPrimaryEmulation flags are set
	for (UINT x = 1; x <= 12; x++)
	{
		if (Config.DXPrimaryEmulation[x])
		{
			isAppCompatDataSet = true;
		}
	}

	// Disable DDrawCompat process affinity if dxwrapper's SingleProcAffinity is enabled
	if (SingleProcAffinity)
	{
		DDrawCompatNoProcAffinity = true;
	}

	// Set mouse scroll factor
	if (abs(MouseMovementFactor) < 0.01f || abs(MouseMovementFactor - 1.0f) < 0.01f)
	{
		MouseMovementFactor = 1.0f;
	}
	else if (MouseMovementFactor != 0.0f)
	{
		FixHighFrequencyMouse = true;
	}

	// Mouse movement padding
	if (MouseMovementPadding)
	{
		FixHighFrequencyMouse = true;
	}

	// Limit surface emulation size
	DdrawExtraEmulationSize = DdrawExtraEmulationSize == 1 ? 4000 : min(DdrawExtraEmulationSize, 10000);

	// Windows Lie
	WinVersionLieSP = (WinVersionLieSP > 0 && WinVersionLieSP <= 5) ? WinVersionLieSP : 0;

	// Set unset options
	CacheClipPlane = (CacheClipPlane != 0);
	DdrawResolutionHack = (DdrawResolutionHack != 0);
	LimitStateBlocks = (LimitStateBlocks != NOT_EXIST) ? LimitStateBlocks : (Dd7to9 || D3d8to9);
	WindowModeGammaShader = (WindowModeGammaShader != 0);
}
