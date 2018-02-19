#pragma once

#define VISIT_PROCS_D3DIM700(visit) \
	visit(CreateTexture, jmpaddr) \
	visit(D3DBreakVBLock, jmpaddr) \
	visit(D3DTextureUpdate, jmpaddr) \
	visit(DestroyTexture, jmpaddr) \
	visit(GetLOD, jmpaddr) \
	visit(GetPriority, jmpaddr) \
	visit(SetLOD, jmpaddr) \
	visit(SetPriority, jmpaddr)

#ifdef PROC_CLASS
PROC_CLASS(d3dim700, dll, VISIT_PROCS_D3DIM700)
#endif
