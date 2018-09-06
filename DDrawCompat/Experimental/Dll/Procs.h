#pragma once

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>

#define VISIT_UNMODIFIED_PROCS(visit) \
	visit(AcquireDDThreadLock) \
	visit(CompleteCreateSysmemSurface) \
	visit(D3DParseUnknownCommand) \
	visit(DDGetAttachedSurfaceLcl) \
	visit(DDInternalLock) \
	visit(DDInternalUnlock) \
	visit(DSoundHelp) \
	visit(DirectDrawCreateClipper) \
	visit(DirectDrawEnumerateA) \
	visit(DirectDrawEnumerateExA) \
	visit(DirectDrawEnumerateExW) \
	visit(DirectDrawEnumerateW) \
	visit(DllCanUnloadNow) \
	visit(GetDDSurfaceLocal) \
	visit(GetOLEThunkData) \
	visit(GetSurfaceFromDC) \
	visit(RegisterSpecialCase) \
	visit(ReleaseDDThreadLock) \
	visit(SetAppCompatData)

#define VISIT_MODIFIED_PROCS(visit) \
	visit(DirectDrawCreate) \
	visit(DirectDrawCreateEx) \
	visit(DllGetClassObject)

#define VISIT_ALL_PROCS(visit) \
	VISIT_UNMODIFIED_PROCS(visit) \
	VISIT_MODIFIED_PROCS(visit)

//********** Begin Edit *************
#include "Wrappers\ddraw.h"
//********** End Edit ***************

#define ADD_FARPROC_MEMBER(memberName) FARPROC memberName;

namespace Dll
{
	struct Procs
	{
		VISIT_ALL_PROCS(ADD_FARPROC_MEMBER);
		FARPROC DirectInputCreateA;
	};

	extern Procs g_origProcs;
}

#undef  ADD_FARPROC_MEMBER

#define CALL_ORIG_PROC(procName, ...) \
	(Dll::g_origProcs.procName ? \
		reinterpret_cast<decltype(procName)*>(Dll::g_origProcs.procName)(__VA_ARGS__) : \
		E_FAIL)
