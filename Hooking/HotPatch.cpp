/**
* Copyright (C) 2018 Elisha Riedlinger
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
*/

// return:
// 0 = patch failed
// 1 = already patched
// addr = address of the original function

#include <vector>
#include "Hook.h"
#include "Logging\Logging.h"

namespace Hook
{
	struct HOTPATCH
	{
		BYTE lpOrgBuffer[12];
		BYTE lpNewBuffer[12];
		void* procaddr;
	};

	std::vector<HOTPATCH> HotPatchProcs;
}

// Hook API using hot patch
void *Hook::HotPatch(void *apiproc, const char *apiname, void *hookproc, bool forcepatch)
{
	DWORD dwPrevProtect;
	BYTE *patch_address;
	void *orig_address;

#ifdef _DEBUG
	Logging::LogFormat("HotPatch: api=%s addr=%p hook=%p", apiname, apiproc, hookproc);
#endif

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
		memcmp("\xCC\xCC\xCC\xCC\xCC\x8B\xFF", patch_address, 7) && memcmp("\xCC\xCC\xCC\xCC\xCC\x89\xFF", patch_address, 7)) ||	// For debugging
		((forcepatch && (!memcmp("\x90\x90\x90\x90\x90", patch_address, 5) || !memcmp("\xCC\xCC\xCC\xCC\xCC", patch_address, 5)))))	// Force hook, overwrites data, patched function may not be usable
	{
		// Backup memory
		HOTPATCH tmpMemory;
		tmpMemory.procaddr = patch_address;
		ReadProcessMemory(GetCurrentProcess(), patch_address, tmpMemory.lpOrgBuffer, 12, nullptr);

		// Set HotPatch hook
		*patch_address = 0xE9; // jmp (4-byte relative)
		*((DWORD *)(patch_address + 1)) = (DWORD)hookproc - (DWORD)patch_address - 5; // relative address
		*((WORD *)apiproc) = 0xF9EB; // should be atomic write (jmp $-5)

		// Get memory after update
		ReadProcessMemory(GetCurrentProcess(), patch_address, tmpMemory.lpNewBuffer, 12, nullptr);

		// Save memory
		HotPatchProcs.push_back(tmpMemory);

		// restore protection
		VirtualProtect(patch_address, 12, dwPrevProtect, &dwPrevProtect);
#ifdef _DEBUG
		Logging::LogFormat("HotPatch: api=%s addr=%p->%p hook=%p", apiname, apiproc, orig_address, hookproc);
#endif
		return orig_address;
	}

	// Check if API is just a pointer to another API
	else if (!(memcmp("\x90\x90\x90\x90\x90\xFF\x25", patch_address, 7) && memcmp("\xCC\xCC\xCC\xCC\xCC\xFF\x25", patch_address, 7)))
	{
		// Get memory address to function
		DWORD *patchAddr;
		memcpy(&patchAddr, ((BYTE*)apiproc + 2), sizeof(DWORD));

		// restore protection
		VirtualProtect(patch_address, 12, dwPrevProtect, &dwPrevProtect);
#ifdef _DEBUG
		logf("HotPatch: api=%s addr=%p->%p hook=%p", apiname, apiproc, orig_address, hookproc);
#endif

		return HotPatch((void*)(*patchAddr), apiname, hookproc);
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

			// Log memory
			BYTE lpBuffer[12];
			if (ReadProcessMemory(GetCurrentProcess(), patch_address, lpBuffer, 12, nullptr))
			{
				char buffer[120] = { '\0' };
				sprintf_s(buffer, "Bytes in memory are: \\x%02X\\x%02X\\x%02X\\x%02X\\x%02X\\x%02X\\x%02X\\x%02X\\x%02X\\x%02X\\x%02X\\x%02X",
					lpBuffer[0], lpBuffer[1], lpBuffer[2], lpBuffer[3],
					lpBuffer[4], lpBuffer[5], lpBuffer[6], lpBuffer[7],
					lpBuffer[8], lpBuffer[9], lpBuffer[10], lpBuffer[11]);
				Logging::LogFormat(buffer);
			}

			return 0; // not hot patch "aware"
		}
	}
}

