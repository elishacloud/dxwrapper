#ifndef DGAME_H    // To make sure you don't declare the function more than once by including the header multiple times.
#define DGAME_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#ifndef DDRAWLOG_H
#include "DDrawLog.h"
#endif

// Main resource file details
#define APP_NAME				"DirectX Dynamic Link Library"
#define APP_MAJOR				1
#define APP_MINOR				0
#define APP_BUILDNUMBER			15
#define APP_COMPANYNAME			"Sadrate Presents"
#define APP_DESCRPTION			"Wraps or hooks DirectX files to fix compatibility issues in older games. Also allows code to be executed from inside the application. Wraps the following files: d3d8.dll, d3d9.dll, ddraw.dll, dplayx.dll, dsound.dll, dxgi.dll or winmm.dll"
#define APP_COPYRIGHT			"Copyright (C) 2017 Elisha Riedlinger"
#define APP_ORIGINALVERSION		"dxwrapper.dll"
#define APP_INTERNALNAME		"dxwrapper"

// dllmain.cpp
extern HMODULE hModule_dll;
extern CRITICAL_SECTION CriticalSection;
void CallCheckCurrentScreenRes();
void RunExitFunctions(bool = false);

// dwrapper.cpp
void DllAttach();
HMODULE LoadDll(char*, DWORD);
void DllDetach();

// ddraw.cpp
void LoadDdraw();
void SetSharedDdraw(HMODULE);
void FreeDdrawLibrary();

// d3d9.cpp
void LoadD3d9();
void SetSharedD3d9(HMODULE);
void FreeD3d9Library();

// d3d8.cpp
void LoadD3d8();
void FreeD3d8Library();

// winmm.cpp
void LoadWinmm();
void FreeWinmmLibrary();

// dsound.cpp
void LoadDsound();
void FreeDsoundLibrary();

// dxgi.cpp
void LoadDxgi();
void FreeDxgiLibrary();

// dplayx.cpp
void LoadDplayx();
void FreeDplayxLibrary();

// ddrawcompat (dllmaincompat.cpp)
bool StartDdrawCompat(HINSTANCE);
void UnloadDdrawCompat();

// dxwnd (init.cpp)
void InitDxWnd();
void DxWndEndHook();

// dsound_hook.cpp
HMODULE LoadDsoundHook();

// fullscreen.cpp
struct screen_res
{
	long Width = 0;
	long Height = 0;

	screen_res& operator=(const screen_res& a)
	{
		Width = a.Width;
		Height = a.Height;
		return *this;
	}

	bool operator==(const screen_res& a) const
	{
		return (Width == a.Width && Height == a.Height);
	}

	bool operator!=(const screen_res& a) const
	{
		return (Width != a.Width || Height != a.Height);
	}
};
void CheckCurrentScreenRes(screen_res&);
void SetScreen(screen_res);
void ResetScreen();
void StartThread();
bool IsMyThreadRunning();
void StopThread();
void CreateTimer(HWND);

// utils.cpp
void Shell(char*);
void DisableHighDPIScaling();
void SetSingleProcessAffinity(bool);
void SetSingleCoreAffinity();

// WriteMemory.cpp
void HotPatchMemory();

// initdisasm.cpp
void HookExceptionHandler();
void UnHookExceptionHandler();

// iatpatch.cpp
void *IATPatch(HMODULE, DWORD, char*, void*, const char*, void*);

// hotpatch.cpp
void *HotPatch(void*, const char*, void*);
void StopHotpatchThread();

// cfg.cpp
bool IfStringExistsInList(char*, char*[256], uint8_t, bool = true);
void SetConfigList(char*[], uint8_t&, char*);
struct MEMORYINFO						// Used for hot patching memory
{
	DWORD AddressPointer = 0;			// Hot patch address
	byte* Bytes;						// Hot patch bytes
	size_t SizeOfBytes = 0;				// Size of bytes to hot patch
};
struct CONFIG
{
	void Init();						// Initialize the config setting
	void CleanUp();						// Deletes all 'new' varables created by config
	bool Affinity;						// Sets the CPU affinity for this process and thread
	bool D3d8to9;						// Converts D3D8 to D3D9
	bool DdrawCompat;					// Enables DdrawCompat functions https://github.com/narzoul/DDrawCompat/
	bool DpiAware;						// Disables display scaling on high DPI settings
	bool DxWnd;							// Enables dXwnd https://sourceforge.net/projects/dxwnd/
	bool FullScreen;					// Sets the main window to fullscreen
	bool ForceTermination;				// Terminates application when main window closes
	bool ForceWindowResize;				// Forces main window to fullscreen
	bool HandleExceptions;				// Handles unhandled exceptions in the application
	bool HookDsound;					// Work in progress
	bool SendAltEnter;					// Sends an Alt+Enter message to the wind to tell it to go into fullscreen
	bool WaitForProcess;				// Enables 'WindowSleepTime'
	bool WaitForWindowChanges;			// Waits for window handle to stabilize before setting fullsreen
	bool Debug;							// Enables debug logging
	DWORD LoopSleepTime;				// Time to sleep between each window handle check loop
	DWORD ResetMemoryAfter;				// Undo hot patch after this amount of time
	DWORD WindowSleepTime;				// Time to wait (sleep) for window handle and screen updates to finish
	DWORD SetFullScreenLayer;			// The layer to be selected for fullscreen
	DWORD RealWrapperMode;				// Internal wrapper mode
	DWORD WrapperMode;					// 0			= Auto
										// 1 ... 245	= DLLTYPE
										// 255			= All
	uint8_t AddressPointerCount = 0;	// Count of addresses to hot patch
	uint8_t BytesToWriteCount = 0;		// Count of bytes to hot patch
	uint8_t CustomDllCount = 0;			// Count of custom dlls to load
	uint8_t NamedLayerCount = 0;		// Count of names layers to select for fullscreen
	uint8_t IgnoreWindowCount = 0;		// Count of window classes to ignore
	MEMORYINFO VerifyMemoryInfo;		// Memory used for verification before hot patching
	MEMORYINFO MemoryInfo[256];			// Addresses and memory used in hot patching
	char szDllPath[MAX_PATH];			// Manually set Dll to wrap
	char szShellPath[MAX_PATH];			// Process to run on load
	char* szCustomDllPath[256];			// List of custom dlls to load
	char* szSetNamedLayer[256];			// List of named layers to select for fullscreen
	char* szIgnoreWindowName[256];		// Lit of window classes to ignore
};
extern CONFIG Config;

struct DLLTYPE
{
	DWORD ddraw = 1;
	DWORD d3d9 = 2;
	DWORD d3d8 = 3;
	DWORD winmm = 4;
	DWORD dsound = 5;
	DWORD dxgi = 6;
	DWORD dplayx = 7;
};
extern DLLTYPE dtype;

#endif
