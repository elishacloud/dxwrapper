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
		HMODULE dll = nullptr;
		VISIT_DDRAW_PROCS(ADD_FARPROC_MEMBER);

		void Load()
		{
			// Load real dll
			dll = Wrapper::LoadDll(dtype.ddraw);

			// Load dll functions
			if (dll)
			{
				VISIT_DDRAW_PROCS(LOAD_ORIGINAL_PROC);
				dsound::module.DllCanUnloadNow = Wrapper::GetProcAddress(dll, "DllCanUnloadNow", jmpaddr);			 // <---  Shared with dsound.dll
				dsound::module.DllGetClassObject = Wrapper::GetProcAddress(dll, "DllGetClassObject", jmpaddr);		 // <---  Shared with dsound.dll

				// Enable DDrawCompat
				if (Config.DDrawCompat && Config.RealWrapperMode == dtype.ddraw)
				{
					DirectDrawCreate = Wrapper::GetProcAddress(hModule_dll, "_DirectDrawCreate", jmpaddr);
					DirectDrawCreateEx = Wrapper::GetProcAddress(hModule_dll, "_DirectDrawCreateEx", jmpaddr);
				}

				// Hook ddraw APIs for DDrawCompat
				else if (Config.DDrawCompat)
				{
					Logging::Log() << "Hooking ddraw.dll APIs...";
					DirectDrawCreate = (FARPROC)Hook::HookAPI(hModule_dll, dtypename[dtype.ddraw], Hook::GetFunctionAddress(dll, "DirectDrawCreate"), "DirectDrawCreate", Hook::GetFunctionAddress(hModule_dll, "_DirectDrawCreate"));
					DirectDrawCreateEx = (FARPROC)Hook::HookAPI(hModule_dll, dtypename[dtype.ddraw], Hook::GetFunctionAddress(dll, "DirectDrawCreateEx"), "DirectDrawCreateEx", Hook::GetFunctionAddress(hModule_dll, "_DirectDrawCreateEx"));
				}
			}
		}
	};

	extern ddraw_dll module;
}
