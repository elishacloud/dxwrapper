/**
* Copyright (C) 2017 Elisha Riedlinger
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
		Logging::LogProcessNameAndPID();

		// Initialize config
		Config.Init();

		// Create Mutex to ensure only one copy of DxWrapper is running
		char MutexName[MAX_PATH];
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
		if (Config.RealWrapperMode != dtype.dxwrapper)
		{
			HMODULE dll = Wrapper::CreateWrapper((Config.RealDllPath.size()) ? Config.RealDllPath.c_str() : nullptr, (Config.WrapperMode.size()) ? Config.WrapperMode.c_str() : Config.WrapperName.c_str());
			if (dll)
			{
				Utils::AddHandleToVector(dll, Config.WrapperName.c_str());
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
			// If wrapper mode is ddraw update wrapper
			if (Config.RealWrapperMode == dtype.ddraw)
			{
				// Update dxwrapper.dll -> DdrawWrapper
				if (Config.isDdrawWrapperEnabled)
				{
					DdrawWrapper::AcquireDDThreadLock_out = ddraw::AcquireDDThreadLock_var;
					DdrawWrapper::CheckFullscreen_out = ddraw::CheckFullscreen_var;
					DdrawWrapper::CompleteCreateSysmemSurface_out = ddraw::CompleteCreateSysmemSurface_var;
					DdrawWrapper::D3DParseUnknownCommand_out = ddraw::D3DParseUnknownCommand_var;
					DdrawWrapper::DDGetAttachedSurfaceLcl_out = ddraw::DDGetAttachedSurfaceLcl_var;
					DdrawWrapper::DDInternalLock_out = ddraw::DDInternalLock_var;
					DdrawWrapper::DDInternalUnlock_out = ddraw::DDInternalUnlock_var;
					DdrawWrapper::DSoundHelp_out = ddraw::DSoundHelp_var;
					DdrawWrapper::DirectDrawCreate_out = ddraw::DirectDrawCreate_var;
					DdrawWrapper::DirectDrawCreateClipper_out = ddraw::DirectDrawCreateClipper_var;
					DdrawWrapper::DirectDrawCreateEx_out = ddraw::DirectDrawCreateEx_var;
					DdrawWrapper::DirectDrawEnumerateA_out = ddraw::DirectDrawEnumerateA_var;
					DdrawWrapper::DirectDrawEnumerateExA_out = ddraw::DirectDrawEnumerateExA_var;
					DdrawWrapper::DirectDrawEnumerateExW_out = ddraw::DirectDrawEnumerateExW_var;
					DdrawWrapper::DirectDrawEnumerateW_out = ddraw::DirectDrawEnumerateW_var;
					DdrawWrapper::DllCanUnloadNow_out = ShardProcs::DllCanUnloadNow_var;
					DdrawWrapper::DllGetClassObject_out = ShardProcs::DllGetClassObject_var;
					DdrawWrapper::GetDDSurfaceLocal_out = ddraw::GetDDSurfaceLocal_var;
					DdrawWrapper::GetOLEThunkData_out = ddraw::GetOLEThunkData_var;
					DdrawWrapper::GetSurfaceFromDC_out = ddraw::GetSurfaceFromDC_var;
					DdrawWrapper::RegisterSpecialCase_out = ddraw::RegisterSpecialCase_var;
					DdrawWrapper::ReleaseDDThreadLock_out = ddraw::ReleaseDDThreadLock_var;
					DdrawWrapper::SetAppCompatData_out = ddraw::SetAppCompatData_var;

					// dxwrapper.dll -> DdrawWrapper
					ddraw::AcquireDDThreadLock_var = DdrawWrapper::AcquireDDThreadLock_in;
					ddraw::CheckFullscreen_var = DdrawWrapper::CheckFullscreen_in;
					ddraw::CompleteCreateSysmemSurface_var = DdrawWrapper::CompleteCreateSysmemSurface_in;
					ddraw::D3DParseUnknownCommand_var = DdrawWrapper::D3DParseUnknownCommand_in;
					ddraw::DDGetAttachedSurfaceLcl_var = DdrawWrapper::DDGetAttachedSurfaceLcl_in;
					ddraw::DDInternalLock_var = DdrawWrapper::DDInternalLock_in;
					ddraw::DDInternalUnlock_var = DdrawWrapper::DDInternalUnlock_in;
					ddraw::DSoundHelp_var = DdrawWrapper::DSoundHelp_in;
					ddraw::DirectDrawCreate_var = DdrawWrapper::DirectDrawCreate_in;
					ddraw::DirectDrawCreateClipper_var = DdrawWrapper::DirectDrawCreateClipper_in;
					ddraw::DirectDrawCreateEx_var = DdrawWrapper::DirectDrawCreateEx_in;
					ddraw::DirectDrawEnumerateA_var = DdrawWrapper::DirectDrawEnumerateA_in;
					ddraw::DirectDrawEnumerateExA_var = DdrawWrapper::DirectDrawEnumerateExA_in;
					ddraw::DirectDrawEnumerateExW_var = DdrawWrapper::DirectDrawEnumerateExW_in;
					ddraw::DirectDrawEnumerateW_var = DdrawWrapper::DirectDrawEnumerateW_in;
					ShardProcs::DllCanUnloadNow_var = DdrawWrapper::DllCanUnloadNow_in;
					ShardProcs::DllGetClassObject_var = DdrawWrapper::DllGetClassObject_in;
					ddraw::GetDDSurfaceLocal_var = DdrawWrapper::GetDDSurfaceLocal_in;
					ddraw::GetOLEThunkData_var = DdrawWrapper::GetOLEThunkData_in;
					ddraw::GetSurfaceFromDC_var = DdrawWrapper::GetSurfaceFromDC_in;
					ddraw::RegisterSpecialCase_var = DdrawWrapper::RegisterSpecialCase_in;
					ddraw::ReleaseDDThreadLock_var = DdrawWrapper::ReleaseDDThreadLock_in;
					ddraw::SetAppCompatData_var = DdrawWrapper::SetAppCompatData_in;

					// Update DdrawWrapper -> DDrawCompat
					if (Config.DDrawCompat)
					{
						DdrawWrapper::DirectDrawCreate_out = DDrawCompat::DirectDrawCreate;
						DdrawWrapper::DirectDrawCreateEx_out = DDrawCompat::DirectDrawCreateEx;
						DdrawWrapper::DllGetClassObject_out = DDrawCompat::DllGetClassObject;
					}
				}
				// Update dxwrapper.dll -> DDrawCompat
				else
				{
					ddraw::DirectDrawCreate_var = DDrawCompat::DirectDrawCreate;
					ddraw::DirectDrawCreateEx_var = DDrawCompat::DirectDrawCreateEx;
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
					ddraw::DirectDrawCreate_var = (FARPROC)Hook::HookAPI(hModule_dll, dtypename[dtype.ddraw], Hook::GetProcAddress(dll, "DirectDrawCreate"), "DirectDrawCreate", DdrawWrapper::DirectDrawCreate_in);
					ddraw::DirectDrawCreateEx_var = (FARPROC)Hook::HookAPI(hModule_dll, dtypename[dtype.ddraw], Hook::GetProcAddress(dll, "DirectDrawCreateEx"), "DirectDrawCreateEx", DdrawWrapper::DirectDrawCreateEx_in);
					if (Config.DDrawCompat)
					{
						DdrawWrapper::DirectDrawCreate_out = DDrawCompat::DirectDrawCreate;
						DdrawWrapper::DirectDrawCreateEx_out = DDrawCompat::DirectDrawCreateEx;
						ShardProcs::DllGetClassObject_var = (FARPROC)Hook::HookAPI(hModule_dll, dtypename[dtype.ddraw], Hook::GetProcAddress(dll, "DllGetClassObject"), "DllGetClassObject", DDrawCompat::DllGetClassObject);
					}
					else
					{
						DdrawWrapper::DirectDrawCreate_out = ddraw::DirectDrawCreate_var;
						DdrawWrapper::DirectDrawCreateEx_out = ddraw::DirectDrawCreateEx_var;
					}
				}
				else
				{
					ddraw::DirectDrawCreate_var = (FARPROC)Hook::HookAPI(hModule_dll, dtypename[dtype.ddraw], Hook::GetProcAddress(dll, "DirectDrawCreate"), "DirectDrawCreate", DDrawCompat::DirectDrawCreate);
					ddraw::DirectDrawCreateEx_var = (FARPROC)Hook::HookAPI(hModule_dll, dtypename[dtype.ddraw], Hook::GetProcAddress(dll, "DirectDrawCreateEx"), "DirectDrawCreateEx", DDrawCompat::DirectDrawCreateEx);
					ShardProcs::DllGetClassObject_var = (FARPROC)Hook::HookAPI(hModule_dll, dtypename[dtype.ddraw], Hook::GetProcAddress(dll, "DllGetClassObject"), "DllGetClassObject", DDrawCompat::DllGetClassObject);
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
				HMODULE dll = LoadLibrary(dtypename[dtype.d3d8]);

				// Hook d3d8.dll -> D3d8to9	
				Logging::Log() << "Hooking d3d8.dll APIs...";
				(FARPROC)Hook::HookAPI(hModule_dll, dtypename[dtype.d3d8], Hook::GetProcAddress(dll, "Direct3DCreate8"), "Direct3DCreate8", D3d8to9::Direct3DCreate8);
			}

			// Load d3d9 functions
			HMODULE dll = LoadLibrary(dtypename[dtype.d3d9]);
			d3d8::Direct3D8EnableMaximizedWindowedModeShim_var = Wrapper::GetProcAddress(dll, "Direct3D9EnableMaximizedWindowedModeShim", Wrapper::jmpaddr);
			ShardProcs::DebugSetMute_var = Wrapper::GetProcAddress(dll, "DebugSetMute", Wrapper::jmpaddr);
			D3d8to9::Direct3DCreate9 = Wrapper::GetProcAddress(dll, "Direct3DCreate9", Wrapper::jmpaddr);
		}

		// Start d3d9.dll module
		if (Config.isD3d9WrapperEnabled)
		{
			// If wrapper mode is d3d9 update wrapper
			if (Config.RealWrapperMode == dtype.d3d9)
			{
				D3d9Wrapper::Direct3DShaderValidatorCreate9_out = d3d9::Direct3DShaderValidatorCreate9_var;
				D3d9Wrapper::PSGPError_out = d3d9::PSGPError_var;
				D3d9Wrapper::PSGPSampleTexture_out = d3d9::PSGPSampleTexture_var;
				D3d9Wrapper::D3DPERF_BeginEvent_out = d3d9::D3DPERF_BeginEvent_var;
				D3d9Wrapper::D3DPERF_EndEvent_out = d3d9::D3DPERF_EndEvent_var;
				D3d9Wrapper::D3DPERF_GetStatus_out = d3d9::D3DPERF_GetStatus_var;
				D3d9Wrapper::D3DPERF_QueryRepeatFrame_out = d3d9::D3DPERF_QueryRepeatFrame_var;
				D3d9Wrapper::D3DPERF_SetMarker_out = d3d9::D3DPERF_SetMarker_var;
				D3d9Wrapper::D3DPERF_SetOptions_out = d3d9::D3DPERF_SetOptions_var;
				D3d9Wrapper::D3DPERF_SetRegion_out = d3d9::D3DPERF_SetRegion_var;
				D3d9Wrapper::DebugSetLevel_out = d3d9::DebugSetLevel_var;
				D3d9Wrapper::DebugSetMute_out = ShardProcs::DebugSetMute_var;
				D3d9Wrapper::Direct3D9EnableMaximizedWindowedModeShim_out = d3d9::Direct3D9EnableMaximizedWindowedModeShim_var;
				D3d9Wrapper::Direct3DCreate9_out = d3d9::Direct3DCreate9_var;
				D3d9Wrapper::Direct3DCreate9Ex_out = d3d9::Direct3DCreate9Ex_var;

				// Update dxwrapper.dll -> d3d9_wrap
				d3d9::Direct3DShaderValidatorCreate9_var = D3d9Wrapper::Direct3DShaderValidatorCreate9_in;
				d3d9::PSGPError_var = D3d9Wrapper::PSGPError_in;
				d3d9::PSGPSampleTexture_var = D3d9Wrapper::PSGPSampleTexture_in;
				d3d9::D3DPERF_BeginEvent_var = D3d9Wrapper::D3DPERF_BeginEvent_in;
				d3d9::D3DPERF_EndEvent_var = D3d9Wrapper::D3DPERF_EndEvent_in;
				d3d9::D3DPERF_GetStatus_var = D3d9Wrapper::D3DPERF_GetStatus_in;
				d3d9::D3DPERF_QueryRepeatFrame_var = D3d9Wrapper::D3DPERF_QueryRepeatFrame_in;
				d3d9::D3DPERF_SetMarker_var = D3d9Wrapper::D3DPERF_SetMarker_in;
				d3d9::D3DPERF_SetOptions_var = D3d9Wrapper::D3DPERF_SetOptions_in;
				d3d9::D3DPERF_SetRegion_var = D3d9Wrapper::D3DPERF_SetRegion_in;
				d3d9::DebugSetLevel_var = D3d9Wrapper::DebugSetLevel_in;
				ShardProcs::DebugSetMute_var = D3d9Wrapper::DebugSetMute_in;
				d3d9::Direct3D9EnableMaximizedWindowedModeShim_var = D3d9Wrapper::Direct3D9EnableMaximizedWindowedModeShim_in;
				d3d9::Direct3DCreate9_var = D3d9Wrapper::Direct3DCreate9_in;
				d3d9::Direct3DCreate9Ex_var = D3d9Wrapper::Direct3DCreate9Ex_in;
			}
			// Hook d3d9 APIs for d3d9_wrap
			else
			{
				// Load d3d9 procs
				HMODULE dll = LoadLibrary(dtypename[dtype.d3d9]);

				// Hook d3d9.dll -> d3d9_wrap
				Logging::Log() << "Hooking d3d9.dll APIs...";
				D3d9Wrapper::Direct3DCreate9_out = (FARPROC)Hook::HookAPI(hModule_dll, dtypename[dtype.d3d9], Hook::GetProcAddress(dll, "Direct3DCreate9"), "Direct3DCreate9", D3d9Wrapper::Direct3DCreate9_in);
			}
		}

		// Start DSoundCtrl module
		if (Config.DSoundCtrl)
		{
			// If wrapper mode is dsound update wrapper
			if (Config.RealWrapperMode == dtype.dsound)
			{
				// Update dxwrapper.dll -> DSoundCtrl
				dsound::DirectSoundCreate_var = DSoundCtrl::DirectSoundCreate;
				dsound::DirectSoundCreate8_var = DSoundCtrl::DirectSoundCreate8;
				ShardProcs::DllGetClassObject_var = DSoundCtrl::DllGetClassObject;
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
				dsound::DirectSoundCreate_var = (FARPROC)Hook::HookAPI(hModule_dll, dtypename[dtype.dsound], Hook::GetProcAddress(dll, "DirectSoundCreate"), "DirectSoundCreate", DSoundCtrl::DirectSoundCreate);
				dsound::DirectSoundCreate8_var = (FARPROC)Hook::HookAPI(hModule_dll, dtypename[dtype.dsound], Hook::GetProcAddress(dll, "DirectSoundCreate8"), "DirectSoundCreate8", DSoundCtrl::DirectSoundCreate8);
				ShardProcs::DllGetClassObject_var = (FARPROC)Hook::HookAPI(hModule_dll, dtypename[dtype.dsound], Hook::GetProcAddress(dll, "DllGetClassObject"), "DllGetClassObject", DSoundCtrl::DllGetClassObject);
			}

			// Start DSoundCtrl
			DllMain_DSoundCtrl(hModule, fdwReason, nullptr);
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
