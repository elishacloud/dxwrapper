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
					IsHooked = true;
					Logging::Log() << "Hooking ddraw.dll APIs...";
					// DirectDrawCreate
					h_DirectDrawCreate.apiproc = Hook::GetFunctionAddress(dll, "DirectDrawCreate");
					h_DirectDrawCreate.hookproc = DDrawCompat::_DirectDrawCreate;
					DirectDrawCreate = (FARPROC)Hook::HookAPI(hModule_dll, dtypename[dtype.ddraw], h_DirectDrawCreate.apiproc, "DirectDrawCreate", h_DirectDrawCreate.hookproc);
					// DirectDrawCreateEx
					h_DirectDrawCreateEx.apiproc = Hook::GetFunctionAddress(dll, "DirectDrawCreateEx");
					h_DirectDrawCreateEx.hookproc = DDrawCompat::_DirectDrawCreateEx;
					DirectDrawCreateEx = (FARPROC)Hook::HookAPI(hModule_dll, dtypename[dtype.ddraw], h_DirectDrawCreateEx.apiproc, "DirectDrawCreateEx", h_DirectDrawCreateEx.hookproc);
					// DllGetClassObject
					h_DllGetClassObject.apiproc = Hook::GetFunctionAddress(dll, "DllGetClassObject");
					h_DllGetClassObject.hookproc = DDrawCompat::_DllGetClassObject;
					dsound::module.DllGetClassObject = (FARPROC)Hook::HookAPI(hModule_dll, dtypename[dtype.ddraw], h_DllGetClassObject.apiproc, "DllGetClassObject", h_DllGetClassObject.hookproc);
				}
			}
		}

		void Unhook()
		{
			// If hooks are installed
			if (IsHooked)
			{
				IsHooked = false;
				Logging::Log() << "Unhooking ddraw.dll APIs...";
				Hook::UnhookAPI(hModule_dll, dtypename[dtype.ddraw], h_DirectDrawCreate.apiproc, "DirectDrawCreate", h_DirectDrawCreate.hookproc);
				Hook::UnhookAPI(hModule_dll, dtypename[dtype.ddraw], h_DirectDrawCreateEx.apiproc, "DirectDrawCreateEx", h_DirectDrawCreateEx.hookproc);
				Hook::UnhookAPI(hModule_dll, dtypename[dtype.ddraw], h_DllGetClassObject.apiproc, "DllGetClassObject", h_DllGetClassObject.hookproc);
			}
		}

	private:
		bool IsHooked = false;
		Hook::HOOKVARS h_DirectDrawCreate;
		Hook::HOOKVARS h_DirectDrawCreateEx;
		Hook::HOOKVARS h_DllGetClassObject;
	};

	extern ddraw_dll module;
}
