#pragma once

#define VISIT_PROCS(visit) \
	visit(Direct3DCreateTexture) \
	visit(Direct3DGetSWRastZPixFmts) \
	visit(FlushD3DDevices2)

PROC_CLASS(d3dim, dll)

#undef VISIT_PROCS
