v2.2.00/01
major code rewriting - introduced dxwCore class
preliminary FPS handling: Limit, Skip & Count with configurable delay
Hide Multi Monitor configuration flag - used for "Dream Acquarium" on multimonitor PC.

v2.2.02
preliminary time stretching: so far applies to GetTickCount() only, and is controlled by Alt-F5 / F6 keys.
Fixed bug in GDI BitBlt call: stretching must be made on screen DC only, or it's made twice.

v2.02.03/04:
code cleanup - see syslib calls hooking
new configuration flags: Hook GDI and Hook OpenGL
OpenGL custom library field
API hooking fix with module specification
Time stretching by keyboard control (Alt F5/F6) and/or Time Slider dialog

v2.02.05:
hooked winmm timeGetTime() API: makes time stretching work for Age of Empires series
changed time stretching grain: now it's not the coarse grained 2x, 4x,... series in 9 possible values but the fine grained series 1.5x, 2x, 3x,.... in 17 possible values
added status and time stretching view panels to tray icon menu

v2.02.06:
preliminary FPS counter overlapped on game screen, Alt-F7 to toggle display on/off.
fixed buf on time stretch logging (and possible game crash).
revised GetDC handling with 8BPP paletized surfaces: avoided need to emulate reverse-blitting and got an impressive speed improvement for games such as Age of Empires I & II and Hyperblade.

v2.02.07:
many fixes on the FPS and time control features.

v2.02.08:
fixed some errors in the main directdraw palette descriptor. That gives better compatibility and less complicated source code.
added Fake Version feature: now Dungeon Keeper II (original version, not GOG hack) can detect a fake Win2000 / WinXP environment on Win7 and newer. Tested and working on Win7. Many thanks to Maxim for pushing me hard to win my lazyness and implement this new feature.

v2.02.09:
Fixed some x,y window coordinates bugs
Fixed some proxy log messages (missing \n line terminator)
Fixed Trace "DirectX" flag.
improved GetDC handling in 8BPP palette mode: AddPalette called on demand, and on any surface (including backbuffers): makes Emergency work with no "Handle DC" flag set. Beware: this may affect the "Map GDI HDC on Primary DC" flag causing surface locks.
Fixed limit FPS timing issues: now the max possible FPS is 1000/delay.
Fixed EndPaint bug causing HDC lock in "Map GDI HDC to Primary DC" mode.

v2.02.10:
Added "Full RECT Blit" mode: may be useful to handle problematic situations (e.g. "Urban Assault" intro movies)
Fixed ClientToScreen and ScreenToClient hookers to properly handle scaled windows. This makes "Postal" working.
Fixed global palette reference count (??) in DirectDraw::Release hook
Fixed Window messages handling for SWP_NOMOVE, SWP_NOSIZE modes.

v2.02.11:
Added debug messages for GetSystemMetrics() modes, MapWindowPoints() points, DirectDrawEnumerate/Ex() devices.
ompiled with #define _WIN32_WINNT 0x0600 -> handles Vista modes
Added NOPALETTEUPDATE ("Palette update don't Blit" flag) to eliminate flickering when ddraw and GDI methods conflict
Hooked all LoadLibraryA/W and LoadLibraryExA/W calls
Hooked 
extDirectDrawEnumerate/Ex ddraw calls to handle Hide multi-monitor option.
Detected directshow activation through CoCreateInstance and hooked quartz.dll segment: now Urban Assault movies don't require "Full RECT Blit" option to be set.
Updated DDSurface::Release hook 

v2.02.12
GUI: Tabbed setup panel. More space for more future options.
DLL: (optional) splash screen 
probably, some regression bugs....
 
v2.02.13
Added decoding of WINDOWPOS.flags field in trace log
revised whole hooking procedures to use HMODULE handle instead of module name
Added WM_GETMINMAXINFO and WM_NCCALCSIZE handling in WinProcess hook
Attempt to handle double buffering through surface attach to backbuffer (???)
Fixed CHILD window positioning for BIG windows in CreateWindowExA hook
Added GlobalMemoryStatus hook to fix huge values when value exceeds DWORD range. Fixes Nocturne intro warning message.

V2.02.14
Started dll injection to handle startup code
Fixed directx CreateSurface hook to fix "Wargames" error in emulated mode
Fixed directx SetClipper hook to properly handle backbuffer clipping and fix "Wargames" clipping problems

v2.02.15
Fixed clipping handling of primary/backbuffer surfaces 
Added option to set AERO compatibility for Vista/Win7/Win8 platforms. Need to call a undocumented ddraw API. Thanks to Vovchik that discovered it. Use at your own risk!

v2.02.16
DLL injection finally working! Quake 2 is supported.
Added Wireframe option for OpenGL games
Improved fix for clipper handling of primary/backbuffer surfaces 

v2.02.18
Improved LoadLibrary family hook and other improvements to let DxWnd better retrieve OpenGL libs:  now Homeworld 2 is working
Fixed a bug in backbuffer ZBUFFER attach emulation. Now Dungeon Keeper 2 crashes no more.
Moved "Remap client rect" option from directx tab to main program tab
Added the "Force Hook" in the OpenGL tab. Useless for now.
Preliminary work for d3d10/d3d11 wrapping. Unfinished and not working so far.
Added LoadLibraryEx flags explaination in log
Added support for mouse X,Y coordinates display in status window
OpenGL hooking: fixed bug to prevent hooking same call twice
OpenGL hooking: added hook for wglMakeCurrent call to keep track of rendered window
OpenGL hooking: fixed glViewport and extglScissor hook coordinates handling
D3D hooking: Added Wireframe option for D3D games
Added hooking of CLSID_DxDiagProvider through CoCreateInstance
Fixed (further simplified) clipping handling of primary/backbuffer surfaces 
ChangeDisplaySettings hook: fixed x,y coordinate inversion in log message

v2.02.19
Saves GUI coordinates
Fixed "Remap client rect" option for Diablo's windows and Premier Manager 98 mouse movements
Added "Highlight blit to primary" option to draw a yellow bounding box around blits to primary surface
Fixed some exception conditions when closing the programs
Fixed CreateSurface handling to allow Premier Manager 98 start in emulated mode
Fixed ONEPIXELFIX handling
Fixed BIG WIN handling for Diablo's windows
Fixed FillRect hook to prevent filling outside virtual desktop
Disabled hooking of system libraries
Fixed a nasty bug that caused your desktop to freeze until shutdown !!!
Fixed GetWindowRect handling for windows not created by the task: Diablo queries the explorer window size! Now the retrieved RECT can't be larger than the virtual desktop

v2.02.20
Fixed an error in D3D10/11 preliminary code: AoE III working again
Added "Hook all DLLs" option to simplify congiguration: see new Diablo setup
Added screen coordinates to log
Some code rearrangement

v2.02.21
it's a w.i.p. release, wait 'till finished....

v2.02.22
Major code rearrangement in API hooking. Expect some troubles...
Added desktop workarea and centered coordinate settings
Fixed imelib hooking
Added HOOKENABLED flag
Fixed ChangeDisplaySettings 
GUI: added ListView icons
GUI: added pause command
GUI: updated commands layout

v2.02.23/24
Fixed "disable setting gamma ramp" flag to intercept both GDI and D3D calls
Fixed client workarea setting to occupy the whole client area even when preserving aspect ratio (it draws black rectangles to the left/right or top/bottom side)
Added DisableThreadLibraryCalls optimization
Added B&W screen simulation (for primary emulation only)
Improved the primary emulation capacity to handle 3D games
Added the "preserve surface capabilities" flag
Fixed an hooking bug for user32.dll

