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
					DirectSoundCreate = GetFunctionAddress(hModule_dll, "_DirectSoundCreate", jmpaddr);
					DirectSoundEnumerateA = GetFunctionAddress(hModule_dll, "_DirectSoundEnumerateA", jmpaddr);
					DirectSoundEnumerateW = GetFunctionAddress(hModule_dll, "_DirectSoundEnumerateW", jmpaddr);
					DllCanUnloadNow = GetFunctionAddress(hModule_dll, "_DllCanUnloadNow_DSoundCtrl", jmpaddr);
					DllGetClassObject = GetFunctionAddress(hModule_dll, "_DllGetClassObject_DSoundCtrl", jmpaddr);
					DirectSoundCaptureCreate = GetFunctionAddress(hModule_dll, "_DirectSoundCaptureCreate", jmpaddr);
					DirectSoundCaptureEnumerateA = GetFunctionAddress(hModule_dll, "_DirectSoundCaptureEnumerateA", jmpaddr);
					DirectSoundCaptureEnumerateW = GetFunctionAddress(hModule_dll, "_DirectSoundCaptureEnumerateW", jmpaddr);
					GetDeviceID = GetFunctionAddress(hModule_dll, "_GetDeviceID", jmpaddr);
					DirectSoundFullDuplexCreate = GetFunctionAddress(hModule_dll, "_DirectSoundFullDuplexCreate", jmpaddr);
					DirectSoundCreate8 = GetFunctionAddress(hModule_dll, "_DirectSoundCreate8", jmpaddr);
					DirectSoundCaptureCreate8 = GetFunctionAddress(hModule_dll, "_DirectSoundCaptureCreate8", jmpaddr);
				}

				// Hook APIs for DSoundCtrl functions
				else if (Config.DSoundCtrl)
				{
					LOG << "Hooking dll APIs...";
					DirectSoundCreate = (FARPROC)HookAPI(hModule_dll, dtypename[dtype.dsound], GetFunctionAddress(dll, "DirectSoundCreate"), "DirectSoundCreate", GetFunctionAddress(hModule_dll, "_DirectSoundCreate"));
					DirectSoundEnumerateA = (FARPROC)HookAPI(hModule_dll, dtypename[dtype.dsound], GetFunctionAddress(dll, "DirectSoundEnumerateA"), "DirectSoundEnumerateA", GetFunctionAddress(hModule_dll, "_DirectSoundEnumerateA"));
					DirectSoundEnumerateW = (FARPROC)HookAPI(hModule_dll, dtypename[dtype.dsound], GetFunctionAddress(dll, "DirectSoundEnumerateW"), "DirectSoundEnumerateW", GetFunctionAddress(hModule_dll, "_DirectSoundEnumerateW"));
					DirectSoundCaptureCreate = (FARPROC)HookAPI(hModule_dll, dtypename[dtype.dsound], GetFunctionAddress(dll, "DirectSoundCaptureCreate"), "DirectSoundCaptureCreate", GetFunctionAddress(hModule_dll, "_DirectSoundCaptureCreate"));
					DirectSoundCaptureEnumerateA = (FARPROC)HookAPI(hModule_dll, dtypename[dtype.dsound], GetFunctionAddress(dll, "DirectSoundCaptureEnumerateA"), "DirectSoundCaptureEnumerateA", GetFunctionAddress(hModule_dll, "_DirectSoundCaptureEnumerateA"));
					DirectSoundCaptureEnumerateW = (FARPROC)HookAPI(hModule_dll, dtypename[dtype.dsound], GetFunctionAddress(dll, "DirectSoundCaptureEnumerateW"), "DirectSoundCaptureEnumerateW", GetFunctionAddress(hModule_dll, "_DirectSoundCaptureEnumerateW"));
					GetDeviceID = (FARPROC)HookAPI(hModule_dll, dtypename[dtype.dsound], GetFunctionAddress(dll, "GetDeviceID"), "GetDeviceID", GetFunctionAddress(hModule_dll, "_GetDeviceID"));
					DirectSoundFullDuplexCreate = (FARPROC)HookAPI(hModule_dll, dtypename[dtype.dsound], GetFunctionAddress(dll, "DirectSoundFullDuplexCreate"), "DirectSoundFullDuplexCreate", GetFunctionAddress(hModule_dll, "_DirectSoundFullDuplexCreate"));
					DirectSoundCreate8 = (FARPROC)HookAPI(hModule_dll, dtypename[dtype.dsound], GetFunctionAddress(dll, "DirectSoundCreate8"), "DirectSoundCreate8", GetFunctionAddress(hModule_dll, "_DirectSoundCreate8"));
					DirectSoundCaptureCreate8 = (FARPROC)HookAPI(hModule_dll, dtypename[dtype.dsound], GetFunctionAddress(dll, "DirectSoundCaptureCreate8"), "DirectSoundCaptureCreate8", GetFunctionAddress(hModule_dll, "_DirectSoundCaptureCreate8"));
				}
			}
		}
		HMODULE dll = nullptr;
		VISIT_DSOUND_PROCS(ADD_FARPROC_MEMBER);
	};

	extern dsound_dll module;
}
