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
			if (Config.WrapperMode != dtype.dsound && Config.WrapperMode != dtype.Auto && !Config.DSoundCtrl)
			{
				return;
			}

			// Load real dll
			dll = LoadDll(dtype.dsound);

			// Load dll functions
			if (dll)
			{
				VISIT_DSOUND_PROCS(LOAD_ORIGINAL_PROC);

				// Enable DSoundCtrl functions
				if (Config.DSoundCtrl && Config.RealWrapperMode == dtype.dsound)
				{
					DirectSoundCreate = DSoundCtrl::_DirectSoundCreate;
					DirectSoundEnumerateA = DSoundCtrl::_DirectSoundEnumerateA;
					DirectSoundEnumerateW = DSoundCtrl::_DirectSoundEnumerateW;
					DllCanUnloadNow = DSoundCtrl::_DllCanUnloadNow;
					DllGetClassObject = DSoundCtrl::_DllGetClassObject;
					DirectSoundCaptureCreate = DSoundCtrl::_DirectSoundCaptureCreate;
					DirectSoundCaptureEnumerateA = DSoundCtrl::_DirectSoundCaptureEnumerateA;
					DirectSoundCaptureEnumerateW = DSoundCtrl::_DirectSoundCaptureEnumerateW;
					GetDeviceID = DSoundCtrl::_GetDeviceID;
					DirectSoundFullDuplexCreate = DSoundCtrl::_DirectSoundFullDuplexCreate;
					DirectSoundCreate8 = DSoundCtrl::_DirectSoundCreate8;
					DirectSoundCaptureCreate8 = DSoundCtrl::_DirectSoundCaptureCreate8;
				}

				// Hook APIs for DSoundCtrl functions
				else if (Config.DSoundCtrl)
				{
					IsHooked = true;
					Logging::Log() << "Hooking dsound.dll APIs...";
					// DirectSoundCreate
					h_DirectSoundCreate.apiproc = Hook::GetFunctionAddress(dll, "DirectSoundCreate");
					h_DirectSoundCreate.hookproc = DSoundCtrl::_DirectSoundCreate;
					DirectSoundCreate = (FARPROC)Hook::HookAPI(hModule_dll, dtypename[dtype.dsound], h_DirectSoundCreate.apiproc, "DirectSoundCreate", h_DirectSoundCreate.hookproc);
					// DirectSoundEnumerateA
					h_DirectSoundEnumerateA.apiproc = Hook::GetFunctionAddress(dll, "DirectSoundEnumerateA");
					h_DirectSoundEnumerateA.hookproc = DSoundCtrl::_DirectSoundEnumerateA;
					DirectSoundEnumerateA = (FARPROC)Hook::HookAPI(hModule_dll, dtypename[dtype.dsound], h_DirectSoundEnumerateA.apiproc, "DirectSoundEnumerateA", h_DirectSoundEnumerateA.hookproc);
					// DirectSoundEnumerateW
					h_DirectSoundEnumerateW.apiproc = Hook::GetFunctionAddress(dll, "DirectSoundEnumerateW");
					h_DirectSoundEnumerateW.hookproc = DSoundCtrl::_DirectSoundEnumerateW;
					DirectSoundEnumerateW = (FARPROC)Hook::HookAPI(hModule_dll, dtypename[dtype.dsound], h_DirectSoundEnumerateW.apiproc, "DirectSoundEnumerateW", h_DirectSoundEnumerateW.hookproc);
					// DirectSoundCaptureCreate
					h_DirectSoundCaptureCreate.apiproc = Hook::GetFunctionAddress(dll, "DirectSoundCaptureCreate");
					h_DirectSoundCaptureCreate.hookproc = DSoundCtrl::_DirectSoundCaptureCreate;
					DirectSoundCaptureCreate = (FARPROC)Hook::HookAPI(hModule_dll, dtypename[dtype.dsound], h_DirectSoundCaptureCreate.apiproc, "DirectSoundCaptureCreate", h_DirectSoundCaptureCreate.hookproc);
					// DirectSoundCaptureEnumerateA
					h_DirectSoundCaptureEnumerateA.apiproc = Hook::GetFunctionAddress(dll, "DirectSoundCaptureEnumerateA");
					h_DirectSoundCaptureEnumerateA.hookproc = DSoundCtrl::_DirectSoundCaptureEnumerateA;
					DirectSoundCaptureEnumerateA = (FARPROC)Hook::HookAPI(hModule_dll, dtypename[dtype.dsound], h_DirectSoundCaptureEnumerateA.apiproc, "DirectSoundCaptureEnumerateA", h_DirectSoundCaptureEnumerateA.hookproc);
					// DirectSoundCaptureEnumerateW
					h_DirectSoundCaptureEnumerateW.apiproc = Hook::GetFunctionAddress(dll, "DirectSoundCaptureEnumerateW");
					h_DirectSoundCaptureEnumerateW.hookproc = DSoundCtrl::_DirectSoundCaptureEnumerateW;
					DirectSoundCaptureEnumerateW = (FARPROC)Hook::HookAPI(hModule_dll, dtypename[dtype.dsound], h_DirectSoundCaptureEnumerateW.apiproc, "DirectSoundCaptureEnumerateW", h_DirectSoundCaptureEnumerateW.hookproc);
					// GetDeviceID
					h_GetDeviceID.apiproc = Hook::GetFunctionAddress(dll, "GetDeviceID");
					h_GetDeviceID.hookproc = DSoundCtrl::_GetDeviceID;
					GetDeviceID = (FARPROC)Hook::HookAPI(hModule_dll, dtypename[dtype.dsound], h_GetDeviceID.apiproc, "GetDeviceID", h_GetDeviceID.hookproc);
					// DirectSoundFullDuplexCreate
					h_DirectSoundFullDuplexCreate.apiproc = Hook::GetFunctionAddress(dll, "DirectSoundFullDuplexCreate");
					h_DirectSoundFullDuplexCreate.hookproc = DSoundCtrl::_DirectSoundFullDuplexCreate;
					DirectSoundFullDuplexCreate = (FARPROC)Hook::HookAPI(hModule_dll, dtypename[dtype.dsound], h_DirectSoundFullDuplexCreate.apiproc, "DirectSoundFullDuplexCreate", h_DirectSoundFullDuplexCreate.hookproc);
					// DirectSoundCreate8
					h_DirectSoundCreate8.apiproc = Hook::GetFunctionAddress(dll, "DirectSoundCreate8");
					h_DirectSoundCreate8.hookproc = DSoundCtrl::_DirectSoundCreate8;
					DirectSoundCreate8 = (FARPROC)Hook::HookAPI(hModule_dll, dtypename[dtype.dsound], h_DirectSoundCreate8.apiproc, "DirectSoundCreate8", h_DirectSoundCreate8.hookproc);
					// DirectSoundCaptureCreate8
					h_DirectSoundCaptureCreate8.apiproc = Hook::GetFunctionAddress(dll, "DirectSoundCaptureCreate8");
					h_DirectSoundCaptureCreate8.hookproc = DSoundCtrl::_DirectSoundCaptureCreate8;
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
