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

// Hook API using IAT patch
void *Hook::IATPatch(HMODULE module, DWORD ordinal, const char *dll, void *apiproc, const char *apiname, void *hookproc)
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

	static constexpr DWORD BuffSize = 250;
	char buffer[BuffSize];

#ifdef _DEBUG
	sprintf_s(buffer, "IATPatch: module=%p ordinal=%x name=%s dll=%s", module, ordinal, apiname, dll);
	Logging::LogText(buffer);
#endif

	base = (DWORD)module;
	org = 0; // by default, ret = 0 => API not found

	__try
	{
		pnth = PIMAGE_NT_HEADERS(PBYTE(base) + PIMAGE_DOS_HEADER(base)->e_lfanew);
		if (!pnth)
		{
			sprintf_s(buffer, "IATPatch: ERROR no PNTH at %d", __LINE__);
			Logging::LogText(buffer);
			return 0;
		}
		rva = pnth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
		if (!rva)
		{
			sprintf_s(buffer, "IATPatch: ERROR no RVA at %d", __LINE__);
			Logging::LogText(buffer);
			return 0;
		}
		pidesc = (PIMAGE_IMPORT_DESCRIPTOR)(base + rva);

		while (pidesc->FirstThunk)
		{
			impmodule = (PSTR)(base + pidesc->Name);
#ifdef _DEBUG
			//sprintf_s(buffer, "IATPatch: analyze impmodule=%s", impmodule);
			//LogText(buffer);
#endif
			char *fname = impmodule;
			for (; *fname; fname++); for (; !*fname; fname++);

			if (!lstrcmpi(dll, impmodule))
			{
#ifdef _DEBUG
				sprintf_s(buffer, "IATPatch: dll=%s found at %p", dll, impmodule);
				Logging::LogText(buffer);
#endif

				ptaddr = (PIMAGE_THUNK_DATA)(base + (DWORD)pidesc->FirstThunk);
				ptname = (pidesc->OriginalFirstThunk) ? (PIMAGE_THUNK_DATA)(base + (DWORD)pidesc->OriginalFirstThunk) : nullptr;

				while (ptaddr->u1.Function)
				{
#ifdef _DEBUG
					//sprintf_s(buffer, "IATPatch: address=%x ptname=%x", ptaddr->u1.AddressOfData, ptname);
					//LogText(buffer);
#endif

					if (ptname)
					{
						// examining by function name
						if (!IMAGE_SNAP_BY_ORDINAL(ptname->u1.Ordinal))
						{
							piname = (PIMAGE_IMPORT_BY_NAME)(base + (DWORD)ptname->u1.AddressOfData);
#ifdef _DEBUG
							sprintf_s(buffer, "IATPatch: BYNAME ordinal=%x address=%x name=%s hint=%x", ptaddr->u1.Ordinal, ptaddr->u1.AddressOfData, (char *)piname->Name, piname->Hint);
							Logging::LogText(buffer);
#endif
							if (!lstrcmpi(apiname, (char *)piname->Name))
							{
								break;
							}
						}
						else
						{
#ifdef _DEBUG
							//sprintf_s(buffer, "IATPatch: BYORD target=%x ord=%x", ordinal, IMAGE_ORDINAL32(ptname->u1.Ordinal));
							//LogText(buffer);
#endif
							// skip unknow ordinal 0
							if (ordinal && (IMAGE_ORDINAL32(ptname->u1.Ordinal) == ordinal))
							{
#ifdef _DEBUG
								sprintf_s(buffer, "IATPatch: BYORD ordinal=%x addr=%x", ptname->u1.Ordinal, ptaddr->u1.Function);
								//sprintf_s(buffer, "IATPatch: BYORD GetProcAddress=%x", GetProcAddress(GetModuleHandle(dll), MAKEINTRESOURCE(IMAGE_ORDINAL32(ptname->u1.Ordinal))));	
								Logging::LogText(buffer);
#endif
								break;
							}
						}

					}
					else
					{
#ifdef _DEBUG
						//sprintf_s(buffer, "IATPatch: fname=%s", fname);
						//LogText(buffer);
#endif
						if (!lstrcmpi(apiname, fname))
						{
#ifdef _DEBUG
							sprintf_s(buffer, "IATPatch: BYSCAN ordinal=%x address=%x name=%s", ptaddr->u1.Ordinal, ptaddr->u1.AddressOfData, fname);
							Logging::LogText(buffer);
#endif
							break;
						}
						for (; *fname; fname++); for (; !*fname; fname++);
					}

					if (apiproc)
					{
						// examining by function addr
						if (ptaddr->u1.Function == (DWORD)apiproc)
						{
							break;
						}
					}
					ptaddr++;
					if (ptname) ptname++;
				}

				if (ptaddr->u1.Function)
				{
					org = (void *)ptaddr->u1.Function;
					if (org == hookproc) return (void *)1; // already hooked

					if (!VirtualProtect(&ptaddr->u1.Function, 4, PAGE_EXECUTE_READWRITE, &oldprotect))
					{
#ifdef _DEBUG
						sprintf_s(buffer, "IATPatch: VirtualProtect error %d at %d", GetLastError(), __LINE__);
						Logging::LogText(buffer);
#endif
						return 0;
					}
					ptaddr->u1.Function = (DWORD)hookproc;
					if (!VirtualProtect(&ptaddr->u1.Function, 4, oldprotect, &oldprotect))
					{
#ifdef _DEBUG
						sprintf_s(buffer, "IATPatch: VirtualProtect error %d at %d", GetLastError(), __LINE__);
						Logging::LogText(buffer);
#endif
						return 0;
					}
					if (!FlushInstructionCache(GetCurrentProcess(), &ptaddr->u1.Function, 4))
					{
#ifdef _DEBUG
						sprintf_s(buffer, "IATPatch: FlushInstructionCache error %d at %d", GetLastError(), __LINE__);
						Logging::LogText(buffer);
#endif
						return 0;
					}
#ifdef _DEBUG
					sprintf_s(buffer, "IATPatch hook=%s address=%p->%p", apiname, org, hookproc);
					Logging::LogText(buffer);
#endif

					return org;
				}
			}
			pidesc++;
		}
		if (!pidesc->FirstThunk)
		{
#ifdef _DEBUG
			sprintf_s(buffer, "IATPatch: PE unreferenced function %s:%s", dll, apiname);
			Logging::LogText(buffer);
#endif
			return 0;
		}

	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		sprintf_s(buffer, "IATPatchEx: EXCEPTION hook=%s:%s Hook Failed.", dll, apiname);
		Logging::LogText(buffer);
	}
	return org;
}

