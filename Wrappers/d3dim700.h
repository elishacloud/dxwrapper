#pragma once

#define VISIT_PROCS(visit) \
	visit(CreateTexture) \
	visit(D3DBreakVBLock) \
	visit(D3DTextureUpdate) \
	visit(DestroyTexture) \
	visit(GetLOD) \
	visit(GetPriority) \
	visit(SetLOD) \
	visit(SetPriority)

PROC_CLASS(d3dim700, dll)

#undef VISIT_PROCS
