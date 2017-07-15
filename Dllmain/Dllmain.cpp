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
#include "Dllmain.h"
#include "dxwrapper.h"
#include "Wrappers\wrapper.h"
#include "DDrawCompatExternal.h"
#include "DxWndExternal.h"
#include "DSoundCtrl\DSoundCtrlExternal.h"
#include "Utils\Utils.h"
#include "Fullscreen\Fullscreen.h"
#include "Disasm\initdisasm.h"
#include "Writememory\writememory.h"

// Declare varables
HMODULE hModule_dll = nullptr;
HMODULE dxwnd_dll = nullptr;
screen_res m_ScreenRes;

// Check and store screen resolution
void CallCheckCurrentScreenRes()
{
	CheckCurrentScreenRes(m_ScreenRes);
}

// Clean up tasks and unhook dlls
void RunExitFunctions(bool ForceTerminate)
{
	// *** Force termination ***
	if (ForceTerminate)
	{
		LOG << "Process not exiting, attempting to terminate process...";

		// Setting screen resolution to fix some display errors on exit
		if (Config.ResetScreenRes)
		{
			SetScreen(m_ScreenRes);
		}

		// Reset screen back to original Windows settings to fix some display errors on exit
		ResetScreen();

		// Terminate the current process
		HANDLE processHandle = OpenProcess(PROCESS_ALL_ACCESS, false, GetCurrentProcessId());
		LOG << "Terminating process!";
		TerminateProcess(processHandle, 0);
	}

	// *** Normal exit ***
	LOG << "Quiting dxwrapper";

	// Stop threads
	StopFullscreenThread();
	StopWriteMemoryThread();

	// Setting screen resolution before exit
	// Should be run after StopThread and before unloading anything else
	if (Config.ResetScreenRes)
	{
		SetScreen(m_ScreenRes);
	}

	// Unload and Unhook DxWnd
	if (Config.DxWnd)
	{
		LOG << "Unloading dxwnd";
		DxWndEndHook();
		if (dxwnd_dll)
		{
			FreeLibrary(dxwnd_dll);
		}
	}

	// Unload and Unhook DDrawCompat
	if (Config.DDrawCompat)
	{
		UnloadDdrawCompat();
	}

	// Unload dlls
	DllDetach();

	// Clean up memory
	Config.CleanUp();

	// Unload exception handler
	if (Config.HandleExceptions)
	{
		UnHookExceptionHandler();
	}

	// Reset screen back to original Windows settings to fix some display errors on exit
	ResetScreen();

	// Final log
	LOG << "dxwrapper terminated!";
}

// Dll main function
bool APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	UNREFERENCED_PARAMETER(lpReserved);

	static bool FullscreenThreadStartedFlag = false;

	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
		// Get handle
		hModule_dll = hModule;
		HANDLE hCurrentThread = GetCurrentThread();

		// Set thread priority a trick to reduce concurrency problems at program startup
		SetThreadPriority(hCurrentThread, THREAD_PRIORITY_HIGHEST);

		// Init logs
		LOG << "Starting dxwrapper v" << APP_VERSION;
		LogOSVersion();
		LogProcessNameAndPID();

		// Initialize config
		Config.Init();

		// Launch processes
		if (Config.szShellPath[0] != '\0')
		{
			Shell(Config.szShellPath);
		}

		// Set application compatibility options
		if (Config.AddressPointerCount > 0 && Config.BytesToWriteCount > 0)
		{
			WriteMemory();
		}
		if (Config.HandleExceptions)
		{
			HookExceptionHandler();
		}
		if (Config.DpiAware)
		{
			DisableHighDPIScaling();
		}
		SetAppCompat();
		if (Config.Affinity)
		{
			SetProcessAffinityMask(GetCurrentProcess(), 1);
		}

		// Attach real wrapper dll
		DllAttach();

		// Start compatibility modules
		if (Config.DDrawCompat)
		{
			Config.DDrawCompat = StartDdrawCompat(hModule_dll);
		}
		if (Config.DSoundCtrl)
		{
			RunDSoundCtrl();
		}
		if (Config.DxWnd)
		{
			// Check if dxwnd.dll exists then load it
			dxwnd_dll = LoadLibrary("dxwnd.dll");
			if (dxwnd_dll)
			{
				LOG << "Loading dxwnd";
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
			StartFullscreenThread();
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
			CheckCurrentScreenRes(m_ScreenRes);
		}

		// Check if thread has started
		if (Config.ForceTermination && IsFullscreenThreadRunning())
		{
			FullscreenThreadStartedFlag = true;
		}
		break;
	case DLL_THREAD_DETACH:
		if (Config.ForceTermination)
		{
			// Check if thread has started
			if (IsFullscreenThreadRunning())
			{
				FullscreenThreadStartedFlag = true;
			}

			// Check if thread has stopped
			if (FullscreenThreadStartedFlag && !IsFullscreenThreadRunning())
			{
				RunExitFunctions(true);
			}
		}
		break;
	case DLL_PROCESS_DETACH:
		// Run all clean up functions
		RunExitFunctions();
		break;
	}
	return true;
}