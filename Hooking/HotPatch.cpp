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
* Created from source code found in DxWnd v2.03.99
* https://sourceforge.net/projects/dxwnd/
*
* Updated 2017 by Elisha Riedlinger
*/

// return:
// 0 = patch failed
// 1 = already patched
// addr = address of the original function

#include "Hook.h"
#include "Logging\Logging.h"

// Hook API using hot patch
void *Hook::HotPatch(void *apiproc, const char *apiname, void *hookproc)
{
	DWORD dwPrevProtect;
	BYTE *patch_address;
	void *orig_address;

#ifdef _DEBUG
	sprintf_s(buffer, "HotPatch: api=%s addr=%p hook=%p", apiname, apiproc, hookproc);
	Logging::LogText(buffer);
#endif

	if (!strcmp(apiname, "GetProcAddress"))
	{
		return 0; // do not mess with this one!
	}

	patch_address = ((BYTE *)apiproc) - 5;
	orig_address = (BYTE *)apiproc + 2;

	// Entry point could be at the top of a page? so VirtualProtect first to make sure patch_address is readable
	if (!VirtualProtect(patch_address, 12, PAGE_EXECUTE_WRITECOPY, &dwPrevProtect))
	{
		Logging::LogFormat("HotPatch: access denied.  Cannot hook api=%s at addr=%p err=%x", apiname, apiproc, GetLastError());
		return 0; // access denied
	}

	// Check if API can be patched
	if (!(memcmp("\x90\x90\x90\x90\x90\xEB\x05\x90\x90\x90\x90\x90", patch_address, 12) &&											// Some calls (QueryPerformanceCounter) are sort of hot patched already....
		memcmp("\xCC\xCC\xCC\xCC\xCC\xEB\x05\xCC\xCC\xCC\xCC\xCC", patch_address, 12) &&											// For debugging
		memcmp("\x90\x90\x90\x90\x90\x8B\xFF", patch_address, 7) && memcmp("\x90\x90\x90\x90\x90\x89\xFF", patch_address, 7) &&		// Make sure it is a hotpatchable image... check for 5 nops followed by mov edi,edi
		memcmp("\xCC\xCC\xCC\xCC\xCC\x8B\xFF", patch_address, 7) && memcmp("\xCC\xCC\xCC\xCC\xCC\x89\xFF", patch_address, 7)))		// For debugging
	{
		*patch_address = 0xE9; // jmp (4-byte relative)
		*((DWORD *)(patch_address + 1)) = (DWORD)hookproc - (DWORD)patch_address - 5; // relative address
		*((WORD *)apiproc) = 0xF9EB; // should be atomic write (jmp $-5)

		VirtualProtect(patch_address, 12, dwPrevProtect, &dwPrevProtect); // restore protection
#ifdef _DEBUG
		sprintf_s(buffer, "HotPatch: api=%s addr=%p->%p hook=%p", apiname, apiproc, orig_address, hookproc);
		Logging::LogText(buffer);
#endif
		return orig_address;
	}

	// API cannot be patched
	else
	{
		VirtualProtect(patch_address, 12, dwPrevProtect, &dwPrevProtect); // restore protection
		
		// check it wasn't patched already
		if ((*patch_address == 0xE9) && (*(WORD *)apiproc == 0xF9EB))
		{
			// should never go through here ...
			Logging::LogFormat("HotPatch: '%s' patched already at addr=%p", apiname, apiproc);
			return (void *)1;
		}
		else
		{
			Logging::LogFormat("HotPatch: '%s' is not patch aware at addr=%p", apiname, apiproc);
			return 0; // not hot patch "aware"
		}
	}
}

// Unhook hot patched API
bool Hook::UnhookHotPatch(void *apiproc, const char *apiname, void *hookproc)
{
	DWORD dwPrevProtect;
	BYTE *patch_address;
	void *orig_address;

#ifdef _DEBUG
	sprintf_s(buffer, "UnhookHotPatch: api=%s addr=%p hook=%p", apiname, apiproc, hookproc);
	Logging::LogText(buffer);
#endif

	if (!strcmp(apiname, "GetProcAddress"))
	{
		return false; // do not mess with this one!
	}

	patch_address = ((BYTE *)apiproc) - 5;
	orig_address = (BYTE *)apiproc + 2;

	// Entry point could be at the top of a page? so VirtualProtect first to make sure patch_address is readable
	if (!VirtualProtect(patch_address, 12, PAGE_EXECUTE_WRITECOPY, &dwPrevProtect))
	{
		Logging::LogFormat("UnhookHotPatch: access denied.  Cannot hook api=%s at addr=%p err=%x", apiname, apiproc, GetLastError());
		return false; // access denied
	}

	// Check if API is hot patched
	if ((*patch_address == 0xE9) && (*(WORD *)apiproc == 0xF9EB) &&
		*((DWORD *)(patch_address + 1)) == (DWORD)hookproc - (DWORD)patch_address - 5)
	{
		*patch_address = 0x90; // nop
		*((DWORD *)(patch_address + 1)) = 0x90909090; // 4 nops
		*((WORD *)(patch_address + 5)) = 0x9090; // 2 nops

		VirtualProtect(patch_address, 12, dwPrevProtect, &dwPrevProtect); // restore protection
#ifdef _DEBUG
		sprintf_s(buffer, "UnhookHotPatch: api=%s addr=%p->%p hook=%p", apiname, apiproc, orig_address, hookproc);
		Logging::LogText(buffer);
#endif
		return true;
	}

	VirtualProtect(patch_address, 12, dwPrevProtect, &dwPrevProtect); // restore protection
#ifdef _DEBUG
	sprintf_s(buffer, "UnhookHotPatch: api=%s addr=%p->%p hook=%p", apiname, apiproc, orig_address, hookproc);
	Logging::LogText(buffer);
#endif
	return false;
}
