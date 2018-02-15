#pragma once

#define VISIT_PROCS(visit) \
	visit(CreateTexture, jmpaddr) \
	visit(D3DBreakVBLock, jmpaddr) \
	visit(D3DTextureUpdate, jmpaddr) \
	visit(DestroyTexture, jmpaddr) \
	visit(GetLOD, jmpaddr) \
	visit(GetPriority, jmpaddr) \
	visit(SetLOD, jmpaddr) \
	visit(SetPriority, jmpaddr)

PROC_CLASS(d3dim700, dll)

#undef VISIT_PROCS
