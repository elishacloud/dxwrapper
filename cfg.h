#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#ifndef DDRAWLOG_H
#include "DDrawLog.h"
#endif

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
	bool EnableStoppedDriverWorkaround;
};
extern CONFIG Config;

struct DLLTYPE
{
	const uint8_t ddraw = 1;
	const uint8_t d3d9 = 2;
	const uint8_t d3d8 = 3;
	const uint8_t winmm = 4;
	const uint8_t dsound = 5;
	const uint8_t dxgi = 6;
	const uint8_t dplayx = 7;
	const uint8_t winspool = 8;
};
extern DLLTYPE dtype;

struct APPCOMPATDATATYPE
{
	const uint8_t LockEmulation = 1;
	const uint8_t BltEmulation = 2;
	const uint8_t ForceLockNoWindow = 3;
	const uint8_t ForceBltNoWindow = 4;
	const uint8_t LockColorkey = 5;
	const uint8_t FullscreenWithDWM = 6;
	const uint8_t DisableLockEmulation = 7;
	const uint8_t EnableOverlays = 8;
	const uint8_t DisableSurfaceLocks = 9;
	const uint8_t RedirectPrimarySurfBlts = 10;
	const uint8_t StripBorderStyle = 11;
	const uint8_t DisableMaxWindowedMode = 12;
};
extern APPCOMPATDATATYPE AppCompatDataType;