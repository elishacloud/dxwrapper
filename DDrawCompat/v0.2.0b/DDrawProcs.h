#pragma once

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>

#define VISIT_UNMODIFIED_DDRAW_PROCS(visit) \
	visit(AcquireDDThreadLock) \
	visit(CheckFullscreen) \
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

//********** Begin Edit *************
#define VISIT_MODIFIED_DDRAW_PROCS(visit) \
	visit(DirectDrawCreate) \
	visit(DirectDrawCreateEx) \
	visit(DllGetClassObject)
//********** End Edit ***************

#define VISIT_ALL_DDRAW_PROCS(visit) \
	VISIT_UNMODIFIED_DDRAW_PROCS(visit) \
	VISIT_MODIFIED_DDRAW_PROCS(visit)

#define ADD_FARPROC_MEMBER(memberName) FARPROC memberName;

namespace Compat
{
	struct DDrawProcs
	{
		VISIT_ALL_DDRAW_PROCS(ADD_FARPROC_MEMBER);
		FARPROC DirectInputCreateA;
	};

	extern DDrawProcs origProcs;
}

#undef  ADD_FARPROC_MEMBER

#define CALL_ORIG_DDRAW(procName, ...) \
	(Compat::origProcs.procName ? \
		reinterpret_cast<decltype(procName)*>(Compat::origProcs.procName)(__VA_ARGS__) : \
		E_FAIL)
