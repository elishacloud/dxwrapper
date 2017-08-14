#pragma once

#define VISIT_DINPUT_PROCS(visit) \
	visit(DirectInputCreateA) \
	visit(DirectInputCreateEx) \
	visit(DirectInputCreateW) \
	visit(DllRegisterServer) \
	visit(DllUnregisterServer) \
	//visit(DllCanUnloadNow) \		 // <---  Shared with dsound.dll
	//visit(DllGetClassObject)		 // <---  Shared with dsound.dll

namespace dinput
{
	class dinput_dll
	{
	public:
		HMODULE dll = nullptr;
		VISIT_DINPUT_PROCS(ADD_FARPROC_MEMBER);

		void Load()
		{
			if (Config.WrapperMode != dtype.dinput && Config.WrapperMode != dtype.Auto)
			{
				return;
			}

			// Load real dll
			dll = LoadDll(dtype.dinput);

			// Load dll functions
			if (dll)
			{
				VISIT_DINPUT_PROCS(LOAD_ORIGINAL_PROC);
				dsound::module.DllCanUnloadNow = GetProcAddress(dll, "DllCanUnloadNow", jmpaddr);			 // <---  Shared with dsound.dll
				dsound::module.DllGetClassObject = GetProcAddress(dll, "DllGetClassObject", jmpaddr);		 // <---  Shared with dsound.dll
			}
		}

		void Unhook() {}
	};

	extern dinput_dll module;
}
