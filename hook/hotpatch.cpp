/**
* Created from source code found in DxWnd v2.03.99
* https://sourceforge.net/projects/dxwnd/
*
* Updated 2017 by Elisha Riedlinger
*/

// return:
// 0 = patch failed
// 1 = already patched
// addr = address of the original function

#include "Settings\Settings.h"

void *HotPatch(void *apiproc, const char *apiname, void *hookproc)
{
	static constexpr DWORD BuffSize = 250;
	char buffer[BuffSize];

	DWORD dwPrevProtect;
	BYTE* patch_address;
	void *orig_address;

#ifdef _DEBUG
	sprintf_s(buffer, BuffSize, "HotPatch: api=%s addr=%p hook=%p", apiname, apiproc, hookproc);
	LogText(buffer);
#endif

	if(!strcmp(apiname, "GetProcAddress")) return 0; // do not mess with this one!

	patch_address = ((BYTE *)apiproc) - 5;
	orig_address = (BYTE *)apiproc + 2;

	// entry point could be at the top of a page? so VirtualProtect first to make sure patch_address is readable
	//if(!VirtualProtect(patch_address, 7, PAGE_EXECUTE_READWRITE, &dwPrevProtect)){
	if(!VirtualProtect(patch_address, 12, PAGE_EXECUTE_WRITECOPY, &dwPrevProtect)){
		sprintf_s(buffer, BuffSize, "HotPatch: access denied.  Cannot hook api=%s at addr=%p err=%x", apiname, apiproc, GetLastError());
		LogText(buffer);
		return (void *)0; // access denied
	}

	// some calls (QueryPerformanceCounter) are sort of hot patched already....
	if (!(memcmp("\x90\x90\x90\x90\x90\xEB\x05\x90\x90\x90\x90\x90", patch_address, 12) &&
		memcmp("\xCC\xCC\xCC\xCC\xCC\xEB\x05\xCC\xCC\xCC\xCC\xCC", patch_address, 12)))
	{
		*patch_address = 0xE9; // jmp (4-byte relative)
		*((DWORD *)(patch_address + 1)) = (DWORD)hookproc - (DWORD)patch_address - 5; // relative address
		*((WORD *)apiproc) = 0xF9EB; // should be atomic write (jmp $-5)
		
		VirtualProtect( patch_address, 12, dwPrevProtect, &dwPrevProtect ); // restore protection
#ifdef _DEBUG
		sprintf_s(buffer, BuffSize, "HotPatch: api=%s addr=%p->%p hook=%p", apiname, apiproc, orig_address, hookproc);
		LogText(buffer);
#endif
		return orig_address;
	}

	// make sure it is a hotpatchable image... check for 5 nops followed by mov edi,edi
	if (memcmp("\x90\x90\x90\x90\x90\x8B\xFF", patch_address, 7) && memcmp("\x90\x90\x90\x90\x90\x89\xFF", patch_address, 7) &&
		memcmp("\xCC\xCC\xCC\xCC\xCC\x8B\xFF", patch_address, 7) && memcmp("\xCC\xCC\xCC\xCC\xCC\x89\xFF", patch_address, 7))
	{
		VirtualProtect( patch_address, 12, dwPrevProtect, &dwPrevProtect ); // restore protection
		// check it wasn't patched already
		if((*patch_address==0xE9) && (*(WORD *)apiproc == 0xF9EB)){
			// should never go through here ...
			sprintf_s(buffer, BuffSize, "HotPatch: '%s' patched already at addr=%p", apiname, apiproc);
			LogText(buffer);
			return (void *)1;
		}
		else{
			sprintf_s(buffer, BuffSize, "HotPatch: '%s' is not patch aware at addr=%p", apiname, apiproc);
			LogText(buffer);
			return (void *)0; // not hot patch "aware"
		}
	}
	
	*patch_address = 0xE9; // jmp (4-byte relative)
	*((DWORD *)(patch_address + 1)) = (DWORD)hookproc - (DWORD)patch_address - 5; // relative address
	*((WORD *)apiproc) = 0xF9EB; // should be atomic write (jmp $-5)
	
	VirtualProtect( patch_address, 12, dwPrevProtect, &dwPrevProtect ); // restore protection
#ifdef _DEBUG
	sprintf_s(buffer, BuffSize, "HotPatch: api=%s addr=%p->%p hook=%p", apiname, apiproc, orig_address, hookproc);
	LogText(buffer);
#endif
	return orig_address;
}