// Unhook IAT patched API
bool Hook::UnhookIATPatch(HMODULE module, DWORD ordinal, const char *dll, void *apiproc, const char *apiname, void *hookproc)
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

	static constexpr DWORD BuffSize = 250;
	char buffer[BuffSize];

#ifdef _DEBUG
	sprintf_s(buffer, "IATPatch: module=%p ordinal=%x name=%s dll=%s", module, ordinal, apiname, dll);
	Logging::LogText(buffer);
#endif

	base = (DWORD)module;
	org = 0;

	__try
	{
		pnth = PIMAGE_NT_HEADERS(PBYTE(base) + PIMAGE_DOS_HEADER(base)->e_lfanew);
		if (!pnth)
		{
			sprintf_s(buffer, "IATPatch: ERROR no PNTH at %d", __LINE__);
			Logging::LogText(buffer);
			return false;
		}
		rva = pnth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
		if (!rva)
		{
			sprintf_s(buffer, "IATPatch: ERROR no RVA at %d", __LINE__);
			Logging::LogText(buffer);
			return false;
		}
		pidesc = (PIMAGE_IMPORT_DESCRIPTOR)(base + rva);

		while (pidesc->FirstThunk)
		{
			impmodule = (PSTR)(base + pidesc->Name);
#ifdef _DEBUG
			//sprintf_s(buffer, "IATPatch: analyze impmodule=%s", impmodule);
			//LogText(buffer);
#endif
			char *fname = impmodule;
			for (; *fname; fname++); for (; !*fname; fname++);

			if (!lstrcmpi(dll, impmodule))
			{
#ifdef _DEBUG
				sprintf_s(buffer, "IATPatch: dll=%s found at %p", dll, impmodule);
				Logging::LogText(buffer);
#endif

				ptaddr = (PIMAGE_THUNK_DATA)(base + (DWORD)pidesc->FirstThunk);
				ptname = (pidesc->OriginalFirstThunk) ? (PIMAGE_THUNK_DATA)(base + (DWORD)pidesc->OriginalFirstThunk) : nullptr;

				while (ptaddr->u1.Function)
				{
#ifdef _DEBUG
					//sprintf_s(buffer, "IATPatch: address=%x ptname=%x", ptaddr->u1.AddressOfData, ptname);
					//LogText(buffer);
#endif

					if (ptname)
					{
						// examining by function name
						if (!IMAGE_SNAP_BY_ORDINAL(ptname->u1.Ordinal))
						{
							piname = (PIMAGE_IMPORT_BY_NAME)(base + (DWORD)ptname->u1.AddressOfData);
#ifdef _DEBUG
							sprintf_s(buffer, "IATPatch: BYNAME ordinal=%x address=%x name=%s hint=%x", ptaddr->u1.Ordinal, ptaddr->u1.AddressOfData, (char *)piname->Name, piname->Hint);
							Logging::LogText(buffer);
#endif
							if (!lstrcmpi(apiname, (char *)piname->Name))
							{
								break;
							}
						}
						else
						{
#ifdef _DEBUG
							//sprintf_s(buffer, "IATPatch: BYORD target=%x ord=%x", ordinal, IMAGE_ORDINAL32(ptname->u1.Ordinal));
							//LogText(buffer);
#endif
							// skip unknow ordinal 0
							if (ordinal && (IMAGE_ORDINAL32(ptname->u1.Ordinal) == ordinal))
							{
#ifdef _DEBUG
								sprintf_s(buffer, "IATPatch: BYORD ordinal=%x addr=%x", ptname->u1.Ordinal, ptaddr->u1.Function);
								//sprintf_s(buffer, "IATPatch: BYORD GetProcAddress=%x", GetProcAddress(GetModuleHandle(dll), MAKEINTRESOURCE(IMAGE_ORDINAL32(ptname->u1.Ordinal))));	
								Logging::LogText(buffer);
#endif
								break;
							}
						}

					}
					else
					{
#ifdef _DEBUG
						//sprintf_s(buffer, "IATPatch: fname=%s", fname);
						//LogText(buffer);
#endif
						if (!lstrcmpi(apiname, fname))
						{
#ifdef _DEBUG
							sprintf_s(buffer, "IATPatch: BYSCAN ordinal=%x address=%x name=%s", ptaddr->u1.Ordinal, ptaddr->u1.AddressOfData, fname);
							Logging::LogText(buffer);
#endif
							break;
						}
						for (; *fname; fname++); for (; !*fname; fname++);
					}

					if (apiproc)
					{
						// examining by function addr
						if (ptaddr->u1.Function == (DWORD)apiproc)
						{
							break;
						}
					}
					ptaddr++;
					if (ptname) ptname++;
				}

				if (ptaddr->u1.Function)
				{
					org = (void *)ptaddr->u1.Function;

					// Check if API is patched
					if (org == hookproc)
					{

						if (!VirtualProtect(&ptaddr->u1.Function, 4, PAGE_EXECUTE_READWRITE, &oldprotect))
						{
#ifdef _DEBUG
							sprintf_s(buffer, "IATPatch: VirtualProtect error %d at %d", GetLastError(), __LINE__);
							Logging::LogText(buffer);
#endif
							return false;
						}
						ptaddr->u1.Function = (DWORD)apiproc;
						if (!VirtualProtect(&ptaddr->u1.Function, 4, oldprotect, &oldprotect))
						{
#ifdef _DEBUG
							sprintf_s(buffer, "IATPatch: VirtualProtect error %d at %d", GetLastError(), __LINE__);
							Logging::LogText(buffer);
#endif
							return false;
						}
						if (!FlushInstructionCache(GetCurrentProcess(), &ptaddr->u1.Function, 4))
						{
#ifdef _DEBUG
							sprintf_s(buffer, "IATPatch: FlushInstructionCache error %d at %d", GetLastError(), __LINE__);
							Logging::LogText(buffer);
#endif
							return false;
						}
#ifdef _DEBUG
						sprintf_s(buffer, "IATPatch hook=%s address=%p->%p", apiname, org, hookproc);
						Logging::LogText(buffer);
#endif

						return true;
					}
					return false;
				}
			}
			pidesc++;
		}
		if (!pidesc->FirstThunk)
		{
#ifdef _DEBUG
			sprintf_s(buffer, "IATPatch: PE unreferenced function %s:%s", dll, apiname);
			Logging::LogText(buffer);
#endif
			return false;
		}

	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		sprintf_s(buffer, "IATPatchEx: EXCEPTION hook=%s:%s Hook Failed.", dll, apiname);
		Logging::LogText(buffer);
	}
	return false;
}
