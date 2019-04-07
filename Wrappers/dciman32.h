#pragma once

#define VISIT_PROCS_DCIMAN32(visit) \
	visit(DCIBeginAccess, jmpaddr) \
	visit(DCICloseProvider, jmpaddr) \
	visit(DCICreateOffscreen, jmpaddr) \
	visit(DCICreateOverlay, jmpaddr) \
	visit(DCICreatePrimary, jmpaddr) \
	visit(DCIDestroy, jmpaddr) \
	visit(DCIDraw, jmpaddr) \
	visit(DCIEndAccess, jmpaddr) \
	visit(DCIEnum, jmpaddr) \
	visit(DCIOpenProvider, jmpaddr) \
	visit(DCISetClipList, jmpaddr) \
	visit(DCISetDestination, jmpaddr) \
	visit(DCISetSrcDestClip, jmpaddr) \
	visit(GetDCRegionData, jmpaddr) \
	visit(GetWindowRegionData, jmpaddr) \
	visit(WinWatchClose, jmpaddr) \
	visit(WinWatchDidStatusChange, jmpaddr) \
	visit(WinWatchGetClipList, jmpaddr) \
	visit(WinWatchNotify, jmpaddr) \
	visit(WinWatchOpen, jmpaddr)

#ifdef PROC_CLASS
PROC_CLASS(dciman32, dll, VISIT_PROCS_DCIMAN32, VISIT_PROCS_BLANK)
#endif