v2.02.25
CORE:
Added 'Single CPU Process Affinity' flag to let old games run on a single CPU core.
Fixed bug causing possible crash using 'preserve surface caps' flag
Switched lpDDSBack reference when backbuffer is upgraded through QueryInterface
Eliminated lpDDSBack clearing when refcount is zero - why is this working better? Needs further investigation
Added DDSCAPS_3DDEVICE capability when in EMULATED mode
GUI:
Added Kill process by name functionality (right click menu on program's list)
Fixed GUI initial position: now checks for desktop size to fit GUI within visible borders
Added single cpu process affinity checkbox in compatibility tab

v2.02.26
CORE:
Fixed Black&White mode for 16BPP color depth
Revised hooking code, now more compact and clear....
Fixed DxWnd splash screen, for those who love it
Increased child win table - now 688 hunter killer works perfectly 
GUI:
Added /debug flag to enable debugging options
Revised hooking code, now more compact and clear....
Restored Hook child win option

v2.02.27
CORE:
fixed GetDC/ReleaseDC ddraw implementation to refresh GDI operation on primary surface. Warlords 3 text is now visible.
preliminary implementation of MapWindowPoints - to be tested
GUI:
Fixed log flags wrong initialization

v2.02.28
CORE:
eliminated experimental ICSendMessage and ICOpen hooks, preventing some games (Alien Nations) to work
added mciSendCommand hook to fix video playback
fixed MoveWindow bug - still, it's unknown the reason why some programs (Emergency) tries to MoveWindow upon the hWnd=0 desktop!
fixed MapWindowPoints hook (Alien Nations)
fixed desktop detections in some places, now correctly using dxw.IsDesktop() method
Now "Alien Nations" and "Emergency" working (almost) perfectly.
fixed d3d8/9 hook to Get/SetGammaRamp. This should make a great number of recent games playable again.

v2.02.29
CORE:
hooked DirectDrawSurface::Lock method to center primary surface memory updates centered into the window (in no emulated mode)

v2.02.30
CORE:
Some ddraw::GetDC log message fixes
added preliminary registry emulation - tested ok with "duckman"
added separate flag for preliminary registry operation trace
added CDROM drive type emulation - tested ok with "Fighting Forces" RIP
fixed FixCursorPos routine: cursor x,y compensation must always use pseudo-fullscreen window parameters - fixes Imperialism II mouse problems
fixed Desktop Workarea position mode for ddraw games
added CoCreateInstanceEx hook - "Final Fighter" seems to be using it, but who knows why...?
added "Don't move D3D Rendering Window" window option to make "Fable Lost Chapters" working  
GUI:
updated default values on new entry creation to map most-likely-to-work parameters

v2.02.31
CORE:
disabled annoying compilation security warnings
improved advapi32.dll hooking and logging
handling of complex primary surfaces with backbuffer surface inherited from one directdraw session to the following and with different interfaces
fixed SetCooperativeLevel handling in case of WINDOWED mode against desktop hwnd==NULL
fixed GetGDISurface in EMULATED mode 
hooked gdi32 API set for Imperialism I & II and 688 Hunter Killer:GetClipBox, Polyline, PolyBezierTo, PolylineTo, PolyDraw, MoveToEx, ArcTo, LineTo, StretchDIBits, SetDIBitsToDevice, SetPixel, Ellipse, Polygon, Arc, CreateEllipticRgn, CreateEllipticRgnIndirect, CreateRectRgn, CreateRectRgnIndirect, CreatePolygonRgn, DrawTextA, DrawTextExA.
fixed gdi32.dll hooking for TextOutA, TabbedTextOutA, Rectangle, BitBlt, PatBlt, StretchBlt, CreateFont, CreateFontIndirect
improved ddraw proxy logging
added / fixed user32.dll API FrameRect, TabbedTextOutA, CloseWindow, DestroyWindow, SendMessageW
opengl: fixed glCreateContext, wglMakeCurrent to make Descent III playable in opengl video mode.
fixed DeferWindowPos hook to make Imperialism II working
fixed SM_CXVIRTUALSCREEN, SM_CYVIRTUALSCREEN properties
fixed window class logging causing program crash
GUI:
added "GDI/Scale font parameters" & "DirectX/Disable HAL support" options

v2.02.32
CORE: 
added new GDI handling mode, "Emulated devce context". Currently tested successfully on "G-Nome" only.
fixed "Prevent maximize" windows handling avoiding to disable topmost style for child windows
fixed handling of DDSCAPS_3DDEVICE surfaces in emulated mode: "Nightmare Ned" is working ok, "The Sims" works better (still crashing..)
fixed banner handling in case of device context remmapped coordinates
fixed memory leakage in SetDIBitsToDevice hooker
GUI
added radiobutton group to set the preferred GDI emulation mode

v2.02.33
CORE:
experimental attempt to emulate the ZBUFFER attach to the BACKBUFFER surface
improved log to detail all ddraw object properties
experimental emulation of RGB to YUV conversion to investigate "duckman" problems on Win7
improved grayscale algorythm
fixed missing DDPF_ALPHAPIXELS property causing several blit incompabilities
fixed surface handling for "Submarine Titans"
fixed mousewheel handling
fixed hooking bug causing several problems (i.e. Age of Empires III)
Added FULLSCREENONLY option: fixes "Submarine Titans" intro movies
Added "Bypass font unsupported api" compatibility flag: makes "Stratego" working
fixed several d3d8/9 hooking problems
GUI
added controls for new core features
defaulted ddraw emulation mode to surface emulation

v2.02.34
CORE:
Much better surface description in log
Completely revised CreateSurface hook: emulated and direct code are merged as much as possible, and reference counter are kept accurate. Now most games can work both in emulated and direct mode.
Fixed surface capabilities for "Vangers", "The Sims" (now working in emulated mode again)
Updated "Fixed aspect ratio" option: now uses the x,y size declared in the configuration instead of the fixed 800 x 600 standard one.
Added virtual fullscreen "Desktop" mode.
Completely revised the Peek/GetMessage handling: now it uses the standard SetWindowHook API instead of the address redirection, making the handling more reliable and accurate: now "Age of Empires III" works at last!
GUI:
Added virtual fullscreen "Desktop" mode.
Added "Fix buffered IO after Win98" flag: this is meant to fix an incompatibility in the ReadFile kernel32 API that must read from block boundaries, where a block was smaller up to Win98 and bigger after it.
As a result, games like "Dylan Dog Horror Luna Park" show IO errors while reading data from the CD. Unfortunately, this is not the only problem of this game, so Dylan Dog fans will have to wait further.
Maybe some other programs suffer of this problem, and I will strongly appreciate if anyone finds some.

v2.02.35
fixed BACKBUFFER surface attributes in direct (not emulated) mode: Rayman 2 playable again
added FILTERMESSAGES flag ("filter offending messages") to eliminate some problems to games not developed to work windowized (Rayman 2, Mirror's Edge ...)
fixed bug crashing the program when "keep aspect ratio" is selected with window size set to 0

v2.02.36
hooked GetAttachedSurface to non-PRIMARY surface to track ZBUFFER attach to BACKBUFFER. Useless, so far...
revised capability handling in CreateSurface 
hook trace is now activated from a separate log flag to reduce debug log size
added "Peek all messages in queue" to avoid queue saturation and automatic task kill in Win7 (thank to P K help)
fixed message handling in the case messages are passed to a routine handle

v2.02.37
code reorganization
GUI:
removed useless flags and moved debug options in a hidden tab

v2.02.38-40
Fixed coordinate calculation for blit operations when keeping aspect ratio.
Fixed window coordinate placement in desktop / client area mode for programs with a visible menu bar.
Changed "Lock win coordinates" flag: now it allows window movement / resizing when driven by mouse input.
Fixed a bug in GDI "Map DC to primary surface"mode.
Added palette display window to GUI

v2.02.41
Added "suppress child process creation"flag. This flag is introduced to manage games such as "Heart of Iron 2" that start intro movie by means of a separate process. HoI2 is starting the binkplay.exe program in the game's avi subfolder. This way you don't get a windowed movie, but completely suppress it.

v2.02.42
added hook to kernel32.dll QueryPerformanceCounter API to enable time stretching to Rayman 2
handled GetAttachedSurface emulation for FLIP capability on primary surface - that makes the intro movies of Empire Earth visible.
Some code cleaning
Cleared invisible debug flags ...

v2.02.43
Several changes for Empire Earth:
fix: restore 16BPP color mode after directdraw session termination
fix: recovered window destruction
fix: proper handling of MIPMAP and LOCALVIDMEM surfaces 
fix: color depth handling - now when the program terminates the desktop is brought to the original color depth

v2.02.44
Improved debug logging for all palette operations
fixed a bugged log causing the crash of Empire Earth in debug mode
added show time stretching flag - preliminary version. Some code cleaning as well.
change in automatic screen refresh - should make the "palette update don't blit" flag obsolete.....
added a static definition for the default system palette
added interception for CoCreateInstance following cases:
	case 0xe436ebb3: Module="quartz"; Class="CLSID_FilterGraph";
	case 0x4fd2a832: Module="ddrawex"; Class="CLSID_DirectDrawEx";
	case 0x49c47ce5: Module="amstream"; Class="CLSID_AMMultiMediaStream";
preliminary (proxed) interception for "GetActiveWindow" and "GetForegroundWindow"

v2.02.45
A small fix for ddraw 7 games that makes Praetorians playable, though with some problems!

v2.02.46
Fixed handling of user32 CreateDialogIndirectParam call to avoid processing in non fullscreen mode - fix necessary to start "Crimson Skies".

v2.02.47
GUI:
Configuration tabs reorganization: now some tabs have merged making it easier to reach all options
core:
improved d3d wrapping and sdded ZBUFFER cleanup flags to fix some d3d games. In particular:
Clean ZBUFFER @1.0 fix: to fix "Star Wars Episode I Racer"
Clean ZBUFFER @0.0 fix: to fix "Crimson Skies" black blocks bug on ATI cards

v2.02.48
New options: "Disable fogging", "Textures not power of 2 fix"
A few fix to prevent game crashes

v2.02.49
Separated ddraw, d3d and dxwnd log messages
Hooked GetAvailableVidMem method to limit memory values
Hooked ChangeDisplaySettings call in both versions ANSI and WIDECHAR, to prevent messing up with the screen
Added HookWindowProc() subroutine, called at dxhook.c@1534
Added "Add proxy libs" flag - so far to copy a d3d9.dll proxy lib to fix d3d9 games
Hooked d3d8/9 SetCursorPosition method
Hooked d3d8/9 AddRef / release methods (for logging)
Fixed D3DDevice8/9 hooking
Hooked QueryPerformanceFrequency
Fixed a bug in d3d7 hooking
GUI: added desktop color setting

v2.02.50
Added "Intercept RDTSC" option: so far, tested on Unreal Tournament only.
To implement RDTSC detection, I owe a big thank you to Olly who shared the disasm lib used to browse the assembly code.

v2.02.51/52
Hooked window timers: now "Ed Hunter" can be time stretched
added NOFILLRECT debug option
fixed WM_DISPLAYCHANGE handling (x,y swapped coordinates?)
fixed int64 arithmetic for performance counters
Added (untested !!!) handling for RDTSCP opcode with "Intercept RDTSC" option
Added "Limit screen resolution" option: seems necessary for "Sid Meyer's Civilization III" to work
Improved initial window coordinate & style handling
fixed virtual screen size showing into status panel
hooked SetPixelFormat, GetPixelFormat, ChoosePixelFormat and DescribePixelFormat to redirect desktop hwnd and make wglCreateContext work (needed for Civ III)
fixed log message for TextOut parameters
hooked DisableD3DSpy (invoked by The Bard's Tale)
fixed extglViewport coordinate remapping when invoked with CW_USEDEFAULT values (Civ III)
fixed bug in DirectDarawCreate/Ex hooking with wrong module handle

v2.02.53
Handling of ddraw screen color depth 15BPP - in "Hesperian Wars"
fixed IAT scanning: now function addresses for remapping are searched in possibly multiple instances of dll text segment - in "Aaron Hall's Dungeon Odissey"
hooked SystemParametersInfo call
fixed window size logic to check for window menu bar, considering also the WS_CHILD case.
fixed window move/resize when message processing is enabled: coordinates outside the child area or within ENTER/EXITSIZEMOVE must NOT be altered.
expanded timer processing to include user32 Set/KillTimer - in "Aaron Hall's Dungeon Odissey"
fixed possible recursion while hooking child window procedure identical to parent's one
fixed gdi emulation on top of ddraw surface, enough to play "Sid Meyer's Civilization III".

v2.02.54
fixed a bug in d3d GetAvailableVidMem that was causing too many troubles...
fixed a bug in Suppress IME option
fixed a bug in handled library list, possibly affecting d3d/d3d7 games
avoid fixing window style for non-desktop windows (fixed a Port Royale 2 bug)
eliminated "Disable HAL support", no longer necessary
some incomplete work on gdi game handling
added preliminary, incomplete (not working) glide handling

v2.02.55
added "Hide desktop background" video option

v2.02.56
hooked RegisterClassA API (useless so far...)
fixed bug in SendMessageW hooking: caused truncated text in window titles and form captions
fixed several d3d1-7 hooks: prevented some d3d games to properly work, i.e. "Thief the Dark Project"
fixed d3d GetDisplayMode to return virtual desktop size - fixed "Affari Tuoi", an italian game.
fixed some log messages

v2.02.57
fixed WS_NCCALCSIZE & WM_NCPAINT messages to ensure a proper window's border and size - fixed Black Thorn window problems
fixed "Hide desktop background" flag 

v2.02.58
Added Automatic DirectX mode: not workink always, but should make it much easier to guess a proper surface setting.
Automatic mode allows dynamic mode change depending on the situation: it is necessary for Populous 3 D3D version.
fixed BltFast operation with SRC color key (often used to draw cursor from texures) directly to primary surface: this makes the cursor visible in Populous 3.
minor fixes on logging

v2.02.59
fixed IDIrectDraw reference counter for ddraw release 1 session: fixes "Warhammer 40K Rites Of War" first screen
fixed QueryPerformanceCounter handling: fixes "New York Racer" time stretching problems

v2.02.60
fixed time stretching to handle Total Soccer 2000
fixed ZBUFFER surface creation to handle GOG Incoming troubles on some cards
fixed handling of special effects (wireframe, no fogging, zbuffer always) to be valid on all d3d versions

v2.02.61
added directx "Compensate Flip emulation" flag: used primarily to get rid of mouse artifacts due to emulated Flip rpocedure in windowed mode. Fixes Gruntz issues and some other games...

v2.02.62
fixed d3d7 CreateDevice hook and d3d7 rendering options (wireframe, disablefogging, zbufferalways)
fixed doublebuffer emulation - avoid returning the backbuffer when zbuffer is requested. Fixes rendering problems of many games!

v2.02.63
fixed d3d hooking for Reset and GetDirect3D methods: now Jumpgate works at any available resolution, and the "Add proxy libs" option is no longer necessary
fixed "Compensate Flip emulation" option for non emulated surface mode: fixes mouse artifacts for "Rogue Spear Black Thorn" 
added preliminary handling for "Disable Textures" option

v2.02.64
GUI: implemented multiple file import
DLL:
fixed a ddraw session reference count error that prevented "Jet Moto" to start
fixed "Compensate Flip emulation" for ddraw7 games: now "Empire Earth" supports this flag.
fixed CloseWindow hook: now games that minimize/restore the main window on task switch can possibly recover (e.g. Hundred Swords)
fixed process hook: now it should be a little more robust and efficient. Hopefully should fix some Macromedia Flash problems.
fixed IDDrawSurface::Lock() prototype. Not gameplay improvements, though...
added DirectX "Return 0 ref counter" option as quick & dirty solution to many reference counter problems!
fixed DirectDrawCreateEx failing to register the main ddraw session handle
fixed palette problem in emulated mode: palette must be applied to backbuffer surface as well. Now "Duckman" and "Total Soccer 2000" show perfect colors.

v2.02.65
added "Fine time adjust" flag to finely set timeslider by 10% steps ranging from about :2 to x2
fixed a bug in DWORD timers introduced in v2.02.60: now "Warcraft 2" timing works again
revised logit to calculate delays to emulate VSync timing - now should be more accurate and depending on actual screen refresh rate

v2.02.66
fixed palette bug: "Virtua Fighter PC" now shows correct colors.

v2.02.67
fixed log for D3D CreateDevice method, missing in some D3D interface versions
added wildcarded program path: now you needn't set the full pathname, but it is sufficient to specify the rightmost part, e.g. the executable filename.
added emulated mode color conversion from 32BPP to 16BPP. Despite the fact that the best way to run a 32BPP game is against a 32BPP desktop, now it is possible to downgrade 32BPP colors to 16BPP. This fixed the fact that 32BPP games run on a 16BPP desktop showed simply black screens (e.g. Baldur's Gate II)
fixed logic for EnumDisplayModes implementation: now screen resolutions and, for emulated mode only, also color depth, are generated by DxWnd and not derived directly from the real ones. This make it possible to fake support for very low screen resolutions (e.g. 320x200) on modern screens that support them no longer. This make "Genocide" running on both supported resolutions 320x200 and 320x240.

v2.02.68
added screen resolution choice: either a set of SVGA resolutions (mostly for 3:4 monitors), HDTV resolutions (mostly for 16:9 monitors) or the native set of resolutions offered by your monitor / video card.

v2.02.69
improved exception catching to intercept memory violation exceptions and to neutralize offending code. Thank again to olly didasm lib, now the assembly instruction length is automatically determined so that the opcode can be replaced by the correct number of NOP instructions. If this doesn't mean much for you, just consider that this makes playable the Win95 (patched) release of "Star Wars Tie Fighter".
fixed FillRect user32 call - brings some improvements to Imperialism (still far from being acceptable).

v2.02.70
GUI:
fixed saving wrong coordinates when dxwnd is terminated while minimized
added check for adminstrative capabilities on startup
DLL:
fix: moved DrawText/Ex api hooks into right library
fix: corrected child window procedure handling - fixes "Imperialism" child window closing 
fixed FillRect coordinate handling - fixes "Imperialism" menus
fixed SetWindowPlacement handling

v2.02.71
fix: Set/GetWindowLongA/W are always hooked.
fix: added user32 GetDCEx hook for GDI Emulation & Directraw mode
fix: hooked "FrameRect", "TabbedTextOutA", "DrawTextA", "DrawTextExA", "FillRect" in scaled mode only
fix: FIXNCHITTEST mode
fix: when main win is closed, blit area is made null to avoid messing with a wrong screen area
added "Release mouse outside window" option. This option causes the get cursor position to detect a centered mouse position when the cursor is moved outside the window, allowing interaction with other windows without scrolling ot the windowed program. Mainly, this option is meant to help people with a physical disability to use other programs (e. g. the virtual keyboard) to play games.
added "Launch" field (optional) to start the program with arguments or a separate task

v2.02.72
fix: fixed Launch field used with "Use DLL injection" flag
fix: somehow improved "Use DLL injection" to avoid blocked tasks and allow exception handling

v2.02.73
Add: preliminary support for (unchecked) "Run in window" flag. Now if the flag is unchecked, DxWnd does NOT force the program to run in a window. Still, some rendering modes are not working perfectly.
add: support for Unicode DefWindowProcW API call
fixed some differences in D3D CreateDevice vs. CreateDeviceEx handling
added "Window frame compensation" input flag to set the cursor position properly in "Sub Commando"
fix: properly handled the case of NULL WindowProc routine in CreateDialog/Indirect API calls. Makes "LEGO Stunt Rally" working.
added support for Unicode SystemParametersInfoW API call

v2.02.74
Add: API hot patching, thank to aqrit precious support. Now Doom 3 is played in window.
hooked GetCursorInfo call
fix: recalculate vsync timers in case of time stretching
fix: log failure for CreateWindowExW wchar api
Add: configurable keyboard commands
Add: timestretch toggle command
Add: "Enable hotkeys" flag to enable/disable hotkeys
fix: eliminated dialogs with timer warning messages (to be investigated...)
fix: moved "no banner" flag to main tab again
fix: potential string violation, possible cause for flash crash?

v2.02.75
Hooked D3DDevice::ShowCursor method to apply force show/hide cursor (preliminary...)
fix: dynamic allocation for WndProc stack to eliminate predefined limit of 256 entries. Star Wars Rebellion uses more than 256.
added: "Suppress D3D8/9 Reset" flag
fix: improved show FPS and Time Stretch overlay so that the two overlays won't overlap each other
fix: bug in ddraw "Locked surface" mode preventing output on screen
fix: fixed bug in critical common portion of the code that was crashing even unhooked programs (namely, Flash Player, ...)

v2.02.76
fixed and enhanced several features about registry emulation: added flags "Emulate registry" (to add missing entries) and "Override registry" (to fake existing entries with different values). Fixed "Requiem Avenging Angel" DirectX bogus check bug.
fixed bug in emulate surface palette handling affecting "Requiem Avenging Angel" colors

v2.02.77
GUI: added program icon display
fix: some log messages
fix: handling of GetProcAddress D3D9 api 
fix: handling of EMULATEBUFFER option - fixes "Star Trek Birth of the Federation" 
fix: expanded hot patching scope - now can handle QueryPerformanceCounter and fix time stretching on obfuscated version of "Wind Fantasy SP"
added logging in D3DDevice9::GetAdapterIdentifier 

v2.02.78
fix: hooked D3D8/9Device::BeginStateBlock
ddraw API hot swappable
log: helper for ChangeDisplaySettings() flags
fix: hot patch handling of loaded modules
log: fixed some messages with no line ending for GetObjectType()
d3d begin of code reorganization and cleaning
ole32, user32 API hot patched
handling of DWL_DLGPROC message 

v2.02.79
fix: recovered capability to "pin" ddraw and d3d calls

v2.02.80
Preliminary multilanguage release, english and chinese, many thanks to gsky916 support
fixed Sleep bug for small delays and accelerated timeshift: fixes "Wind Fantasy SP" reported bug
added log for DirectInput::SetCooperativeLevel flags
fixed flip emulation mode when backbuffer is lost: fixes cursor problems in "HellCopter"
fixed Pitch setting in DirectDraw::EnumDisplayModes when in SVGA emulation mode. Let "Outlive" use this setting

v2.02.81
fix: hooked GetMonitorInfoA/W that is a possible way to get the screen resolution. The hooker sets the virtual screen size  updating both rcWork & rcMonitor rects in the MONITORINFO structure. This fixes the mouse control in "Tomb Raider Underworld" and likely in other games with a similar engine.

v2.02.82
fix: completed chinese translation and language selection through dxwnd.ini file.
added very preliminary (and incomplete) italian translation for debugging.

v2.02.83
fix: small error in one coordinate scaling procedure
fix: completed translation in italian (just for testing....)
fix: added palette window activation in icon tray menu
fix: supppressed BackBuffer release within D3D7:CreateDevice: this makes "Tetris Worlds" fully playable
add: build-in check for compatibility modes set!

v2.02.84
fix: removed extra reference to ddraw session causing window movements on fullscreen game exit: fixes another "Wind Fantasy SP" reported bug

v2.02.85
fix: revised handling of d3d D3DFORMAT field: fixes "Call of Cthulhu DCotE" color problems
fix: added recovery for rounded child win coordinates: fixes blitting problems in Diablo when win size is not an exact multiple of native resolution.

v2.02.86
fix: thank to aqrit's research, hooked and hanlded user32.dll GetUpdateRgn API. This fixes refresh and crash problems in Diablo & Hellfire.
added bilinear filtering to directdraw palettized 8BPP emulated mode (where it is mostly needed!). A must try is "Genocide" !

v2.02.87
attempt to fix Win8 missing support for earlier ddraw releases (Croc legend of the Gobbos, Project I.G.I., ...)
fix: optimized bilinear filtering with no horizontal / vertical sawtooth artifacts
added support for 16BPP and 32BPP desktop bilinear filtering
suppressed compatibility checking
fix: doubled default window size when bilinear filtering is active
added repositioning of control parent window when main win is moved 
added debug messages about hooked window class and name
fix: ignore IME window when hooking main win, destroy it when requested
fix: ignore HWND_MESSAGE type windows
added "Diablo tweak" compatibility flag for Diablo proper screen update handling. It is a temporary tweak waiting to understand better how to properly manage it.

v2.02.88
fix: DIABLOTWEAK flag defaulted to FALSE
add: CLEARTARGET flag - forces a Clear CLEAR_TARGET operation on D3D BeginPaint: useful in combination with wireframe mode.
updated "Optimize CPU" behavior: now it emulates a VSync wait also on D3D8/D3D9. Fixes some "swimming" problem in "Soul Reaver 2", and saves CPU time by limiting the FPS to the refresh rate.
fixed positioning of FPS and time stretching indication on screen overlay
cleaned up GUI project: no more japanese and localized resources, no more unreferenced resources.
GUI: added Desktop status window
GUI: added pixel format descriptor to DxWnd status

v2.02.89
fix: added SVGA 512x384 video mode. Necessary for "Outcast" loading screen.
fix: using WINNLSEnableIME call to suppress IME window. 
fix: improved some logging

v2.02.90
added: "Filter winposchange messages" flag - needed for "RollerCoaster Tycoon / Deluxe / II" games
fix: correct handling of texture surfaces for "Zoo Tycoon" in surface emulation mode.
fix: handling of GDI32.dll calls in GetProcAddress
fix: improved logging
fix: GetRgnBox hooking moved to normal window processing, to handle RollerCoaster Tycoon series.
added: GetTempFileName fix to handle Win95 differences. This makes "Powerslide" playable.

v2.02.91
fix: bug in bilinear filtering mode causing possible game crashes when blitting partial rectangles (es. Age of Empires II cursor)
added: AERO friendly mode - brings AERO compatibility and incredible performance boost for ddraw games in surface emulation mode

v2.02.92
fix: "Keep aspect ratio" fixed for AERO environment
fix: improved "Oprtimized for AERO mode" compatibility, both when set / unset.
fix: Skip / Limit FPS now acting on full scren blits only. For instance, they no longer slow cursor sprite.

v2.02.93
fix: handling of primary / backbuffer surfaces when DDSCAPS_3DDEVICE is set (es. Fifa 99 in Direct3D mode)

v2.02.94
fix: handling of "Keep aspect ratio" for aspect ratios different from 4:3
fix: missing resolution 400x300 in "Limit resolution" handling
fix: surface handling switch in automatic mode. 
added two flags for DDSCAPS_SYSTEMMEMORY handling on different contexts. Added extra performances and compatibility with D3D in emulated mode.
added "Updated bigger than 1/4 screen size" flag, to perform FPS counting or limitations upon substantial screen updated (heuristic: bigger than 1/4 of the total size...)
added true bilinear filter. Previous one was renamed as fast 2x filter (BILINEAR2XFILTER).
fix: when the window was minimized & the "do not notify on task switch" flag was on, the game crashed. Now the blitting operations to invalid rect coordinates are suppressed.
fix: debug option "highlight blit to primary" was causing game crashes when the updated rect was too little. 
fix: trapped sporadic CloseHandle(hMenu) crashes in a try/catch clause.
fix: attempt to fix FIXD3DFRAME option
fix: added SetStretchBltMode(hdc,HALFTONE) to SetDIBitsToDevice hooker to improve stretching: fixes "Celtic Kings Rage of War"

v2.02.95
fix: handling of limit resolution field
fix: FPS handling to screen updated made by SetDIBitsToDevice
fix: PeekMessage implementation with "Peek all message in queue": fixes "Shadow Watch"
fix: missing hook to CreateProcess - needed for "Suppress child process creation".
fix: exception for bilinear filtering applied to certain games (e.g. "Shadow Watch")

v2.02.96
fix: FPS inticator on window title was causing the program to become irresponsive (partial fix)
fix: proper setting of ddraw surface capabilities will allow primary surface emulation for most D3D1-7 games
fix: missing initialization of variables in screen size limit handling
fix: processing of mouse messages 
fix: GetSystemMetrics and LoadLibrary* calls hooked by hot patching to fix "Wind Fantasy SP" movie problems
fix: completed winmm multimedia api hooking to fix "Wind Fantasy SP" movie problems, and not only....
fix: revised FPS control to assure more stable fps when a FPS limit delay is set 

v2.02.97
fix: mouse black trails in "Deadlock II"
fix: missing default value for filter mode

v2.02.98
fix: better ddraw surface handling - now  "Darkened Skye" runs in emulated surface mode with perfect colors
fix: fixed RDTSC opcode search loop - fixed time stretching for "Ubik"
add: added "Peplace privileged opcodes" flag - makes unpatched "Ubik" run
fix: revised ddsurface capabilities policy to allow D3D1-7 games to run in emulated mode and bilinear filtering

v2.02.99
fix: completed bilinear filtering for 16bpp desktop color depth
fix: fixed surface handling, Suppress SYSTEMMEMORY options now moved to debug flags (maybe unnecessary)
fix: AERO compatibility is now declared just once
fix: fast bilinear blitting in the 32 -> 32 BPP case
add: texture hooking
add: texture management
add: Direct3D dedicated configuration tab

v2.03.01
fix: fixed some bitmap handling while processing textures
add: texture management dump to texture.out folder and hack from texture.in folder
fix: AERO optimized mode with ddraw7 only surfaces: "Hoyle Casino Empire" works at least in fullscreen mode.
fix: bilinear filters compiled with optimized and openmp flags - big speed improvement.
fix: reduced the maximum amount of visible video card with "Limit available resources" flag: now "Breath of Fire IV" works.

v2.03.02
fix: when creating an IID_IDirectDrawGammaControl object through lpdds->QueryInterface redirect fake primary surface to real one to make the object working. Fixes "Might & Magic 7"
fix: fixed logging of ddraw Set/GetGammaRamp methods. Added SUPPRESSDXERRORS error suppression
add: texture extraction parameters (min & max x,y size) in dxwnd.ini config file
add: texture extraction / hack made in DxWnd install folder, to allow usage also for games running from unwritable (CDRom) directory
fix: in "prevent maximize" mode, set the main window in maximized state. Allow "Hoyle Casino Empire" to continue after the intro movie is terminated
add: Process kill command now warns you about existence of task
fix: DxWnd window position save now works on multiscreen desktop

v2.03.04
fix: changed surface capability policy so that "Risk II" works with identical surface pitches

v2.03.05
add: texture dump for d3d8 & d3d9. Note: highlight & hack yet to be implemented. d3d10 & d3d11 yet to be implemented. Texture types not complete.
fix: handling of d3d10 (Assassin's Creed) 

v2.03.06
fix: do not try to set vsync delays on ddraw surface when not created yet 
fix: catched several sporadic errors before they could crash the application
fix: GetAttachedSurface() now retrieves a backbuffer from the list, instead of referencing the last one - this fixes "Tomb Raider III" GOG release in non emulated mode.
add: "Normalize performance counter" flag to fix an improper use of QueryPerformanceCounter() made by "Cyber Gladiators"
add: "GDI Color conversion" debug flag

v2.03.07
fix: key matching for virtual registry now made case insensitive (needed for "Die Hard Trilogy")
fix: handling of null values passed to extRegQueryValueEx as lpType and lpData arguments (needed for "Die Hard Trilogy")
fix: DirectDrawSurface::GetPalette returns the virtual palette when applied to virtual primary / backup surfaces (needed for "Die Hard Trilogy")
fix: fixed dump for 8BPP palettized textures (needed for "Die Hard Trilogy")
fix: handling (with no operation) of D3DFMT_Q8W8V8U8 texture type and other bumpmap formats (used by "Tiger Woods PGA Tour 08")
fix: handling of LIMITRESOURCES flag for DirectDraw::GetCaps method when memory exceeds 0x70000000 bytes
fix: handling of LIMITRESOURCES flag for Direct3DDevice::GetAvailableTextureMem method when memory exceeds 1GB
fix: don't change screen resolution in SetDisplayMode when wrong (negative) values are passed. Fixes a problem in binkplayer.exe
fix: fixed OutTrace to avoid possible infinite recursion when loading C runtime libraries and logging LoadLibrary activity
fix: eliminated critical races when using DLL injection, thank to Luigi Auriemma's suggestion (inject an endless loop in the main thread and remove it at the end of injection)
fix: implemented DLL injection according to Luigi Auriemma's schema in CreateProcess hooking routine (needed for "Die Hard Trilogy")
fix: using MinHook library to acquire compatibility with all APIs
fix: hooked GetExitCodeProcess to handle "SUPPRESSCHILD" special case
fix: using hot patching for SystemParametersInfo APIs
fix: in SystemParametersInfo suppressed invalid operations in window mode: SPI_SETKEYBOARDDELAY SPI_SETKEYBOARDSPEED
add: son process handling with 4 different cases: 2 old cases (default case and "SUPPRESSCHILD") plus "INJECTSON" and "ENABLESONHOOK" to hook the son process without/with DLL injection
add: debug color conversion mode through GDI routines
add: multi-hooking for multiple processes contemporarily, adding the line "multiprocesshook=1" in [window] section of dxwnd.ini. Use at your own risk!
add: partial logging of Direct3DDevice::GetDeviceCaps output (to be completed)
add: handling of notes in the DxWnd GUI (configuration notes tab)
mod: when log is not possible on program's folder, it is no longer written in %TEMP% dir, is now written in DxWnd local dir.

v2.03.08
add: "GDI mode" ddraw surface handling mode
add: "center to window" option
add: "message pump" input flag - fixes Arxel Tribe games (Faust, Legend of the prophet & assassin, ...)
fix: "Viper Racing" crash
fix: "limit resources" option for "Tiger Woods PGA World Tour 08"
fix: "limit resources" option for "Jeff Gordon XS Racing demo"
add: debug "stress resources" option
fix: added a couple of directinput diagnostic logs
add: debug option "freeze injected son"
add: made check for admin rights configurable in dxwnd.ini
add: "portable" capability to configure relative pathnames
fix: small bug in game menu string width - now game titles should not be trunked any longer.

v2.03.09
code reorganization & reuse
add: "Set texture pixel format" flag, makes "Jeff Gordon XS Racing" working on emulated mode and 32BPP desktop
add: "GDI mode" emulation uses HALFTONE to activate GDI bilinear stretching when "Full Bilinear" filter is activated: slower, but better quality
add: preliminary hooking for EnumZBufferFormats ddraw7 method
fix: eliminated some handle leakage when injecting launched processes

v2.03.10
add: real time logging through OutputDebugString
add: menu command to launch DbgView.exe
add: support for timestamped logs
add: "Erase trace file" flag
fix: handle leakage for primary hdc, causing rapid performance downgrade
fix: log message reorganization
add: SetStretchBltMode HALFTONE in GDI mode to provide bilinear filtered GDI stretching
fix: some GDI mode optimization - should bring performances similar to D3DWindower, with similar configuration
fix: GDI mode bug, crashing when blitting from NULL surface (to do a color fill operation)

v2.03.11
fix: added DDSCAPS_FLIP capability to emulated backbuffer surface. Fixes "Injection" missing flips
fix: added high order bit set to kernel32.dll GetVersion() wrapper in case of Win95/98 emulation. Fixes "Warhead" frontend program
fix: coordinates returned by user32.dll GetClipCursor() must be scaled accordingly with virtual desktop size. Fixes "SubCulture" mouse control problems

v2.03.12
fix: several issues in fake registry hooks
add: continue char '\' in fake registry configuration for hex values
add: GetClipList ddraw hooker: fixes "Full Pipe" blitting coordinates
fix: proper handling of palette reserved entries
fix: dinput GetDeviceData
add: implemented "Release mouse outside window" functionality in dinput calls
fix: now "Release mouse outside window" leaves the mouse close to where it left the window
fix: in GUI, keeps memory of last used paths
fix: several log messages

v2.03.13
add: integrated printscreen facility with 8BPP paletized dump, activated by "printscreen" special key (default Alt-F12)
fix: recovery for lost surfaces (i.e. after a Ctrl+Alt+Del command)
fix: palette display form showing active entries
fix: EnumDisplayModes hook in emulate surface mode
fix: several log improvements

v2.03.14
add: window commands Minimize, Restore & Close
add: made palette behaviour configurable (until better understood...)
add: "updatepaths" tag in dxwnd.ini

v2.03.15
fix: keep aspect ratio calculations
fix: d3d Present method, to properly scale to window size (fixes "Silent Hunter III" rendering)
fix: GetMonitorInfo hooker: in windowed mode the call may fail, a virtual size and ok retcode should be returned
fix: Blt method recovering errors when D3D CreateAdditionalSwapChain method fails: allow to see the intro movies of "Silent Hunter III".
fix: added some missing D3D errorcode labels in log file

v2.03.16
fix: MapWindowPoints hook - added coordinate scaling (fixes "NBA Live 99" components size and position)
fix: using "Suppress D3D8/9 reset" sets the backbuffer area as large as the whole desktop to avoid clipping
add: added "Unlock Z-order" flag to avoid window to stay locked on top of z-order (useful for "NBA Live 99")
add: added "EA Sprots hack" flag to suppress some interfering hooks set by EA games internally (useful for "NBA Live 99")

v2.03.18:
fix: eliminated direct output to primary surface, causing more troubles than goods ("Divine Divinity" flickering...)
fix: handled CoCreateInstance calling CoCreateInstanceEx in hot patch mode.
fix: eliminated InvalidateRect calls in ddrow Unlock hooker: this is not the right way to fix "Deadlock II", sorry.
fix: DirectDrawEnumerateEx log
fix: cursor is hidden in fullscreen mode only
fix: CoCreateInstance & CoCreateInstanceEx wrappers. Now "Crusaders of Might & Magic" is playable in window

v2.03.19:
fix: scaling of movie coordinates rendered through MciSendString 
fix: doubled rendering to screen with fast 2x bilinear filtering
fix: crash when setting "showFPS overlay" with DirectX "hybrid mode"
fix: "keep aspect ratio" with DirectX "GDI mode"
fix: when ddraw hook is set to "none", no hook operations are performed on ddraw
add: possibility to control the D3D8/9 SwapEffect field with the "force swap effect" flag
fix: revised handling of GDI device context shared with DirectDraw primary surface, improved behaviour of "Star trek Armada"

v2.03.20:
fix: revised hookers for kernel32 GetVersionExA/W api, adding support for dwPlatformId field
fix: texture handling for texture dump/hack/highlight
add: "Reserve legacy memory segments" compatibility flag, needed for "Crusaders of Might & Magic" 
add: preliminary support for ddraw7 DirectDrawCreateClipper api
add: "Suppress main window destruction" flag, to make "Prince of Persia 3D" playable
fix: fully revised ddraw surface handling in d3d1-7 versions. Now "Might & Magic IX" is supported

v2.03.21:
fix: full re-design of ddraw "special" surface management (primary & backbuffer): greater compatibility for emulation mode
fix: GDI edit window positioning for "Imperialism" city name selection

v2.03.22:
more code reorganization and cleanup
eliminated GDI over directdraw emulation (MAPGDITOPRIMARY flag)
add: support for texture D3DFMT_L8 dump ("Turok")
fix: eliminate a possible divide by zero error
fix: avoid intercepting WindowProc routines when set to special values 0xFFFFxxxx - makes "The Hulk" demo working

v2.03.23:
fix: implemented the correct interface of registry query calls that can be used to know the key data length 
fix: hexdump routine with NULL input used in registry operation log was causing crash
add: preliminary hooking for user32.dll desktop routines, like CreateDesktop() - makes "STCC Swedish TouringCar Championship" working

v2.03.24:
fix: added missing log for RegSetValueEx key value when type is REG_SZ (string)
fix: handling of DDERR_SURFACEBUSY error in blit operations, recovers "Virtua Cop"
fix: several changes in palette handling, improve (but don't fix!) "Man in Black" palette rendering
fix: EnumDisplayMode hooker, passing wrong vodeo modes to the callback routine. Fixes "Total Annihilation Kingdoms" crash
fix: DxWnd GUI build with very large width, causing slow interface responsiveness.
add: added the "Suppress Release on backbuffer" that makes "Tetris Worlds" working, avoiding to release the backbuffer surface too many times.

v2.03.25:
fix: CoCreateInstance hooking for IDirectDraw interface: fixes "Darius Gaiden"
fix: BackBufferCaps value for NOSYSTEMEMULATED option. Fixes "Forsaken" crashes in AERO mode.
fix: GDI GetDC and GetWindowDC must not redirect the zero hWnd to the current main window if not in fullscreen mode.

v2.03.26:
add: Added "Set KEY_WOW64_64KEY flag" flag to registry operations. It could prove useful for old programs on recent W64 platforms.
add: Added "fix movies color depth" flag. It fixes some problems when windows multimedia functions detect and process the actual video color mode and they should rather consider the virtual value. Fixes "Killing Time" intro movies decoding.
fix: suppressed hooking for comdlg32.dll module. Common dialogues never need to be altered. Fixes "Road Rash" save/load game dialog problems.

v2.03.27:
add: dxwnd proxyes to get standalone windowed mode (ask for instructions)
fix: added SetAppCompatData(1, 0) to AERO compatibility for lock operations - improves AERO compatibility, e.g. "Age of Empires" intro movie
add: added "Disable max window mode" flag for better compatibility with Win8/8.1
fix: cleaned up some opengl hooking code. Mind possible regressions ....

v2.03.28:
add: added "Lock / Unlock pitch fix" flag to compensate for pitch change in emulation mode. Fixes "Snowboard racer" slanted graphic
add: added "Width not power of 2 fix" flag to compensate for pitch change in emulation mode. Fixes "Microsoft Midtown Madness" slanted graphic
add: debug flag "Highlight Locked surfaces" to highlight service surfaces that are Lock-ed and Unlock-ed.

v2.03.29:
fix: wrong d3d7 hook, now "Echelon" works again.
add: "hide taskbar" option. A little experimental, so far. If the game crashes, the taskbar may not be reactivated: use menu commands to recover.

v2.03.30:
fix: reset for flipped DC retrieved from ddraw interface after ddraw creation. Fixes "Powerslide".
add: added "Send WM_ACTIVATEAPP message" flag: fixes "Championship Manager 03 04" mouse and "Thorgal" lock
fix: FillRect hooker. Good for "Premier Manager 98" and "Premier Manager 97".
fix: revised the DDSCAPS_SYSTEMMEMORY suppression, now separated for primary & backbuffer surfaces

v2.03.31:
fix: handling of "No SYSTEMMEMORY on BackBuffer" flag for no-emulation mode: necessary to run "Microsoft Motocross Madness 2" in this mode.
fix: allowing hot patching to GetCursorPos() user32.dll call. This allows correct mouse control with obfuscated exes, like the chinese RPG "Paladin 3".

v2.03.32:
add: flags "Ask confirmation on window close" and "Terminate on window close"
fix: implemented surface stack cleanup on overflow: this should allow "European Air War" to run through several seasons.
debug: added a few diagnostic messages to trap possible errors in DLL injection functions
fix: recovered the menu Edit->Add command that went disabled

v2.03.33:
add: new surface handling, more similar to D3DWindower. The previous mode can be activated by setting the "Flip emulation" flag. The new mode can successfully manage "Microsoft Motocross Madness 2".
fix: Reelease operation for primary surface when used for GDI GetDC operation
fix: protections for GDI ReleaseDC operations against null window or DC

v2.03.34:
fix: handling of real primary/backbuffer surfaces in non-emulated modes and WinXP: now uses system memory if possible, video memory otherwise. 
fix: simulation of backbuffer attach in non emulated modes.
fix: mciSendString hooker, was losing a final command line argument

v2.03.35:
add: added "Set ZBufferBitDepths capability" flag to set a no longer supported fields in the capability structure. This makes ""Banzai Bug" bypass the capability checks.
fix: better handling of surface capabilities in flippable mode
fix: fixed bug in primary surface handling when already created ...
add: added ""Share ddraw and GDI DC" flag: sharing was introduced in release v2.03.19, but proved to be incompatible in many situations. Now it is off by default and enabled when necessary.
fix: the flags for disabling DDSCAPS_SYSTEMMEMORY capability are now used also in non-emulated flipping emulation mode. Depending on the video card, it may help getting compatibility.
fix: handling of NULL DC in non emulated modes: the NULL DC (corresponding to the whole desktop) is replaced by the window DC. This reduces problems like clearing the whole desktop.
fix: EnumDisplayModes was returning wrong modes list in SVGA mode.
fix: applied the error suppression to the DeleteAttachedSurface hooker.
fix: hooked User32 CreateRectRegion/Indirect calls. 

v2.03.36:
fix: fixed "fix movies color depth" flag for color depths greater than 8BPP. Fixes the "Horde 2 the Citadel" intro movie.
add: added the "Set KEY_WOW64_32KEY flag" flag to prevent registry redirection for win32 applications
add: virtual registry configuration integrated in DxWnd configuration and exported files
add: hooking of RegEnumValue() call. Useful for "Star trek Generations"
fix: fixed mouse handling through "Message processing" option for child windows. Fixes "Star Trek Armada" mouse problems
fix: fixed GetDC hooker for child windows. Fixes some "Star Trek Armada" graphic problems
fix: CreateCompatibleDC hooker printing wrong error messages in normal conditions
fix: fixed DrawText, DrawTextEx hookers returning wrong RECT structure. Fixes "Star Trek Armada" text placement problems
add: completed log messages for DrawTextEx, CreateDialogParam, DialogBoxParam and CreateDialogIndirectParam

v2.03.37:
add: handling of RegEnumValue advapi call in virtual registry. Useful for "Star trek Generations"
add: virtual registry now integrated in dxwnd.ini configuration and activated automatically. No more handling of dxwnd.reg file
fix: virtual registry optimization, code reorganization, bug fixing ....
add: "Stretch ActiveMovie window" flag. Fixes "The Thing" intro movies.
fix: apply "Blit from backbuffer" option to both surface emulated and non emulated cases. Should help fixing "Galapagos" screen locks.
fix: apply "Limit screen resolution" to the list of detected video modes in VGA emulated case
fix: eliminated "Automatic" surface emulation mode, no longer supported
fix: fixed InvalidateRect rectangle: avoids flickering in "Microsoft Pandora's Box"
add: added "Bypass MCI calls" flag. 

v2.03.38
fix: handling of WM_ACTIVATE and WM_NCACTIVATE messages, should bring the window to HWND_NOTOPMOST z-order position only when activation is on, not off!
fix: in "Share GDI and draw DC" mode, check if the primary surface really has a DC and switch to another one if the DC is missing.
fix: eliminated some redundant code in GDI ReleaseDC in DC emulated mode

v2.03.39
add: "Fix glPixelZoom args" flag to scale images rendered by OpenGL through GDI. It is needed for "Strength and Honor".
add: "Reuse emulated DC" flag to optimize GDI emulation. experimental.

v2.03.40
fix: detecting main window when setting FULLSCREEN mode against it. Helps handling "Reah"
add: option to de-interlace videos on 16BPP to 32BPP color transformation. Required for deinterlacing "11th Hour" movies
add: cornerized mode, could help handling Battlenet GDI menus
add: preliminary implementation of virtual desktop window. Useless so far.
fix: hooked more GDI calls: CreateICA, GetViewportExtEx, IntersectClipRect, SetRectRgn
fix: ChangeDisplaySettings no longer take note of video mode when called with CDS_TEST flag
fix: merged user32 FillRect and FrameRect handling
fix: fixed bug that was leaving a copy of dxwnd.ini on last visited directory on window close

v2.03.41
fix: filtering of mouse messages in "Message pump" option. Makes games more responsive to mouse clicks. Improves "X-files".
fix: exclusion of DDSCL_SETDEVICEWINDOW, DDSCL_CREATEDEVICEWINDOW and DDSCL_SETFOCUSWINDOW flags in ddraw SetCooperativeLevel windowed mode
fix: guess PRIMARY surface in ddraw1 when no flags (but BACKBUFFERCOUNT) are set. Necessary for "Star Force Deluxe"
delete: suppressed frame compensation option FRAMECOMPENSATION (unused).
add: added debug flag NOWINDOWHOOKS to avoid WindowProc hook.
fix: GetPaletteEntries hook returns 256 value against hpal=NULL. Let "M.I.B." run, though palette is still not ok.
fix: StretchDIBits hook no longer can return an unassigned value
fix: anti-recursion check in DialogBox WindowProc call

v2.03.42
add: option "Syncronize GDI and DDRAW palette" - fixes "Man in Black" palette problems
add: apply "Remap MCI coordinates" options also to movies rendered through MCIWndCreate: fixes "Man in Black" movie problems
fix: fixed InvalidateRect hooker logic - fixes "Imperialism" graphic.

v2.03.43
fix: eliminated SYSTEMMEMORY usage on DDSCAPS_3DDEVICE surfaces. Needed for "Arx Fatalis"
fix: deleted emulation of ZBUFFER attach. Needed for "Arx Fatalis"
fix: added emulation of PixelFormat in getPixelFormat hooker. Needed for "Arx Fatalis"
add: added virtual joystick feature. Required to play "Tie Fighter 95" with no real joystick
v2.03.43.fix1
fix: recovered virtual joystick buttons

v2.03.44
fix: GetSystemPaletteUse error message
fix: transparent virtual joystick icons
fix: RealizePalette hooker crash in "Super Real Mahjong P7"
fix: complete recoding of GDI BitBlt, StretchBlt and PatBlt hookers: it fixes "Reah" problems
fix: changes in USER32 BeginPaint hooker to support DC sharing with ddraw
fix: increased configuration panels heigth do avoid clipping when desktop font size is set to 125%

v2.03.45
add: virtual joystick center-lock feature, helps using the mouse as mouse leaving the joystick locked.
fix: revised dinput hooking. dinput8 still to be completed. 
add: option "Unacquire input devices on focus lost". Needed for Star wars "Tie Fighter 95".
fix: set dinput SetCooperativeLevel flags to DISCL_NONEXCLUSIVE | DISCL_FOREGROUND
fix: fixed CreateIC hooker call method (WINAPI)
fix: emulated Unacquire on virtual joystick. Needed for Star wars "Tie Fighter 95".

v2.03.46
add: added compatibility options "Hook GOG libs" and "Byass GOG libs" to manage GOG games
add: added "Enable Time Freeze" timing option and the virtual key "freezetime"
add: command line option /E to terminate exixting DxWnd session
fix: relocation of WS_POPUP windows. Fixes "Riven" menu window.

v2.03.47
add: added handling of DirectInput8 dinput8.dll
fix: completely revised directinput hooking to manage multiple devices acquire/unacquire
fix: added some dinput error codes to log messages
fix: increased time freeze toggle key hysteresis time to 1 sec to avoid multiple activations

v2.03.48
fix: set proper surface capabilities for 3DDEVICE surfaces, according to VIDEOMEMORY capability.
add: "force clipper" flag to redirect clipper definition to main window and primary surface
fix: better handling of surfaces on minimize / restore events, mainly on WinXP
fix: proper default values for"Share ddraw and GDI DC" and "Lock/Unlock Pitch Fix" flags

v2.03.49
fix: a few improvements in handling NULL lpDDSSource in Blit/Flip operations
add: option to print relative timing to log
fix: avoid handling textures at level greater than 0 since this seems to cause application crash
fix: restore cursor clipping through WM_SETFOCUS and DefWindowProc when focus is gained
fix: "One Must Fall Battlegrounds" keyboard fix - can't SetWindowsHookEx with WH_KEYBOARD and NULL thread

v2.03.50
fix: check for errors that could cause random GDI cursor clipping regions
fix: more accurate hooking of ddraw pointers - fixes several DD_GENERIC_ERROR in Win8/10 platforms, makes "SWAT 3" working
fix: improvements (not finished) in dinput handling
add: the OS version info is traced in the log file
fix: added cursor clipping when requested on new main window creation
fix: handling of cursor clipping when the window gain/lose focus threough DefWindowProc message handling

v2.03.51
fix: WinXP select file dialog bug, not appearing if pointing to nonexistent directory
fix: export file bug - now forces the file to have a .dxw extension
fix: eliminated 6 useless fields in dinput handling, now their value is calculated automatically and at runtime
add: DirectInput / "Emulate mouse relative movement". Eliminates the fake clipped regions inside the window (fix Geneforge series)
fix: directinput mouse mode (absolute vs. relative) now correctly checked using the mouse device
fix: attempt to fix FIXMOVIESCOLOR flag for 32BPP target color depth, but it doesn't still work on Win10
fix: directinput RELEASEMOUSE flag applied to mouse device only

v2.03.52
add: added Win NT 4.0 to the list of detected OS
fix: DirectInput SetCooperativeLevel: should not change window handle - fix startup error in "Storm Angel"
fix: fixed GetWindowRect to replace real desktop with virtual one - fix startup error in "Storm Angel"
fix: ddraw hookers keep different original pointers for each interface: fixes several ddraw UNKNOWN_ERROR
fix: eliminated the ddraw full log functionality, because of the increased complexity
add: added several supported fake screen resolutions - needed for "Last resort" SWAT3 mod
fix: used fake resolutions also in USER32 EnumDisplaySettings
fix: avoid hooking directinput dlls if not requested when loaded dynamically
fix: added recovery of lost device in DirectInput GetDeviceData method
fix: changed SetHook so that it can get function pointers without necessarily replacing them
add: option "Limit ddraw interface" to exclude support for IDirectDrawInterface greater than limit (range 1-7)

v2.03.53(skipped)
v2.03.54 (fix1+2)
add: syslib shared dc mode preliminary implementation, thank to Narzoul's ideas. Works for "MS Golf 98" and "Deadlock II rel. 1.2"
fix: handling of cursor hide/show for programs that don't go through the message window ("Deadlock II rel. 1.2")
add: /R:n command line option to automatically start DxWnd in icon tray, run the n-th program in configuration and terminate.
add: option to disable the disabling of Alt-Tab key through SetWindowHooks through WH_KEYBOARD_LL event. Make it posssible to use Alt-Tab key on "Dungeon Kepper II".
fix: palette object reference count decremented from extra references and zeroed by the "Return 0 refcount" flag: make it possible to run "Dark Earth" that makes a check on the palette reerence counter before starting the game.

v2.03.55
fix: differentiated handling of blitting from memory to DC vs. from DC vs. memory. The second case is less frequent, but altered the correct rendering of "Battlezone 1998 edition", now working both in scaled, emulated DC shared DC and shared ddraw and GDI DC. The fix impacts both BitBlt and ScaledBlt GDI calls.
fix: prevented InvalidateRect to return giving a scaled rect. Fixes "Deadlock 2"partial screen updates.

v2.03.56/fix1
add: capability to hook calls referenced by ordinal number
add: "Acquire admin caps" flag to self elevate DxWnd if configured as necessary
add: added third mode to handle potentially offending messages, that is process them by the Window default process routine WinDefaultProc. Good for "Red Alert 2".
add: "fix clipper area in shared DC", temporary solution to adapt the shared DC mode to the different situations (yet to finish).
fix: fully integrated the patch for "Silver" game: returning backbuffer as a doublebuffer also when requested with DDSCAPS_VIDEOMEMORY caps.
add: FunkyFr3sh addition to process WM_SYSCOMMAND SC_CLOSE and force program termination when requested by "Terminate on window close" option.

v2.03.57
fix: improved handling of clipping regions in shared dc mode
add: added DWM status to log file
add: flags for preliminary hook of DirectSound and Smack libraries
fix: changed icon colors (yellow for red, doesn't suggest the idea of error or danger)
add: menu commands for entry movement (top, bottom, up, down) or duplication

v2.03/58
add: added program manifest stating Win10/8/7/Vista compatibility
add: "Clear compatbility flag" tool
add: "Recover screen mode" tool
add: "Set path" tool
fix: reorganized menu commands
fix: proper handling of DEFAULT win size on CreateWindowEx - fixes "Imperialism" problem
fix: fixed stretced blit operation because Blt behaves differently from StretchBlt. Fixes "3D Minigolf" partial background
fix: GetPixel logging

v2.03.59
fix: added extra ref to surface for ddraw version 2: fixes "Warhammer Chaos Gate"
add: view shims menu command, to analyze Win7/Win10 compatibility patches
GUI: added the hook tab to move hooking flags in a single tab page

v2.03.60 
too many things, sorry, I'll fix it later.