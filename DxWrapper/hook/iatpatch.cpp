/**
* Created from source code found in DxWnd v 2.03.60
* https://sourceforge.net/projects/dxwnd/
*
* Updated 2017 by Elisha Riedlinger
*
*/

#include "dgame.h"

//#include <windows.h>
//#include "dxwnd.h"
//#include "dxwcore.hpp"

void *IATPatch(HMODULE module, DWORD ordinal, char *dll, void *apiproc, const char *apiname, void *hookproc)
{
	PIMAGE_NT_HEADERS pnth;
	PIMAGE_IMPORT_DESCRIPTOR pidesc;
	DWORD base, rva;
	PSTR impmodule;
	PIMAGE_THUNK_DATA ptaddr;
	PIMAGE_THUNK_DATA ptname;
	PIMAGE_IMPORT_BY_NAME piname;
	DWORD oldprotect;
	void *org;
	//OutTraceH("IATPatch: module=%x ordinal=%x name=%s dll=%s\n", module, ordinal, apiname, dll);

	base = (DWORD)module;
	org = 0; // by default, ret = 0 => API not found

	__try{
		pnth = PIMAGE_NT_HEADERS(PBYTE(base) + PIMAGE_DOS_HEADER(base)->e_lfanew);
		if(!pnth) {
			//OutTraceH("IATPatch: ERROR no PNTH at %d\n", __LINE__);
			return 0;
		}
		rva = pnth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
		if(!rva) {
			//OutTraceH("IATPatch: ERROR no RVA at %d\n", __LINE__);
			return 0;
		}
		pidesc = (PIMAGE_IMPORT_DESCRIPTOR)(base + rva);

		while(pidesc->FirstThunk){
			impmodule = (PSTR)(base + pidesc->Name);
			//OutTraceH("IATPatch: analyze impmodule=%s\n", impmodule);
			char *fname = impmodule;
			for(; *fname; fname++); for(; !*fname; fname++);

			if(!lstrcmpi(dll, impmodule)) {
				//OutTraceH("IATPatch: dll=%s found at %x\n", dll, impmodule);

				ptaddr = (PIMAGE_THUNK_DATA)(base + (DWORD)pidesc->FirstThunk);
				ptname = (pidesc->OriginalFirstThunk) ? (PIMAGE_THUNK_DATA)(base + (DWORD)pidesc->OriginalFirstThunk) : NULL;

				while(ptaddr->u1.Function){
					// OutTraceH("IATPatch: address=%x ptname=%x\n", ptaddr->u1.AddressOfData, ptname);
					
					if (ptname){
						// examining by function name
						if(!IMAGE_SNAP_BY_ORDINAL(ptname->u1.Ordinal)){
							piname = (PIMAGE_IMPORT_BY_NAME)(base + (DWORD)ptname->u1.AddressOfData);
							//OutTraceH("IATPatch: BYNAME ordinal=%x address=%x name=%s hint=%x\n", ptaddr->u1.Ordinal, ptaddr->u1.AddressOfData, (char *)piname->Name, piname->Hint);
							if(!lstrcmpi(apiname, (char *)piname->Name)) break;
						}
						else{
							// OutTraceH("IATPatch: BYORD target=%x ord=%x\n", ordinal, IMAGE_ORDINAL32(ptname->u1.Ordinal));
							if(ordinal && (IMAGE_ORDINAL32(ptname->u1.Ordinal) == ordinal)) { // skip unknow ordinal 0
							//OutTraceH("IATPatch: BYORD ordinal=%x addr=%x\n", ptname->u1.Ordinal, ptaddr->u1.Function);
							//OutTraceH("IATPatch: BYORD GetProcAddress=%x\n", GetProcAddress(GetModuleHandle(dll), MAKEINTRESOURCE(IMAGE_ORDINAL32(ptname->u1.Ordinal))));	
								break;
							}
						}

					}
					else {
						// OutTraceH("IATPatch: fname=%s\n", fname);
						if(!lstrcmpi(apiname, fname)) {
							//OutTraceH("IATPatch: BYSCAN ordinal=%x address=%x name=%s\n", ptaddr->u1.Ordinal, ptaddr->u1.AddressOfData, fname);
							break;
						}
						for(; *fname; fname++); for(; !*fname; fname++);
					}

					if (apiproc){
						// examining by function addr
						if(ptaddr->u1.Function == (DWORD)apiproc) break;
					}
					ptaddr ++;
					if (ptname) ptname ++;
				}

				if(ptaddr->u1.Function) {
					org = (void *)ptaddr->u1.Function;
					if(org == hookproc) return 0; // already hooked
						
					if(!VirtualProtect(&ptaddr->u1.Function, 4, PAGE_EXECUTE_READWRITE, &oldprotect)) {
						//OutTraceDW("IATPatch: VirtualProtect error %d at %d\n", GetLastError(), __LINE__);
						return 0;
					}
					ptaddr->u1.Function = (DWORD)hookproc;
					if(!VirtualProtect(&ptaddr->u1.Function, 4, oldprotect, &oldprotect)) {
						//OutTraceDW("IATPatch: VirtualProtect error %d at %d\n", GetLastError(), __LINE__);
						return 0;
					}
					if (!FlushInstructionCache(GetCurrentProcess(), &ptaddr->u1.Function, 4)) {
						//OutTraceDW("IATPatch: FlushInstructionCache error %d at %d\n", GetLastError(), __LINE__);
						return 0;
					}
					//OutTraceH("IATPatch hook=%s address=%x->%x\n", apiname, org, hookproc);

					return org;
				}
			}
			pidesc ++;
		}
		if(!pidesc->FirstThunk) {
			//OutTraceH("IATPatch: PE unreferenced function %s:%s\n", dll, apiname);
			return 0;
		}

	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{       
		//OutTraceH("IATPatch: EXCEPTION hook=%s:%s Hook Failed.\n", dll, apiname);
	}
	return org;
}

void *IATPatch(HMODULE module, char *dll, void *apiproc, const char *apiname, void *hookproc)
{
	return IATPatch(module, 0, dll, apiproc, apiname, hookproc);
}