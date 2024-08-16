#define WIN32_LEAN_AND_MEAN
#define CINTERFACE

#include <string>
#include <vector>

#include <DDrawCompat/v0.3.1/Common/CompatPtr.h>
#include <DDrawCompat/v0.3.1/Common/Hook.h>
#include <DDrawCompat/v0.3.1/DDraw/DirectDraw.h>
#include <DDrawCompat/v0.3.1/DDraw/ScopedThreadLock.h>
#include <DDrawCompat/v0.3.1/Gdi/Gdi.h>
#include <DDrawCompat/v0.3.1/Gdi/VirtualScreen.h>
#include <DDrawCompat/v0.3.1/Win32/DisplayMode.h>

BOOL WINAPI DWM8And16Bit_IsShimApplied_CallOut() { return FALSE; }
BOOL WINAPI SE_COM_HookInterface(CLSID*, GUID*, DWORD, DWORD) { return 0; }

namespace
{
	template <typename Char>
	struct EnumParams
	{
		std::basic_string<Char> deviceName;
		DWORD flags;

		bool operator==(const EnumParams& other) const
		{
			return deviceName == other.deviceName && flags == other.flags;
		}

		bool operator!=(const EnumParams& other) const
		{
			return !(*this == other);
		}
	};

	DWORD g_origBpp = 0;
	DWORD g_currentBpp = 0;
	DWORD g_lastBpp = 0;

	BOOL WINAPI dwm8And16BitIsShimAppliedCallOut();
	BOOL WINAPI seComHookInterface(CLSID* clsid, GUID* iid, DWORD unk1, DWORD unk2);

	template <typename CStr, typename DevMode, typename ChangeDisplaySettingsExFunc, typename EnumDisplaySettingsExFunc>
	LONG changeDisplaySettingsEx(
		ChangeDisplaySettingsExFunc origChangeDisplaySettingsEx,
		EnumDisplaySettingsExFunc origEnumDisplaySettingsEx,
		CStr lpszDeviceName, DevMode* lpDevMode, HWND hwnd, DWORD dwflags, LPVOID lParam)
	{
		//********** Begin Edit *************
		if (DDrawCompat::IsEnabled())
		{
			DDraw::ScopedThreadLock lock;
		}
		//********** End Edit ***************

		DevMode prevDevMode = {};
		if (!(dwflags & CDS_TEST))
		{
			prevDevMode.dmSize = sizeof(prevDevMode);
			origEnumDisplaySettingsEx(lpszDeviceName, ENUM_CURRENT_SETTINGS, &prevDevMode, 0);
		}

		BOOL result = FALSE;
		if (lpDevMode)
		{
			DWORD origBpp = lpDevMode->dmBitsPerPel;
			lpDevMode->dmBitsPerPel = 32;
			result = origChangeDisplaySettingsEx(lpszDeviceName, lpDevMode, hwnd, dwflags, lParam);
			lpDevMode->dmBitsPerPel = origBpp;
		}
		else
		{
			result = origChangeDisplaySettingsEx(lpszDeviceName, lpDevMode, hwnd, dwflags, lParam);
		}

		if (SUCCEEDED(result) && !(dwflags & CDS_TEST))
		{
			if (lpDevMode)
			{
				g_currentBpp = lpDevMode->dmBitsPerPel;
				g_lastBpp = lpDevMode->dmBitsPerPel;
			}
			else
			{
				g_currentBpp = g_origBpp;
			}

			DevMode currDevMode = {};
			currDevMode.dmSize = sizeof(currDevMode);
			origEnumDisplaySettingsEx(lpszDeviceName, ENUM_CURRENT_SETTINGS, &currDevMode, 0);

			if (currDevMode.dmPelsWidth == prevDevMode.dmPelsWidth &&
				currDevMode.dmPelsHeight == prevDevMode.dmPelsHeight &&
				currDevMode.dmBitsPerPel == prevDevMode.dmBitsPerPel &&
				currDevMode.dmDisplayFrequency == prevDevMode.dmDisplayFrequency &&
				currDevMode.dmDisplayFlags == prevDevMode.dmDisplayFlags)
			{
				HANDLE dwmDxFullScreenTransitionEvent = OpenEventW(
					EVENT_MODIFY_STATE, FALSE, L"DWM_DX_FULLSCREEN_TRANSITION_EVENT");
				SetEvent(dwmDxFullScreenTransitionEvent);
				CloseHandle(dwmDxFullScreenTransitionEvent);
			}

			//********** Begin Edit *************
			if (DDrawCompat::IsEnabled())
			{
				Gdi::VirtualScreen::update();
			}
			//********** End Edit ***************
		}

		return result;
	}

