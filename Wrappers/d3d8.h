#pragma once

#define VISIT_D3D8_PROCS(visit) \
	visit(Direct3DCreate8) \
	visit(ValidateVertexShader) \
	visit(ValidatePixelShader) \
	visit(DebugSetMute)

namespace d3d8
{
	class d3d8_dll
	{
	public:
		HMODULE dll = nullptr;
		VISIT_D3D8_PROCS(ADD_FARPROC_MEMBER);

		void Load()
		{
			if (Config.WrapperMode != dtype.d3d8 && Config.WrapperMode != dtype.Auto && !Config.D3d8to9)
			{
				return;
			}

			// Enable d3d8to9 conversion
			if (Config.D3d8to9 && Config.RealWrapperMode == dtype.d3d8)
			{
				Direct3DCreate8 = D3d8to9::_Direct3DCreate8;
				ValidateVertexShader = (FARPROC)*_ValidateVertexShader;
				ValidatePixelShader = (FARPROC)*_ValidatePixelShader;
			}

			// Load normal dll
			else
			{
				// Load real dll
				dll = LoadDll(dtype.d3d8);

				// Load dll functions
				if (dll)
				{
					VISIT_D3D8_PROCS(LOAD_ORIGINAL_PROC);

					// Hook APIs for d3d8to9 conversion
					if (Config.D3d8to9)
					{
						IsHooked = true;
						Logging::Log() << "Hooking d3d8.dll APIs...";
						// Direct3DCreate8
						h_Direct3DCreate8.apiproc = Hook::GetFunctionAddress(dll, "Direct3DCreate8");
						h_Direct3DCreate8.hookproc = D3d8to9::_Direct3DCreate8;
						Direct3DCreate8 = (FARPROC)Hook::HookAPI(hModule_dll, dtypename[dtype.d3d8], h_Direct3DCreate8.apiproc, "Direct3DCreate8", h_Direct3DCreate8.hookproc);
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
				Logging::Log() << "Unhooking d3d8.dll APIs...";
				Hook::UnhookAPI(hModule_dll, dtypename[dtype.d3d8], h_Direct3DCreate8.apiproc, "Direct3DCreate8", h_Direct3DCreate8.hookproc);
			}
		}

	private:
		bool IsHooked = false;
		Hook::HOOKVARS h_Direct3DCreate8;
	};

	extern d3d8_dll module;
}
