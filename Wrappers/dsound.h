#pragma once

#define VISIT_DSOUND_PROCS(visit) \
	visit(DirectSoundCreate) \
	visit(DirectSoundEnumerateA) \
	visit(DirectSoundEnumerateW) \
	visit(DllCanUnloadNow) \
	visit(DllGetClassObject) \
	visit(DirectSoundCaptureCreate) \
	visit(DirectSoundCaptureEnumerateA) \
	visit(DirectSoundCaptureEnumerateW) \
	visit(GetDeviceID) \
	visit(DirectSoundFullDuplexCreate) \
	visit(DirectSoundCreate8) \
	visit(DirectSoundCaptureCreate8)

namespace dsound
{
	class dsound_dll
	{
	public:
		HMODULE dll = nullptr;
		VISIT_DSOUND_PROCS(ADD_FARPROC_MEMBER);

		void Load()
		{
			// Load real dll
			dll = Wrapper::LoadDll(dtype.dsound);

			// Load dll functions
			if (dll)
			{
				VISIT_DSOUND_PROCS(LOAD_ORIGINAL_PROC);

				// Enable DSoundCtrl functions
				if (Config.DSoundCtrl && Config.RealWrapperMode == dtype.dsound)
				{
					DirectSoundCreate = Wrapper::GetProcAddress(hModule_dll, "_DirectSoundCreate", jmpaddr);
					DirectSoundEnumerateA = Wrapper::GetProcAddress(hModule_dll, "_DirectSoundEnumerateA", jmpaddr);
					DirectSoundEnumerateW = Wrapper::GetProcAddress(hModule_dll, "_DirectSoundEnumerateW", jmpaddr);
					DllCanUnloadNow = Wrapper::GetProcAddress(hModule_dll, "_DllCanUnloadNow_DSoundCtrl", jmpaddr);
					DllGetClassObject = Wrapper::GetProcAddress(hModule_dll, "_DllGetClassObject_DSoundCtrl", jmpaddr);
					DirectSoundCaptureCreate = Wrapper::GetProcAddress(hModule_dll, "_DirectSoundCaptureCreate", jmpaddr);
					DirectSoundCaptureEnumerateA = Wrapper::GetProcAddress(hModule_dll, "_DirectSoundCaptureEnumerateA", jmpaddr);
					DirectSoundCaptureEnumerateW = Wrapper::GetProcAddress(hModule_dll, "_DirectSoundCaptureEnumerateW", jmpaddr);
					GetDeviceID = Wrapper::GetProcAddress(hModule_dll, "_GetDeviceID", jmpaddr);
					DirectSoundFullDuplexCreate = Wrapper::GetProcAddress(hModule_dll, "_DirectSoundFullDuplexCreate", jmpaddr);
					DirectSoundCreate8 = Wrapper::GetProcAddress(hModule_dll, "_DirectSoundCreate8", jmpaddr);
					DirectSoundCaptureCreate8 = Wrapper::GetProcAddress(hModule_dll, "_DirectSoundCaptureCreate8", jmpaddr);
				}

				// Hook APIs for DSoundCtrl functions
				else if (Config.DSoundCtrl)
				{
					Logging::Log() << "Hooking dll APIs...";
					DirectSoundCreate = (FARPROC)Hook::HookAPI(hModule_dll, dtypename[dtype.dsound], Hook::GetFunctionAddress(dll, "DirectSoundCreate"), "DirectSoundCreate", Hook::GetFunctionAddress(hModule_dll, "_DirectSoundCreate"));
					DirectSoundEnumerateA = (FARPROC)Hook::HookAPI(hModule_dll, dtypename[dtype.dsound], Hook::GetFunctionAddress(dll, "DirectSoundEnumerateA"), "DirectSoundEnumerateA", Hook::GetFunctionAddress(hModule_dll, "_DirectSoundEnumerateA"));
					DirectSoundEnumerateW = (FARPROC)Hook::HookAPI(hModule_dll, dtypename[dtype.dsound], Hook::GetFunctionAddress(dll, "DirectSoundEnumerateW"), "DirectSoundEnumerateW", Hook::GetFunctionAddress(hModule_dll, "_DirectSoundEnumerateW"));
					DirectSoundCaptureCreate = (FARPROC)Hook::HookAPI(hModule_dll, dtypename[dtype.dsound], Hook::GetFunctionAddress(dll, "DirectSoundCaptureCreate"), "DirectSoundCaptureCreate", Hook::GetFunctionAddress(hModule_dll, "_DirectSoundCaptureCreate"));
					DirectSoundCaptureEnumerateA = (FARPROC)Hook::HookAPI(hModule_dll, dtypename[dtype.dsound], Hook::GetFunctionAddress(dll, "DirectSoundCaptureEnumerateA"), "DirectSoundCaptureEnumerateA", Hook::GetFunctionAddress(hModule_dll, "_DirectSoundCaptureEnumerateA"));
					DirectSoundCaptureEnumerateW = (FARPROC)Hook::HookAPI(hModule_dll, dtypename[dtype.dsound], Hook::GetFunctionAddress(dll, "DirectSoundCaptureEnumerateW"), "DirectSoundCaptureEnumerateW", Hook::GetFunctionAddress(hModule_dll, "_DirectSoundCaptureEnumerateW"));
					GetDeviceID = (FARPROC)Hook::HookAPI(hModule_dll, dtypename[dtype.dsound], Hook::GetFunctionAddress(dll, "GetDeviceID"), "GetDeviceID", Hook::GetFunctionAddress(hModule_dll, "_GetDeviceID"));
					DirectSoundFullDuplexCreate = (FARPROC)Hook::HookAPI(hModule_dll, dtypename[dtype.dsound], Hook::GetFunctionAddress(dll, "DirectSoundFullDuplexCreate"), "DirectSoundFullDuplexCreate", Hook::GetFunctionAddress(hModule_dll, "_DirectSoundFullDuplexCreate"));
					DirectSoundCreate8 = (FARPROC)Hook::HookAPI(hModule_dll, dtypename[dtype.dsound], Hook::GetFunctionAddress(dll, "DirectSoundCreate8"), "DirectSoundCreate8", Hook::GetFunctionAddress(hModule_dll, "_DirectSoundCreate8"));
					DirectSoundCaptureCreate8 = (FARPROC)Hook::HookAPI(hModule_dll, dtypename[dtype.dsound], Hook::GetFunctionAddress(dll, "DirectSoundCaptureCreate8"), "DirectSoundCaptureCreate8", Hook::GetFunctionAddress(hModule_dll, "_DirectSoundCaptureCreate8"));
				}
			}
		}
	};

	extern dsound_dll module;
}