	LONG WINAPI changeDisplaySettingsExA(
		LPCSTR lpszDeviceName, DEVMODEA* lpDevMode, HWND hwnd, DWORD dwflags, LPVOID lParam)
	{
		LOG_FUNC("ChangeDisplaySettingsExA", lpszDeviceName, lpDevMode, hwnd, dwflags, lParam);
		return LOG_RESULT(changeDisplaySettingsEx(
			CALL_ORIG_FUNC(ChangeDisplaySettingsExA),
			CALL_ORIG_FUNC(EnumDisplaySettingsExA),
			lpszDeviceName, lpDevMode, hwnd, dwflags, lParam));
	}

	LONG WINAPI changeDisplaySettingsExW(
		LPCWSTR lpszDeviceName, DEVMODEW* lpDevMode, HWND hwnd, DWORD dwflags, LPVOID lParam)
	{
		LOG_FUNC("ChangeDisplaySettingsExW", lpszDeviceName, lpDevMode, hwnd, dwflags, lParam);
		return LOG_RESULT(changeDisplaySettingsEx(
			CALL_ORIG_FUNC(ChangeDisplaySettingsExW),
			CALL_ORIG_FUNC(EnumDisplaySettingsExW),
			lpszDeviceName, lpDevMode, hwnd, dwflags, lParam));
	}

	void disableDwm8And16BitMitigation()
	{
		auto user32 = GetModuleHandle("user32");
		Compat32::removeShim(user32, "ChangeDisplaySettingsA");
		Compat32::removeShim(user32, "ChangeDisplaySettingsW");
		Compat32::removeShim(user32, "ChangeDisplaySettingsExA");
		Compat32::removeShim(user32, "ChangeDisplaySettingsExW");
		Compat32::removeShim(user32, "EnumDisplaySettingsA");
		Compat32::removeShim(user32, "EnumDisplaySettingsW");
		Compat32::removeShim(user32, "EnumDisplaySettingsExA");
		Compat32::removeShim(user32, "EnumDisplaySettingsExW");

		HOOK_FUNCTION(apphelp, DWM8And16Bit_IsShimApplied_CallOut, dwm8And16BitIsShimAppliedCallOut);
		HOOK_FUNCTION(apphelp, SE_COM_HookInterface, seComHookInterface);
	}

	BOOL WINAPI dwm8And16BitIsShimAppliedCallOut()
	{
		LOG_FUNC("DWM8And16Bit_IsShimApplied_CallOut");
		return LOG_RESULT(FALSE);
	}

	template <typename Char, typename DevMode, typename EnumDisplaySettingsExFunc>
	BOOL enumDisplaySettingsEx(EnumDisplaySettingsExFunc origEnumDisplaySettingsEx,
		const Char* lpszDeviceName, DWORD iModeNum, DevMode* lpDevMode, DWORD dwFlags)
	{
		if (ENUM_REGISTRY_SETTINGS == iModeNum || !lpDevMode || !lpDevMode->dmSize)
		{
			BOOL result = origEnumDisplaySettingsEx(lpszDeviceName, iModeNum, lpDevMode, dwFlags);
			return result;
		}

		if (ENUM_CURRENT_SETTINGS == iModeNum)
		{
			BOOL result = origEnumDisplaySettingsEx(lpszDeviceName, iModeNum, lpDevMode, dwFlags);
			if (result)
			{
				lpDevMode->dmBitsPerPel = g_currentBpp;
			}
			return result;
		}

		thread_local std::vector<DevMode> devModes;
		thread_local EnumParams<Char> lastEnumParams = {};

		EnumParams<Char> currentEnumParams = {
			lpszDeviceName ? lpszDeviceName : std::basic_string<Char>(), dwFlags };

		if (0 == iModeNum || devModes.empty() || currentEnumParams != lastEnumParams)
		{
			devModes.clear();
			lastEnumParams = currentEnumParams;

			DWORD modeNum = 0;
			DevMode dm = {};
			dm.dmSize = min(lpDevMode->dmSize, sizeof(dm));
			while (origEnumDisplaySettingsEx(lpszDeviceName, modeNum, &dm, dwFlags))
			{
				if (32 == dm.dmBitsPerPel)
				{
					dm.dmBitsPerPel = 8;
					devModes.push_back(dm);
					dm.dmBitsPerPel = 16;
					devModes.push_back(dm);
					dm.dmBitsPerPel = 32;
					devModes.push_back(dm);
				}
				++modeNum;
			}
		}

		if (iModeNum >= devModes.size())
		{
			return FALSE;
		}

		memcpy_s(lpDevMode, lpDevMode->dmSize, &devModes[iModeNum], min(lpDevMode->dmSize, sizeof(DevMode)));
		return TRUE;
	}

