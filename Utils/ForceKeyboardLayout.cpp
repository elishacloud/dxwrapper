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
#include "Settings\Settings.h"
#include "Logging\Logging.h"

namespace {
	bool g_Enabled = false;
	HKL g_ForcedHKL = nullptr;
	HKL g_PreviousHKL = nullptr;
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

	// Convert LANGID to HKL form if only low word is provided
	// Many Windows APIs expect HKL = (DWORD)((device<<16) | langid)
	// If user provides only 0x0409, convert to 0x04090409
	g_ForcedHKL = reinterpret_cast<HKL>((HIWORD(layoutID) == 0) ? MAKELONG(langId, langId) : layoutID);

	g_Enabled = true;

	Logging::Log() << __FUNCTION__ << " Successfully forced keyboard layout: " << std::uppercase << Logging::hex(layoutID) << std::nouppercase;
}

void KeyboardLayout::SetForcedKeyboardLayout()
{
	if (Config.Exiting)
	{
		DisableForcedKeyboardLayout();
	}
	else if (g_Enabled)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Forcing keyboard layout!");

		HKL hkl = ActivateKeyboardLayout(g_ForcedHKL, KLF_SETFORPROCESS);

		if (!g_PreviousHKL && hkl)
		{
			g_PreviousHKL = hkl;
		}
	}
}

static void UnSetLayout()
{
	if (g_PreviousHKL)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Removing keyboard layout!");

		ActivateKeyboardLayout(g_PreviousHKL, KLF_SETFORPROCESS);
	}
}

void KeyboardLayout::UnSetForcedKeyboardLayout()
{
	if (Config.Exiting)
	{
		DisableForcedKeyboardLayout();
	}
	else
	{
		UnSetLayout();
	}
}

void KeyboardLayout::DisableForcedKeyboardLayout()
{
	if (g_Enabled)
	{
		g_Enabled = false;

		HWND hwndForeground = GetForegroundWindow();
		DWORD fgThread = GetWindowThreadProcessId(hwndForeground, NULL);
		DWORD curThread = GetCurrentThreadId();

		AttachThreadInput(curThread, fgThread, TRUE);

		UnSetLayout();

		AttachThreadInput(curThread, fgThread, FALSE);
	}
}
