/**
* Copyright (C) 2025 Elisha Riedlinger
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
#include "winmm.h"
#include "GDI\GDI.h"
#include "GDI\WndProc.h"
#include "External\Hooking\Hook.h"
#ifdef DDRAWCOMPAT
#include "DDrawCompat\DDrawCompatExternal.h"
#include "DDrawCompat\v0.3.2\Win32\Version.h"
#endif // DDRAWCOMPAT
#include "Utils\Utils.h"
#include "Logging\Logging.h"
// Wrappers last
#include "IClassFactory\IClassFactory.h"
#include "d3d9\d3d9External.h"
#include "ddraw\ddrawExternal.h"
#include "dinput\dinputExternal.h"
#include "dinput8\dinput8External.h"
#include "d3d8\d3d8External.h"
#include "dsound\dsoundExternal.h"
#include "Libraries\ScopeGuard.h"
#include "dxwrapper.h"

#include <sstream>

typedef PVOID(WINAPI* AddHandlerFunc)(ULONG, PVECTORED_EXCEPTION_HANDLER);
typedef ULONG(WINAPI* RemoveHandlerFunc)(PVOID);

static PVOID g_exception_handle = nullptr;
static AddHandlerFunc add_handler = nullptr;
static RemoveHandlerFunc remove_handler = nullptr;

#define SHIM_WRAPPED_PROC(procName, unused) \
	Wrapper::ShimProc(procName ## _var, procName ## _in, procName ## _out);

#define SET_WRAPPED_PROC(procName, unused) \
	procName ## _var = procName ## _in;

#define SET_WRAPPED_PROC_SHARED(procName, unused) \
	ShardProcs::procName ## _var = procName ## _funct;

#define HOOK_WRAPPED_PROC(procName, unused) \
	if (GetProcAddress(dll, #procName)) \
	{ \
		FARPROC prodAddr = (FARPROC)Hook::HotPatch(Hook::GetProcAddress(dll, #procName), #procName, procName ## _funct); \
		if (prodAddr) \
		{ \
			procName ## _var = prodAddr; \
			Logging::LogDebug() << __FUNCTION__ << " " << #procName << " addr: " << prodAddr; \
		} \
	}

__declspec(dllexport) void WINAPI DxWrapperSettings(DXWAPPERSETTINGS *DxSettings)
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

__declspec(dllexport) void WINAPI DxWrapperLogging(const char* LogMessage)
{
	Logging::Log() << __FUNCTION__ << " " << LogMessage;
}

typedef HMODULE(*LoadProc)(const char *ProxyDll, const char *MyDllName);

static HMODULE LoadHookedDll(const char *dllname, LoadProc Load, DWORD HookSystem32)
{
	HMODULE dll = Load(nullptr, Config.WrapperName.c_str());

	if (dllname)
	{
		CreateScopedHeapBuffer(char, path, MAX_PATH);
		if (GetSystemDirectoryA(path, MAX_PATH))
		{
			PathAppend(path, dllname);

			if (Config.IsSet(HookSystem32) || (HookSystem32 == NOT_EXIST && GetModuleHandleA(path)))
			{
				HMODULE lib = LoadLibrary(path);
				if (lib)
				{
					dll = lib;
				}
			}
		}
	}

	if (dll)
	{
		Utils::AddHandleToVector(dll, dllname);
	}

	return dll;
}

static bool CheckForDuplicateLoad(HMODULE hModule, HANDLE& hMutex)
{
	// Get mutex name
	CreateScopedHeapBuffer(char, MutexName, MAX_PATH);
	if (Config.RealWrapperMode == dtype.dxwrapper)
	{
		sprintf_s(MutexName, MAX_PATH, "DxWrapper_%d", GetCurrentProcessId());
	}
	else
	{
		sprintf_s(MutexName, MAX_PATH, "DxWrapper_%d_%s", GetCurrentProcessId(), Config.WrapperName.c_str());
	}

	// Check if mutex exists
	HANDLE existingMutex = OpenMutex(SYNCHRONIZE, FALSE, MutexName);
	if (existingMutex)
	{
		CloseHandle(existingMutex);

		// Prepare message
		std::stringstream message;
		CreateScopedHeapBuffer(char, dllPath, MAX_PATH);

		// Get the DLL path if possible
		if (GetModuleFileNameA(hModule, dllPath, MAX_PATH))
		{
			message << "DxWrapper is attempting to load '" << dllPath << "' at address " << hModule
				<< " but is already loaded at a different address (" << MutexName << ").";
		}
		else
		{
			message << "DxWrapper is attempting to load at address " << hModule
				<< " but is already loaded at a different address (" << MutexName << ").";
		}

		// Show message box
		MessageBoxA(nullptr, message.str().c_str(), "DxWrapper Duplicate Load Detected", MB_ICONWARNING | MB_OK | MB_TOPMOST);

		return TRUE; // Causes DLL to unload on process attach
	}

	// Create mutex
	hMutex = CreateMutex(nullptr, FALSE, MutexName);

	// Check if mutex creation was successful
	if (hMutex == nullptr)
	{
		// Prepare message
		DWORD errorCode = GetLastError();
		std::stringstream message;
		message << "Failed to create mutex for DxWrapper (" << MutexName << "). Error code: " << errorCode;

		// Show message box
		MessageBoxA(nullptr, message.str().c_str(), "Mutex Creation Error", MB_ICONERROR | MB_OK | MB_TOPMOST);

		return TRUE; // If mutex creation fails, return true to unload the DLL
	}

	return FALSE; // Allow the DLL to continue loading
}

// Declare variables
HMODULE hModule_dll = nullptr;

// Dll main function
BOOL APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved)
{
	UNREFERENCED_PARAMETER(lpReserved);

	static HANDLE hMutex = nullptr;
	static bool FullscreenThreadStartedFlag = false;

	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
	{
		// Get handle
		hModule_dll = hModule;

		// Check if already loaded
		{
			HMODULE dxw_dll = GetModuleHandleA("dxwrapper.dll");
			if (dxw_dll && dxw_dll != hModule_dll)
			{
				return FALSE;
			}
			HMODULE dxw_asi = GetModuleHandleA("dxwrapper.asi");
			if (dxw_asi && dxw_asi != hModule_dll)
			{
				return FALSE;
			}
		}

		// Initialize config
		Config.Init();

		// Ensure only one copy of DxWrapper is running
		if (CheckForDuplicateLoad(hModule, hMutex))
		{
			return FALSE;
		}

		// Init logs
		Logging::EnableLogging = !Config.DisableLogging;
		Logging::InitLog();
		bool IsRunningFromMemory = false;
		Logging::Log() << "Starting DxWrapper v" << APP_VERSION;
		{
			CreateScopedHeapBuffer(char, path, MAX_PATH);
			SetLastError(0);
			DWORD size = GetModuleFileName(hModule, path, MAX_PATH);
			DWORD errorCode = GetLastError();
			if (errorCode == ERROR_INSUFFICIENT_BUFFER)
			{
				Logging::Log() << "Warning: folder path is too long!";
			}
			else if (errorCode != ERROR_SUCCESS || size == NULL)
			{
				Logging::Log() << "Error: getting module name!";
			}

			// Check if it is loading from a file
			if (PathFileExists(path))
			{
				Logging::Log() << "Running from: " << path << " (" << hModule << ")";
			}
			else
			{
				IsRunningFromMemory = true;
				Logging::Log() << "Module is running from MEMORY!";
			}
		}
		Config.SetConfig();			// Finish setting up config
		Logging::LogComputerManufacturer();
		Logging::LogVideoCard();
		Logging::LogOSVersion();
		Logging::LogProcessNameAndPID();
		Logging::Log() << GetCommandLineA();
		Logging::LogGameType();
		Logging::LogCompatLayer();
		Logging::Log() << "Windows is newer than," <<
			" Windows Vista: " << Utils::IsWindowsVistaOrNewer() <<
			" Windows 7: " << Utils::IsWindows7OrNewer() <<
			" Windows 8: " << Utils::IsWindows8OrNewer();

		// Check system modules
		if (Config.Dd7to9 && Utils::CheckIfSystemModuleLoaded("ddraw.dll"))
		{
			Logging::Log() << "Warning: System 'ddraw.dll' is already loaded before dxwrapper!";
		}
		if (Config.D3d8to9 && Utils::CheckIfSystemModuleLoaded("d3d8.dll"))
		{
			Logging::Log() << "Warning: System 'd3d8.dll' is already loaded before dxwrapper!";
		}
		if (Config.Dinputto8 && Utils::CheckIfSystemModuleLoaded("dinput.dll"))
		{
			Logging::Log() << "Warning: System 'dinput.dll' is already loaded before dxwrapper!";
		}

		// Check if process is excluded
		if (Config.ProcessExcluded)
		{
			Logging::Log() << "Disabling DxWrapper...";
			Settings::ClearConfigSettings();
		}

		// Attach real dll
		HMODULE kernel32 = GetModuleHandleA("Kernel32.dll");
		if (Config.RealWrapperMode == dtype.dxwrapper)
		{
			// Hook GetModuleFileName to fix module name in modules loaded from memory
			if (IsRunningFromMemory)
			{
				if (kernel32)
				{
					Logging::Log() << "Hooking 'GetModuleFileName' API...";
					InterlockedExchangePointer((PVOID*)&Utils::GetModuleFileNameA_out, Hook::HotPatch(Hook::GetProcAddress(kernel32, "GetModuleFileNameA"), "GetModuleFileNameA", Utils::GetModuleFileNameAHandler));
					InterlockedExchangePointer((PVOID*)&Utils::GetModuleFileNameW_out, Hook::HotPatch(Hook::GetProcAddress(kernel32, "GetModuleFileNameW"), "GetModuleFileNameW", Utils::GetModuleFileNameWHandler));
				}
			}
		}
		else if (!(Config.Dd7to9 && Config.RealWrapperMode == dtype.ddraw) && !(Config.D3d8to9 && Config.RealWrapperMode == dtype.d3d8) && !(Config.Dinputto8 && Config.RealWrapperMode == dtype.dinput))
		{
			// Load real dll and attach wrappers
			HMODULE dll = Wrapper::CreateWrapper((Config.RealDllPath.size()) ? Config.RealDllPath.c_str() : nullptr, (Config.WrapperMode.size()) ? Config.WrapperMode.c_str() : nullptr, Config.WrapperName.c_str());
			if (dll)
			{
				// Pin real dll module
				HMODULE dummy = nullptr;
				GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_PIN | GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, reinterpret_cast<LPCSTR>(dll), &dummy);

				Utils::AddHandleToVector(dll, Config.WrapperName.c_str());

				// Hook GetProcAddress to handle wrapped functions that are missing or not available in the OS
				if (kernel32)
				{
					Logging::Log() << "Hooking 'GetProcAddress' API...";
					InterlockedExchangePointer((PVOID*)&Utils::GetProcAddress_out, Hook::HotPatch(Hook::GetProcAddress(kernel32, "GetProcAddress"), "GetProcAddress", Utils::GetProcAddressHandler));
				}
			}
		}

		// Dynamically load AddVectoredExceptionHandler and RemoveVectoredExceptionHandler
		if (kernel32)
		{
			add_handler = (AddHandlerFunc)GetProcAddress(kernel32, "AddVectoredExceptionHandler");
			remove_handler = (RemoveHandlerFunc)GetProcAddress(kernel32, "RemoveVectoredExceptionHandler");

			// Add the exception handler to the end of the chain if available
			if (add_handler)
			{
				g_exception_handle = add_handler(0, Utils::Vectored_Exception_Handler);  // Set to 0 for end of chain
			}
		}

		// Pin current module
		HMODULE dummy = nullptr;
		GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_PIN | GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, reinterpret_cast<LPCSTR>(DllMain), &dummy);

		// Fix QPC uptime issues (before other compatibility options)
		if (Config.FixPerfCounterUptime && Utils::InitUpTimeOffsets())
		{
			using namespace GdiWrapper;
			if (kernel32)
			{
				Utils::QueryPerformanceFrequency_out = (FARPROC)Hook::HotPatch(GetProcAddress(kernel32, "QueryPerformanceFrequency"), "QueryPerformanceFrequency", Utils::kernel_QueryPerformanceFrequency);
				Utils::QueryPerformanceCounter_out = (FARPROC)Hook::HotPatch(GetProcAddress(kernel32, "QueryPerformanceCounter"), "QueryPerformanceCounter", Utils::kernel_QueryPerformanceCounter);
				Utils::GetTickCount_out = (FARPROC)Hook::HotPatch(GetProcAddress(kernel32, "GetTickCount"), "GetTickCount", Utils::kernel_GetTickCount);
#if (_WIN32_WINNT >= 0x0502)
				Utils::GetTickCount64_out = (FARPROC)Hook::HotPatch(GetProcAddress(kernel32, "GetTickCount64"), "GetTickCount64", Utils::kernel_GetTickCount64);
#endif
			}
			HMODULE winmm = LoadLibrary("winmm.dll");
			if (winmm)
			{
				Logging::Log() << "Installing winmm hooks";
				Utils::timeGetTime_out = (FARPROC)Hook::HotPatch(GetProcAddress(winmm, "timeGetTime"), "timeGetTime", Utils::winmm_timeGetTime);
				Utils::timeGetSystemTime_out = (FARPROC)Hook::HotPatch(GetProcAddress(winmm, "timeGetSystemTime"), "timeGetSystemTime", Utils::winmm_timeGetSystemTime);
			}
		}

		// Hook keyboard layout
		if (Config.ForceKeyboardLayout)
		{
			KeyboardLayout::ForceKeyboardLayout(Config.ForceKeyboardLayout);

			// Hook all windows in this process starting with the first top-level window
			{
				const DWORD pid = GetCurrentProcessId();

				HWND hWnd = GetTopWindow(NULL);

				while (hWnd)
				{
					DWORD wpid = 0;
					GetWindowThreadProcessId(hWnd, &wpid);

					// Add the top-level window
					if (wpid == pid && WndProc::ShouldHook(hWnd))
					{
						WndProc::AddWndProc(hWnd);
					}

					// Next top-level window
					hWnd = GetWindow(hWnd, GW_HWNDNEXT);
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
			WriteMemory::WriteMemory();
		}
		if (Config.DisableHighDPIScaling)
		{
			Utils::DisableHighDPIScaling();
		}
		if (Config.SingleProcAffinity)
		{
			if (!Utils::CreateThread_out)
			{
				Utils::CreateThread_out = (FARPROC)Hook::HotPatch(GetProcAddress(kernel32, "CreateThread"), "CreateThread", Utils::kernel_CreateThread);
			}

			Utils::ApplyThreadAffinity();
		}
		if (Config.DisableGameUX)
		{
			Utils::DisableGameUX();
		}
		if (Config.HandleExceptions)
		{
			Utils::SetCustomExceptionHandler();
		}

		// Hook CoCreateInstance
		if (Config.EnableDdrawWrapper || Config.Dd7to9 || Config.EnableDinput8Wrapper || Config.Dinputto8 || Config.EnableDsoundWrapper)
		{
			HMODULE ole32_dll = LoadLibraryA("ole32.dll");
			if (ole32_dll)
			{
				Logging::Log() << "Hooking ole32.dll APIs...";
				InterlockedExchangePointer((PVOID*)&CoGetClassObject_out, Hook::HotPatch(GetProcAddress(ole32_dll, "CoGetClassObject"), "CoGetClassObject", *CoGetClassObjectHandle));
				InterlockedExchangePointer((PVOID*)&CoCreateInstance_out, Hook::HotPatch(GetProcAddress(ole32_dll, "CoCreateInstance"), "CoCreateInstance", *CoCreateInstanceHandle));
			}
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
				const char *dllname = dtypename[dtype.dsound];
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
				const char *dllname = dtypename[dtype.dinput];
				HMODULE dll = LoadHookedDll(dllname, Load, Config.DinputHookSystem32);

				// Hook dinput.dll APIs
				Logging::Log() << "Hooking dinput.dll APIs...";
				VISIT_PROCS_DINPUT(HOOK_WRAPPED_PROC);
				VISIT_PROCS_DINPUT_SHARED(HOOK_WRAPPED_PROC);
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
				const char *dllname = dtypename[dtype.dinput8];
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
				const char *dllname = dtypename[dtype.ddraw];
				HMODULE dll = LoadHookedDll(dllname, Load, Config.DdrawHookSystem32);

				// Hook ddraw.dll APIs
				Logging::Log() << "Hooking ddraw.dll APIs...";
				if (!Config.Dd7to9)
				{
					VISIT_DOCUMENTED_DDRAW_PROCS(HOOK_WRAPPED_PROC);
				}
				else
				{
					VISIT_PROCS_DDRAW(HOOK_WRAPPED_PROC);
					VISIT_PROCS_DDRAW_SHARED(HOOK_WRAPPED_PROC);
				}
			}

			// Add Dd7to9 to the chain
			if (Config.Dd7to9)
			{
				using namespace ddraw;
				using namespace DdrawWrapper;
				VISIT_PROCS_DDRAW(SHIM_WRAPPED_PROC);
				VISIT_PROCS_DDRAW_SHARED(SHIM_WRAPPED_PROC);
				HMODULE d3d9_dll = LoadLibrary("d3d9.dll");
				DdrawWrapper::Direct3DCreate9_out = GetProcAddress(d3d9_dll, "Direct3DCreate9");
			}
			else
			{
#ifdef DDRAWCOMPAT
				// Add DDrawCompat to the chain
				if (Config.DDrawCompat)
				{
					Logging::Log() << "Enabling DDrawCompat";
					using namespace ddraw;
					using namespace DDrawCompat;
					DDrawCompat::Prepare();
					if (Config.DDrawCompat32)
					{
						VISIT_DOCUMENTED_DDRAW_PROCS(SHIM_WRAPPED_PROC);
					}
					else
					{
						VISIT_BASIC_DDRAW_PROCS(SHIM_WRAPPED_PROC);
					}
					DDrawCompat::Start(hModule_dll, fdwReason);
				}
#endif // DDRAWCOMPAT

				// Add DdrawWrapper to the chain
				if (Config.EnableDdrawWrapper)
				{
					Logging::Log() << "Enabling ddraw wrapper";
					using namespace ddraw;
					using namespace DdrawWrapper;
					VISIT_PROCS_DDRAW(SHIM_WRAPPED_PROC);
					VISIT_PROCS_DDRAW_SHARED(SHIM_WRAPPED_PROC);
				}
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
				const char *dllname = dtypename[dtype.d3d8];
				HMODULE dll = LoadHookedDll(dllname, Load, Config.D3d8HookSystem32);

				// Hook d3d8.dll -> D3d8to9
				Logging::Log() << "Hooking d3d8.dll APIs...";
				VISIT_PROCS_D3D8(HOOK_WRAPPED_PROC);
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
				const char *dllname = dtypename[dtype.d3d9];
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

			// Get default display resolution
			Utils::GetScreenSize(nullptr, InitWidth, InitHeight);
		}

		bool DDrawCompatEnabed = false;
#ifdef DDRAWCOMPAT
		DDrawCompatEnabed = DDrawCompat::IsEnabled();
#endif // DDRAWCOMPAT

		// Set timer
		if (!DDrawCompatEnabed)
		{
			timeBeginPeriod(1);
		}

#ifdef DDRAWCOMPAT
		// Extra compatibility hooks from DDrawCompat
		if (!DDrawCompatEnabed && (Config.Dd7to9 || Config.D3d8to9))
		{
			DDrawCompat::InstallDd7to9Hooks(hModule);
		}
		Win32::Version::installWinLieHooks();
#endif // DDRAWCOMPAT

		// Start Dd7to9
		if (Config.Dd7to9)
		{
			InitDDraw();
		}
		else if (Config.ForceKeyboardLayout)
		{
			Logging::Log() << "Installing User32 hooks";
			HMODULE user32 = GetModuleHandleA("user32.dll");
			if (user32)
			{
				using namespace GdiWrapper;
				CreateWindowExA_out = (FARPROC)Hook::HotPatch(GetProcAddress(user32, "CreateWindowExA"), "CreateWindowExA", user_CreateWindowExA);
				CreateWindowExW_out = (FARPROC)Hook::HotPatch(GetProcAddress(user32, "CreateWindowExW"), "CreateWindowExW", user_CreateWindowExW);
			}
		}

		// Hook Comdlg functions
		if (Config.EnableOpenDialogHook)
		{
			using namespace GdiWrapper;
			if (!GetModuleHandleA("comdlg32.dll")) LoadLibrary("comdlg32.dll");
			HMODULE comdlg32 = GetModuleHandleA("Comdlg32.dll");
			if (comdlg32)
			{
				Logging::Log() << "Installing Comdlg32 hooks";
				GetOpenFileNameA_out = (FARPROC)Hook::HotPatch(GetProcAddress(comdlg32, "GetOpenFileNameA"), "GetOpenFileNameA", comdlg_GetOpenFileNameA);
				GetOpenFileNameW_out = (FARPROC)Hook::HotPatch(GetProcAddress(comdlg32, "GetOpenFileNameW"), "GetOpenFileNameW", comdlg_GetOpenFileNameW);
				GetSaveFileNameA_out = (FARPROC)Hook::HotPatch(GetProcAddress(comdlg32, "GetSaveFileNameA"), "GetSaveFileNameA", comdlg_GetSaveFileNameA);
				GetSaveFileNameW_out = (FARPROC)Hook::HotPatch(GetProcAddress(comdlg32, "GetSaveFileNameW"), "GetSaveFileNameW", comdlg_GetSaveFileNameW);
			}
		}

		// Hook gamma ramp
		if (Config.DisableGDIGammaRamp)
		{
			using namespace GdiWrapper;
			if (!GetModuleHandleA("gdi32.dll")) LoadLibrary("gdi32.dll");
			HMODULE gdi32 = GetModuleHandleA("gdi32.dll");
			if (gdi32)
			{
				SetDeviceGammaRamp_out = (FARPROC)Hook::HotPatch(GetProcAddress(gdi32, "SetDeviceGammaRamp"), "SetDeviceGammaRamp", gdi_SetDeviceGammaRamp);
			}
		}

		// Start fullscreen thread
		if (Config.FullScreen || Config.ForceTermination)
		{
			Fullscreen::StartThread();
		}

		// Start thread priority monitor thread
		//Utils::StartPriorityMonitor();

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

		// Loaded
		Logging::Log() << "DxWrapper loaded!";
	}
	break;
	case DLL_THREAD_ATTACH:

		// Apply affinity to new threads
		if (Config.SingleProcAffinity)
		{
			Utils::SetThreadAffinity(GetCurrentThreadId());
		}

#ifdef DDRAWCOMPAT
		// Unload and Unhook DDrawCompat
		if (DDrawCompat::IsEnabled())
		{
			DDrawCompat::Start(hModule, fdwReason);
		}
#endif // DDRAWCOMPAT

		// Check if thread has started
		if (Config.ForceTermination && Fullscreen::IsThreadRunning())
		{
			FullscreenThreadStartedFlag = true;
		}
		break;
	case DLL_THREAD_DETACH:
#ifdef DDRAWCOMPAT
		// Unload and Unhook DDrawCompat
		if (DDrawCompat::IsEnabled())
		{
			DDrawCompat::Start(hModule, fdwReason);
		}
#endif // DDRAWCOMPAT

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
		Fullscreen::StopThread();
		WriteMemory::StopThread();
		//Utils::StopPriorityMonitor();

		// Unload DdrawWrapper
		if (Config.Dd7to9)
		{
			ExitDDraw();
		}

#ifdef DDRAWCOMPAT
		// Unload and Unhook DDrawCompat
		if (DDrawCompat::IsEnabled())
		{
			DDrawCompat::Start(hModule, fdwReason);
		}
#endif // DDRAWCOMPAT

		// Unload loaded dlls
		Utils::UnloadAllDlls();

		// Unhook keyboard layout
		if (Config.ForceKeyboardLayout)
		{
			KeyboardLayout::DisableForcedKeyboardLayout();
		}

		// Unload exception handler
		if (Config.HandleExceptions)
		{
			Utils::RemoveCustomExceptionHandler();
		}

		// Remove the exception handler if it was added
		if (remove_handler && g_exception_handle)
		{
			remove_handler(g_exception_handle);
			g_exception_handle = nullptr;
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
			hMutex = nullptr;
		}

		// Final log
		Logging::Log() << "DxWrapper terminated!";
		Logging::EnableLogging = false;
		break;
	}
	return TRUE;
}
