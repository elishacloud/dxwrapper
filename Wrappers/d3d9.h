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
		HMODULE dll = nullptr;
		VISIT_D3D9_PROCS(ADD_FARPROC_MEMBER);

		void Load()
		{
			if (Config.WrapperMode != dtype.d3d9 && Config.WrapperMode != dtype.Auto && !Config.D3d8to9)
			{
				return;
			}

			// Load real dll
			dll = Wrapper::LoadDll(dtype.d3d9);

			// Load dll functions
			if (dll)
			{
				VISIT_D3D9_PROCS(LOAD_ORIGINAL_PROC);
				d3d8::module.DebugSetMute = Wrapper::GetProcAddress(dll, "DebugSetMute", jmpaddr);		// <---  Shared with d3d8.dll

				// Enable debug logging
				if (Config.WrapperLogging)
				{
					Wrapper::Direct3DCreate9_Proc = Direct3DCreate9;
					Direct3DCreate9 = Wrapper::Direct3DCreate9_Logging;
					if (Config.RealWrapperMode != dtype.d3d9)
					{
						IsHooked = true;
						Logging::Log() << "Hooking d3d9.dll APIs...";
						// Direct3DCreate9
						h_Direct3DCreate9.apiproc = Hook::GetFunctionAddress(dll, "Direct3DCreate9");
						h_Direct3DCreate9.hookproc = Wrapper::Direct3DCreate9_Logging;
						Wrapper::Direct3DCreate9_Proc = (FARPROC)Hook::HookAPI(hModule_dll, dtypename[dtype.d3d9], h_Direct3DCreate9.apiproc, "Direct3DCreate9", h_Direct3DCreate9.hookproc);
					}
				}
			}
		}

		void Unhook()
		{
			// If hooks are installed
			if (IsHooked)
			{
				IsHooked = false;
				Logging::Log() << "Unhooking d3d9.dll APIs...";
				Hook::UnhookAPI(hModule_dll, dtypename[dtype.d3d9], h_Direct3DCreate9.apiproc, "Direct3DCreate9", h_Direct3DCreate9.hookproc);
			}
		}

	private:
		bool IsHooked = false;
		Hook::HOOKVARS h_Direct3DCreate9;
	};

	extern d3d9_dll module;
}
