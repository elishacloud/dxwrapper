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

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>
#include <sstream>
#include "External\Hooking\Hook.h"
#include "Utils.h"
#include "Logging\Logging.h"

namespace {
	HKL g_ForcedHKL = nullptr;
	HKL g_PreviousHKL = nullptr;
	std::string g_LayoutKLIDA;	// e.g. "00000409"
	std::wstring g_LayoutKLIDW;   // e.g. L"00000409"
	DWORD g_LayoutID = 0x409;
	UINT g_ACP = 1252;
	UINT g_OEMCP = 437;
	bool g_Enabled = false;

	// Real functions
	HKL(WINAPI* real_GetKeyboardLayout)(DWORD) = nullptr;
	HKL(WINAPI* real_ActivateKeyboardLayout)(HKL, UINT) = nullptr;
	HKL(WINAPI* real_LoadKeyboardLayoutA)(LPCSTR, UINT) = nullptr;
	HKL(WINAPI* real_LoadKeyboardLayoutW)(LPCWSTR, UINT) = nullptr;
	BOOL(WINAPI* real_GetKeyboardLayoutNameA)(LPSTR) = nullptr;
	BOOL(WINAPI* real_GetKeyboardLayoutNameW)(LPWSTR) = nullptr;
	int(WINAPI* real_GetKeyboardLayoutList)(int, HKL*) = nullptr;
	LCID(WINAPI* real_GetUserDefaultLCID)() = nullptr;
	LCID(WINAPI* real_GetSystemDefaultLCID)() = nullptr;
	LANGID(WINAPI* real_GetUserDefaultUILanguage)() = nullptr;
	LANGID(WINAPI* real_GetSystemDefaultUILanguage)() = nullptr;
	UINT(WINAPI* real_GetACP)() = nullptr;
	UINT(WINAPI* real_GetKBCodePage)() = nullptr;
	UINT(WINAPI* real_GetOEMCP)() = nullptr;
	UINT(WINAPI* real_GetConsoleCP)() = nullptr;
	UINT(WINAPI* real_GetConsoleOutputCP)() = nullptr;
	int(WINAPI* real_GetLocaleInfoA)(LCID lcid, LCTYPE LCType, LPSTR lpLCData, int cchData) = nullptr;
	int(WINAPI* real_GetLocaleInfoW)(LCID lcid, LCTYPE LCType, LPWSTR lpLCData, int cchData) = nullptr;
};

// Helper
static HKL FindInstalledHKL(WORD langId)
{
	HKL layouts[64] = {};
	int count = GetKeyboardLayoutList(_countof(layouts), layouts);
	if (count <= 0) {
		return nullptr;
	}

	for (int i = 0; i < count; i++)
	{
		HKL hkl = layouts[i];

		// Compare only LANGID part
		if (LOWORD((UINT_PTR)hkl) == langId)
		{
			return hkl;   // Found a real layout installed on system
		}
	}

	return nullptr; // Not installed
}


// Hooked Functions
static HKL WINAPI Hook_GetKeyboardLayout(DWORD idThread)
{
	if (g_Enabled)
	{
		return g_ForcedHKL;
	}

	return real_GetKeyboardLayout(idThread);
}

static HKL WINAPI Hook_ActivateKeyboardLayout(HKL hkl, UINT flags)
{
	if (g_Enabled)
	{
		return g_ForcedHKL;
	}

	return real_ActivateKeyboardLayout(hkl, flags);
}

static HKL WINAPI Hook_LoadKeyboardLayoutA(LPCSTR klid, UINT flags)
{
	if (g_Enabled)
	{
		return g_ForcedHKL;
	}

	return real_LoadKeyboardLayoutA(klid, flags);
}

static HKL WINAPI Hook_LoadKeyboardLayoutW(LPCWSTR klid, UINT flags)
{
	if (g_Enabled)
	{
		return g_ForcedHKL;
	}

	return real_LoadKeyboardLayoutW(klid, flags);
}

