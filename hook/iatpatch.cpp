/**
* Created from source code found in DxWnd v2.03.60
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
	Compat::Log() << "IATPatch: module=" << module << " ordinal=" << std::showbase << std::hex << ordinal << std::dec << std::noshowbase << " name=" << apiname << " dll=" << dll;
#endif

	base = (DWORD)module;
	org = 0; // by default, ret = 0 => API not found

	__try {
		pnth = PIMAGE_NT_HEADERS(PBYTE(base) + PIMAGE_DOS_HEADER(base)->e_lfanew);
		if (!pnth) {
			//Compat::Log() << "IATPatch: ERROR no PNTH at " << __LINE__;
			return 0;
		}
		rva = pnth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
		if (!rva) {
			//Compat::Log() << "IATPatch: ERROR no RVA at " << __LINE__;
			return 0;
		}
		pidesc = (PIMAGE_IMPORT_DESCRIPTOR)(base + rva);

		while (pidesc->FirstThunk) {
			impmodule = (PSTR)(base + pidesc->Name);
			//Compat::Log() << "IATPatch: analyze impmodule=" << impmodule;
			char *fname = impmodule;
			for (; *fname; fname++); for (; !*fname; fname++);

			if (!lstrcmpi(dll, impmodule)) {
				//Compat::Log() << "IATPatch: dll=" << dll << " found at " << impmodule;

				ptaddr = (PIMAGE_THUNK_DATA)(base + (DWORD)pidesc->FirstThunk);
				ptname = (pidesc->OriginalFirstThunk) ? (PIMAGE_THUNK_DATA)(base + (DWORD)pidesc->OriginalFirstThunk) : NULL;

				while (ptaddr->u1.Function) {
					//Compat::Log() << "IATPatch: address=" << std::showbase << std::hex << ptaddr->u1.AddressOfData << std::dec << std::noshowbase << " ptname=" << ptname;

					if (ptname) {
						// examining by function name
						if (!IMAGE_SNAP_BY_ORDINAL(ptname->u1.Ordinal)) {
							piname = (PIMAGE_IMPORT_BY_NAME)(base + (DWORD)ptname->u1.AddressOfData);
							/*Compat::Log() << "IATPatch: BYNAME ordinal=" << std::showbase << std::hex << ptaddr->u1.Ordinal
								<< " address=" << ptaddr->u1.AddressOfData << std::dec << std::noshowbase
								<< "name=" << (char *)piname->Name << " hint=" << piname->Hint;*/
							if (!lstrcmpi(apiname, (char *)piname->Name)) break;
						}
						else {
							//Compat::Log() << "IATPatch: BYORD target=" << std::showbase << std::hex << ordinal << " ord=" << IMAGE_ORDINAL32(ptname->u1.Ordinal) << std::dec << std::noshowbase;
							if (ordinal && (IMAGE_ORDINAL32(ptname->u1.Ordinal) == ordinal)) { // skip unknow ordinal 0
								//Compat::Log() << "IATPatch: BYORD ordinal=" << std::showbase << std::hex << ptname->u1.Ordinal << " addr=" << ptaddr->u1.Function << std::dec << std::noshowbase;
								//Compat::Log() << "IATPatch: BYORD GetProcAddress=" << std::showbase << std::hex << GetProcAddress(GetModuleHandle(dll), MAKEINTRESOURCE(IMAGE_ORDINAL32(ptname->u1.Ordinal))) << std::dec << td::noshowbase;
								break;
							}
						}

					}
					else {
						//Compat::Log() << "IATPatch: fname=" << fname;
						if (!lstrcmpi(apiname, fname)) {
							//Compat::Log() << "IATPatch: BYSCAN ordinal=" << std::showbase << std::hex << ptaddr->u1.Ordinal << " address=" << ptaddr->u1.AddressOfData << std::dec << std::noshowbase << " name=" << fname;
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
						//Compat::Log() << "IATPatch: VirtualProtect error " << GetLastError() << " at " << __LINE__;
						return 0;
					}
					ptaddr->u1.Function = (DWORD)hookproc;
					if (!VirtualProtect(&ptaddr->u1.Function, 4, oldprotect, &oldprotect)) {
						//Compat::Log() << "IATPatch: VirtualProtect error " << GetLastError() << " at " << __LINE__;
						return 0;
					}
					if (!FlushInstructionCache(GetCurrentProcess(), &ptaddr->u1.Function, 4)) {
						//Compat::Log() << "IATPatch: FlushInstructionCache error " << GetLastError() << " at " << __LINE__;
						return 0;
					}
					//Compat::Log() << "IATPatch hook=" << apiname << " address=" << std::showbase << std::hex << org << "->" << hookproc << std::dec << std::noshowbase;

					return org;
				}
			}
			pidesc++;
		}
		if (!pidesc->FirstThunk) {
			//Compat::Log() << "IATPatch: PE unreferenced function " << dll << ":" << apiname;
			return 0;
		}

	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		//Compat::Log() << "IATPatch: EXCEPTION hook=" << dll << ":" << apiname << " Hook Failed.";
	}
	return org;
}

void *IATPatch(HMODULE module, char *dll, void *apiproc, const char *apiname, void *hookproc)
{
	return IATPatch(module, 0, dll, apiproc, apiname, hookproc);
}