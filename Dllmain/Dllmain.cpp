/**
* Copyright (C) 2018 Elisha Riedlinger
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
#include "Hooking\Hook.h"
#include "D3d8to9\d3d8to9External.h"
#include "DDrawCompat\DDrawCompatExternal.h"
#include "DxWnd\DxWndExternal.h"
#include "DSoundCtrl\DSoundCtrlExternal.h"
#include "Utils\Utils.h"
#include "Logging\Logging.h"
// Wrappers last
#include "d3d9\d3d9External.h"
#include "ddraw\ddrawExternal.h"

// Declare varables
HMODULE hModule_dll = nullptr;

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

		// Init logs
		Logging::Log() << "Starting DxWrapper v" << APP_VERSION;
		Logging::LogOSVersion();
		Logging::LogVideoCard();
		Logging::LogProcessNameAndPID();

		// Initialize config
		Config.Init();

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
		else
		{
			// Load real dll and attach wrappers
			HMODULE dll = Wrapper::CreateWrapper((Config.RealDllPath.size()) ? Config.RealDllPath.c_str() : nullptr, (Config.WrapperMode.size()) ? Config.WrapperMode.c_str() : Config.WrapperName.c_str());
			if (dll)
			{
				Utils::AddHandleToVector(dll, (Config.RealDllPath.size()) ? Config.RealDllPath.c_str() : Config.WrapperName.c_str());
			}

			// Hook GetProcAddress to handle wrapped functions that are missing or not available in the OS
			dll = LoadLibrary("kernel32.dll");
			if (dll)
			{
				Logging::Log() << "Hooking 'GetProcAddress' API...";
				InterlockedExchangePointer((PVOID*)&Utils::pGetProcAddress, Hook::HookAPI(dll, "kernel32.dll", Hook::GetProcAddress(dll, "GetProcAddress"), "GetProcAddress", Utils::GetProcAddressHandler));
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
			using namespace ddraw;
			using namespace DdrawWrapper;

			// If wrapper mode is ddraw update wrapper
			if (Config.RealWrapperMode == dtype.ddraw)
			{
				// Update dxwrapper.dll -> DdrawWrapper
				if (Config.isDdrawWrapperEnabled)
				{
					Wrapper::ShimProc(AcquireDDThreadLock_var, AcquireDDThreadLock_in, AcquireDDThreadLock_out);
					Wrapper::ShimProc(CheckFullscreen_var, CheckFullscreen_in, CheckFullscreen_out);
					Wrapper::ShimProc(CompleteCreateSysmemSurface_var, CompleteCreateSysmemSurface_in, CompleteCreateSysmemSurface_out);
					Wrapper::ShimProc(D3DParseUnknownCommand_var, D3DParseUnknownCommand_in, D3DParseUnknownCommand_out);
					Wrapper::ShimProc(DDGetAttachedSurfaceLcl_var, DDGetAttachedSurfaceLcl_in, DDGetAttachedSurfaceLcl_out);
					Wrapper::ShimProc(DDInternalLock_var, DDInternalLock_in, DDInternalLock_out);
					Wrapper::ShimProc(DDInternalUnlock_var, DDInternalUnlock_in, DDInternalUnlock_out);
					Wrapper::ShimProc(DSoundHelp_var, DSoundHelp_in, DSoundHelp_out);
					Wrapper::ShimProc(DirectDrawCreate_var, DirectDrawCreate_in, DirectDrawCreate_out);
					Wrapper::ShimProc(DirectDrawCreateClipper_var, DirectDrawCreateClipper_in, DirectDrawCreateClipper_out);
					Wrapper::ShimProc(DirectDrawCreateEx_var, DirectDrawCreateEx_in, DirectDrawCreateEx_out);
					Wrapper::ShimProc(DirectDrawEnumerateA_var, DirectDrawEnumerateA_in, DirectDrawEnumerateA_out);
					Wrapper::ShimProc(DirectDrawEnumerateExA_var, DirectDrawEnumerateExA_in, DirectDrawEnumerateExA_out);
					Wrapper::ShimProc(DirectDrawEnumerateExW_var, DirectDrawEnumerateExW_in, DirectDrawEnumerateExW_out);
					Wrapper::ShimProc(DirectDrawEnumerateW_var, DirectDrawEnumerateW_in, DirectDrawEnumerateW_out);
					Wrapper::ShimProc(ShardProcs::DllCanUnloadNow_var, DllCanUnloadNow_in, DllCanUnloadNow_out);
					Wrapper::ShimProc(ShardProcs::DllGetClassObject_var, DllGetClassObject_in, DllGetClassObject_out);
					Wrapper::ShimProc(GetDDSurfaceLocal_var, GetDDSurfaceLocal_in, GetDDSurfaceLocal_out);
					Wrapper::ShimProc(GetOLEThunkData_var, GetOLEThunkData_in, GetOLEThunkData_out);
					Wrapper::ShimProc(GetSurfaceFromDC_var, GetSurfaceFromDC_in, GetSurfaceFromDC_out);
					Wrapper::ShimProc(RegisterSpecialCase_var, RegisterSpecialCase_in, RegisterSpecialCase_out);
					Wrapper::ShimProc(ReleaseDDThreadLock_var, ReleaseDDThreadLock_in, ReleaseDDThreadLock_out);
					Wrapper::ShimProc(SetAppCompatData_var, SetAppCompatData_in, SetAppCompatData_out);

					// Update DdrawWrapper -> DDrawCompat
					if (Config.DDrawCompat)
					{
						DirectDrawCreate_out = DDrawCompat::DirectDrawCreate;
						DirectDrawCreateEx_out = DDrawCompat::DirectDrawCreateEx;
						DllGetClassObject_out = DDrawCompat::DllGetClassObject;
					}
				}
				// Update dxwrapper.dll -> DDrawCompat
				else
				{
					DirectDrawCreate_var = DDrawCompat::DirectDrawCreate;
					DirectDrawCreateEx_var = DDrawCompat::DirectDrawCreateEx;
					ShardProcs::DllGetClassObject_var = DDrawCompat::DllGetClassObject;
				}
			}
			// Hook ddraw APIs for DDrawCompat
			else
			{
				// Load ddraw procs
				HMODULE dll = ddraw::Load(nullptr);
				if (dll)
				{
					Utils::AddHandleToVector(dll, dtypename[dtype.ddraw]);
				}

				// Hook ddraw.dll -> DDrawCompat
				Logging::Log() << "Hooking ddraw.dll APIs...";
				if (Config.isDdrawWrapperEnabled)
				{
					DirectDrawCreate_var = (FARPROC)Hook::HookAPI(dll, dtypename[dtype.ddraw], Hook::GetProcAddress(dll, "DirectDrawCreate"), "DirectDrawCreate", DirectDrawCreate_in);
					DirectDrawCreateEx_var = (FARPROC)Hook::HookAPI(dll, dtypename[dtype.ddraw], Hook::GetProcAddress(dll, "DirectDrawCreateEx"), "DirectDrawCreateEx", DirectDrawCreateEx_in);
					if (Config.DDrawCompat)
					{
						DirectDrawCreate_out = DDrawCompat::DirectDrawCreate;
						DirectDrawCreateEx_out = DDrawCompat::DirectDrawCreateEx;
						ShardProcs::DllGetClassObject_var = (FARPROC)Hook::HookAPI(dll, dtypename[dtype.ddraw], Hook::GetProcAddress(dll, "DllGetClassObject"), "DllGetClassObject", DDrawCompat::DllGetClassObject);
					}
					else
					{
						DirectDrawCreate_out = DirectDrawCreate_var;
						DirectDrawCreateEx_out = DirectDrawCreateEx_var;
					}
				}
				else
				{
					DirectDrawCreate_var = (FARPROC)Hook::HookAPI(dll, dtypename[dtype.ddraw], Hook::GetProcAddress(dll, "DirectDrawCreate"), "DirectDrawCreate", DDrawCompat::DirectDrawCreate);
					DirectDrawCreateEx_var = (FARPROC)Hook::HookAPI(dll, dtypename[dtype.ddraw], Hook::GetProcAddress(dll, "DirectDrawCreateEx"), "DirectDrawCreateEx", DDrawCompat::DirectDrawCreateEx);
					ShardProcs::DllGetClassObject_var = (FARPROC)Hook::HookAPI(dll, dtypename[dtype.ddraw], Hook::GetProcAddress(dll, "DllGetClassObject"), "DllGetClassObject", DDrawCompat::DllGetClassObject);
				}
			}

			// Start DDrawCompat
			if (Config.DDrawCompat)
			{
				Config.DDrawCompat = (DllMain_DDrawCompat(hModule_dll, fdwReason, nullptr) == TRUE);
			}
		}

		// Start D3d8to9 module
		if (Config.D3d8to9)
		{
			// If wrapper mode is d3d8 update wrapper
			if (Config.RealWrapperMode == dtype.d3d8)
			{
				// Update dxwrapper.dll -> D3d8to9
				d3d8::Direct3DCreate8_var = D3d8to9::Direct3DCreate8;
			}
			// Hook d3d8 APIs for D3d8to9
			else
			{
				// Load d3d8 procs
				HMODULE dll = d3d8::Load(nullptr);
				if (dll)
				{
					Utils::AddHandleToVector(dll, dtypename[dtype.d3d8]);
				}

				// Hook d3d8.dll -> D3d8to9	
				Logging::Log() << "Hooking d3d8.dll APIs...";
				(FARPROC)Hook::HookAPI(dll, dtypename[dtype.d3d8], Hook::GetProcAddress(dll, "Direct3DCreate8"), "Direct3DCreate8", D3d8to9::Direct3DCreate8);
			}

			// Load d3d9 functions
			HMODULE dll = LoadLibrary(dtypename[dtype.d3d9]);
			d3d8::Direct3D8EnableMaximizedWindowedModeShim_var = Utils::GetProcAddress(dll, "Direct3D9EnableMaximizedWindowedModeShim", d3d8::Direct3D8EnableMaximizedWindowedModeShim_var);
			ShardProcs::DebugSetMute_var = Utils::GetProcAddress(dll, "DebugSetMute", ShardProcs::DebugSetMute_var);
			D3d8to9::Direct3DCreate9 = Utils::GetProcAddress(dll, "Direct3DCreate9", D3d8to9::Direct3DCreate9);
		}

		// Start d3d9.dll module
		if (Config.isD3d9WrapperEnabled)
		{
			using namespace d3d9;
			using namespace D3d9Wrapper;

			// If wrapper mode is d3d9 update wrapper
			if (Config.RealWrapperMode == dtype.d3d9)
			{
				Wrapper::ShimProc(Direct3DShaderValidatorCreate9_var, Direct3DShaderValidatorCreate9_in, Direct3DShaderValidatorCreate9_out);
				Wrapper::ShimProc(PSGPError_var, PSGPError_in, PSGPError_out);
				Wrapper::ShimProc(PSGPSampleTexture_var, PSGPSampleTexture_in, PSGPSampleTexture_out);
				Wrapper::ShimProc(D3DPERF_BeginEvent_var, D3DPERF_BeginEvent_in, D3DPERF_BeginEvent_out);
				Wrapper::ShimProc(D3DPERF_EndEvent_var, D3DPERF_EndEvent_in, D3DPERF_EndEvent_out);
				Wrapper::ShimProc(D3DPERF_GetStatus_var, D3DPERF_GetStatus_in, D3DPERF_GetStatus_out);
				Wrapper::ShimProc(D3DPERF_QueryRepeatFrame_var, D3DPERF_QueryRepeatFrame_in, D3DPERF_QueryRepeatFrame_out);
				Wrapper::ShimProc(D3DPERF_SetMarker_var, D3DPERF_SetMarker_in, D3DPERF_SetMarker_out);
				Wrapper::ShimProc(D3DPERF_SetOptions_var, D3DPERF_SetOptions_in, D3DPERF_SetOptions_out);
				Wrapper::ShimProc(D3DPERF_SetRegion_var, D3DPERF_SetRegion_in, D3DPERF_SetRegion_out);
				Wrapper::ShimProc(DebugSetLevel_var, DebugSetLevel_in, DebugSetLevel_out);
				Wrapper::ShimProc(ShardProcs::DebugSetMute_var, DebugSetMute_in, DebugSetMute_out);
				Wrapper::ShimProc(Direct3D9EnableMaximizedWindowedModeShim_var, Direct3D9EnableMaximizedWindowedModeShim_in, Direct3D9EnableMaximizedWindowedModeShim_out);
				Wrapper::ShimProc(Direct3DCreate9_var, Direct3DCreate9_in, Direct3DCreate9_out);
				Wrapper::ShimProc(Direct3DCreate9Ex_var, Direct3DCreate9Ex_in, Direct3DCreate9Ex_out);
			}
			// Hook d3d9 APIs
			else
			{
				// Load d3d9 procs
				HMODULE dll = d3d9::Load(nullptr);
				if (dll)
				{
					Utils::AddHandleToVector(dll, dtypename[dtype.d3d9]);
				}

				// Hook d3d9.dll -> D3d9Wrapper
				Logging::Log() << "Hooking d3d9.dll APIs...";
				Direct3DCreate9_out = (FARPROC)Hook::HookAPI(dll, dtypename[dtype.d3d9], Hook::GetProcAddress(dll, "Direct3DCreate9"), "Direct3DCreate9", Direct3DCreate9_in);
				Direct3DCreate9Ex_out = (FARPROC)Hook::HookAPI(dll, dtypename[dtype.d3d9], Hook::GetProcAddress(dll, "Direct3DCreate9Ex"), "Direct3DCreate9Ex", Direct3DCreate9Ex_in);
			}
		}

		// Start DSoundCtrl module
		if (Config.DSoundCtrl)
		{
			using namespace dsound;
			using namespace DSoundCtrl;

			// If wrapper mode is dsound update wrapper
			if (Config.RealWrapperMode == dtype.dsound)
			{
				Wrapper::ShimProc(DirectSoundCreate_var, DirectSoundCreate_in, DirectSoundCreate_out);
				Wrapper::ShimProc(DirectSoundCreate8_var, DirectSoundCreate8_in, DirectSoundCreate8_out);
				Wrapper::ShimProc(GetDeviceID_var, GetDeviceID_in, GetDeviceID_out);
				Wrapper::ShimProc(DirectSoundEnumerateA_var, DirectSoundEnumerateA_in, DirectSoundEnumerateA_out);
				Wrapper::ShimProc(DirectSoundEnumerateW_var, DirectSoundEnumerateW_in, DirectSoundEnumerateW_out);
				Wrapper::ShimProc(DirectSoundCaptureCreate_var, DirectSoundCaptureCreate_in, DirectSoundCaptureCreate_out);
				Wrapper::ShimProc(DirectSoundCaptureEnumerateA_var, DirectSoundCaptureEnumerateA_in, DirectSoundCaptureEnumerateA_out);
				Wrapper::ShimProc(DirectSoundCaptureEnumerateW_var, DirectSoundCaptureEnumerateW_in, DirectSoundCaptureEnumerateW_out);
				Wrapper::ShimProc(DirectSoundCaptureCreate8_var, DirectSoundCaptureCreate8_in, DirectSoundCaptureCreate8_out);
				Wrapper::ShimProc(DirectSoundFullDuplexCreate_var, DirectSoundFullDuplexCreate_in, DirectSoundFullDuplexCreate_out);
				Wrapper::ShimProc(ShardProcs::DllGetClassObject_var, DllGetClassObject_in, DllGetClassObject_out);
				Wrapper::ShimProc(ShardProcs::DllCanUnloadNow_var, DllCanUnloadNow_in, DllCanUnloadNow_out);
			}
			// Hook dsound APIs for DSoundCtrl
			else
			{
				// Load dsound procs
				HMODULE dll = dsound::Load(nullptr);
				if (dll)
				{
					Utils::AddHandleToVector(dll, dtypename[dtype.dsound]);
				}

				// Hook dsound.dll -> DSoundCtrl
				Logging::Log() << "Hooking dsound.dll APIs...";
				DirectSoundCreate_var = (FARPROC)Hook::HookAPI(dll, dtypename[dtype.dsound], Hook::GetProcAddress(dll, "DirectSoundCreate"), "DirectSoundCreate", DirectSoundCreate_in);
				DirectSoundCreate8_var = (FARPROC)Hook::HookAPI(dll, dtypename[dtype.dsound], Hook::GetProcAddress(dll, "DirectSoundCreate8"), "DirectSoundCreate8", DirectSoundCreate8_in);
				DirectSoundEnumerateA_var = (FARPROC)Hook::HookAPI(dll, dtypename[dtype.dsound], Hook::GetProcAddress(dll, "DirectSoundEnumerateA"), "DirectSoundEnumerateA", DirectSoundEnumerateA_in);
				DirectSoundEnumerateW_var = (FARPROC)Hook::HookAPI(dll, dtypename[dtype.dsound], Hook::GetProcAddress(dll, "DirectSoundEnumerateW"), "DirectSoundEnumerateW", DirectSoundEnumerateW_in);
				ShardProcs::DllGetClassObject_var = (FARPROC)Hook::HookAPI(dll, dtypename[dtype.dsound], Hook::GetProcAddress(dll, "DllGetClassObject"), "DllGetClassObject", DllGetClassObject_in);
			}

			// Start DSoundCtrl
			DllMain_DSoundCtrl(hModule_dll, fdwReason, nullptr);
		}

		// Start DxWnd module
		if (Config.DxWnd)
		{
			// Check if dxwnd.dll exists then load it
			HMODULE dxwnd_dll = LoadLibrary("dxwnd.dll");
			if (dxwnd_dll)
			{
				Logging::Log() << "Loading dxwnd";
				InitDxWnd(dxwnd_dll);
			}
			// If dxwnd.dll does not exist than disable dxwnd setting
			else
			{
				Config.DxWnd = false;
			}
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

		// Resetting thread priority
		SetThreadPriority(hCurrentThread, dwPriorityClass);

		// Closing handle
		CloseHandle(hCurrentThread);
	}
	break;
	case DLL_THREAD_ATTACH:
		// Check and store screen resolution
		if (Config.ResetScreenRes)
		{
			Utils::Fullscreen::CheckCurrentScreenRes();
		}

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
					Utils::Fullscreen::ResetScreen();
				}

				// Terminate the current process
				Logging::Log() << "Terminating process!";
				TerminateProcess(OpenProcess(PROCESS_ALL_ACCESS, false, GetCurrentProcessId()), 0);
			}
		}
		break;
	case DLL_PROCESS_DETACH:
		// Run all clean up functions
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
			DllMain_DDrawCompat(nullptr, DLL_PROCESS_DETACH, nullptr);
		}

		// Unload DSoundCtrl
		if (Config.DSoundCtrl)
		{
			DllMain_DSoundCtrl(nullptr, DLL_PROCESS_DETACH, nullptr);
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
			Utils::Fullscreen::ResetScreen();
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
