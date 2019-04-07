#pragma once

#define VISIT_PROCS_D3D8(visit) \
	visit(Direct3D8EnableMaximizedWindowedModeShim, jmpaddrvoid) \
	visit(ValidateVertexShader, jmpaddr) \
	visit(ValidatePixelShader, jmpaddr) \
	visit(Direct3DCreate8, jmpaddr)

#define VISIT_PROCS_D3D8_SHARED(visit) \
	visit(DebugSetMute, jmpaddrvoid)

#define VISIT_SHARED_D3D8_PROCS(visit) \
	visit(DebugSetMute, DebugSetMute_d3d8, jmpaddrvoid)

#ifdef PROC_CLASS
PROC_CLASS(d3d8, dll, VISIT_PROCS_D3D8, VISIT_SHARED_D3D8_PROCS)
#endif
