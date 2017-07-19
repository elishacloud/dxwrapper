#pragma once

#define VISIT_D3D9_PROCS(visit) \
	visit(D3DPERF_BeginEvent) \
	visit(D3DPERF_EndEvent) \
	visit(D3DPERF_GetStatus) \
	visit(D3DPERF_QueryRepeatFrame) \
	visit(D3DPERF_SetMarker) \
	visit(D3DPERF_SetOptions) \
	visit(D3DPERF_SetRegion) \
	visit(DebugSetLevel) \
	visit(Direct3DCreate9) \
	visit(Direct3DCreate9Ex) \
	visit(Direct3DShaderValidatorCreate9) \
	visit(PSGPError) \
	visit(PSGPSampleTexture) \
	//visit(DebugSetMute)		// <---  Shared with d3d8.dll

class d3d9_dll
{
public:
	d3d9_dll() { };
	~d3d9_dll() { };

	void Load();

	HMODULE dll = nullptr;

private:
	VISIT_D3D9_PROCS(ADD_FARPROC_MEMBER);
};

extern d3d9_dll d3d9;
