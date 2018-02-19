#pragma once

#define VISIT_PROCS_DDRAW(visit) \
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

#ifdef PROC_CLASS
PROC_CLASS(ddraw, dll, VISIT_PROCS_DDRAW)
#endif