static BOOL WINAPI Hook_GetKeyboardLayoutNameA(LPSTR name)
{
	if (g_Enabled && name)
	{
		strcpy_s(name, 9, g_LayoutKLIDA.c_str());
		return TRUE;
	}

	return real_GetKeyboardLayoutNameA(name);
}

static BOOL WINAPI Hook_GetKeyboardLayoutNameW(LPWSTR name)
{
	if (g_Enabled && name)
	{
		wcscpy_s(name, 9, g_LayoutKLIDW.c_str());
		return TRUE;
	}

	return real_GetKeyboardLayoutNameW(name);
}

static int WINAPI Hook_GetKeyboardLayoutList(int nBuff, HKL* lpList)
{
	if (g_Enabled)
	{
		if (lpList && nBuff > 0)
		{
			lpList[0] = g_ForcedHKL;
		}
		return 1; // we only have 1 layout
	}

	return real_GetKeyboardLayoutList(nBuff, lpList);
}

static LCID WINAPI Hook_GetUserDefaultLCID()
{
	if (g_Enabled)
	{
		return MAKELCID(g_LayoutID, SORT_DEFAULT);
	}

	return real_GetUserDefaultLCID();
}

static LCID WINAPI Hook_GetSystemDefaultLCID()
{
	if (g_Enabled)
	{
		return MAKELCID(g_LayoutID, SORT_DEFAULT);
	}

	return real_GetSystemDefaultLCID();
}

static LANGID WINAPI Hook_GetUserDefaultUILanguage()
{
	if (g_Enabled)
	{
		return LOWORD(g_LayoutID);
	}

	return real_GetUserDefaultUILanguage();
}

static LANGID WINAPI Hook_GetSystemDefaultUILanguage()
{
	if (g_Enabled)
	{
		return LOWORD(g_LayoutID);
	}

	return real_GetSystemDefaultUILanguage();
}

static UINT WINAPI Hook_GetACP()
{
	if (g_Enabled)
	{
		return g_ACP;
	}

	return real_GetACP();
}

static UINT WINAPI Hook_GetKBCodePage()
{
	if (g_Enabled)
	{
		return g_OEMCP;
	}

	return real_GetKBCodePage();
}

static UINT WINAPI Hook_GetOEMCP()
{
	if (g_Enabled)
	{
		return g_OEMCP;
	}

	return real_GetOEMCP();
}

static UINT WINAPI Hook_GetConsoleCP()
{
	if (g_Enabled)
	{
		return g_OEMCP;
	}

	return real_GetConsoleCP();
}

static UINT WINAPI Hook_GetConsoleOutputCP()
{
	if (g_Enabled)
	{
		return g_OEMCP;
	}

	return real_GetConsoleOutputCP();
}

static int WINAPI Hook_GetLocaleInfoA(LCID lcid, LCTYPE LCType, LPSTR lpLCData, int cchData)
{
	if (!g_Enabled)
	{
		return real_GetLocaleInfoA(lcid, LCType, lpLCData, cchData);
	}

	auto WriteA = [&](const char* buf)
		{
			size_t len = strlen(buf);

			// Correct handling of cchData == 0 or lpLCData == nullptr
			if (!lpLCData || cchData == 0)
			{
				return (int)(len + 1);   // required size including null
			}

			size_t copyLen = (len < (size_t)(cchData - 1)) ? len : (size_t)(cchData - 1);
			memcpy(lpLCData, buf, copyLen);
			lpLCData[copyLen] = '\0';

			return (int)len;			// number of chars excluding null
		};

	switch (LCType)
	{
	case LOCALE_IDEFAULTANSICODEPAGE:
	{
		char buf[16];
		_snprintf_s(buf, sizeof(buf), _TRUNCATE, "%u", g_ACP);
		return WriteA(buf);
	}

	case LOCALE_IDEFAULTCODEPAGE:
	{
		char buf[16];
		_snprintf_s(buf, sizeof(buf), _TRUNCATE, "%u", g_OEMCP);
		return WriteA(buf);
	}

	case LOCALE_ILANGUAGE:
	{
		char buf[16];
		_snprintf_s(buf, sizeof(buf), _TRUNCATE, "%04X", (unsigned)LOWORD(g_LayoutID));
		return WriteA(buf);
	}

	case LOCALE_SISO639LANGNAME:
	{
		char buf[16] = {};
		int ret = real_GetLocaleInfoA(LOWORD(g_LayoutID), LCType, buf, sizeof(buf));

		if (ret > 0)
		{
			return WriteA(buf);
		}
		break;
	}
	}

	return real_GetLocaleInfoA(lcid, LCType, lpLCData, cchData);
}

