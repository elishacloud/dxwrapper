#pragma once

#define VISIT_PROCS(visit) \
	visit(Direct3DShaderValidatorCreate9) \
	visit(PSGPError) \
	visit(PSGPSampleTexture) \
	visit(D3DPERF_BeginEvent) \
	visit(D3DPERF_EndEvent) \
	visit(D3DPERF_GetStatus) \
	visit(D3DPERF_QueryRepeatFrame) \
	visit(D3DPERF_SetMarker) \
	visit(D3DPERF_SetOptions) \
	visit(D3DPERF_SetRegion) \
	visit(DebugSetLevel) \
	visit(Direct3D9EnableMaximizedWindowedModeShim) \
	visit(Direct3DCreate9) \
	visit(Direct3DCreate9Ex)

PROC_CLASS(d3d9, dll)

#undef VISIT_PROCS
