#pragma once

#define VISIT_PROCS(visit) \
	visit(Direct3D8EnableMaximizedWindowedModeShim) \
	visit(ValidateVertexShader) \
	visit(ValidatePixelShader) \
	visit(Direct3DCreate8)

PROC_CLASS(d3d8, dll)

#undef VISIT_PROCS
