#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#ifndef DDRAWLOG_H
#include "DDrawLog.h"
#endif

void LogText(char*);

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
	bool AffinityNotSet;				// If the CPU affinity option exists in the config file
	bool D3d8to9;						// Converts Direct3D8 (d3d8.dll) to Direct3D9 (d3d9.dll) https://github.com/crosire/d3d8to9
	bool DDrawCompat;					// Enables DDrawCompat functions https://github.com/narzoul/DDrawCompat/
	bool DDrawCompatDisableGDIHook;		// Disables DDrawCompat GDI hooks
	bool DpiAware;						// Disables display scaling on high DPI settings
	bool DxWnd;							// Enables dXwnd https://sourceforge.net/projects/dxwnd/
	bool FullScreen;					// Sets the main window to fullscreen
	bool ForceTermination;				// Terminates application when main window closes
	bool ForceWindowResize;				// Forces main window to fullscreen
	bool HandleExceptions;				// Handles unhandled exceptions in the application
	bool DSoundCtrl;					// Enables DirectSoundControl https://github.com/nRaecheR/DirectSoundControl
	bool ResetScreenRes;				// Reset the screen resolution on close
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
	uint8_t AddressPointerCount;		// Count of addresses to hot patch
	uint8_t BytesToWriteCount;			// Count of bytes to hot patch
	uint8_t CustomDllCount;				// Count of custom dlls to load
	uint8_t NamedLayerCount;			// Count of names layers to select for fullscreen
	uint8_t IgnoreWindowCount;			// Count of window classes to ignore
	MEMORYINFO VerifyMemoryInfo;		// Memory used for verification before hot patching
	MEMORYINFO MemoryInfo[256];			// Addresses and memory used in hot patching
	char szDllPath[MAX_PATH];			// Manually set Dll to wrap
	char szShellPath[MAX_PATH];			// Process to run on load
	char* szCustomDllPath[256];			// List of custom dlls to load
	char* szSetNamedLayer[256];			// List of named layers to select for fullscreen
	char* szIgnoreWindowName[256];		// Lit of window classes to ignore
	bool DXPrimaryEmulation[256];		// SetAppCompatData exported functions from ddraw.dll http://www.blitzbasic.com/Community/posts.php?topic=99477
	DWORD LockColorkey;					// DXPrimaryEmulation option that needs a second parameter
	bool DisableMaxWindowedModeNotSet;	// If the DisableMaxWindowedMode option exists in the config file

	// DSoundCtrl
	int Num2DBuffers;
	int Num3DBuffers;
	bool ForceCertification;
	bool ForceExclusiveMode;
	bool ForceSoftwareMixing;
	bool ForceHardwareMixing;
	bool PreventSpeakerSetup;
	bool ForceHQ3DSoftMixing;
	bool ForceNonStaticBuffers;
	bool ForceVoiceManagement;
	bool ForcePrimaryBufferFormat;
	int PrimaryBufferBits;
	int PrimaryBufferSamples;
	int PrimaryBufferChannels;
	bool ForceSpeakerConfig;
	int SpeakerConfig;
	bool StoppedDriverWorkaround;
};
extern CONFIG Config;

struct DLLTYPE
{
	static constexpr int ddraw = 1;
	static constexpr int d3d9 = 2;
	static constexpr int d3d8 = 3;
	static constexpr int winmm = 4;
	static constexpr int dsound = 5;
	static constexpr int dxgi = 6;
	static constexpr int dplayx = 7;
	static constexpr int winspool = 8;
	static constexpr int dinput = 9;
};
extern DLLTYPE dtype;

// Designated Initializer does not work in VS 2015 so must pay attention to the order
static constexpr char* dtypename[] = {
	nullptr,		// 0
	"ddraw.dll",	// 1
	"d3d9.dll",		// 2
	"d3d8.dll",		// 3
	"winmm.dll",	// 4
	"dsound.dll",	// 5
	"dxgi.dll",		// 6
	"dplayx.dll",	// 7
	"winspool.drv",	// 8
	"dinput.dll",	// 9
};
static constexpr int dtypeArraySize = (sizeof(dtypename) / sizeof(*dtypename));

struct APPCOMPATDATATYPE
{
	static constexpr int LockEmulation = 1;
	static constexpr int BltEmulation = 2;
	static constexpr int ForceLockNoWindow = 3;
	static constexpr int ForceBltNoWindow = 4;
	static constexpr int LockColorkey = 5;
	static constexpr int FullscreenWithDWM = 6;
	static constexpr int DisableLockEmulation = 7;
	static constexpr int EnableOverlays = 8;
	static constexpr int DisableSurfaceLocks = 9;
	static constexpr int RedirectPrimarySurfBlts = 10;
	static constexpr int StripBorderStyle = 11;
	static constexpr int DisableMaxWindowedMode = 12;
};
extern APPCOMPATDATATYPE AppCompatDataType;