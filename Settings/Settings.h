#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <vector>
#include <string>

#define VISIT_CONFIG_SETTINGS(visit) \
	visit(AntiAliasing) \
	visit(D3d8to9) \
	visit(DDrawCompat) \
	visit(DDrawCompatDisableGDIHook) \
	visit(DisableHighDPIScaling) \
	visit(DSoundCtrl) \
	visit(DxWnd) \
	visit(ExcludeProcess) \
	visit(ForceCertification) \
	visit(ForceExclusiveMode) \
	visit(ForceHardwareMixing) \
	visit(ForceHQ3DSoftMixing) \
	visit(ForceNonStaticBuffers) \
	visit(ForcePrimaryBufferFormat) \
	visit(ForceSoftwareMixing) \
	visit(ForceSpeakerConfig) \
	visit(ForceTermination) \
	visit(ForceVoiceManagement) \
	visit(ForceWindowResize) \
	visit(FullScreen) \
	visit(HandleExceptions) \
	visit(IgnoreWindowName) \
	visit(IncludeProcess) \
	visit(LoadCustomDllPath) \
	visit(LoadFromScriptsOnly) \
	visit(LoadPlugins) \
	visit(LockColorkey) \
	visit(LoopSleepTime) \
	visit(Num2DBuffers) \
	visit(Num3DBuffers) \
	visit(PreventSpeakerSetup) \
	visit(PrimaryBufferBits) \
	visit(PrimaryBufferChannels) \
	visit(PrimaryBufferSamples) \
	visit(RealDllPath) \
	visit(ResetMemoryAfter) \
	visit(ResetScreenRes) \
	visit(RunProcess) \
	visit(SendAltEnter) \
	visit(SetFullScreenLayer) \
	visit(SetNamedLayer) \
	visit(SingleProcAffinity) \
	visit(SpeakerConfig) \
	visit(StoppedDriverWorkaround) \
	visit(WaitForProcess) \
	visit(WaitForWindowChanges) \
	visit(WindowSleepTime) \
	visit(WrapperMode)

#define VISIT_APPCOMPATDATA_SETTINGS(visit) \
	visit(LockEmulation) \
	visit(BltEmulation) \
	visit(ForceLockNoWindow) \
	visit(ForceBltNoWindow) \
	visit(LockColorkey) \
	visit(FullscreenWithDWM) \
	visit(DisableLockEmulation) \
	visit(EnableOverlays) \
	visit(DisableSurfaceLocks) \
	visit(RedirectPrimarySurfBlts) \
	visit(StripBorderStyle) \
	visit(DisableMaxWindowedMode)

typedef unsigned char byte;

struct MEMORYINFO						// Used for hot patching memory
{
	void* AddressPointer = nullptr;		// Hot patch address
	byte* Bytes = nullptr;				// Hot patch bytes
	size_t SizeOfBytes = 0;				// Size of bytes to hot patch
};

struct DLLTYPE
{
	const DWORD dxwrapper = 0;
	const DWORD ddraw = 1;
	const DWORD d3d8 = 2;
	const DWORD d3d9 = 3;
	const DWORD dsound = 4;
	const DWORD winmm = 5;
};
static const DLLTYPE dtype;

// Designated Initializer does not work in VS 2015 so must pay attention to the order
static constexpr char* dtypename[] = {
	"dxwrapper.dll",// 0
	"ddraw.dll",	// 1
	"d3d8.dll",		// 2
	"d3d9.dll",		// 3
	"dsound.dll",	// 4
	"winmm.dll",	// 5
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
	bool SingleProcAffinity;			// Sets the CPU affinity for this process and thread
	bool SingleProcAffinityNotSet;		// If the CPU affinity option exists in the config file
	bool D3d8to9;						// Converts Direct3D8 (d3d8.dll) to Direct3D9 (d3d9.dll) https://github.com/crosire/d3d8to9
	bool DDrawCompat;					// Enables DDrawCompat functions https://github.com/narzoul/DDrawCompat/
	bool DDrawCompatDisableGDIHook;		// Disables DDrawCompat GDI hooks
	bool DisableHighDPIScaling;			// Disables display scaling on high DPI settings
	bool DSoundCtrl;					// Enables DirectSoundControl https://github.com/nRaecheR/DirectSoundControl
	bool DxWnd;							// Enables dXwnd https://sourceforge.net/projects/dxwnd/
	bool FullScreen;					// Sets the main window to fullscreen
	bool ForceTermination;				// Terminates application when main window closes
	bool ForceWindowResize;				// Forces main window to fullscreen
	bool HandleExceptions;				// Handles unhandled exceptions in the application
	bool LoadPlugins;					// Loads ASI plugins
	bool LoadFromScriptsOnly;			// Loads ASI plugins from 'scripts' and 'plugins' folder only
	bool ProcessExcluded;				// Reset the screen resolution on close
	bool ResetScreenRes;				// Reset the screen resolution on close
	bool SendAltEnter;					// Sends an Alt+Enter message to the wind to tell it to go into fullscreen
	bool WaitForProcess;				// Enables 'WindowSleepTime'
	bool WaitForWindowChanges;			// Waits for window handle to stabilize before setting fullsreen
	DWORD LoopSleepTime;				// Time to sleep between each window handle check loop
	DWORD ResetMemoryAfter;				// Undo hot patch after this amount of time
	DWORD WindowSleepTime;				// Time to wait (sleep) for window handle and screen updates to finish
	DWORD SetFullScreenLayer;			// The layer to be selected for fullscreen
	DWORD AntiAliasing;					// Enable AntiAliasing for d3d9 CreateDevice
	DWORD RealWrapperMode;				// Internal wrapper mode
	MEMORYINFO VerifyMemoryInfo;		// Memory used for verification before hot patching
	std::vector<MEMORYINFO> MemoryInfo;	// Addresses and memory used in hot patching
	std::string RealDllPath;			// Manually set Dll to wrap
	std::string RunProcess;				// Process to run on load
	std::string WrapperMode;			// Name of dxwrapper
	std::string WrapperName;			// Name of dxwrapper
	std::vector<std::string> SetNamedLayer;		// List of named layers to select for fullscreen
	std::vector<std::string> IgnoreWindowName;	// List of window classes to ignore
	std::vector<std::string> LoadCustomDllPath;	// List of custom dlls to load
	std::vector<std::string> ExcludeProcess;	// List of excluded applications
	std::vector<std::string> IncludeProcess;	// List of included applications

	// SetAppCompatData
	bool DXPrimaryEmulation[13];		// SetAppCompatData exported functions from ddraw.dll http://www.blitzbasic.com/Community/posts.php?topic=99477
	DWORD LockColorkey;					// DXPrimaryEmulation option that needs a second parameter
	bool DisableMaxWindowedModeNotSet;	// If the DisableMaxWindowedMode option exists in the config file

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

namespace Settings
{
	bool IfStringExistsInList(const char*, std::vector<std::string>, bool = true);
	void SetValue(char*, char*, std::vector<std::string>*);
}
