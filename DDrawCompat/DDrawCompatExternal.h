#pragma once

#include "Settings\Settings.h"
#include "Wrappers\Wrapper.h"

namespace DDrawCompat
{
#define INITUALIZE_WRAPPED_PROC(procName) \
	extern FARPROC procName ## _in; \
	extern FARPROC procName ## _out;

	VISIT_ALL_DDRAW_PROCS(INITUALIZE_WRAPPED_PROC);

#undef INITUALIZE_WRAPPED_PROC

	void Prepare();
	bool Start(HINSTANCE, DWORD);
}
