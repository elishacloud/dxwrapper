/**
* Created from source code found in DxWnd v2.03.99
* https://sourceforge.net/projects/dxwnd/
*
* Updated 2017 by Elisha Riedlinger
*/

#include "cfg.h"

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

#ifdef _DEBUG
	sprintf_s(buffer, BuffSize, "IATPatch: module=%x ordinal=%x name=%s dll=%s", module, ordinal, apiname, dll);
	LogText(buffer);
#endif

	base = (DWORD)module;
	org = 0; // by default, ret = 0 => API not found

	const DWORD BuffSize = 250;
	char buffer[BuffSize];

	__try {
		pnth = PIMAGE_NT_HEADERS(PBYTE(base) + PIMAGE_DOS_HEADER(base)->e_lfanew);
		if (!pnth) {
			sprintf_s(buffer, BuffSize, "IATPatch: ERROR no PNTH at %d", __LINE__);
			LogText(buffer);
			return 0;
		}
		rva = pnth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
		if (!rva) {
			sprintf_s(buffer, BuffSize, "IATPatch: ERROR no RVA at %d", __LINE__);
			LogText(buffer);
			return 0;
		}
		pidesc = (PIMAGE_IMPORT_DESCRIPTOR)(base + rva);

		while (pidesc->FirstThunk) {
			impmodule = (PSTR)(base + pidesc->Name);
#ifdef _DEBUG
			//sprintf_s(buffer, BuffSize, "IATPatch: analyze impmodule=%s", impmodule);
			//LogText(buffer);
#endif
			char *fname = impmodule;
			for (; *fname; fname++); for (; !*fname; fname++);

			if (!lstrcmpi(dll, impmodule)) {
#ifdef _DEBUG
				sprintf_s(buffer, BuffSize, "IATPatch: dll=%s found at %x", dll, impmodule);
				LogText(buffer);
#endif

				ptaddr = (PIMAGE_THUNK_DATA)(base + (DWORD)pidesc->FirstThunk);
				ptname = (pidesc->OriginalFirstThunk) ? (PIMAGE_THUNK_DATA)(base + (DWORD)pidesc->OriginalFirstThunk) : NULL;

				while (ptaddr->u1.Function) {
#ifdef _DEBUG
					//sprintf_s(buffer, BuffSize, "IATPatch: address=%x ptname=%x", ptaddr->u1.AddressOfData, ptname);
					//LogText(buffer);
#endif

					if (ptname) {
						// examining by function name
						if (!IMAGE_SNAP_BY_ORDINAL(ptname->u1.Ordinal)) {
							piname = (PIMAGE_IMPORT_BY_NAME)(base + (DWORD)ptname->u1.AddressOfData);
#ifdef _DEBUG
							sprintf_s(buffer, BuffSize, "IATPatch: BYNAME ordinal=%x address=%x name=%s hint=%x", ptaddr->u1.Ordinal, ptaddr->u1.AddressOfData, (char *)piname->Name, piname->Hint);
							LogText(buffer);
#endif
							if (!lstrcmpi(apiname, (char *)piname->Name)) break;
						}
						else {
#ifdef _DEBUG
							//sprintf_s(buffer, BuffSize, "IATPatch: BYORD target=%x ord=%x", ordinal, IMAGE_ORDINAL32(ptname->u1.Ordinal));
							//LogText(buffer);
#endif
							if (ordinal && (IMAGE_ORDINAL32(ptname->u1.Ordinal) == ordinal)) { // skip unknow ordinal 0
#ifdef _DEBUG
								sprintf_s(buffer, BuffSize, "IATPatch: BYORD ordinal=%x addr=%x", ptname->u1.Ordinal, ptaddr->u1.Function);
								//sprintf_s(buffer, BuffSize, "IATPatch: BYORD GetProcAddress=%x", GetProcAddress(GetModuleHandle(dll), MAKEINTRESOURCE(IMAGE_ORDINAL32(ptname->u1.Ordinal))));	
								LogText(buffer);
#endif
								break;
							}
						}

					}
					else {
#ifdef _DEBUG
						//sprintf_s(buffer, BuffSize, "IATPatch: fname=%s", fname);
						//LogText(buffer);
#endif
						if (!lstrcmpi(apiname, fname)) {
#ifdef _DEBUG
							sprintf_s(buffer, BuffSize, "IATPatch: BYSCAN ordinal=%x address=%x name=%s", ptaddr->u1.Ordinal, ptaddr->u1.AddressOfData, fname);
							LogText(buffer);
#endif
							break;
						}
						for (; *fname; fname++); for (; !*fname; fname++);
					}

					if (apiproc) {
						// examining by function addr
						if (ptaddr->u1.Function == (DWORD)apiproc) break;
					}
					ptaddr++;
					if (ptname) ptname++;
				}

				if (ptaddr->u1.Function) {
					org = (void *)ptaddr->u1.Function;
					if (org == hookproc) return 0; // already hooked

					if (!VirtualProtect(&ptaddr->u1.Function, 4, PAGE_EXECUTE_READWRITE, &oldprotect)) {
#ifdef _DEBUG
						sprintf_s(buffer, BuffSize, "IATPatch: VirtualProtect error %d at %d", GetLastError(), __LINE__);
						LogText(buffer);
#endif
						return 0;
					}
					ptaddr->u1.Function = (DWORD)hookproc;
					if (!VirtualProtect(&ptaddr->u1.Function, 4, oldprotect, &oldprotect)) {
#ifdef _DEBUG
						sprintf_s(buffer, BuffSize, "IATPatch: VirtualProtect error %d at %d", GetLastError(), __LINE__);
						LogText(buffer);
#endif
						return 0;
					}
					if (!FlushInstructionCache(GetCurrentProcess(), &ptaddr->u1.Function, 4)) {
#ifdef _DEBUG
						sprintf_s(buffer, BuffSize, "IATPatch: FlushInstructionCache error %d at %d", GetLastError(), __LINE__);
						LogText(buffer);
#endif
						return 0;
					}
#ifdef _DEBUG
					sprintf_s(buffer, BuffSize, "IATPatch hook=%s address=%x->%x", apiname, org, hookproc);
					LogText(buffer);
#endif

					return org;
				}
			}
			pidesc++;
		}
		if (!pidesc->FirstThunk) {
#ifdef _DEBUG
			sprintf_s(buffer, BuffSize, "IATPatch: PE unreferenced function %s:%s", dll, apiname);
			LogText(buffer);
#endif
			return 0;
		}

	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		sprintf_s(buffer, BuffSize, "IATPatchEx: EXCEPTION hook=%s:%s Hook Failed.", dll, apiname);
		LogText(buffer);
	}
	return org;
}

void *IATPatch(HMODULE module, char *dll, void *apiproc, const char *apiname, void *hookproc)
{
	return IATPatch(module, 0, dll, apiproc, apiname, hookproc);
}