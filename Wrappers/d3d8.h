#pragma once

#define VISIT_PROCS(visit) \
	visit(Direct3D8EnableMaximizedWindowedModeShim, jmpaddrvoid) \
	visit(ValidateVertexShader, jmpaddr) \
	visit(ValidatePixelShader, jmpaddr) \
	visit(Direct3DCreate8, jmpaddr)

PROC_CLASS(d3d8, dll)

#undef VISIT_PROCS
