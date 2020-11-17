#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <vector>
#include <string>
#include "ReadParse.h"

#define VISIT_CONFIG_SETTINGS(visit) \
	visit(AnisotropicFiltering) \
	visit(AntiAliasing) \
	visit(AudioClipDetection) \
	visit(AudioFadeOutDelayMS) \
	visit(AutoFrameSkip) \
	visit(Dd7to9) \
	visit(D3d8to9) \
	visit(Dinputto8) \
	visit(DDrawCompat) \
	visit(DDrawCompat20) \
	visit(DDrawCompat21) \
	visit(DDrawCompatExperimental) \
	visit(DDrawCompatDisableGDIHook) \
	visit(DDrawCompatNoProcAffinity) \
	visit(DdrawClippedWidth) \
	visit(DdrawClippedHeight) \
	visit(DdrawEmulateSurface) \
	visit(DdrawReadFromGDI) \
	visit(DdrawWriteToGDI) \
	visit(DdrawIntegerScalingClamp) \
	visit(DdrawLimitDisplayModeCount) \
	visit(DdrawMaintainAspectRatio) \
	visit(DdrawOverrideBitMode) \
	visit(DdrawOverrideWidth) \
	visit(DdrawOverrideHeight) \
	visit(DdrawOverrideRefreshRate) \
	visit(DdrawResolutionHack) \
	visit(DdrawUseNativeResolution) \
	visit(DisableGameUX) \
	visit(DisableHighDPIScaling) \
	visit(DisableLogging) \
	visit(DSoundCtrl) \
	visit(DxWnd) \
	visit(CacheClipPlane) \
	visit(ConvertToDirectDraw7) \
	visit(ConvertToDirect3D7) \
	visit(EnableDdrawWrapper) \
	visit(EnableDinput8Wrapper) \
	visit(EnableDsoundWrapper) \
	visit(EnableVSync) \
	visit(EnableWindowMode) \
	visit(ExcludeProcess) \
	visit(FilterNonActiveInput) \
	visit(FixSpeakerConfigType) \
	visit(ForceExclusiveMode) \
	visit(ForceHardwareMixing) \
	visit(ForceHQ3DSoftMixing) \
	visit(ForceNonStaticBuffers) \
	visit(ForcePrimaryBufferFormat) \
	visit(ForceSoftwareMixing) \
	visit(ForceTermination) \
	visit(ForceVoiceManagement) \
	visit(ForceVsyncMode) \
	visit(ForceWindowResize) \
	visit(FullScreen) \
	visit(FullscreenWindowMode) \
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
	visit(StoppedDriverWorkaround) \
	visit(WaitForProcess) \
	visit(WaitForWindowChanges) \
	visit(WindowSleepTime) \
	visit(WindowModeBorder) \
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
	std::vector<byte> Bytes;			// Hot patch bytes
};

struct DLLTYPE
{
	const DWORD dxwrapper = 0;
	const DWORD ddraw = 1;
	const DWORD d3d8 = 2;
	const DWORD d3d9 = 3;
	const DWORD dsound = 4;
	const DWORD dinput = 5;
	const DWORD dinput8 = 6;
	const DWORD winmm = 7;
};
static const DLLTYPE dtype;