static int WINAPI Hook_GetLocaleInfoW(LCID lcid, LCTYPE LCType, LPWSTR lpLCData, int cchData)
{
	if (!g_Enabled)
	{
		return real_GetLocaleInfoW(lcid, LCType, lpLCData, cchData);
	}

	auto WriteW = [&](const wchar_t* buf)
		{
			size_t len = wcslen(buf);

			// Correct handling of cchData == 0 or lpLCData == nullptr
			if (!lpLCData || cchData == 0)
			{
				return (int)(len + 1);  // required size including null
			}

			size_t copyLen = (len < (size_t)(cchData - 1)) ? len : (size_t)(cchData - 1);
			wmemcpy(lpLCData, buf, copyLen);
			lpLCData[copyLen] = L'\0';

			return (int)len;		   // number of chars excluding null
		};

	switch (LCType)
	{
	case LOCALE_IDEFAULTANSICODEPAGE:
	{
		wchar_t buf[16];
		swprintf_s(buf, _countof(buf), L"%u", g_ACP);
		return WriteW(buf);
	}

	case LOCALE_IDEFAULTCODEPAGE:
	{
		wchar_t buf[16];
		swprintf_s(buf, _countof(buf), L"%u", g_OEMCP);
		return WriteW(buf);
	}

	case LOCALE_ILANGUAGE:
	{
		wchar_t buf[16];
		swprintf_s(buf, _countof(buf), L"%04X", (unsigned)LOWORD(g_LayoutID));
		return WriteW(buf);
	}

	case LOCALE_SISO639LANGNAME:
	{
		wchar_t buf[16] = {};
		int ret = real_GetLocaleInfoW(LOWORD(g_LayoutID), LCType, buf, _countof(buf));

		if (ret > 0)
		{
			return WriteW(buf);
		}
		break;
	}
	}

	return real_GetLocaleInfoW(lcid, LCType, lpLCData, cchData);
}


