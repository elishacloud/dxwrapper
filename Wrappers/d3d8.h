#pragma once

#define VISIT_PROCS_D3D8(visit) \
	visit(ValidateVertexShader, jmpaddr) \
	visit(ValidatePixelShader, jmpaddr) \
	visit(Direct3DCreate8, jmpaddr)

#ifdef PROC_CLASS
PROC_CLASS(d3d8, dll, VISIT_PROCS_D3D8, VISIT_PROCS_BLANK)
#endif
