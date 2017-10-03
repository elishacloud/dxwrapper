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
#include "D3d9\D3d9External.h"
#include "DDrawCompat\DDrawCompatExternal.h"
#include "DxWnd\DxWndExternal.h"
#include "DSoundCtrl\DSoundCtrlExternal.h"
#include "Utils\Utils.h"
#include "Logging\Logging.h"

// Declare varables
HMODULE hModule_dll = nullptr;

// Dll main function
bool APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved)
{
	UNREFERENCED_PARAMETER(lpReserved);

	static bool FullscreenThreadStartedFlag = false;

	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
	{
		// Get handle
		hModule_dll = hModule;
		HANDLE hCurrentThread = GetCurrentThread();

		// Set thread priority a trick to reduce concurrency problems at program startup
		SetThreadPriority(hCurrentThread, THREAD_PRIORITY_HIGHEST);

		// Init logs
		Logging::Log() << "Starting DxWrapper v" << APP_VERSION;
		Logging::LogOSVersion();
		Logging::LogProcessNameAndPID();

		// Initialize config
		Config.Init();

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

		// Attach real wrapper dll
		if (Config.RealWrapperMode != dtype.dxwrapper)
		{
			HMODULE dll = Wrapper::CreateWrapper(hModule_dll, (Config.RealDllPath.size()) ? Config.RealDllPath.c_str() : nullptr, (Config.WrapperMode.size()) ? Config.WrapperMode.c_str() : nullptr);
			if (dll)
			{
				Utils::AddHandleToVector(dll, Config.WrapperName.c_str());
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

		// Start DDrawCompat module
		if (Config.DDrawCompat)
		{
			// If wrapper mode is ddraw update wrapper
			if (Config.RealWrapperMode == dtype.ddraw)
			{
				// Update dxwrapper.dll -> DDrawCompat
				ddraw::DirectDrawCreate_var = DDrawCompat::DirectDrawCreate;
				ddraw::DirectDrawCreateEx_var = DDrawCompat::DirectDrawCreateEx;
				ShardProcs::DllGetClassObject_var = DDrawCompat::DllGetClassObject;
			}
			// Hook ddraw APIs for DDrawCompat
			else
			{
				// Load ddraw procs
				HMODULE dll = ddraw::Load((Config.RealDllPath.size()) ? Config.RealDllPath.c_str() : nullptr);
				if (dll)
				{
					Utils::AddHandleToVector(dll, dtypename[dtype.ddraw]);
				}

				// Hook ddraw.dll -> DDrawCompat
				Logging::Log() << "Hooking ddraw.dll APIs...";
				ddraw::DirectDrawCreate_var = (FARPROC)Hook::HookAPI(hModule_dll, dtypename[dtype.ddraw], Hook::GetProcAddress(dll, "DirectDrawCreate"), "DirectDrawCreate", DDrawCompat::DirectDrawCreate);
				ddraw::DirectDrawCreateEx_var = (FARPROC)Hook::HookAPI(hModule_dll, dtypename[dtype.ddraw], Hook::GetProcAddress(dll, "DirectDrawCreateEx"), "DirectDrawCreateEx", DDrawCompat::DirectDrawCreateEx);
				ShardProcs::DllGetClassObject_var = (FARPROC)Hook::HookAPI(hModule_dll, dtypename[dtype.ddraw], Hook::GetProcAddress(dll, "DllGetClassObject"), "DllGetClassObject", DDrawCompat::DllGetClassObject);
			}

			// Start DDrawCompat
			Config.DDrawCompat = (DllMain_DDrawCompat(hModule_dll, fdwReason, nullptr) == TRUE);
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
				// Load d3d8
				HMODULE dll = LoadLibrary(dtypename[dtype.d3d8]);

				// Hook d3d8.dll -> D3d8to9	
				Logging::Log() << "Hooking d3d8.dll APIs...";
				(FARPROC)Hook::HookAPI(hModule_dll, dtypename[dtype.d3d8], Hook::GetProcAddress(dll, "Direct3DCreate8"), "Direct3DCreate8", D3d8to9::Direct3DCreate8);
			}
		}

		// Check for d3d9_wrap module
		bool StartD3d9Wrap = false;
		if (Config.AntiAliasing)
		{
			StartD3d9Wrap = true;
		}
#ifdef D3D9LOGGING
		StartD3d9Wrap = true;
#endif // D3D9LOGGING

		// Start d3d9_wrap module
		if (StartD3d9Wrap)
		{
			// Load d3d9 procs
			HMODULE dll = LoadLibrary(dtypename[dtype.d3d9]);
			d3d9_wrap::Direct3DCreate9_Proxy = Hook::GetProcAddress(dll, "Direct3DCreate9");

			// If wrapper mode is d3d9 update wrapper
			if (Config.RealWrapperMode == dtype.d3d9)
			{
				// Update dxwrapper.dll -> d3d9_wrap
				d3d8::Direct3DCreate8_var = d3d9_wrap::Direct3DCreate9;
			}
			// Hook d3d9 APIs for d3d9_wrap
			else
			{
				// Hook d3d9.dll -> d3d9_wrap
				Logging::Log() << "Hooking d3d9.dll APIs...";
				d3d9_wrap::Direct3DCreate9_Proxy = (FARPROC)Hook::HookAPI(hModule_dll, dtypename[dtype.d3d9], Hook::GetProcAddress(dll, "Direct3DCreate9"), "Direct3DCreate9", d3d9_wrap::Direct3DCreate9);
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
				HMODULE dll = dsound::Load((Config.RealDllPath.size()) ? Config.RealDllPath.c_str() : nullptr);
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

		// Start fullscreen thread
		if (Config.FullScreen || Config.ForceTermination)
		{
			Utils::Fullscreen::StartThread();
		}

		// Resetting thread priority
		SetThreadPriority(hCurrentThread, THREAD_PRIORITY_NORMAL);

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
		Logging::Log() << "Unhooking APIs";
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

		// Clean up memory
		Config.CleanUp();

		// Final log
		Logging::Log() << "DxWrapper terminated!";
		break;
	}
	return true;
}
