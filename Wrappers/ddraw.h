#pragma once

#define VISIT_DDRAW_PROCS(visit) \
	visit(AcquireDDThreadLock) \
	visit(CheckFullscreen) \
	visit(CompleteCreateSysmemSurface) \
	visit(D3DParseUnknownCommand) \
	visit(DDGetAttachedSurfaceLcl) \
	visit(DDInternalLock) \
	visit(DDInternalUnlock) \
	visit(DSoundHelp) \
	visit(DirectDrawCreate) \
	visit(DirectDrawCreateClipper) \
	visit(DirectDrawCreateEx) \
	visit(DirectDrawEnumerateA) \
	visit(DirectDrawEnumerateExA) \
	visit(DirectDrawEnumerateExW) \
	visit(DirectDrawEnumerateW) \
	visit(GetDDSurfaceLocal) \
	visit(GetOLEThunkData) \
	visit(GetSurfaceFromDC) \
	visit(RegisterSpecialCase) \
	visit(ReleaseDDThreadLock) \
	visit(SetAppCompatData) \
	//visit(DllCanUnloadNow) \		 // <---  Shared with dsound.dll
	//visit(DllGetClassObject)		 // <---  Shared with dsound.dll

namespace ddraw
{
	class ddraw_dll
	{
	public:
		void Load()
		{
			// Load real dll
			dll = Wrapper.LoadDll(dtype.ddraw);

			// Load dll functions
			if (dll)
			{
				VISIT_DDRAW_PROCS(LOAD_ORIGINAL_PROC);
				dsound::module.DllCanUnloadNow = GetFunctionAddress(dll, "DllCanUnloadNow", jmpaddr);			 // <---  Shared with dsound.dll
				dsound::module.DllGetClassObject = GetFunctionAddress(dll, "DllGetClassObject", jmpaddr);		 // <---  Shared with dsound.dll

				// Enable DDrawCompat
				if (Config.DDrawCompat && Config.RealWrapperMode == dtype.ddraw)
				{
					DirectDrawCreate = GetFunctionAddress(hModule_dll, "_DirectDrawCreate", jmpaddr);
					DirectDrawCreateEx = GetFunctionAddress(hModule_dll, "_DirectDrawCreateEx", jmpaddr);
				}

				// Hook ddraw APIs for DDrawCompat
				else if (Config.DDrawCompat)
				{
					LOG << "Hooking ddraw.dll APIs...";
					DirectDrawCreate = (FARPROC)HookAPI(hModule_dll, dtypename[dtype.ddraw], GetFunctionAddress(dll, "DirectDrawCreate"), "DirectDrawCreate", GetFunctionAddress(hModule_dll, "_DirectDrawCreate"));
					DirectDrawCreateEx = (FARPROC)HookAPI(hModule_dll, dtypename[dtype.ddraw], GetFunctionAddress(dll, "DirectDrawCreateEx"), "DirectDrawCreateEx", GetFunctionAddress(hModule_dll, "_DirectDrawCreateEx"));
				}
			}
		}

		HMODULE dll = nullptr;
		VISIT_DDRAW_PROCS(ADD_FARPROC_MEMBER);
	};

	extern ddraw_dll module;
}
