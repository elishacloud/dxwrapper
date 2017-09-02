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
			if (Config.WrapperMode != dtype.ddraw && Config.WrapperMode != dtype.Auto && !Config.DDrawCompat)
			{
				return;
			}

			// Load real dll
			dll = LoadDll(dtype.ddraw);

			// Load dll functions
			if (dll)
			{
				VISIT_DDRAW_PROCS(LOAD_ORIGINAL_PROC);
				dsound::module.DllCanUnloadNow = GetProcAddress(dll, "DllCanUnloadNow", jmpaddr);			 // <---  Shared with dsound.dll
				dsound::module.DllGetClassObject = GetProcAddress(dll, "DllGetClassObject", jmpaddr);		 // <---  Shared with dsound.dll

				// Enable DDrawCompat
				if (Config.DDrawCompat && Config.RealWrapperMode == dtype.ddraw)
				{
					DirectDrawCreate = DDrawCompat::_DirectDrawCreate;
					DirectDrawCreateEx = DDrawCompat::_DirectDrawCreateEx;
					dsound::module.DllGetClassObject = DDrawCompat::_DllGetClassObject;
				}

				// Hook ddraw APIs for DDrawCompat
				else if (Config.DDrawCompat)
				{
					Logging::Log() << "Hooking ddraw.dll APIs...";
					// DirectDrawCreate
					DirectDrawCreate = (FARPROC)Hook::HookAPI(hModule_dll, dtypename[dtype.ddraw], Hook::GetProcAddress(dll, "DirectDrawCreate"), "DirectDrawCreate", DDrawCompat::_DirectDrawCreate);
					// DirectDrawCreateEx
					DirectDrawCreateEx = (FARPROC)Hook::HookAPI(hModule_dll, dtypename[dtype.ddraw], Hook::GetProcAddress(dll, "DirectDrawCreateEx"), "DirectDrawCreateEx", DDrawCompat::_DirectDrawCreateEx);
					// DllGetClassObject
					dsound::module.DllGetClassObject = (FARPROC)Hook::HookAPI(hModule_dll, dtypename[dtype.ddraw], Hook::GetProcAddress(dll, "DllGetClassObject"), "DllGetClassObject", DDrawCompat::_DllGetClassObject);
				}
			}
		}
	};

	extern ddraw_dll module;
}