// Designated Initializer does not work in VS 2015 so must pay attention to the order
static constexpr char* dtypename[] = {
	"dxwrapper.dll",// 0
	"ddraw.dll",	// 1
	"d3d8.dll",		// 2
	"d3d9.dll",		// 3
	"dsound.dll",	// 4
	"dinput.dll",	// 5
	"dinput8.dll",	// 6
	"winmm.dll",	// 7
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
	bool Exiting = false;				// Dxwrapper is being unloaded
	bool AutoFrameSkip;					// Automatically skips frames to reduce input lag
	bool isDdrawWrapperEnabled;			// Specifies if DdrawWrapper needs to be enabled
	bool isD3d9WrapperEnabled;			// Specifies if D3d9Wrapper needs to be enabled
	bool isDsoundWrapperEnabled;		// Specifies if DsoundWrapper needs to be enabled
	bool Dd7to9;						// Converts DirectDraw/Direct3D (ddraw.dll) to Direct3D9 (d3d9.dll)
	bool D3d8to9;						// Converts Direct3D8 (d3d8.dll) to Direct3D9 (d3d9.dll) https://github.com/crosire/d3d8to9
	bool Dinputto8;						// Converts DirectInput (dinput.dll) to DirectInput8 (dinput8.dll)
	bool DDrawCompat;					// Enables the default DDrawCompat functions https://github.com/narzoul/DDrawCompat/
	bool DDrawCompat20;					// Enables DDrawCompat v0.2.0b
	bool DDrawCompat21;					// Enables DDrawCompat v0.2.1
	bool DDrawCompatExperimental;		// Enables DDrawCompat Experimental version
	bool DDrawCompatDisableGDIHook;		// Disables DDrawCompat GDI hooks
	bool DDrawCompatNoProcAffinity;		// Disables DDrawCompat single processor affinity
	bool DdrawResolutionHack;			// Removes the artificial resolution limit from Direct3D7 and below https://github.com/UCyborg/LegacyD3DResolutionHack
	bool DdrawEmulateSurface;			// Emulates the ddraw surface using device context for Dd7to9
	bool DdrawReadFromGDI;				// Read from GDI bfore passing surface to program
	bool DdrawWriteToGDI;				// Blt surface directly to GDI rather than Direct3D9
	bool DdrawIntegerScalingClamp;		// Scales the screen by an integer value to help preserve video quality
	bool DdrawMaintainAspectRatio;		// Keeps the current DirectDraw aspect ratio when overriding the game's resolution
	bool DdrawUseNativeResolution;		// Uses the current screen resolution for Dd7to9
	DWORD DdrawClippedWidth;			// Used to scaled Direct3d9 to use this width when using Dd7to9
	DWORD DdrawClippedHeight;			// Used to scaled Direct3d9 to use this height when using Dd7to9
	DWORD DdrawLimitDisplayModeCount;	// Limits the number of display modes sent to program, some games crash when you feed them with too many resolutions
	DWORD DdrawOverrideBitMode;			// Forces DirectX to use specified bit mode: 8, 16, 24, 32
	DWORD DdrawOverrideWidth;			// Force Direct3d9 to use this width when using Dd7to9
	DWORD DdrawOverrideHeight;			// Force Direct3d9 to use this height when using Dd7to9
	DWORD DdrawOverrideRefreshRate;		// Force Direct3d9 to use this refresh rate when using Dd7to9
	bool DisableGameUX;					// Disables the Microsoft Game Explorer which can sometimes cause high CPU in rundll32.exe and hang the game process
	bool DisableHighDPIScaling;			// Disables display scaling on high DPI settings
	bool DisableLogging;				// Disables the logging file
	bool DSoundCtrl;					// Enables DirectSoundControl https://github.com/nRaecheR/DirectSoundControl
	bool DxWnd;							// Enables DxWnd https://sourceforge.net/projects/dxwnd/
	bool CacheClipPlane;				// Caches the ClipPlane for Direct3D9 to fix an issue in d3d9 on Windows 8 and newer
	bool ConvertToDirectDraw7;			// Converts DirectDraw 1-6 to DirectDraw 7
	bool ConvertToDirect3D7;			// Converts Direct3D 1-6 to Direct3D 7
	bool EnableDdrawWrapper;			// Enables the ddraw wrapper
	bool EnableDinput8Wrapper;			// Enables the dinput8 wrapper
	bool EnableDsoundWrapper;			// Enables the dsound wrapper
	bool EnableWindowMode;				// Enables WndMode for d3d9 wrapper
	bool EnableVSync;					// Enables VSync for d3d9 wrapper
	bool FullScreen;					// Sets the main window to fullscreen
	bool FullscreenWindowMode;			// Enables fullscreen windowed mode, requires EnableWindowMode
	bool ForceTermination;				// Terminates application when main window closes
	bool ForceWindowResize;				// Forces main window to fullscreen, requires FullScreen
	bool ForceVsyncMode;				// Forces d3d9 game to use EnableVsync option
	bool HandleExceptions;				// Handles unhandled exceptions in the application
	bool LoadPlugins;					// Loads ASI plugins
	bool LoadFromScriptsOnly;			// Loads ASI plugins from 'scripts' and 'plugins' folder only
	bool ProcessExcluded;				// Set if this process is excluded from dxwrapper functions
	bool ResetScreenRes;				// Reset the screen resolution on close
	bool SendAltEnter;					// Sends an Alt+Enter message to the wind to tell it to go into fullscreen, requires FullScreen
	bool WaitForProcess;				// Waits for process to end before continuing, requires FullScreen
	bool WaitForWindowChanges;			// Waits for window handle to stabilize before setting fullsreen, requires FullScreen
	bool WindowModeBorder;				// Enables the window border when EnableWindowMode is set, requires EnableWindowMode
	DWORD LoopSleepTime;				// Time to sleep between each window handle check loop, requires FullScreen
	DWORD ResetMemoryAfter;				// Undo hot patch after this amount of time
	DWORD WindowSleepTime;				// Time to wait (sleep) for window handle and screen updates to finish, requires FullScreen
	DWORD SingleProcAffinity;			// Sets the CPU affinity for this process
	DWORD SetFullScreenLayer;			// The layer to be selected for fullscreen, requires FullScreen
	DWORD AnisotropicFiltering;			// Enable Anisotropic Filtering for d3d9
	DWORD AntiAliasing;					// Enable AntiAliasing for d3d9 CreateDevice
	DWORD RealWrapperMode;				// Internal wrapper mode
	MEMORYINFO VerifyMemoryInfo;		// Memory used for verification before hot patching
	std::vector<MEMORYINFO> MemoryInfo;	// Addresses and memory used in hot patching
	std::string RealDllPath;			// Manually set Dll to wrap
	std::string RunProcess;				// Process to run on load
	std::string WrapperMode;			// Mode of dxwrapper from config file
	std::string WrapperName;			// dxwrapper dll filename
	std::vector<std::string> SetNamedLayer;		// List of named layers to select for fullscreen
	std::vector<std::string> IgnoreWindowName;	// List of window classes to ignore
	std::vector<std::string> LoadCustomDllPath;	// List of custom dlls to load
	std::vector<std::string> ExcludeProcess;	// List of excluded applications
	std::vector<std::string> IncludeProcess;	// List of included applications

	// Dinput8
	bool FilterNonActiveInput;

	// SetAppCompatData
	bool DXPrimaryEmulation[13];		// SetAppCompatData exported functions from ddraw.dll http://www.blitzbasic.com/Community/posts.php?topic=99477
	DWORD LockColorkey;					// DXPrimaryEmulation option that needs a second parameter

	// DSoundCtrl
	DWORD Num2DBuffers;
	DWORD Num3DBuffers;
	bool ForceCertification;
	bool ForceExclusiveMode;
	bool ForceSoftwareMixing;
	bool ForceHardwareMixing;
	bool ForceHQ3DSoftMixing;
	bool ForceNonStaticBuffers;
	bool ForceVoiceManagement;
	bool ForcePrimaryBufferFormat;
	DWORD PrimaryBufferBits;
	DWORD PrimaryBufferSamples;
	DWORD PrimaryBufferChannels;
	bool AudioClipDetection;
	DWORD AudioFadeOutDelayMS;
	bool FixSpeakerConfigType;
	bool StoppedDriverWorkaround;
};
extern CONFIG Config;

namespace Settings
{
	bool IfStringExistsInList(const char*, std::vector<std::string>, bool = true);
	void SetValue(char*, char*, std::vector<std::string>*);
	void ClearConfigSettings();
}
