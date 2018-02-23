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

#undef VISIT_UNMODIFIED_DDRAW_PROCS
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

#undef VISIT_MODIFIED_DDRAW_PROCS
#define VISIT_MODIFIED_DDRAW_PROCS(visit) \
	visit(DirectDrawCreate) \
	visit(DirectDrawCreateEx) \
	visit(DllGetClassObject)

#undef VISIT_ALL_DDRAW_PROCS
#define VISIT_ALL_DDRAW_PROCS(visit) \
	VISIT_UNMODIFIED_DDRAW_PROCS(visit) \
	VISIT_MODIFIED_DDRAW_PROCS(visit)

#undef VISIT_UNMODIFIED_PROCS
#define VISIT_UNMODIFIED_PROCS(visit) \
	VISIT_UNMODIFIED_DDRAW_PROCS(visit)

#undef VISIT_MODIFIED_PROCS
#define VISIT_MODIFIED_PROCS(visit) \
	VISIT_MODIFIED_DDRAW_PROCS(visit)

#undef VISIT_ALL_PROCS
#define VISIT_ALL_PROCS(visit) \
	VISIT_ALL_DDRAW_PROCS(visit)

#ifdef PROC_CLASS
PROC_CLASS(ddraw, dll, VISIT_PROCS_DDRAW)
#endif