// Install Hooks
static bool InstallHooks()
{
	HMODULE user32 = GetModuleHandleA("user32.dll");
	HMODULE kernel32 = GetModuleHandleA("kernel32.dll");
	if (!user32 || !kernel32) return false;

	InterlockedExchangePointer((PVOID*)&real_GetKeyboardLayout, Hook::HotPatch(Hook::GetProcAddress(user32, "GetKeyboardLayout"), "GetKeyboardLayout", Hook_GetKeyboardLayout));
	InterlockedExchangePointer((PVOID*)&real_ActivateKeyboardLayout, Hook::HotPatch(Hook::GetProcAddress(user32, "ActivateKeyboardLayout"), "ActivateKeyboardLayout", Hook_ActivateKeyboardLayout));
	InterlockedExchangePointer((PVOID*)&real_LoadKeyboardLayoutA, Hook::HotPatch(Hook::GetProcAddress(user32, "LoadKeyboardLayoutA"), "LoadKeyboardLayoutA", Hook_LoadKeyboardLayoutA));
	InterlockedExchangePointer((PVOID*)&real_LoadKeyboardLayoutW, Hook::HotPatch(Hook::GetProcAddress(user32, "LoadKeyboardLayoutW"), "LoadKeyboardLayoutW", Hook_LoadKeyboardLayoutW));
	InterlockedExchangePointer((PVOID*)&real_GetKeyboardLayoutNameA, Hook::HotPatch(Hook::GetProcAddress(user32, "GetKeyboardLayoutNameA"), "GetKeyboardLayoutNameA", Hook_GetKeyboardLayoutNameA));
	InterlockedExchangePointer((PVOID*)&real_GetKeyboardLayoutNameW, Hook::HotPatch(Hook::GetProcAddress(user32, "GetKeyboardLayoutNameW"), "GetKeyboardLayoutNameW", Hook_GetKeyboardLayoutNameW));
	InterlockedExchangePointer((PVOID*)&real_GetKeyboardLayoutList, Hook::HotPatch(Hook::GetProcAddress(user32, "GetKeyboardLayoutList"), "GetKeyboardLayoutList", Hook_GetKeyboardLayoutList));
	InterlockedExchangePointer((PVOID*)&real_GetKBCodePage, Hook::HotPatch(Hook::GetProcAddress(user32, "GetKBCodePage"), "GetKBCodePage", Hook_GetKBCodePage));

	InterlockedExchangePointer((PVOID*)&real_GetUserDefaultLCID, Hook::HotPatch(Hook::GetProcAddress(kernel32, "GetUserDefaultLCID"), "GetUserDefaultLCID", Hook_GetUserDefaultLCID));
	InterlockedExchangePointer((PVOID*)&real_GetSystemDefaultLCID, Hook::HotPatch(Hook::GetProcAddress(kernel32, "GetSystemDefaultLCID"), "GetSystemDefaultLCID", Hook_GetSystemDefaultLCID));
	InterlockedExchangePointer((PVOID*)&real_GetUserDefaultUILanguage, Hook::HotPatch(Hook::GetProcAddress(kernel32, "GetUserDefaultUILanguage"), "GetUserDefaultUILanguage", Hook_GetUserDefaultUILanguage));
	InterlockedExchangePointer((PVOID*)&real_GetSystemDefaultUILanguage, Hook::HotPatch(Hook::GetProcAddress(kernel32, "GetSystemDefaultUILanguage"), "GetSystemDefaultUILanguage", Hook_GetSystemDefaultUILanguage));
	InterlockedExchangePointer((PVOID*)&real_GetACP, Hook::HotPatch(Hook::GetProcAddress(kernel32, "GetACP"), "GetACP", Hook_GetACP));
	InterlockedExchangePointer((PVOID*)&real_GetOEMCP, Hook::HotPatch(Hook::GetProcAddress(kernel32, "GetOEMCP"), "GetOEMCP", Hook_GetOEMCP));
	InterlockedExchangePointer((PVOID*)&real_GetConsoleCP, Hook::HotPatch(Hook::GetProcAddress(kernel32, "GetConsoleCP"), "GetConsoleCP", Hook_GetConsoleCP));
	InterlockedExchangePointer((PVOID*)&real_GetConsoleOutputCP, Hook::HotPatch(Hook::GetProcAddress(kernel32, "GetConsoleOutputCP"), "GetConsoleOutputCP", Hook_GetConsoleOutputCP));
	InterlockedExchangePointer((PVOID*)&real_GetLocaleInfoA, Hook::HotPatch(Hook::GetProcAddress(kernel32, "GetLocaleInfoA"), "GetLocaleInfoA", Hook_GetLocaleInfoA));
	InterlockedExchangePointer((PVOID*)&real_GetLocaleInfoW, Hook::HotPatch(Hook::GetProcAddress(kernel32, "GetLocaleInfoW"), "GetLocaleInfoW", Hook_GetLocaleInfoW));

	return true;
}


