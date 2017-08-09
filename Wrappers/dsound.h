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
					IsHooked = true;
					Logging::Log() << "Hooking dsound.dll APIs...";
					// DirectSoundCreate
					h_DirectSoundCreate.apiproc = Hook::GetFunctionAddress(dll, "DirectSoundCreate");
					h_DirectSoundCreate.hookproc = Hook::GetFunctionAddress(hModule_dll, "_DirectSoundCreate");
					DirectSoundCreate = (FARPROC)Hook::HookAPI(hModule_dll, dtypename[dtype.dsound], h_DirectSoundCreate.apiproc, "DirectSoundCreate", h_DirectSoundCreate.hookproc);
					// DirectSoundEnumerateA
					h_DirectSoundEnumerateA.apiproc = Hook::GetFunctionAddress(dll, "DirectSoundEnumerateA");
					h_DirectSoundEnumerateA.hookproc = Hook::GetFunctionAddress(hModule_dll, "_DirectSoundEnumerateA");
					DirectSoundEnumerateA = (FARPROC)Hook::HookAPI(hModule_dll, dtypename[dtype.dsound], h_DirectSoundEnumerateA.apiproc, "DirectSoundEnumerateA", h_DirectSoundEnumerateA.hookproc);
					// DirectSoundEnumerateW
					h_DirectSoundEnumerateW.apiproc = Hook::GetFunctionAddress(dll, "DirectSoundEnumerateW");
					h_DirectSoundEnumerateW.hookproc = Hook::GetFunctionAddress(hModule_dll, "_DirectSoundEnumerateW");
					DirectSoundEnumerateW = (FARPROC)Hook::HookAPI(hModule_dll, dtypename[dtype.dsound], h_DirectSoundEnumerateW.apiproc, "DirectSoundEnumerateW", h_DirectSoundEnumerateW.hookproc);
					// DirectSoundCaptureCreate
					h_DirectSoundCaptureCreate.apiproc = Hook::GetFunctionAddress(dll, "DirectSoundCaptureCreate");
					h_DirectSoundCaptureCreate.hookproc = Hook::GetFunctionAddress(hModule_dll, "_DirectSoundCaptureCreate");
					DirectSoundCaptureCreate = (FARPROC)Hook::HookAPI(hModule_dll, dtypename[dtype.dsound], h_DirectSoundCaptureCreate.apiproc, "DirectSoundCaptureCreate", h_DirectSoundCaptureCreate.hookproc);
					// DirectSoundCaptureEnumerateA
					h_DirectSoundCaptureEnumerateA.apiproc = Hook::GetFunctionAddress(dll, "DirectSoundCaptureEnumerateA");
					h_DirectSoundCaptureEnumerateA.hookproc = Hook::GetFunctionAddress(hModule_dll, "_DirectSoundCaptureEnumerateA");
					DirectSoundCaptureEnumerateA = (FARPROC)Hook::HookAPI(hModule_dll, dtypename[dtype.dsound], h_DirectSoundCaptureEnumerateA.apiproc, "DirectSoundCaptureEnumerateA", h_DirectSoundCaptureEnumerateA.hookproc);
					// DirectSoundCaptureEnumerateW
					h_DirectSoundCaptureEnumerateW.apiproc = Hook::GetFunctionAddress(dll, "DirectSoundCaptureEnumerateW");
					h_DirectSoundCaptureEnumerateW.hookproc = Hook::GetFunctionAddress(hModule_dll, "_DirectSoundCaptureEnumerateW");
					DirectSoundCaptureEnumerateW = (FARPROC)Hook::HookAPI(hModule_dll, dtypename[dtype.dsound], h_DirectSoundCaptureEnumerateW.apiproc, "DirectSoundCaptureEnumerateW", h_DirectSoundCaptureEnumerateW.hookproc);
					// GetDeviceID
					h_GetDeviceID.apiproc = Hook::GetFunctionAddress(dll, "GetDeviceID");
					h_GetDeviceID.hookproc = Hook::GetFunctionAddress(hModule_dll, "_GetDeviceID");
					GetDeviceID = (FARPROC)Hook::HookAPI(hModule_dll, dtypename[dtype.dsound], h_GetDeviceID.apiproc, "GetDeviceID", h_GetDeviceID.hookproc);
					// DirectSoundFullDuplexCreate
					h_DirectSoundFullDuplexCreate.apiproc = Hook::GetFunctionAddress(dll, "DirectSoundFullDuplexCreate");
					h_DirectSoundFullDuplexCreate.hookproc = Hook::GetFunctionAddress(hModule_dll, "_DirectSoundFullDuplexCreate");
					DirectSoundFullDuplexCreate = (FARPROC)Hook::HookAPI(hModule_dll, dtypename[dtype.dsound], h_DirectSoundFullDuplexCreate.apiproc, "DirectSoundFullDuplexCreate", h_DirectSoundFullDuplexCreate.hookproc);
					// DirectSoundCreate8
					h_DirectSoundCreate8.apiproc = Hook::GetFunctionAddress(dll, "DirectSoundCreate8");
					h_DirectSoundCreate8.hookproc = Hook::GetFunctionAddress(hModule_dll, "_DirectSoundCreate8");
					DirectSoundCreate8 = (FARPROC)Hook::HookAPI(hModule_dll, dtypename[dtype.dsound], h_DirectSoundCreate8.apiproc, "DirectSoundCreate8", h_DirectSoundCreate8.hookproc);
					// DirectSoundCaptureCreate8
					h_DirectSoundCaptureCreate8.apiproc = Hook::GetFunctionAddress(dll, "DirectSoundCaptureCreate8");
					h_DirectSoundCaptureCreate8.hookproc = Hook::GetFunctionAddress(hModule_dll, "_DirectSoundCaptureCreate8");
					DirectSoundCaptureCreate8 = (FARPROC)Hook::HookAPI(hModule_dll, dtypename[dtype.dsound], h_DirectSoundCaptureCreate8.apiproc, "DirectSoundCaptureCreate8", h_DirectSoundCaptureCreate8.hookproc);
				}
			}
		}

		void Unhook()
		{
			// If hooks are installed
			if (IsHooked)
			{
				IsHooked = false;
				Logging::Log() << "Unhooking dsound.dll APIs...";
				Hook::UnhookAPI(hModule_dll, dtypename[dtype.dsound], h_DirectSoundCreate.apiproc, "DirectSoundCreate", h_DirectSoundCreate.hookproc);
				Hook::UnhookAPI(hModule_dll, dtypename[dtype.dsound], h_DirectSoundEnumerateA.apiproc, "DirectSoundEnumerateA", h_DirectSoundEnumerateA.hookproc);
				Hook::UnhookAPI(hModule_dll, dtypename[dtype.dsound], h_DirectSoundEnumerateW.apiproc, "DirectSoundEnumerateW", h_DirectSoundEnumerateW.hookproc);
				Hook::UnhookAPI(hModule_dll, dtypename[dtype.dsound], h_DirectSoundCaptureCreate.apiproc, "DirectSoundCaptureCreate", h_DirectSoundCaptureCreate.hookproc);
				Hook::UnhookAPI(hModule_dll, dtypename[dtype.dsound], h_DirectSoundCaptureEnumerateA.apiproc, "DirectSoundCaptureEnumerateA", h_DirectSoundCaptureEnumerateA.hookproc);
				Hook::UnhookAPI(hModule_dll, dtypename[dtype.dsound], h_DirectSoundCaptureEnumerateW.apiproc, "DirectSoundCaptureEnumerateW", h_DirectSoundCaptureEnumerateW.hookproc);
				Hook::UnhookAPI(hModule_dll, dtypename[dtype.dsound], h_GetDeviceID.apiproc, "GetDeviceID", h_GetDeviceID.hookproc);
				Hook::UnhookAPI(hModule_dll, dtypename[dtype.dsound], h_DirectSoundFullDuplexCreate.apiproc, "DirectSoundFullDuplexCreate", h_DirectSoundFullDuplexCreate.hookproc);
				Hook::UnhookAPI(hModule_dll, dtypename[dtype.dsound], h_DirectSoundCreate8.apiproc, "DirectSoundCreate8", h_DirectSoundCreate8.hookproc);
				Hook::UnhookAPI(hModule_dll, dtypename[dtype.dsound], h_DirectSoundCaptureCreate8.apiproc, "DirectSoundCaptureCreate8", h_DirectSoundCaptureCreate8.hookproc);
			}
		}

	private:
		bool IsHooked = false;
		Hook::HOOKVARS h_DirectSoundCreate;
		Hook::HOOKVARS h_DirectSoundEnumerateA;
		Hook::HOOKVARS h_DirectSoundEnumerateW;
		Hook::HOOKVARS h_DirectSoundCaptureCreate;
		Hook::HOOKVARS h_DirectSoundCaptureEnumerateA;
		Hook::HOOKVARS h_DirectSoundCaptureEnumerateW;
		Hook::HOOKVARS h_GetDeviceID;
		Hook::HOOKVARS h_DirectSoundFullDuplexCreate;
		Hook::HOOKVARS h_DirectSoundCreate8;
		Hook::HOOKVARS h_DirectSoundCaptureCreate8;
	};

	extern dsound_dll module;
}
