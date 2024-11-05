#pragma once

#include "DDrawCompat\DDrawCompatExternal.h"

#define DDRAWCOMPAT_32

namespace Compat32
{
	void Prepair_DDrawCompat();
	BOOL WINAPI DllMain_DDrawCompat(HINSTANCE, DWORD, LPVOID);

	void InstallDd7to9Hooks(HMODULE hModule);

#define INITIALIZE_WRAPPED_PROC(procName) \
	extern FARPROC procName ## _proc;

	VISIT_ALL_DDRAW_PROCS(INITIALIZE_WRAPPED_PROC);

#undef INITIALIZE_WRAPPED_PROC
}
