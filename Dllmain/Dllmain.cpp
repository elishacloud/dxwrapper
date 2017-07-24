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
#include "DDrawCompat\DDrawCompatExternal.h"
#include "DxWnd\DxWndExternal.h"
#include "DSoundCtrl\DSoundCtrlExternal.h"
#include "Utils\Utils.h"
#include "Fullscreen\Fullscreen.h"
#include "Logging\Logging.h"

// Declare varables
HMODULE hModule_dll = nullptr;

// Dll main function
bool APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved)
{
	UNREFERENCED_PARAMETER(lpReserved);

	static bool FullscreenThreadStartedFlag = false;
	static HMODULE dxwnd_dll = nullptr;

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
		Logging::Log() << "Starting dxwrapper v" << APP_VERSION;
		Logging::LogOSVersion();
		Logging::LogProcessNameAndPID();

		// Initialize config
		Config.Init();

		// Launch processes
		if (Config.szShellPath[0] != '\0')
		{
			Utils::Shell(Config.szShellPath);
		}

		// Set application compatibility options
		if (Config.AddressPointerCount > 0 && Config.BytesToWriteCount > 0)
		{
			Utils::WriteMemory();
		}
		if (Config.HandleExceptions)
		{
			Utils::HookExceptionHandler();
		}
		if (Config.DpiAware)
		{
			Utils::DisableHighDPIScaling();
		}
		Utils::SetAppCompat();
		if (Config.Affinity)
		{
			SetProcessAffinityMask(GetCurrentProcess(), 1);
		}

		// Attach real wrapper dll
		Wrapper::DllAttach();

		// Start compatibility modules
		if (Config.DDrawCompat)
		{
			Config.DDrawCompat = (DllMain_DDrawCompat(hModule_dll, fdwReason, nullptr) != 0);
		}
		if (Config.DSoundCtrl)
		{
			DllMain_DSoundCtrl(hModule, fdwReason, nullptr);
		}
		if (Config.DxWnd)
		{
			// Check if dxwnd.dll exists then load it
			dxwnd_dll = LoadLibrary("dxwnd.dll");
			if (dxwnd_dll)
			{
				Logging::Log() << "Loading dxwnd";
				InitDxWnd();
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
			Fullscreen::StartThread();
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
			Fullscreen::CheckCurrentScreenRes();
		}

		// Check if thread has started
		if (Config.ForceTermination && Fullscreen::IsThreadRunning())
		{
			FullscreenThreadStartedFlag = true;
		}
		break;
	case DLL_THREAD_DETACH:
		if (Config.ForceTermination)
		{
			// Check if thread has started
			if (Fullscreen::IsThreadRunning())
			{
				FullscreenThreadStartedFlag = true;
			}

			// Check if thread has stopped
			if (FullscreenThreadStartedFlag && !Fullscreen::IsThreadRunning())
			{
				Logging::Log() << "Process not exiting, attempting to terminate process...";

				// Reset screen back to original Windows settings to fix some display errors on exit
				Fullscreen::ResetScreen();

				// Terminate the current process
				HANDLE processHandle = OpenProcess(PROCESS_ALL_ACCESS, false, GetCurrentProcessId());
				Logging::Log() << "Terminating process!";
				TerminateProcess(processHandle, 0);
			}
		}
		break;
	case DLL_PROCESS_DETACH:
		// Run all clean up functions
		Logging::Log() << "Quiting dxwrapper";

		// Stop threads
		Fullscreen::StopThread();
		Utils::StopWriteMemoryThread();

		// Unload and Unhook DxWnd
		if (Config.DxWnd)
		{
			Logging::Log() << "Unloading dxwnd";
			DxWndEndHook();
			if (dxwnd_dll)
			{
				FreeLibrary(dxwnd_dll);
			}
		}

		// Unload and Unhook DDrawCompat
		if (Config.DDrawCompat)
		{
			DllMain_DDrawCompat(nullptr, DLL_PROCESS_DETACH, nullptr);
		}
		if (Config.DSoundCtrl)
		{
			DllMain_DSoundCtrl(nullptr, DLL_PROCESS_DETACH, nullptr);
		}

		// Unload dlls
		Wrapper::DllDetach();

		// Clean up memory
		Config.CleanUp();

		// Reset screen back to original Windows settings to fix some display errors on exit
		Fullscreen::ResetScreen();

		// Final log
		Logging::Log() << "dxwrapper terminated!";
		break;
	}
	return true;
}
