#pragma once

#include "Settings\Settings.h"
#include "Wrappers\Wrapper.h"

namespace DDrawCompat
{
#define INITIALIZE_WRAPPED_PROC(procName) \
	extern FARPROC procName ## _in; \
	extern FARPROC procName ## _out;

	VISIT_ALL_DDRAW_PROCS(INITIALIZE_WRAPPED_PROC);

#undef INITIALIZE_WRAPPED_PROC

	void Prepare();
	bool Start(HINSTANCE, DWORD);
	void InstallHooks();
}
