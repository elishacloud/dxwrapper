#pragma once

// All procs
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

// DSoundCtrl procs
#define VISIT_DSOUNDCTR_PROCS(visit) \
	visit(DirectSoundCreate) \
	visit(DirectSoundEnumerateA) \
	visit(DirectSoundEnumerateW) \
	visit(DirectSoundCaptureCreate) \
	visit(DirectSoundCaptureEnumerateA) \
	visit(DirectSoundCaptureEnumerateW) \
	visit(GetDeviceID) \
	visit(DirectSoundFullDuplexCreate) \
	visit(DirectSoundCreate8) \
	visit(DirectSoundCaptureCreate8)

#define HOOK_ALL_PROC(apiname) \
	apiname = (FARPROC)Hook::HookAPI(hModule_dll, dtypename[dtype.dsound], Hook::GetProcAddress(dll, #apiname), #apiname, DSoundCtrl::_ ## apiname);

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
					Logging::Log() << "Hooking dsound.dll APIs...";
					VISIT_DSOUNDCTR_PROCS(HOOK_ALL_PROC);
				}
			}
		}
	};

	extern dsound_dll module;
}