	BOOL WINAPI enumDisplaySettingsExA(
		LPCSTR lpszDeviceName, DWORD iModeNum, DEVMODEA* lpDevMode, DWORD dwFlags)
	{
		LOG_FUNC("EnumDisplaySettingsExA", lpszDeviceName, iModeNum, lpDevMode, dwFlags);
		return LOG_RESULT(enumDisplaySettingsEx(CALL_ORIG_FUNC(EnumDisplaySettingsExA),
			lpszDeviceName, iModeNum, lpDevMode, dwFlags));
	}

	BOOL WINAPI enumDisplaySettingsExW(
		LPCWSTR lpszDeviceName, DWORD iModeNum, DEVMODEW* lpDevMode, DWORD dwFlags)
	{
		LOG_FUNC("EnumDisplaySettingsExW", lpszDeviceName, iModeNum, lpDevMode, dwFlags);
		return LOG_RESULT(enumDisplaySettingsEx(CALL_ORIG_FUNC(EnumDisplaySettingsExW),
			lpszDeviceName, iModeNum, lpDevMode, dwFlags));
	}

	int WINAPI getDeviceCaps(HDC hdc, int nIndex)
	{
		LOG_FUNC("GetDeviceCaps", hdc, nIndex);
		switch (nIndex)
		{
		case BITSPIXEL:
			if (Gdi::isDisplayDc(hdc))
			{
				return LOG_RESULT(g_currentBpp);
			}
			break;

		case COLORRES:
			if (8 == g_currentBpp && Gdi::isDisplayDc(hdc))
			{
				return 24;
			}
			break;

		case NUMCOLORS:
		case NUMRESERVED:
			if (8 == g_currentBpp && Gdi::isDisplayDc(hdc))
			{
				return 20;
			}
			break;

		case RASTERCAPS:
			if (8 == g_currentBpp && Gdi::isDisplayDc(hdc))
			{
				return LOG_RESULT(CALL_ORIG_FUNC(GetDeviceCaps)(hdc, nIndex) | RC_PALETTE);
			}
			break;

		case SIZEPALETTE:
			if (8 == g_currentBpp && Gdi::isDisplayDc(hdc))
			{
				return 256;
			}
			break;
		}
		return LOG_RESULT(CALL_ORIG_FUNC(GetDeviceCaps)(hdc, nIndex));
	}

	BOOL WINAPI seComHookInterface(CLSID* clsid, GUID* iid, DWORD unk1, DWORD unk2)
	{
		LOG_FUNC("SE_COM_HookInterface", clsid, iid, unk1, unk2);
		if (clsid && (CLSID_DirectDraw == *clsid || CLSID_DirectDraw7 == *clsid))
		{
			return LOG_RESULT(0);
		}
		return LOG_RESULT(CALL_ORIG_FUNC(SE_COM_HookInterface)(clsid, iid, unk1, unk2));
	}
}

namespace Win32
{
	namespace DisplayMode
	{
		DWORD getBpp()
		{
			return g_currentBpp;
		}

		ULONG queryDisplaySettingsUniqueness()
		{
			static auto ddQueryDisplaySettingsUniqueness = reinterpret_cast<ULONG(APIENTRY*)()>(
				GetProcAddress(GetModuleHandle("gdi32"), "GdiEntry13"));
			return ddQueryDisplaySettingsUniqueness();
		}

		void installHooks()
		{
			DEVMODEA devMode = {};
			devMode.dmSize = sizeof(devMode);
			EnumDisplaySettingsEx(nullptr, ENUM_CURRENT_SETTINGS, &devMode, 0);
			g_origBpp = devMode.dmBitsPerPel;
			g_currentBpp = g_origBpp;
			g_lastBpp = g_origBpp;

			if (32 != devMode.dmBitsPerPel)
			{
				devMode.dmBitsPerPel = 32;
				ChangeDisplaySettings(&devMode, 0);
			}

			HOOK_FUNCTION(user32, ChangeDisplaySettingsExA, changeDisplaySettingsExA);
			HOOK_FUNCTION(user32, ChangeDisplaySettingsExW, changeDisplaySettingsExW);
			HOOK_FUNCTION(user32, EnumDisplaySettingsExA, enumDisplaySettingsExA);
			HOOK_FUNCTION(user32, EnumDisplaySettingsExW, enumDisplaySettingsExW);

			//********** Begin Edit *************
			if (DDrawCompat::IsEnabled())
			{
				HOOK_FUNCTION(gdi32, GetDeviceCaps, getDeviceCaps);
			}
			//********** End Edit ***************

			disableDwm8And16BitMitigation();
		}
	}
}
