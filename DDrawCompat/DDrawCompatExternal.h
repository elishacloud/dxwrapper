#pragma once

#include "Settings\Settings.h"
#include "Wrappers\Wrapper.h"
#ifdef DDRAWCOMPAT
#include"v0.3.2/Win32/DisplayMode.h"
#endif

namespace DDrawCompat
{
#define INITIALIZE_WRAPPED_PROC(procName) \
	extern volatile FARPROC procName ## _in; \
	extern volatile FARPROC procName ## _out;

	VISIT_ALL_DDRAW_PROCS(INITIALIZE_WRAPPED_PROC);

#undef INITIALIZE_WRAPPED_PROC

	void Prepare();
	void Start(HINSTANCE, DWORD);
	bool IsEnabled();
	void InstallDd7to9Hooks(HMODULE hModule);
}
