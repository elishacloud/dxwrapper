/**
* Copyright (C) 2019 Elisha Riedlinger
*
* This software is  provided 'as-is', without any express  or implied  warranty. In no event will the
* authors be held liable for any damages arising from the use of this software.
* Permission  is granted  to anyone  to use  this software  for  any  purpose,  including  commercial
* applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*   1. The origin of this software must not be misrepresented; you must not claim that you  wrote the
*      original  software. If you use this  software  in a product, an  acknowledgment in the product
*      documentation would be appreciated but is not required.
*   2. Altered source versions must  be plainly  marked as such, and  must not be  misrepresented  as
*      being the original software.
*   3. This notice may not be removed or altered from any source distribution.
*/

#include "Settings\Settings.h"
#include "dxwrapper.h"
#include "Wrappers\wrapper.h"
#include "External\Hooking\Hook.h"
#include "DDrawCompat\DDrawCompatExternal.h"
#include "DxWnd\DxWndExternal.h"
#include "Utils\Utils.h"
#include "Logging\Logging.h"
// Wrappers last
#include "ddraw\ddrawExternal.h"
#include "dinput\dinputExternal.h"
#include "d3d8\d3d8External.h"
#include "d3d9\d3d9External.h"
#include "dsound\dsoundExternal.h"

