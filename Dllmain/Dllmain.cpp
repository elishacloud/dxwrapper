/**
* Copyright (C) 2022 Elisha Riedlinger
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

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Shlwapi.h>
#include "Settings\Settings.h"
#include "Wrappers\wrapper.h"
#include "External\Hooking\Hook.h"
#include "DDrawCompat\DDrawCompatExternal.h"
#include "DxWnd\DxWndExternal.h"
#include "Utils\Utils.h"
#include "Logging\Logging.h"
// Wrappers last
#include "IClassFactory\IClassFactory.h"
#include "GDI\GDI.h"
#include "ddraw\ddrawExternal.h"
#include "dinput\dinputExternal.h"
#include "dinput8\dinput8External.h"
#include "d3d8\d3d8External.h"
#include "d3d9\d3d9External.h"
#include "dsound\dsoundExternal.h"
#include "dxwrapper.h"

#define SHIM_WRAPPED_PROC(procName, unused) \
	Wrapper::ShimProc(procName ## _var, procName ## _in, procName ## _out);

#define SET_WRAPPED_PROC(procName, unused) \
	procName ## _var = procName ## _in;

#define SET_WRAPPED_PROC_SHARED(procName, unused) \
	ShardProcs::procName ## _var = procName ## _funct;

#define HOOK_WRAPPED_PROC(procName, unused) \
	if (GetProcAddress(dll, #procName)) \
	{ \
		FARPROC prodAddr = (FARPROC)Hook::HookAPI(dll, dllname, Hook::GetProcAddress(dll, #procName), #procName, procName ## _funct); \
		if (prodAddr) \
		{ \
			procName ## _var = prodAddr; \
			Logging::LogDebug() << __FUNCTION__ << " " << #procName << " addr: " << prodAddr; \
		} \
	}

#define HOOK_FORCE_WRAPPED_PROC(procName, unused) \
	if (GetProcAddress(dll, #procName)) \
	{ \
		FARPROC prodAddr = (FARPROC)Hook::HotPatch(Hook::GetProcAddress(dll, #procName), #procName, procName ## _funct, true); \
		Logging::LogDebug() << __FUNCTION__ << " " << #procName << " addr: " << prodAddr; \
	}

void WINAPI DxWrapperSettings(DXWAPPERSETTINGS *DxSettings)
{
	Logging::LogDebug() << __FUNCTION__ << " Called!";
	if (!DxSettings)
	{
		return;
	}

	DxSettings->Dd7to9 = Config.Dd7to9;
	DxSettings->D3d8to9 = Config.D3d8to9;
	DxSettings->Dinputto8 = Config.Dinputto8;
}

typedef HMODULE(*LoadProc)(const char *ProxyDll, const char *MyDllName);

HMODULE LoadHookedDll(char *dllname, LoadProc Load, DWORD HookSystem32)
{
	HMODULE dll = Load(nullptr, Config.WrapperName.c_str());
	HMODULE currentdll = GetModuleHandle(dllname);

	if (dllname && (Config.IsSet(HookSystem32) || (HookSystem32 == NOT_EXIST && currentdll && GetProcAddress(currentdll, "DxWrapperSettings") == nullptr)))
	{
		char path[MAX_PATH];
		GetSystemDirectory(path, MAX_PATH);
		PathAppend(path, dllname);
		HMODULE lib = LoadLibrary(path);
		if (lib)
		{
			dll = lib;
		}
	}

	if (dll)
	{
		Utils::AddHandleToVector(dll, dllname);
	}

	return dll;
}

// Declare variables
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

		// Initialize config
		Config.Init();

		// Init logs
		Logging::EnableLogging = !Config.DisableLogging;
		Logging::InitLog();
		Logging::Log() << "Starting DxWrapper v" << APP_VERSION;
		{
			char path[MAX_PATH];
			GetModuleFileName(hModule, path, MAX_PATH);
			Logging::Log() << "Running from: " << path;
		}
		Config.SetConfig();			// Finish setting up config
		Logging::LogComputerManufacturer();
		Logging::LogVideoCard();
		Logging::LogOSVersion();
		Logging::LogProcessNameAndPID();
		Logging::LogGameType();
		Logging::LogCompatLayer();

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

			// Disable wrapper
			Logging::Log() << "Disabling DxWrapper...";
			Settings::ClearConfigSettings();

			if (Config.RealWrapperMode == dtype.dxwrapper)
			{
				// Return false on process attach causes dll to get unloaded
				return true;
			}
			else
			{
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
			char path[MAX_PATH] = { 0 };
			GetModuleFileName(hModule, path, MAX_PATH);

			// Hook GetModuleFileName to fix module name in modules loaded from memory
			if (!PathFileExists(path))
			{
				HMODULE dll = LoadLibrary("kernel32.dll");
				if (dll)
				{
					Logging::Log() << "Hooking 'GetModuleFileName' API...";
					InterlockedExchangePointer((PVOID*)&Utils::pGetModuleFileNameA, Hook::HookAPI(dll, "kernel32.dll", Hook::GetProcAddress(dll, "GetModuleFileNameA"), "GetModuleFileNameA", Utils::GetModuleFileNameAHandler));
					InterlockedExchangePointer((PVOID*)&Utils::pGetModuleFileNameW, Hook::HookAPI(dll, "kernel32.dll", Hook::GetProcAddress(dll, "GetModuleFileNameW"), "GetModuleFileNameW", Utils::GetModuleFileNameWHandler));
				}
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
		if (Config.ResetScreenRes)
		{
			Utils::GetScreenSettings();
		}
		if (Config.MemoryInfo.size() != 0)
		{
			Utils::WriteMemory::WriteMemory();
		}
		if (Config.DisableHighDPIScaling)
		{
			Utils::DisableHighDPIScaling();
		}
		if (Config.SingleProcAffinity)
		{
			Utils::SetProcessAffinity();
		}
		if (Config.DisableGameUX)
		{
			Utils::DisableGameUX();
		}
		if (Config.HandleExceptions)
		{
			Utils::HookExceptionHandler();
		}

		// Hook CoCreateInstance
		if (Config.EnableDdrawWrapper || Config.Dd7to9 || Config.EnableDinput8Wrapper || Config.Dinputto8)
		{
			InterlockedExchangePointer((PVOID*)&p_CoCreateInstance, Hook::HotPatch(GetProcAddress(LoadLibraryA("ole32.dll"), "CoCreateInstance"), "CoCreateInstance", *CoCreateInstanceHandle));
		}

		// Start dsound.dll module
		if (Config.EnableDsoundWrapper)
		{
			using namespace dsound;
			using namespace DsoundWrapper;

			// Initialize dsound wrapper procs
			if (Config.RealWrapperMode == dtype.dsound)
			{
				VISIT_PROCS_DSOUND_SHARED(SET_WRAPPED_PROC_SHARED);
			}
			else
			{
				// Load dsound procs
				char *dllname = dtypename[dtype.dsound];
				HMODULE dll = LoadHookedDll(dllname, Load, Config.DsoundHookSystem32);

				// Hook dsound.dll -> DsoundWrapper
				Logging::Log() << "Hooking dsound.dll APIs...";
				VISIT_PROCS_DSOUND(HOOK_WRAPPED_PROC);
				VISIT_PROCS_DSOUND_SHARED(HOOK_WRAPPED_PROC);
			}

			// Prepare wrapper
			VISIT_PROCS_DSOUND(SHIM_WRAPPED_PROC);
			VISIT_PROCS_DSOUND_SHARED(SHIM_WRAPPED_PROC);
		}

		// Start dinput.dll module
		if (Config.Dinputto8)
		{
			Logging::Log() << "Enabling dinputto8 wrapper";

			using namespace dinput;
			using namespace DinputWrapper;

			// Initialize dinput wrapper procs
			if (Config.RealWrapperMode == dtype.dinput)
			{
				VISIT_PROCS_DINPUT_SHARED(SET_WRAPPED_PROC_SHARED);
			}
			else
			{
				// Load dinput procs
				char *dllname = dtypename[dtype.dinput];
				HMODULE dll = LoadHookedDll(dllname, Load, Config.DinputHookSystem32);

				// Hook dinput.dll APIs
				Logging::Log() << "Hooking dinput.dll APIs...";
				VISIT_PROCS_DINPUT(HOOK_FORCE_WRAPPED_PROC);
				VISIT_PROCS_DINPUT_SHARED(HOOK_FORCE_WRAPPED_PROC);
			}

			// Prepare wrapper
			VISIT_PROCS_DINPUT(SET_WRAPPED_PROC);
			VISIT_PROCS_DINPUT_SHARED(SET_WRAPPED_PROC);
		}

		// Start dinput8.dll module
		if (Config.EnableDinput8Wrapper || Config.Dinputto8)
		{
			Logging::Log() << "Enabling dinput8 wrapper";

			using namespace dinput8;
			using namespace Dinput8Wrapper;

			// Initialize dinput8 wrapper procs
			if (Config.RealWrapperMode == dtype.dinput8)
			{
				VISIT_PROCS_DINPUT8_SHARED(SET_WRAPPED_PROC_SHARED);
			}
			else
			{
				// Load dinput8 procs
				char *dllname = dtypename[dtype.dinput8];
				HMODULE dll = LoadHookedDll(dllname, Load, Config.Dinput8HookSystem32);

				// Hook dinput8.dll APIs
				if (Config.EnableDinput8Wrapper)
				{
					Logging::Log() << "Hooking dinput8.dll APIs...";
					VISIT_PROCS_DINPUT8(HOOK_WRAPPED_PROC);
					VISIT_PROCS_DINPUT8_SHARED(HOOK_WRAPPED_PROC);
				}
			}

			// dinputto8 -> dinput8Wrapper
			if (Config.Dinputto8)
			{
				DinputWrapper::DirectInput8Create_out = DirectInput8Create_in;
				DinputWrapper::DllCanUnloadNow_out = DllCanUnloadNow_in;
				DinputWrapper::DllGetClassObject_out = DllGetClassObject_in;
				DinputWrapper::DllRegisterServer_out = DllRegisterServer_in;
				DinputWrapper::DllUnregisterServer_out = DllUnregisterServer_in;
			}

			// Prepare wrapper
			VISIT_PROCS_DINPUT8(SHIM_WRAPPED_PROC);
			VISIT_PROCS_DINPUT8_SHARED(SHIM_WRAPPED_PROC);
		}

		// Start ddraw.dll module
		if (Config.EnableDdrawWrapper || Config.DDrawCompat)
		{
			// Initialize ddraw wrapper procs
			if (Config.RealWrapperMode == dtype.ddraw)
			{
				using namespace ddraw;
				using namespace DdrawWrapper;
				VISIT_PROCS_DDRAW_SHARED(SET_WRAPPED_PROC_SHARED);
			}
			else
			{
				using namespace ddraw;
				using namespace DdrawWrapper;

				// Load ddraw procs
				char *dllname = dtypename[dtype.ddraw];
				HMODULE dll = LoadHookedDll(dllname, Load, Config.DdrawHookSystem32);

				// Hook ddraw.dll APIs
				Logging::Log() << "Hooking ddraw.dll APIs...";
				if (!Config.Dd7to9)
				{
					VISIT_DOCUMENTED_DDRAW_PROCS(HOOK_WRAPPED_PROC);
				}
				else
				{
					VISIT_PROCS_DDRAW(HOOK_FORCE_WRAPPED_PROC);
					VISIT_PROCS_DDRAW_SHARED(HOOK_FORCE_WRAPPED_PROC);
				}
			}

			// Start Dd7to9
			if (Config.Dd7to9)
			{
				InitDDraw();
				using namespace ddraw;
				using namespace DdrawWrapper;
				VISIT_PROCS_DDRAW(SET_WRAPPED_PROC);
				VISIT_PROCS_DDRAW_SHARED(SET_WRAPPED_PROC);
				HMODULE d3d9_dll = LoadLibrary("d3d9.dll");
				DdrawWrapper::Direct3DCreate9_out = GetProcAddress(d3d9_dll, "Direct3DCreate9");
			}

			// Add DDrawCompat to the chain
			if (Config.DDrawCompat)
			{
				Logging::Log() << "Enabling DDrawCompat";
				using namespace ddraw;
				using namespace DDrawCompat;
				DDrawCompat::Prepare();
				VISIT_PROCS_DDRAW(SHIM_WRAPPED_PROC);
				VISIT_PROCS_DDRAW_SHARED(SHIM_WRAPPED_PROC);
			}

			// Add DdrawWrapper to the chain
			if (Config.EnableDdrawWrapper)
			{
				Logging::Log() << "Enabling ddraw wrapper";
				using namespace ddraw;
				using namespace DdrawWrapper;
				VISIT_PROCS_DDRAW(SHIM_WRAPPED_PROC);
				VISIT_PROCS_DDRAW_SHARED(SHIM_WRAPPED_PROC);
			}

			// Start DDrawCompat
			if (Config.DDrawCompat)
			{
				Config.DDrawCompat = DDrawCompat::Start(hModule_dll, DLL_PROCESS_ATTACH);
			}
		}

		// Start d3d8.dll module
		if (Config.D3d8to9)
		{
			Logging::Log() << "Enabling d3d8to9 wrapper";

			using namespace d3d8;
			using namespace D3d8Wrapper;

			// Initialize d3d8 wrapper procs
			if (Config.RealWrapperMode != dtype.d3d8)
			{
				// Load d3d8 procs
				char *dllname = dtypename[dtype.d3d8];
				HMODULE dll = LoadHookedDll(dllname, Load, Config.D3d8HookSystem32);

				// Hook d3d8.dll -> D3d8to9
				Logging::Log() << "Hooking d3d8.dll APIs...";
				VISIT_PROCS_D3D8(HOOK_FORCE_WRAPPED_PROC);
			}

			// Prepare wrapper
			VISIT_PROCS_D3D8(SET_WRAPPED_PROC);
		}

		// Start d3d9.dll module
		if (Config.EnableD3d9Wrapper || Config.D3d8to9 || Config.Dd7to9)
		{
			Logging::Log() << "Enabling d3d9 wrapper";

			using namespace d3d9;
			using namespace D3d9Wrapper;

			// Initialize d3d9 wrapper procs
			if (Config.RealWrapperMode != dtype.d3d9)
			{
				// Load d3d9 procs
				char *dllname = dtypename[dtype.d3d9];
				HMODULE dll = LoadHookedDll(dllname, Load, Config.D3d9HookSystem32);

				// Hook d3d9.dll -> D3d9Wrapper
				if (Config.EnableD3d9Wrapper)
				{
					Logging::Log() << "Hooking d3d9.dll APIs...";
					HOOK_WRAPPED_PROC(Direct3DCreate9, unused);
					HOOK_WRAPPED_PROC(Direct3DCreate9Ex, unused);
					HOOK_WRAPPED_PROC(Direct3DCreate9On12, unused);
					HOOK_WRAPPED_PROC(Direct3DCreate9On12Ex, unused);
				}
			}

			// Redirect d3d8to9 -> D3d9Wrapper
			if (Config.D3d8to9)
			{
				D3d8Wrapper::Direct3DCreate9_out = Direct3DCreate9_in;
			}

			// Redirect DdrawWrapper -> D3d9Wrapper
			if (Config.Dd7to9)
			{
				DdrawWrapper::Direct3DCreate9_out = Direct3DCreate9_in;
			}

			// Prepare wrapper
			VISIT_PROCS_D3D9(SHIM_WRAPPED_PROC);
		}

		// Hook other gdi32 and user32 APIs
		if (Config.Dd7to9)
		{
			using namespace GdiWrapper;
			GetDeviceCaps_out = (FARPROC)Hook::HotPatch(Hook::GetProcAddress(LoadLibrary("gdi32.dll"), "GetDeviceCaps"), "GetDeviceCaps", gdi_GetDeviceCaps);
			CreateWindowExA_out = (FARPROC)Hook::HotPatch(Hook::GetProcAddress(LoadLibrary("user32.dll"), "CreateWindowExA"), "CreateWindowExA", user_CreateWindowExA);
			CreateWindowExW_out = (FARPROC)Hook::HotPatch(Hook::GetProcAddress(LoadLibrary("user32.dll"), "CreateWindowExW"), "CreateWindowExW", user_CreateWindowExW);
			DestroyWindow_out = (FARPROC)Hook::HotPatch(Hook::GetProcAddress(LoadLibrary("user32.dll"), "DestroyWindow"), "DestroyWindow", user_DestroyWindow);
			GetSystemMetrics_out = (FARPROC)Hook::HotPatch(Hook::GetProcAddress(LoadLibrary("user32.dll"), "GetSystemMetrics"), "GetSystemMetrics", user_GetSystemMetrics);
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
					Utils::ResetScreenSettings();
				}

				// Terminate the current process
				Logging::Log() << __FUNCTION__ << " Terminating process!";
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
		if (Config.Dd7to9)
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
			Utils::ResetScreenSettings();
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
