#pragma once

#define VISIT_PROCS(visit) \
	visit(DCIBeginAccess) \
	visit(DCICloseProvider) \
	visit(DCICreateOffscreen) \
	visit(DCICreateOverlay) \
	visit(DCICreatePrimary) \
	visit(DCIDestroy) \
	visit(DCIDraw) \
	visit(DCIEndAccess) \
	visit(DCIEnum) \
	visit(DCIOpenProvider) \
	visit(DCISetClipList) \
	visit(DCISetDestination) \
	visit(DCISetSrcDestClip) \
	visit(GetDCRegionData) \
	visit(GetWindowRegionData) \
	visit(WinWatchClose) \
	visit(WinWatchDidStatusChange) \
	visit(WinWatchGetClipList) \
	visit(WinWatchNotify) \
	visit(WinWatchOpen)

PROC_CLASS(dciman32, dll)

#undef VISIT_PROCS