// Restore all addresses hooked
bool Hook::UnHotPatchAll()
{
	bool flag = true;
	BYTE lpBuffer[12];
	while (HotPatchProcs.size() != 0)
	{
		// VirtualProtect first to make sure patch_address is readable
		DWORD dwPrevProtect;
		if (VirtualProtect(HotPatchProcs.back().procaddr, 12, PAGE_EXECUTE_WRITECOPY, &dwPrevProtect))
		{
			// Read memory
			if (ReadProcessMemory(GetCurrentProcess(), HotPatchProcs.back().procaddr, lpBuffer, 12, nullptr))
			{
				// Check if memory is as expected
				if (!memcmp(lpBuffer, HotPatchProcs.back().lpNewBuffer, 12))
				{
					// Write to memory
					memcpy(HotPatchProcs.back().procaddr, HotPatchProcs.back().lpOrgBuffer, 12);
				}
				else
				{
					// Memory different than expected
					flag = false;
					Logging::LogFormat("UnHotPatchAll: Memory different than expected procaddr: %p", HotPatchProcs.back().procaddr);
				}
			}
			else
			{
				// Failed to read memory
				flag = false;
				Logging::LogFormat("UnHotPatchAll: Failed to read memory procaddr: %p", HotPatchProcs.back().procaddr);
			}

			// Restore protection
			VirtualProtect(HotPatchProcs.back().procaddr, 12, dwPrevProtect, &dwPrevProtect);
		}
		else
		{
			// Access denied
			flag = false;
			Logging::LogFormat("UnHotPatchAll: access denied. procaddr: %p", HotPatchProcs.back().procaddr);
		}
		HotPatchProcs.pop_back();
	}
	HotPatchProcs.clear();
	return flag;
}

// Unhook hot patched API
bool Hook::UnhookHotPatch(void *apiproc, const char *apiname, void *hookproc)
{
	DWORD dwPrevProtect;
	BYTE *patch_address;
	void *orig_address;

#ifdef _DEBUG
	Logging::LogFormat("UnhookHotPatch: api=%s addr=%p hook=%p", apiname, apiproc, hookproc);
#endif

	patch_address = ((BYTE *)apiproc) - 5;
	orig_address = (BYTE *)apiproc + 2;

	// Check if this address is stored in the vector and restore memory
	BYTE lpBuffer[12];
	for (UINT x = 0; x < HotPatchProcs.size(); ++x)
	{
		// Check for address
		if (HotPatchProcs[x].procaddr == patch_address)
		{
			// VirtualProtect first to make sure patch_address is readable
			if (VirtualProtect(HotPatchProcs[x].procaddr, 12, PAGE_EXECUTE_WRITECOPY, &dwPrevProtect))
			{
				// Read memory
				if (ReadProcessMemory(GetCurrentProcess(), HotPatchProcs[x].procaddr, lpBuffer, 12, nullptr))
				{
					// Check if memory is as expected
					if (!memcmp(lpBuffer, HotPatchProcs[x].lpNewBuffer, 12))
					{
						// Write to memory
						memcpy(HotPatchProcs[x].procaddr, HotPatchProcs[x].lpOrgBuffer, 12);

						// If not at the end then move back to current loc and pop_back
						if (x + 1 != HotPatchProcs.size())
						{
							HotPatchProcs[x].procaddr = HotPatchProcs.back().procaddr;
							memcpy(HotPatchProcs[x].lpOrgBuffer, HotPatchProcs.back().lpOrgBuffer, 12);
							memcpy(HotPatchProcs[x].lpNewBuffer, HotPatchProcs.back().lpNewBuffer, 12);
						}
						HotPatchProcs.pop_back();

						// Restore protection
						VirtualProtect(patch_address, 12, dwPrevProtect, &dwPrevProtect);

						// Return
						return true;
					}
				}

				// Restore protection
				VirtualProtect(patch_address, 12, dwPrevProtect, &dwPrevProtect);
			}
		}
	}

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
		Logging::LogFormat("UnhookHotPatch: api=%s addr=%p->%p hook=%p", apiname, apiproc, orig_address, hookproc);
#endif
		return true;
	}

	Logging::LogFormat("HotPatch: failed to unhook '%s' at addr=%p", apiname, apiproc);

	VirtualProtect(patch_address, 12, dwPrevProtect, &dwPrevProtect); // restore protection
#ifdef _DEBUG
	Logging::LogFormat("UnhookHotPatch: api=%s addr=%p->%p hook=%p", apiname, apiproc, orig_address, hookproc);
#endif
	return false;
}
