#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace Hook
{
	FARPROC GetFunctionAddress(HMODULE, LPCSTR);
	void *HookAPI(HMODULE, const char *, void *, const char *, void *);
	void *HotPatch(void*, const char*, void*);
	void *IATPatch(HMODULE, char *, void *, const char *, void *);
	void *IATPatch(HMODULE, DWORD, const char*, void*, const char*, void*);
}