// Public API - call this to force layout
void KeyboardLayout::ForceKeyboardLayout(DWORD layoutID)
{
	// Validate layout ID (basic validation)
	// Layouts are 16-bit LANGID values (low word), often combined with
	// a device ID. We require at least a valid LANGID.
	WORD langId = LOWORD(layoutID);
	if (langId == 0)
	{
		Logging::Log() << __FUNCTION__ << " Invalid layout id: " << std::uppercase << Logging::hex(layoutID) << std::nouppercase;
		return;
	}

	// Try GetKeyboardLayoutList to check if this layout is installed
	{
		HKL foundID = FindInstalledHKL(langId);
		if (!foundID)
		{
			WCHAR klidStr[16] = {}; swprintf_s(klidStr, L"%08X", layoutID);
			HKL test = LoadKeyboardLayoutW(klidStr, KLF_NOTELLSHELL | KLF_SUBSTITUTE_OK);

			if (!test)
			{
				Logging::Log() << __FUNCTION__ << " Keyboard layout not supported: " << std::hex << layoutID;
				return;
			}
		}
	}

	g_LayoutID = layoutID;

	// Convert LANGID to HKL form if only low word is provided
	// Many Windows APIs expect HKL = (DWORD)((device<<16) | langid)
	// If user provides only 0x0409, convert to 0x04090409
	g_ForcedHKL = reinterpret_cast<HKL>((HIWORD(layoutID) == 0) ? MAKELONG(langId, langId) : layoutID);

	// Convert langid into hex value
	std::stringstream ss;
	ss << std::uppercase << std::hex << layoutID << std::dec << std::nouppercase;

	// Store the layout globally for use by the detoured functions
	// layoutHex like: "409" or "0409" or "00000409"
	std::string s = ss.str();

	// normalize to 8 characters
	while (s.length() < 8)
	{
		s = "0" + s;
	}

	g_LayoutKLIDA.assign(s.begin(), s.end());
	g_LayoutKLIDW.assign(s.begin(), s.end());

	// Get correct ANSI code page
	{
		LCID lcid = MAKELCID(layoutID, SORT_DEFAULT);
		char buffer[10] = {};
		int ret = GetLocaleInfoA(lcid, LOCALE_IDEFAULTANSICODEPAGE, buffer, sizeof(buffer));
		if (ret > 0)
		{
			g_ACP = atoi(buffer); // correct ANSI code page
		}
	}

	// Get OEM code page from LCID
	{
		LCID lcid = MAKELCID(layoutID, SORT_DEFAULT);
		char buffer[10] = {};

		int ret = GetLocaleInfoA(lcid, LOCALE_IDEFAULTCODEPAGE, buffer, sizeof(buffer));
		if (ret > 0)
		{
			g_OEMCP = atoi(buffer); // correct OEM code page
		}
	}

	// Install your hotpatch hooks
	if (!InstallHooks())
	{
		Logging::Log() << __FUNCTION__ << " Failed to install keyboard layout hooks!";
		return;
	}
	g_Enabled = true;

	Logging::Log() << __FUNCTION__ << " Successfully forced keyboard layout: " << std::uppercase << Logging::hex(layoutID) << std::nouppercase;
}

void KeyboardLayout::SetForcedKeyboardLayout()
{
	if (g_Enabled)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Forcing keyboard layout!");

		HKL hkl = nullptr;
		if (real_ActivateKeyboardLayout)
		{
			hkl = real_ActivateKeyboardLayout(g_ForcedHKL, KLF_SETFORPROCESS);
		}
		else
		{
			hkl = ActivateKeyboardLayout(g_ForcedHKL, KLF_SETFORPROCESS);
		}

		if (!g_PreviousHKL && hkl)
		{
			g_PreviousHKL = hkl;
		}
	}
}

void KeyboardLayout::UnSetForcedKeyboardLayout()
{
	if (g_PreviousHKL)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Removing keyboard layout!");

		if (real_ActivateKeyboardLayout)
		{
			real_ActivateKeyboardLayout(g_PreviousHKL, KLF_SETFORPROCESS);
		}
		else
		{
			ActivateKeyboardLayout(g_PreviousHKL, KLF_SETFORPROCESS);
		}
	}
}

void KeyboardLayout::DisableForcedKeyboardLayout()
{
	if (g_Enabled)
	{
		g_Enabled = false;
		UnSetForcedKeyboardLayout();
	}
}
