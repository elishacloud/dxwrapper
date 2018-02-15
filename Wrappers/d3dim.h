#pragma once

#define VISIT_PROCS(visit) \
	visit(Direct3DCreateTexture, jmpaddr) \
	visit(Direct3DGetSWRastZPixFmts, jmpaddr) \
	visit(FlushD3DDevices2, jmpaddr)

PROC_CLASS(d3dim, dll)

#undef VISIT_PROCS
