#pragma once
#include "Settings\Settings.h"
#include "Wrappers\Wrapper.h"

BOOL WINAPI DllMain_DDrawCompat(HINSTANCE, DWORD, LPVOID);

namespace DDrawCompat
{
#define INITUALIZE_WRAPPED_PROC(procName, unused) \
	extern FARPROC procName ## _in; \
	extern FARPROC procName ## _out;

	VISIT_PROCS_DDRAW(INITUALIZE_WRAPPED_PROC);

#undef INITUALIZE_WRAPPED_PROC

	extern FARPROC DllCanUnloadNow_in;
	extern FARPROC DllCanUnloadNow_out;
	extern FARPROC DllGetClassObject_in;
	extern FARPROC DllGetClassObject_out;
}
