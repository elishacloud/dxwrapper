#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <vector>
#include <string>
#include "ReadParse.h"

#define NOT_EXIST 0xFFFF

#define VISIT_CONFIG_SETTINGS(visit) \
	visit(AnisotropicFiltering) \
	visit(AntiAliasing) \
	visit(AudioClipDetection) \
	visit(AudioFadeOutDelayMS) \
	visit(Dd7to9) \
	visit(D3d8to9) \
	visit(Dinputto8) \
	visit(DDrawCompat) \
	visit(DDrawCompat20) \
	visit(DDrawCompat21) \
	visit(DDrawCompat32) \
	visit(DDrawCompatDisableGDIHook) \
	visit(DDrawCompatNoProcAffinity) \
	visit(DdrawAutoFrameSkip) \
	visit(DdrawClippedWidth) \
	visit(DdrawClippedHeight) \
	visit(DdrawCustomWidth) \
	visit(DdrawCustomHeight) \
	visit(DdrawEnableByteAlignment) \
	visit(DdrawDisableDirect3DCaps) \
	visit(DdrawEmulateLock) \
	visit(DdrawFillSurfaceColor) \
	visit(DdrawForceMipMapAutoGen) \
	visit(DdrawFlipFillColor) \
	visit(DdrawRemoveScanlines) \
	visit(DdrawRemoveInterlacing) \
	visit(DdrawFixByteAlignment) \
	visit(DdrawIntroVideoFix) \
	visit(DdrawEmulateSurface) \
	visit(DdrawReadFromGDI) \
	visit(DdrawWriteToGDI) \
	visit(DdrawIntegerScalingClamp) \
	visit(DdrawLimitDisplayModeCount) \
	visit(DdrawMaintainAspectRatio) \
	visit(DdrawOverrideBitMode) \
	visit(DdrawOverrideWidth) \
	visit(DdrawOverrideHeight) \
	visit(DdrawOverrideStencilFormat) \
	visit(DdrawResolutionHack) \
	visit(DdrawUseDirect3D9Ex) \
	visit(DdrawConvertHomogeneousW) \
	visit(DdrawConvertHomogeneousToWorld) \
	visit(DdrawConvertHomogeneousToWorldUseGameCamera) \
	visit(DdrawConvertHomogeneousToWorldFOV) \
	visit(DdrawConvertHomogeneousToWorldNearPlane) \
	visit(DdrawConvertHomogeneousToWorldFarPlane) \
	visit(DdrawConvertHomogeneousToWorldDepthOffset) \
	visit(DdrawUseNativeResolution) \
	visit(DdrawEnableMouseHook) \
	visit(DdrawDisableLighting) \
	visit(DdrawHookSystem32) \
	visit(D3d8HookSystem32) \
	visit(D3d9HookSystem32) \
	visit(DinputHookSystem32) \
	visit(Dinput8HookSystem32) \
	visit(DsoundHookSystem32) \
	visit(SetSwapEffectShim) \
	visit(DeviceLookupCacheTime) \
	visit(DisableGameUX) \
	visit(DisableHighDPIScaling) \
	visit(DisableLogging) \
	visit(DirectShowEmulation) \
	visit(CacheClipPlane) \
	visit(EnvironmentMapCubeFix) \
	visit(ConvertToDirectDraw7) \
	visit(ConvertToDirect3D7) \
	visit(EnableDdrawWrapper) \
	visit(EnableD3d9Wrapper) \
	visit(EnableDinput8Wrapper) \
	visit(EnableDsoundWrapper) \
	visit(EnableImgui) \
	visit(EnableOpenDialogHook) \
	visit(EnableVSync) \
	visit(EnableWindowMode) \
	visit(ExcludeProcess) \
	visit(ForceDirect3D9On12) \
	visit(ForceExclusiveFullscreen) \
	visit(ForceMixedVertexProcessing) \
	visit(ForceSystemMemVertexCache) \
	visit(FilterNonActiveInput) \
	visit(FixHighFrequencyMouse) \
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
	visit(GraphicsHybridAdapter) \
	visit(HandleExceptions) \
	visit(IgnoreWindowName) \
	visit(IncludeProcess) \
	visit(InitialWindowPositionLeft) \
	visit(InitialWindowPositionTop) \
	visit(isAppCompatDataSet) \
	visit(LimitPerFrameFPS) \
	visit(LoadCustomDllPath) \
	visit(LoadFromScriptsOnly) \
	visit(LoadPlugins) \
	visit(LockColorkey) \
	visit(LoopSleepTime) \
	visit(MouseMovementFactor) \
	visit(MouseMovementPadding) \
	visit(Num2DBuffers) \
	visit(Num3DBuffers) \
	visit(OverrideRefreshRate) \
	visit(PrimaryBufferBits) \
	visit(PrimaryBufferChannels) \
	visit(PrimaryBufferSamples) \
	visit(RealDllPath) \
	visit(ResetMemoryAfter) \
	visit(ResetScreenRes) \
	visit(LimitStateBlocks) \
	visit(RunProcess) \
	visit(SendAltEnter) \
	visit(SetFullScreenLayer) \
	visit(SetInitialWindowPosition) \
	visit(SetNamedLayer) \
	visit(SingleProcAffinity) \
	visit(StoppedDriverWorkaround) \
	visit(WaitForProcess) \
	visit(WaitForWindowChanges) \
	visit(WindowSleepTime) \
	visit(WindowModeBorder) \
	visit(WinVersionLie) \
	visit(WinVersionLieSP) \
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
	std::string PatternString;			// Hot patch pattern
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
static constexpr const char* dtypename[] = {
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
	void Init();								// Initialize the config setting
	void SetConfig();							// Set additional settings
	bool IsSet(DWORD Value);					// Check if a value is set
	bool Exiting = false;						// Dxwrapper is being unloaded
	bool Dd7to9 = false;						// Converts DirectDraw/Direct3D (ddraw.dll) to Direct3D9 (d3d9.dll)
	bool D3d8to9 = false;						// Converts Direct3D8 (d3d8.dll) to Direct3D9 (d3d9.dll) https://github.com/crosire/d3d8to9
	bool Dinputto8 = false;						// Converts DirectInput (dinput.dll) to DirectInput8 (dinput8.dll)
	bool DDrawCompat = false;					// Enables the default DDrawCompat functions https://github.com/narzoul/DDrawCompat/
	bool DDrawCompat20 = false;					// Enables DDrawCompat v0.2.0b
	bool DDrawCompat21 = false;					// Enables DDrawCompat v0.2.1
	bool DDrawCompat32 = false;					// Enables DDrawCompat v0.3.2
	bool DDrawCompatDisableGDIHook = false;		// Disables DDrawCompat GDI hooks
	bool DDrawCompatNoProcAffinity = false;		// Disables DDrawCompat single processor affinity
	bool DdrawAutoFrameSkip = false;			// Automatically skips frames to reduce input lag
	DWORD DdrawFixByteAlignment = false;		// Fixes lock with surfaces that have unaligned byte sizes, 1) just byte align, 2) byte align + D3DTEXF_NONE, 3) byte align + D3DTEXF_LINEAR
	bool DdrawEnableByteAlignment = false;		// Disables 32bit / 64bit byte alignment
	bool DdrawIntroVideoFix = false;			// Enables some fixes that may help with showing intro videos
	DWORD DdrawResolutionHack = 0;				// Removes the artificial resolution limit from Direct3D7 and below https://github.com/UCyborg/LegacyD3DResolutionHack
	bool DdrawRemoveScanlines = false;			// Experimental feature to removing interlaced black lines in a single frame
	bool DdrawRemoveInterlacing = false;		// Experimental feature to removing interlacing between frames
	bool DdrawFillSurfaceColor = false;			// After creating surface fill with random color for testing black screen or objects
	bool DdrawEmulateSurface = false;			// Emulates the ddraw surface using device context for Dd7to9
	bool DdrawEmulateLock = false;				// Emulates the lock to prevent crashes when an application tries to read data outside Lock/Unlock pair
	bool DdrawReadFromGDI = false;				// Read from GDI bfore passing surface to program
	bool DdrawWriteToGDI = false;				// Blt surface directly to GDI rather than Direct3D9
	bool DdrawIntegerScalingClamp = false;		// Scales the screen by an integer value to help preserve video quality
	bool DdrawMaintainAspectRatio = false;		// Keeps the current DirectDraw aspect ratio when overriding the game's resolution
	bool DdrawUseDirect3D9Ex = false;			// Use Direct3D9Ex extensions for Dd7to9
	bool DdrawConvertHomogeneousW = false;		// Convert primites using D3DFVF_XYZRHW to D3DFVF_XYZW.
	bool DdrawConvertHomogeneousToWorld = false;				// Convert primitives back into a world space. Needed for RTX.
	bool DdrawConvertHomogeneousToWorldUseGameCamera = false;	// Use the game's view matrix instead of replacing it with our own.
	float DdrawConvertHomogeneousToWorldFOV = 0.0f;				// The field of view of the camera used to reconstruct the original 3D world.
	float DdrawConvertHomogeneousToWorldNearPlane = 0.0f;		// The near plane of the camera used to reconstruct the original 3D world.
	float DdrawConvertHomogeneousToWorldFarPlane = 0.0f;		// The far plane of the camera used to reconstruct the original 3D world.
	float DdrawConvertHomogeneousToWorldDepthOffset = 0.0f;		// The offset to add to the geometry so it does not clip into the near plane.
	bool DdrawUseNativeResolution = false;		// Uses the current screen resolution for Dd7to9
	DWORD DdrawClippedWidth = 0;				// Used to scaled Direct3d9 to use this width when using Dd7to9
	DWORD DdrawClippedHeight = 0;				// Used to scaled Direct3d9 to use this height when using Dd7to9
	DWORD DdrawCustomWidth = 0;					// Custom resolution width for Dd7to9 when using DdrawLimitDisplayModeCount, resolution must be supported by video card and monitor
	DWORD DdrawCustomHeight = 0;				// Custom resolution height for Dd7to9 when using DdrawLimitDisplayModeCount, resolution must be supported by video card and monitor
	bool DdrawDisableDirect3DCaps = false;		// Disable caps for Direct3D to try and force the game to use DirectDraw instaed of Direct3D
	bool DdrawLimitDisplayModeCount = false;	// Limits the number of display modes sent to program, some games crash when you feed them with too many resolutions
	DWORD DdrawOverrideBitMode = 0;				// Forces DirectX to use specified bit mode: 8, 16, 24, 32
	DWORD DdrawOverrideWidth = 0;				// Force Direct3d9 to use this width when using Dd7to9
	DWORD DdrawOverrideHeight = 0;				// Force Direct3d9 to use this height when using Dd7to9
	DWORD OverrideRefreshRate = 0;				// Force Direct3d9 to use this refresh rate, only works in exclusive fullscreen mode
	DWORD DdrawOverrideStencilFormat = 0;		// Force Direct3d9 to use this AutoStencilFormat when using Dd7to9
	DWORD DdrawFlipFillColor = 0;				// Color used to fill the primary surface before flipping
	bool DdrawForceMipMapAutoGen = false;		// Force Direct3d9 to use this AutoStencilFormat when using Dd7to9
	bool DdrawEnableMouseHook = false;			// Allow to hook into mouse to limit it to the chosen resolution
	bool DdrawDisableLighting = false;			// Allow to disable lighting
	DWORD DdrawHookSystem32 = 0;				// Hooks the ddraw.dll file in the Windows System32 folder
	DWORD D3d8HookSystem32 = 0;					// Hooks the d3d8.dll file in the Windows System32 folder
	DWORD D3d9HookSystem32 = 0;					// Hooks the d3d9.dll file in the Windows System32 folder
	DWORD DinputHookSystem32 = 0;				// Hooks the dinput.dll file in the Windows System32 folder
	DWORD Dinput8HookSystem32 = 0;				// Hooks the dinput8.dll file in the Windows System32 folder
	DWORD DsoundHookSystem32 = 0;				// Hooks the dsound.dll file in the Windows System32 folder
	DWORD DeviceLookupCacheTime = 0;			// Number of seconds to cache the DeviceEnum callback data
	bool DirectShowEmulation = false;			// Emulates DirectShow APIs
	bool DisableGameUX = false;					// Disables the Microsoft Game Explorer which can sometimes cause high CPU in rundll32.exe and hang the game process
	bool DisableHighDPIScaling = false;			// Disables display scaling on high DPI settings
	bool DisableLogging = false;				// Disables the logging file
	DWORD SetSwapEffectShim = 0;				// Disables the call to d3d9.dll 'Direct3D9SetSwapEffectUpgradeShim' to switch present mode
	DWORD CacheClipPlane = 0;					// Caches the ClipPlane for Direct3D9 to fix an issue in d3d9 on Windows 8 and newer
	DWORD EnvironmentMapCubeFix = 0;			// Fixes environment cube maps when no texture is applied, issue exists in d3d8
	bool ConvertToDirectDraw7 = false;			// Converts DirectDraw 1-6 to DirectDraw 7
	bool ConvertToDirect3D7 = false;			// Converts Direct3D 1-6 to Direct3D 7
	bool EnableDdrawWrapper = false;			// Enables the ddraw wrapper
	DWORD EnableD3d9Wrapper = 0;				// Enables the d3d9 wrapper
	bool EnableDinput8Wrapper = false;			// Enables the dinput8 wrapper
	bool EnableDsoundWrapper = false;			// Enables the dsound wrapper
	bool EnableImgui = false;					// Enables imgui for debugging
	bool EnableOpenDialogHook = false;			// Enables the hooks for the open dialog box
	bool EnableWindowMode = false;				// Enables WndMode for d3d9 wrapper
	bool EnableVSync = false;					// Enables VSync for d3d9 wrapper
	bool FixHighFrequencyMouse = false;			// Gets the latest mouse status by merging the DirectInput buffer data
	float MouseMovementFactor = 1.0f;			// Sets the mouse movement speed factor, requires enabling FixHighFrequencyMouse
	DWORD MouseMovementPadding = 0;				// Adds extra mouse movement to overcome issues with input deadzone in some games, requires enabling FixHighFrequencyMouse
	bool ForceDirect3D9On12 = false;			// Forces Direct3D9 to use CreateDirect3D9On12
	bool ForceExclusiveFullscreen = false;		// Forces exclusive fullscreen mode in d3d9
	bool ForceMixedVertexProcessing = false;	// Forces Mixed mode for vertex processing in d3d9
	bool ForceSystemMemVertexCache = false;		// Forces System Memory caching for vertexes in d3d9
	bool FullScreen = false;					// Sets the main window to fullscreen
	bool FullscreenWindowMode = false;			// Enables fullscreen windowed mode, requires EnableWindowMode
	bool ForceTermination = false;				// Terminates application when main window closes
	bool ForceWindowResize = false;				// Forces main window to fullscreen, requires FullScreen
	bool ForceVsyncMode = false;				// Forces d3d9 game to use EnableVsync option
	DWORD GraphicsHybridAdapter = 0;			// Sets the Direct3D9 Hybrid Enumeration Mode to allow using a secondary display adapter
	bool HandleExceptions = false;				// Handles unhandled exceptions in the application
	bool isAppCompatDataSet = false;			// Flag that holds tells whether any of the AppCompatData flags are set
	float LimitPerFrameFPS = 0;					// Limits each frame by adding a delay if the frame is to fast
	bool LoadPlugins = false;					// Loads ASI plugins
	bool LoadFromScriptsOnly = false;			// Loads ASI plugins from 'scripts' and 'plugins' folder only
	bool ProcessExcluded = false;				// Set if this process is excluded from dxwrapper functions
	bool ResetScreenRes = false;				// Reset the screen resolution on close
	DWORD LimitStateBlocks = 0;					// Reuses state block interfaces to prevent memory leaks
	bool SendAltEnter = false;					// Sends an Alt+Enter message to the wind to tell it to go into fullscreen, requires FullScreen
	bool WaitForProcess = false;				// Waits for process to end before continuing, requires FullScreen
	bool WaitForWindowChanges = false;			// Waits for window handle to stabilize before setting fullsreen, requires FullScreen
	bool WindowModeBorder = false;				// Enables the window border when EnableWindowMode is set, requires EnableWindowMode
	bool SetInitialWindowPosition = false;		// Enable Initial window position
	DWORD InitialWindowPositionLeft;			// Initial left window position for application
	DWORD InitialWindowPositionTop;				// Initial top window position for application
	DWORD LoopSleepTime = 0;					// Time to sleep between each window handle check loop, requires FullScreen
	DWORD ResetMemoryAfter = 0;					// Undo hot patch after this amount of time
	DWORD WindowSleepTime = 0;					// Time to wait (sleep) for window handle and screen updates to finish, requires FullScreen
	DWORD SingleProcAffinity = 0;				// Sets the CPU affinity for this process
	DWORD SetFullScreenLayer = 0;				// The layer to be selected for fullscreen, requires FullScreen
	DWORD AnisotropicFiltering = 0;				// Enable Anisotropic Filtering for d3d9
	DWORD AntiAliasing = 0;						// Enable AntiAliasing for d3d9 CreateDevice
	DWORD RealWrapperMode = 0;					// Internal wrapper mode
	MEMORYINFO VerifyMemoryInfo;				// Memory used for verification before hot patching
	std::string WinVersionLie = "";				// Using DDrawCompat WinVersionLie to tell the OS a different OS
	DWORD WinVersionLieSP = 0;					// Using DDrawCompat WinVersionLie to tell the OS a different OS
	std::vector<MEMORYINFO> MemoryInfo;			// Addresses and memory used in hot patching
	std::string RealDllPath;					// Manually set Dll to wrap
	std::string RunProcess;						// Process to run on load
	std::string WrapperMode;					// Mode of dxwrapper from config file
	std::string WrapperName;					// dxwrapper dll filename
	std::vector<std::string> SetNamedLayer;		// List of named layers to select for fullscreen
	std::vector<std::string> IgnoreWindowName;	// List of window classes to ignore
	std::vector<std::string> LoadCustomDllPath;	// List of custom dlls to load
	std::vector<std::string> ExcludeProcess;	// List of excluded applications
	std::vector<std::string> IncludeProcess;	// List of included applications

	// Dinput8
	bool FilterNonActiveInput = 0;

	// SetAppCompatData
	bool DXPrimaryEmulation[13] = { false };	// SetAppCompatData exported functions from ddraw.dll
	DWORD LockColorkey = 0;						// DXPrimaryEmulation option that needs a second parameter
	bool DisableMaxWindowedModeNotSet = false;	// If the DisableMaxWindowedMode option exists in the config file

	// DirectSoundControl https://github.com/nRaecheR/DirectSoundControl
	DWORD Num2DBuffers = 0;
	DWORD Num3DBuffers = 0;
	bool ForceCertification = false;
	bool ForceExclusiveMode = false;
	bool ForceSoftwareMixing = false;
	bool ForceHardwareMixing = false;
	bool ForceHQ3DSoftMixing = false;
	bool ForceNonStaticBuffers = false;
	bool ForceVoiceManagement = false;
	bool ForcePrimaryBufferFormat = false;
	DWORD PrimaryBufferBits = 0;
	DWORD PrimaryBufferSamples = 0;
	DWORD PrimaryBufferChannels = 0;
	bool AudioClipDetection = false;
	DWORD AudioFadeOutDelayMS = 0;
	bool FixSpeakerConfigType = false;
	bool StoppedDriverWorkaround = false;
};
extern CONFIG Config;

namespace Settings
{
	bool IfStringExistsInList(const char*, std::vector<std::string>, bool = true);
	void SetValue(char*, char*, std::vector<std::string>*);
	void ClearConfigSettings();
}
