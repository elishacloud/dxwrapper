#pragma once

#include "wrapper.h"

#define VISIT_PROCS(visit) \
	visit(AcquireDDThreadLock) \
	visit(CheckFullscreen) \
	visit(CompleteCreateSysmemSurface) \
	visit(D3DParseUnknownCommand) \
	visit(DDGetAttachedSurfaceLcl) \
	visit(DDInternalLock) \
	visit(DDInternalUnlock) \
	visit(DSoundHelp) \
	visit(DirectDrawCreate) \
	visit(DirectDrawCreateClipper) \
	visit(DirectDrawCreateEx) \
	visit(DirectDrawEnumerateA) \
	visit(DirectDrawEnumerateExA) \
	visit(DirectDrawEnumerateExW) \
	visit(DirectDrawEnumerateW) \
	visit(GetDDSurfaceLocal) \
	visit(GetOLEThunkData) \
	visit(GetSurfaceFromDC) \
	visit(RegisterSpecialCase) \
	visit(ReleaseDDThreadLock) \
	visit(SetAppCompatData)

PROC_CLASS(ddraw, dll)

#undef VISIT_PROCS
