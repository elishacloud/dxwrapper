#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <vector>
#include <string>

typedef unsigned char byte;

namespace Settings
{
	bool IfStringExistsInList(char*, std::vector<std::string>, bool = true);
	void SetConfigList(std::vector<std::string>&, char*);
}

struct MEMORYINFO						// Used for hot patching memory
{
	DWORD AddressPointer = 0;			// Hot patch address
	byte* Bytes = nullptr;				// Hot patch bytes
	size_t SizeOfBytes = 0;				// Size of bytes to hot patch
};

struct DLLTYPE
{
	const DWORD dciman32 = 0;
	const DWORD ddraw = 1;
	const DWORD d3d9 = 2;
	const DWORD d3d8 = 3;
	const DWORD winmm = 4;
	const DWORD dsound = 5;
	const DWORD dxgi = 6;
	const DWORD dplayx = 7;
	const DWORD dinput = 8;
	const DWORD bcrypt = 9;
	const DWORD cryptsp = 10;
	const DWORD winspool = 11;
	const DWORD Auto = 255;
};
static const DLLTYPE dtype;

// Designated Initializer does not work in VS 2015 so must pay attention to the order
static constexpr char* dtypename[] = {
	"dciman32.dll",	// 0
	"ddraw.dll",	// 1
	"d3d9.dll",		// 2
	"d3d8.dll",		// 3
	"winmm.dll",	// 4
	"dsound.dll",	// 5
	"dxgi.dll",		// 6
	"dplayx.dll",	// 7
	"dinput.dll",	// 8
	"bcrypt.dll",	// 9
	"cryptsp.dll",	// 10
	"winspool.drv",	// 11
};
static constexpr int dtypeArraySize = (sizeof(dtypename) / sizeof(*dtypename));

struct APPCOMPATDATATYPE
{
	const DWORD Empty = 0;
	const DWORD LockEmulation = 1;
	const DWORD BltEmulation = 2;
	const DWORD ForceLockNoWindow = 3;
	const DWORD ForceBltNoWindow = 4;
	const DWORD LockColorkey = 5;
	const DWORD FullscreenWithDWM = 6;
	const DWORD DisableLockEmulation = 7;
	const DWORD EnableOverlays = 8;
	const DWORD DisableSurfaceLocks = 9;
	const DWORD RedirectPrimarySurfBlts = 10;
	const DWORD StripBorderStyle = 11;
	const DWORD DisableMaxWindowedMode = 12;
};
static const APPCOMPATDATATYPE AppCompatDataType;

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
	DWORD AntiAliasing;					// Enable AntiAliasing for d3d9 CreateDevice
	DWORD RealWrapperMode;				// Internal wrapper mode
	DWORD WrapperMode;					// 0 ... 254	= DLLTYPE
										// 255			= Auto
	bool DXPrimaryEmulation[13];		// SetAppCompatData exported functions from ddraw.dll http://www.blitzbasic.com/Community/posts.php?topic=99477
	DWORD LockColorkey;					// DXPrimaryEmulation option that needs a second parameter
	bool DisableMaxWindowedModeNotSet;	// If the DisableMaxWindowedMode option exists in the config file
	MEMORYINFO VerifyMemoryInfo;		// Memory used for verification before hot patching
	std::vector<MEMORYINFO> MemoryInfo;	// Addresses and memory used in hot patching
	std::string szDllPath;				// Manually set Dll to wrap
	std::string szShellPath;			// Process to run on load
	std::vector<std::string> szCustomDllPath;		// List of custom dlls to load
	std::vector<std::string> szSetNamedLayer;		// List of named layers to select for fullscreen
	std::vector<std::string> szIgnoreWindowName;	// List of window classes to ignore

	// DSoundCtrl
	DWORD Num2DBuffers;
	DWORD Num3DBuffers;
	bool ForceCertification;
	bool ForceExclusiveMode;
	bool ForceSoftwareMixing;
	bool ForceHardwareMixing;
	bool PreventSpeakerSetup;
	bool ForceHQ3DSoftMixing;
	bool ForceNonStaticBuffers;
	bool ForceVoiceManagement;
	bool ForcePrimaryBufferFormat;
	DWORD PrimaryBufferBits;
	DWORD PrimaryBufferSamples;
	DWORD PrimaryBufferChannels;
	bool ForceSpeakerConfig;
	DWORD SpeakerConfig;
	bool StoppedDriverWorkaround;
};
extern CONFIG Config;
