#pragma once

#include "wrapper.h"

#define VISIT_PROCS(visit) \
	visit(AcquireDDThreadLock, jmpaddrvoid) \
	visit(CheckFullscreen, jmpaddr) \
	visit(CompleteCreateSysmemSurface, jmpaddr) \
	visit(D3DParseUnknownCommand, jmpaddr) \
	visit(DDGetAttachedSurfaceLcl, jmpaddr) \
	visit(DDInternalLock, jmpaddr) \
	visit(DDInternalUnlock, jmpaddr) \
	visit(DSoundHelp, jmpaddr) \
	visit(DirectDrawCreate, jmpaddr) \
	visit(DirectDrawCreateClipper, jmpaddr) \
	visit(DirectDrawCreateEx, jmpaddr) \
	visit(DirectDrawEnumerateA, jmpaddr) \
	visit(DirectDrawEnumerateExA, jmpaddr) \
	visit(DirectDrawEnumerateExW, jmpaddr) \
	visit(DirectDrawEnumerateW, jmpaddr) \
	visit(GetDDSurfaceLocal, jmpaddr) \
	visit(GetOLEThunkData, jmpaddr) \
	visit(GetSurfaceFromDC, jmpaddr) \
	visit(RegisterSpecialCase, jmpaddr) \
	visit(ReleaseDDThreadLock, jmpaddrvoid) \
	visit(SetAppCompatData, jmpaddr)

PROC_CLASS(ddraw, dll)

#undef VISIT_PROCS
