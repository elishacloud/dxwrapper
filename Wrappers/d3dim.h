#pragma once

#define VISIT_PROCS_D3DIM(visit) \
	visit(Direct3DCreateTexture, jmpaddr) \
	visit(Direct3DGetSWRastZPixFmts, jmpaddr) \
	visit(FlushD3DDevices2, jmpaddr)

#ifdef PROC_CLASS
PROC_CLASS(d3dim, dll, VISIT_PROCS_D3DIM)
#endif
