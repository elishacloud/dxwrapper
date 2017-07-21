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

namespace d3d9
{
	class d3d9_dll
	{
	public:
		void Load()
		{
			// Load real dll
			dll = Wrapper.LoadDll(dtype.d3d9);

			// Load dll functions
			if (dll)
			{
				VISIT_D3D9_PROCS(LOAD_ORIGINAL_PROC);
				d3d8::module.DebugSetMute = GetFunctionAddress(dll, "DebugSetMute", jmpaddr);		// <---  Shared with d3d8.dll
			}
		}
		HMODULE dll = nullptr;
		VISIT_D3D9_PROCS(ADD_FARPROC_MEMBER);
	};

	extern d3d9_dll module;
}