#define SHIM_WRAPPED_PROC(procName, unused) \
	Wrapper::ShimProc(procName ## _var, procName ## _in, procName ## _out);

#define SET_WRAPPED_PROC(procName, unused) \
	procName ## _var = procName ## _in;

// Declare variables
HMODULE hModule_dll = nullptr;

// Screen settings
HDC hDC;
std::string lpRamp((3 * 256 * 2), '\0');

// Dll main function
bool APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved)
{
	UNREFERENCED_PARAMETER(lpReserved);

	static HANDLE hMutex = nullptr;
	static HANDLE n_hMutex = nullptr;
	static bool FullscreenThreadStartedFlag = false;

	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
	{
		// Get handle
		hModule_dll = hModule;
		HANDLE hCurrentThread = GetCurrentThread();

		// Set thread priority a trick to reduce concurrency problems at program startup
		int dwPriorityClass = GetThreadPriority(hCurrentThread);
		dwPriorityClass = (GetLastError() == THREAD_PRIORITY_ERROR_RETURN) ? THREAD_PRIORITY_NORMAL : dwPriorityClass;
		SetThreadPriority(hCurrentThread, THREAD_PRIORITY_HIGHEST);

		// Initialize config
		Config.Init();

		// Init logs
		Logging::EnableLogging = !Config.DisableLogging;
		Logging::InitLog();
		Logging::Log() << "Starting DxWrapper v" << APP_VERSION;
		Logging::LogComputerManufacturer();
		Logging::LogVideoCard();
		Logging::LogOSVersion();
		Logging::LogProcessNameAndPID();

		// Create Mutex to ensure only one copy of DxWrapper is running
		char MutexName[MAX_PATH] = { 0 };
		sprintf_s(MutexName, MAX_PATH, "DxWrapper %d", GetCurrentProcessId());
		hMutex = CreateMutex(nullptr, false, MutexName);
		bool IsAlreadyRunning = (GetLastError() == ERROR_ALREADY_EXISTS);

		// Allow DxWrapper to be loaded more than once from the same dll
		if (Config.RealWrapperMode != dtype.dxwrapper)
		{
			sprintf_s(MutexName, MAX_PATH, "DxWrapper %d %s", GetCurrentProcessId(), Config.WrapperName.c_str());
			n_hMutex = CreateMutex(nullptr, false, MutexName);
			IsAlreadyRunning = IsAlreadyRunning && (GetLastError() != ERROR_ALREADY_EXISTS);
		}

		// Check Mutex or if process is excluded to see if DxWrapper should exit
		if (IsAlreadyRunning || Config.ProcessExcluded)
		{
			// DxWrapper already running
			if (IsAlreadyRunning)
			{
				Logging::Log() << "DxWrapper already running!";
			}

			if (Config.RealWrapperMode == dtype.dxwrapper)
			{
				// Resetting thread priority
				SetThreadPriority(hCurrentThread, dwPriorityClass);

				// Closing handle
				CloseHandle(hCurrentThread);

				// Return false on process attach causes dll to get unloaded
				return false;
			}
			else
			{
				// Disable wrapper
				Logging::Log() << "Disabling DxWrapper...";
				Settings::ClearConfigSettings();

				// Release named Mutex
				if (n_hMutex)
				{
					ReleaseMutex(n_hMutex);
				}
			}
		}

		// Initialize ddraw wrapper procs
		if (Config.RealWrapperMode == dtype.ddraw || Config.DDrawCompat)
		{
			using namespace ddraw;
			using namespace DdrawWrapper;

			VISIT_PROCS_DDRAW(SET_WRAPPED_PROC);
			ShardProcs::DllCanUnloadNow_var = DllCanUnloadNow_in;
			ShardProcs::DllGetClassObject_var = DllGetClassObject_in;
		}
		// Initialize dinput wrapper procs
		if (Config.RealWrapperMode == dtype.dinput)
		{
			using namespace dinput;
			using namespace DinputWrapper;

			VISIT_PROCS_DINPUT(SET_WRAPPED_PROC);
			ShardProcs::DllCanUnloadNow_var = DllCanUnloadNow_in;
			ShardProcs::DllGetClassObject_var = DllGetClassObject_in;
			ShardProcs::DllRegisterServer_var = DllRegisterServer_in;
			ShardProcs::DllUnregisterServer_var = DllUnregisterServer_in;
		}
		// Initialize d3d8 wrapper procs
		if (Config.RealWrapperMode == dtype.d3d8)
		{
			using namespace d3d8;
			using namespace D3d8Wrapper;

			VISIT_PROCS_D3D8(SET_WRAPPED_PROC);
			ShardProcs::DebugSetMute_var = DebugSetMute_in;
		}
		// Initialize d3d9 wrapper procs
		else if (Config.RealWrapperMode == dtype.d3d9)
		{
			using namespace d3d9;
			using namespace D3d9Wrapper;

			VISIT_PROCS_D3D9(SET_WRAPPED_PROC);
			ShardProcs::DebugSetMute_var = DebugSetMute_in;
		}
		// Initialize dsound wrapper procs
		else if (Config.RealWrapperMode == dtype.dsound)
		{
			using namespace dsound;
			using namespace DsoundWrapper;

			VISIT_PROCS_DSOUND(SET_WRAPPED_PROC);
			ShardProcs::DllCanUnloadNow_var = DllCanUnloadNow_in;
			ShardProcs::DllGetClassObject_var = DllGetClassObject_in;
		}

		// Attach real dll
		if (Config.RealWrapperMode == dtype.dxwrapper)
		{
			// Hook GetModuleFileName to fix module name in modules loaded from memory
			HMODULE dll = LoadLibrary("kernel32.dll");
			if (dll)
			{
				Logging::Log() << "Hooking 'GetModuleFileName' API...";
				InterlockedExchangePointer((PVOID*)&Utils::pGetModuleFileNameA, Hook::HookAPI(dll, "kernel32.dll", Hook::GetProcAddress(dll, "GetModuleFileNameA"), "GetModuleFileNameA", Utils::GetModuleFileNameAHandler));
				InterlockedExchangePointer((PVOID*)&Utils::pGetModuleFileNameW, Hook::HookAPI(dll, "kernel32.dll", Hook::GetProcAddress(dll, "GetModuleFileNameW"), "GetModuleFileNameW", Utils::GetModuleFileNameWHandler));
			}
		}
		else if (!(Config.Dd7to9 && Config.RealWrapperMode == dtype.ddraw) && !(Config.D3d8to9 && Config.RealWrapperMode == dtype.d3d8) && !(Config.Dinputto8 && Config.RealWrapperMode == dtype.dinput))
		{
			// Load real dll and attach wrappers
			HMODULE dll = Wrapper::CreateWrapper((Config.RealDllPath.size()) ? Config.RealDllPath.c_str() : nullptr, (Config.WrapperMode.size()) ? Config.WrapperMode.c_str() : nullptr, Config.WrapperName.c_str());
			if (dll)
			{
				Utils::AddHandleToVector(dll, Config.WrapperName.c_str());

				// Hook GetProcAddress to handle wrapped functions that are missing or not available in the OS
				dll = LoadLibrary("kernel32.dll");
				if (dll)
				{
					Logging::Log() << "Hooking 'GetProcAddress' API...";
					InterlockedExchangePointer((PVOID*)&Utils::pGetProcAddress, Hook::HookAPI(dll, "kernel32.dll", Hook::GetProcAddress(dll, "GetProcAddress"), "GetProcAddress", Utils::GetProcAddressHandler));
				}
			}
		}

		// Launch processes
		if (!Config.RunProcess.empty())
		{
			Utils::Shell(Config.RunProcess.c_str());
		}

		// Set application compatibility options
		if (Config.MemoryInfo.size() != 0)
		{
			Utils::WriteMemory::WriteMemory();
		}
		if (Config.DisableHighDPIScaling)
		{
			Utils::DisableHighDPIScaling();
		}
		Utils::SetAppCompat();
		if (Config.SingleProcAffinity)
		{
			Utils::SetProcessAffinity();
		}
		if (Config.HandleExceptions)
		{
			Utils::HookExceptionHandler();
		}

		// Start ddraw.dll module
		if (Config.DDrawCompat || Config.isDdrawWrapperEnabled)
		{
			// Check wrapper mode
			if (Config.RealWrapperMode != dtype.ddraw)
			{
				// Load ddraw procs
				HMODULE dll = ddraw::Load(nullptr, Config.WrapperName.c_str());
				if (dll)
				{
					Utils::AddHandleToVector(dll, dtypename[dtype.ddraw]);
				}

				// Hook ddraw.dll APIs
				Logging::Log() << "Hooking ddraw.dll APIs...";
				ddraw::DirectDrawCreate_var = (FARPROC)Hook::HookAPI(dll, dtypename[dtype.ddraw], Hook::GetProcAddress(dll, "DirectDrawCreate"), "DirectDrawCreate", (FARPROC)*(ddraw::DirectDrawCreate));
				ddraw::DirectDrawCreateEx_var = (FARPROC)Hook::HookAPI(dll, dtypename[dtype.ddraw], Hook::GetProcAddress(dll, "DirectDrawCreateEx"), "DirectDrawCreateEx", (FARPROC)*(ddraw::DirectDrawCreateEx));
				ShardProcs::DllGetClassObject_var = (FARPROC)Hook::HookAPI(dll, dtypename[dtype.ddraw], Hook::GetProcAddress(dll, "DllGetClassObject"), "DllGetClassObject", (FARPROC)*(ShardProcs::DllGetClassObject));

				// Hook additional procs when Dd7to9 is enabled
				if (Config.Dd7to9)
				{
#define HOOK_WRAPPED_PROC(nameSpace, procName) \
					{ \
						FARPROC prodAddr = (FARPROC)Hook::HotPatch(Hook::GetProcAddress(dll, #procName), #procName, (FARPROC)*(nameSpace::procName), true); \
						if (prodAddr) \
						{ \
							nameSpace::procName ## _var = prodAddr; \
						} \
					}

#define VISIT_HOOKED_DDRAW_PROCS(visit) \
					visit(ddraw, D3DParseUnknownCommand) \
					visit(ddraw, DirectDrawCreateClipper) \
					visit(ddraw, DirectDrawEnumerateA) \
					visit(ddraw, DirectDrawEnumerateExA) \
					visit(ddraw, DirectDrawEnumerateExW) \
					visit(ddraw, DirectDrawEnumerateW) \
					visit(ddraw, GetSurfaceFromDC)

					VISIT_HOOKED_DDRAW_PROCS(HOOK_WRAPPED_PROC);
				}
			}

			// Add DDrawCompat to the chain
			if (Config.DDrawCompat)
			{
				using namespace ddraw;
				using namespace DDrawCompat;
				DDrawCompat::Prepare();
				VISIT_PROCS_DDRAW(SHIM_WRAPPED_PROC);
				Wrapper::ShimProc(ShardProcs::DllCanUnloadNow_var, DllCanUnloadNow_in, DllCanUnloadNow_out);
				Wrapper::ShimProc(ShardProcs::DllGetClassObject_var, DllGetClassObject_in, DllGetClassObject_out);
			}

			// Add DdrawWrapper to the chain
			if (Config.isDdrawWrapperEnabled)
			{
				Logging::Log() << "Enabling ddraw wrapper";
				using namespace ddraw;
				using namespace DdrawWrapper;
				VISIT_PROCS_DDRAW(SHIM_WRAPPED_PROC);
				Wrapper::ShimProc(ShardProcs::DllCanUnloadNow_var, DllCanUnloadNow_in, DllCanUnloadNow_out);
				Wrapper::ShimProc(ShardProcs::DllGetClassObject_var, DllGetClassObject_in, DllGetClassObject_out);
			}

			// Start DDrawCompat
			if (Config.DDrawCompat)
			{
				Config.DDrawCompat = DDrawCompat::Start(hModule_dll, DLL_PROCESS_ATTACH);
			}

			// Start Dd7to9
			if (Config.Dd7to9)
			{
				InitDDraw();
			}
		}

		// Start dinput.dll module
		if (Config.Dinputto8)
		{
			Logging::Log() << "Enabling dinputto8 wrapper";

			// Check wrapper mode
			if (Config.RealWrapperMode != dtype.dinput)
			{
				// Load dinput procs
				HMODULE dll = dinput::Load(nullptr, Config.WrapperName.c_str());
				if (dll)
				{
					Utils::AddHandleToVector(dll, dtypename[dtype.dinput]);
				}

				// Hook dinput.dll APIs
				Logging::Log() << "Hooking dinput.dll APIs...";
				Hook::HotPatch(Hook::GetProcAddress(dll, "DirectInputCreateA"), "DirectInputCreateA", (FARPROC)*(DinputWrapper::DirectInputCreateA_in), true);
				Hook::HotPatch(Hook::GetProcAddress(dll, "DirectInputCreateW"), "DirectInputCreateW", (FARPROC)*(DinputWrapper::DirectInputCreateW_in), true);
				Hook::HotPatch(Hook::GetProcAddress(dll, "DirectInputCreateEx"), "DirectInputCreateEx", (FARPROC)*(DinputWrapper::DirectInputCreateEx_in), true);
				Hook::HotPatch(Hook::GetProcAddress(dll, "DllCanUnloadNow"), "DllCanUnloadNow", (FARPROC)*(DinputWrapper::DllCanUnloadNow_in), true);
				Hook::HotPatch(Hook::GetProcAddress(dll, "DllGetClassObject"), "DllGetClassObject", (FARPROC)*(DinputWrapper::DllGetClassObject_in), true);
				Hook::HotPatch(Hook::GetProcAddress(dll, "DllRegisterServer"), "DllRegisterServer", (FARPROC)*(DinputWrapper::DllRegisterServer_in), true);
				Hook::HotPatch(Hook::GetProcAddress(dll, "DllUnregisterServer"), "DllUnregisterServer", (FARPROC)*(DinputWrapper::DllUnregisterServer_in), true);
			}
		}

		// Start D3d8to9 module
		if (Config.D3d8to9)
		{
			Logging::Log() << "Enabling d3d8to9 wrapper";

			// Hook d3d8 APIs for D3d8to9
			if (Config.RealWrapperMode != dtype.d3d8)
			{
				// Load d3d8 procs
				HMODULE dll = d3d8::Load(nullptr, Config.WrapperName.c_str());
				if (dll)
				{
					Utils::AddHandleToVector(dll, dtypename[dtype.d3d8]);
				}

				// Hook d3d8.dll -> D3d8to9
				Logging::Log() << "Hooking d3d8.dll APIs...";
				Hook::HookAPI(dll, dtypename[dtype.d3d8], Hook::GetProcAddress(dll, "Direct3DCreate8"), "Direct3DCreate8", D3d8Wrapper::Direct3DCreate8_in);
			}
		}

		// Start d3d9.dll module
		if (Config.isD3d9WrapperEnabled || Config.D3d8to9 || Config.Dd7to9)
		{
			Logging::Log() << "Enabling d3d9 wrapper";

			using namespace d3d9;
			using namespace D3d9Wrapper;

			// Hook d3d9 APIs
			if (Config.RealWrapperMode != dtype.d3d9)
			{
				// Load d3d9 procs
				HMODULE dll = d3d9::Load(nullptr, Config.WrapperName.c_str());
				if (dll)
				{
					Utils::AddHandleToVector(dll, dtypename[dtype.d3d9]);
				}

				// Hook d3d9.dll -> D3d9Wrapper
				if (Config.isD3d9WrapperEnabled)
				{
					Logging::Log() << "Hooking d3d9.dll APIs...";
					Direct3DCreate9_var = (FARPROC)Hook::HookAPI(dll, dtypename[dtype.d3d9], Hook::GetProcAddress(dll, "Direct3DCreate9"), "Direct3DCreate9", Direct3DCreate9_in);
					Direct3DCreate9Ex_var = (FARPROC)Hook::HookAPI(dll, dtypename[dtype.d3d9], Hook::GetProcAddress(dll, "Direct3DCreate9Ex"), "Direct3DCreate9Ex", Direct3DCreate9Ex_in);
				}
			}

			// Redirect d3d8to9 -> D3d9Wrapper
			if (Config.D3d8to9)
			{
				d3d8::Direct3D8EnableMaximizedWindowedModeShim_var = d3d9::Direct3D9EnableMaximizedWindowedModeShim_var;
				D3d8Wrapper::Direct3DCreate9 = Direct3DCreate9_in;
			}

			// Redirect DdrawWrapper -> D3d9Wrapper
			if (Config.Dd7to9)
			{
				DdrawWrapper::Direct3DCreate9 = Direct3DCreate9_in;
			}

			// Prepare wrapper
			VISIT_PROCS_D3D9(SHIM_WRAPPED_PROC);
			Wrapper::ShimProc(ShardProcs::DebugSetMute_var, DebugSetMute_in, DebugSetMute_out);
		}

		// Start dsound.dll module
		if (Config.isDsoundWrapperEnabled)
		{
			using namespace dsound;
			using namespace DsoundWrapper;

			// Hook dsound APIs for DsoundWrapper
			if (Config.RealWrapperMode != dtype.dsound)
			{
				// Load dsound procs
				HMODULE dll = dsound::Load(nullptr, Config.WrapperName.c_str());
				if (dll)
				{
					Utils::AddHandleToVector(dll, dtypename[dtype.dsound]);
				}

				// Hook dsound.dll -> DsoundWrapper
				Logging::Log() << "Hooking dsound.dll APIs...";
				DirectSoundCreate_var = (FARPROC)Hook::HookAPI(dll, dtypename[dtype.dsound], Hook::GetProcAddress(dll, "DirectSoundCreate"), "DirectSoundCreate", DirectSoundCreate_in);
				DirectSoundCreate8_var = (FARPROC)Hook::HookAPI(dll, dtypename[dtype.dsound], Hook::GetProcAddress(dll, "DirectSoundCreate8"), "DirectSoundCreate8", DirectSoundCreate8_in);
				GetDeviceID_var = (FARPROC)Hook::HookAPI(dll, dtypename[dtype.dsound], Hook::GetProcAddress(dll, "GetDeviceID"), "GetDeviceID", GetDeviceID_in);
				DirectSoundEnumerateA_var = (FARPROC)Hook::HookAPI(dll, dtypename[dtype.dsound], Hook::GetProcAddress(dll, "DirectSoundEnumerateA"), "DirectSoundEnumerateA", DirectSoundEnumerateA_in);
				DirectSoundEnumerateW_var = (FARPROC)Hook::HookAPI(dll, dtypename[dtype.dsound], Hook::GetProcAddress(dll, "DirectSoundEnumerateW"), "DirectSoundEnumerateW", DirectSoundEnumerateW_in);
				DirectSoundCaptureEnumerateA_var = (FARPROC)Hook::HookAPI(dll, dtypename[dtype.dsound], Hook::GetProcAddress(dll, "DirectSoundCaptureEnumerateA"), "DirectSoundCaptureEnumerateA", DirectSoundCaptureEnumerateA_in);
				DirectSoundCaptureEnumerateW_var = (FARPROC)Hook::HookAPI(dll, dtypename[dtype.dsound], Hook::GetProcAddress(dll, "DirectSoundCaptureEnumerateW"), "DirectSoundCaptureEnumerateW", DirectSoundCaptureEnumerateW_in);
				DirectSoundCaptureCreate_var = (FARPROC)Hook::HookAPI(dll, dtypename[dtype.dsound], Hook::GetProcAddress(dll, "DirectSoundCaptureCreate"), "DirectSoundCaptureCreate", DirectSoundCaptureCreate_in);
				DirectSoundCaptureCreate8_var = (FARPROC)Hook::HookAPI(dll, dtypename[dtype.dsound], Hook::GetProcAddress(dll, "DirectSoundCaptureCreate8"), "DirectSoundCaptureCreate8", DirectSoundCaptureCreate8_in);
				DirectSoundFullDuplexCreate_var = (FARPROC)Hook::HookAPI(dll, dtypename[dtype.dsound], Hook::GetProcAddress(dll, "DirectSoundFullDuplexCreate"), "DirectSoundFullDuplexCreate", DirectSoundFullDuplexCreate_in);
				ShardProcs::DllGetClassObject_var = (FARPROC)Hook::HookAPI(dll, dtypename[dtype.dsound], Hook::GetProcAddress(dll, "DllGetClassObject"), "DllGetClassObject", DllGetClassObject_in);
			}

			// Prepare wrapper
			VISIT_PROCS_DSOUND(SHIM_WRAPPED_PROC);
			Wrapper::ShimProc(ShardProcs::DllGetClassObject_var, DllGetClassObject_in, DllGetClassObject_out);
			Wrapper::ShimProc(ShardProcs::DllCanUnloadNow_var, DllCanUnloadNow_in, DllCanUnloadNow_out);
		}

		// Start DxWnd module
		if (Config.DxWnd)
		{
			// Check if dxwnd.dll exists then load it
			HMODULE dxwnd_dll = LoadLibrary("dxwnd.dll");
			if (dxwnd_dll)
			{
				Logging::Log() << "Loading DxWnd " << _TO_STRING(APP_DXWNDVERSION);
				InitDxWnd(dxwnd_dll);
			}
			// If dxwnd.dll does not exist than disable dxwnd setting
			else
			{
				Config.DxWnd = false;
			}
		}

		// Store screen settings
		if (Config.ResetScreenRes)
		{
			// Reset screen settings
			hDC = GetDC(nullptr);
			GetDeviceGammaRamp(hDC, &lpRamp[0]);
		}

		// Load custom dlls
		if (Config.LoadCustomDllPath.size() != 0)
		{
			Utils::LoadCustomDll();
		}

		// Load ASI plugins
		if (Config.LoadPlugins)
		{
			Utils::LoadPlugins();
		}

		// Start fullscreen thread
		if (Config.FullScreen || Config.ForceTermination)
		{
			Utils::Fullscreen::StartThread();
		}

		// Loaded
		Logging::Log() << "DxWrapper loaded!";

		// Resetting thread priority
		SetThreadPriority(hCurrentThread, dwPriorityClass);

		// Closing handle
		CloseHandle(hCurrentThread);
	}
	break;
	case DLL_THREAD_ATTACH:
		// Check if thread has started
		if (Config.ForceTermination && Utils::Fullscreen::IsThreadRunning())
		{
			FullscreenThreadStartedFlag = true;
		}
		break;
	case DLL_THREAD_DETACH:
		if (Config.ForceTermination)
		{
			// Check if thread has started
			if (Utils::Fullscreen::IsThreadRunning())
			{
				FullscreenThreadStartedFlag = true;
			}

			// Check if thread has stopped
			if (FullscreenThreadStartedFlag && !Utils::Fullscreen::IsThreadRunning())
			{
				Logging::Log() << "Process not exiting, attempting to terminate process...";

				// Reset screen back to original Windows settings to fix some display errors on exit
				if (Config.ResetScreenRes)
				{
					// Reset screen settings
					Logging::Log() << "Reseting screen resolution";
					SetDeviceGammaRamp(hDC, &lpRamp[0]);
					ChangeDisplaySettingsEx(nullptr, nullptr, nullptr, CDS_RESET, nullptr);
					ReleaseDC(nullptr, hDC);
				}

				// Terminate the current process
				Logging::Log() << "Terminating process!";
				TerminateProcess(OpenProcess(PROCESS_ALL_ACCESS, false, GetCurrentProcessId()), 0);
			}
		}
		break;
	case DLL_PROCESS_DETACH:
		// Run all clean up functions
		Config.Exiting = true;
		Logging::Log() << "Quiting DxWrapper";

		// Stop threads
		Utils::Fullscreen::StopThread();
		Utils::WriteMemory::StopThread();

		// Unload and Unhook DxWnd
		if (Config.DxWnd)
		{
			Logging::Log() << "Unloading DxWnd";
			DxWndEndHook();
		}

		// Unload and Unhook DDrawCompat
		if (Config.DDrawCompat)
		{
			DDrawCompat::Start(nullptr, DLL_PROCESS_DETACH);
		}

		// Unload DdrawWrapper
		if (Config.D3d8to9)
		{
			ExitDDraw();
		}

		// Unhook all APIs
		Hook::UnhookAll();

		// Unload loaded dlls
		Utils::UnloadAllDlls();

		// Unload exception handler
		if (Config.HandleExceptions)
		{
			Utils::UnHookExceptionHandler();
		}

		// Reset screen back to original Windows settings to fix some display errors on exit
		if (Config.ResetScreenRes)
		{
			// Reset screen settings
			Logging::Log() << "Reseting screen resolution";
			SetDeviceGammaRamp(hDC, &lpRamp[0]);
			ChangeDisplaySettingsEx(nullptr, nullptr, nullptr, CDS_RESET, nullptr);
			ReleaseDC(nullptr, hDC);
		}

		// Release Mutex
		if (hMutex)
		{
			ReleaseMutex(hMutex);
		}
		if (n_hMutex)
		{
			ReleaseMutex(n_hMutex);
		}

		// Final log
		Logging::Log() << "DxWrapper terminated!";
		break;
	}
	return true;
}
