#define WIN32_LEAN_AND_MEAN

#include <Windows.h>

#include "Common/Hook.h"
#include "Win32/MsgHooks.h"

namespace
{
	HHOOK WINAPI setWindowsHookExA(int idHook, HOOKPROC lpfn, HINSTANCE hMod, DWORD dwThreadId)
	{
		if (WH_KEYBOARD_LL == idHook && hMod && GetModuleHandle("AcGenral") == hMod)
		{
            // This effectively disables the IgnoreAltTab shim
			return nullptr;
		}
		return CALL_ORIG_FUNC(SetWindowsHookExA)(idHook, lpfn, hMod, dwThreadId);
	}
}

namespace Win32
{
	namespace MsgHooks
	{
		void installHooks()
		{
			HOOK_FUNCTION(user32, SetWindowsHookExA, setWindowsHookExA);
		}
	}
}
