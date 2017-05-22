#include <ddraw.h>
//#include "dxwcore.hpp"

//********** Begin Edit *************
#define APP_DXWNDNAME			DxWnd
#define APP_DXWNDVERSION		v2.03.60
//********** End Edit ***************

#define DXW_IDLE		0
#define DXW_ACTIVE		1
#define DXW_RUNNING		2

#define MAXTARGETS			256

#define ONEPIXELFIX 1 
#define HOOKDDRAWNONE 12

// first flags DWORD dwFlags1:
#define UNNOTIFY			0x00000001 
#define EMULATESURFACE		0x00000002
#define CLIPCURSOR		 	0x00000004 // Force cursor clipping within window
#define NEEDADMINCAPS		0x00000008 // DxWnd needs administrator's privileges to hook this program
#define HOOKDI				0x00000010
#define MODIFYMOUSE			0x00000020
#define HANDLEEXCEPTIONS	0x00000040 // Handles exceptions: Div by 0 ....
#define SAVELOAD			0x00000080
#define EMULATEBUFFER		0x00000100
#define HOOKDI8				0x00000200
#define BLITFROMBACKBUFFER	0x00000400
#define SUPPRESSCLIPPING	0x00000800
#define AUTOREFRESH			0x00001000
#define FIXWINFRAME			0x00002000
#define HIDEHWCURSOR		0x00004000
#define SLOWDOWN			0x00008000 
#define ENABLECLIPPING		0x00010000
#define LOCKWINSTYLE		0x00020000
#define MAPGDITOPRIMARY	 	0x00040000 
#define FIXTEXTOUT			0x00080000
#define KEEPCURSORWITHIN	0x00100000
#define USERGB565			0x00200000
#define SUPPRESSDXERRORS	0x00400000 // suppresses some common dx errors (BUSY)
#define PREVENTMAXIMIZE		0x00800000 // struggle to avoid window maximize
#define LOCKEDSURFACE		0x01000000 // emulates the buffer of the primary surface when locked and written directly
#define FIXPARENTWIN		0x02000000 // fixes parent window size & position
#define SWITCHVIDEOMEMORY 	0x04000000 // when VIDEO memory is over, switches to SYSTEM memory
#define CLIENTREMAPPING		0x08000000 // hooks ClientToRect, RectToClient, GetClientRect, GetWinRect
#define HANDLEALTF4			0x10000000 // forces quitting the program when receiving Alt-F4 key
#define LOCKWINPOS			0x20000000 // prevent the program to change its own windows properties
#define HOOKCHILDWIN		0x40000000 // hook CHILD windows to alter placement coordinates (UNUSED)
#define MESSAGEPROC			0x80000000 // process peek/get messages

// second flags DWORD dxw.dwFlags2:
#define RECOVERSCREENMODE	0x00000001 // actively set screen mode to initial state after hooking
#define REFRESHONRESIZE		0x00000002 // forces a refresh (blitting from backbuffer to primary) upon win resize
#define BACKBUFATTACH		0x00000004 // sets backbuf wxh dim. equal to primary surface so that ZBUFFER is attachable.....
#define MODALSTYLE			0x00000008 // window with no borders
#define KEEPASPECTRATIO		0x00000010 // when resizing, keep original aspect ratio
#define INIT8BPP			0x00000020 // simulate a 8BPP initial desktop setting (in GetDeviceCaps API)
#define FORCEWINRESIZE		0x00000040 // adds NCMOUSEUP/DOWN processing to the win procedure if missing
#define INIT16BPP			0x00000080 // simulate a 16BPP initial desktop setting (in GetDeviceCaps API)
#define KEEPCURSORFIXED		0x00000100 // inhibit SetCursorPos operation
#define DISABLEGAMMARAMP	0x00000200 // let the application retrieve the desktop DC (for capability queries)
//#define DIFFERENTIALMOUSE	0x00000400 // emulates the 360-degrees-free-running mouse style....
#define FIXNCHITTEST		0x00000800 // fixes WM_NCHITTEST message X,Y coordinates 
#define LIMITFPS			0x00001000 // delays primary blit operations to limit FPS 
#define SKIPFPS				0x00002000 // skips primary blit operations up to limit
#define SHOWFPS				0x00004000 // shows FPS value to status win / log / title bar
#define HIDEMULTIMONITOR	0x00008000 // hide multimonitor configurations: GetAdapterCount returns 1.
#define TIMESTRETCH			0x00010000 // make system time stretchable 
#define HOOKOPENGL			0x00020000 // Hook OpenGL calls
#define WALLPAPERMODE		0x00040000 // mouse events are discarded (good for screensaver-like)
#define SHOWHWCURSOR		0x00080000 // enable hardware cursor
#define GDISTRETCHED		0x00100000 // Stretch GDI/user32 coordinates to adapt to window size
#define SHOWFPSOVERLAY		0x00200000 // shows FPS value to status win / log / screen overlay
#define FAKEVERSION			0x00400000 // pretends the platvorm is a given window version / subversion
#define FULLRECTBLT			0x00800000 // blit to primary surface using NULL source & dest rect
#define NOPALETTEUPDATE		0x01000000 // Do not refresh primary surface on palette updates
#define SUPPRESSIME			0x02000000 // suppress IME 
#define NOBANNER			0x04000000 // suppress fancy logo & banneer effects 
#define WINDOWIZE			0x08000000 // Run in a Window (default TRUE) 
#define LIMITRESOURCES		0x10000000 // Limit resources to fit an old program's expectations 
#define STARTDEBUG			0x20000000 // Start in DEBUG mode
#define SETCOMPATIBILITY	0x40000000 // invoke ddraw SetAppCompatData to set aero compatibility mode
#define WIREFRAME			0x80000000 // forces wireframe display for D3D and Glide games

// third flags DWORD dxw.dwFlags3:
#define FORCEHOOKOPENGL		0x00000001 // loads OpenGL32.dll and hooks it
#define MARKBLIT			0x00000002 // higlights the blit to primary surface operation by surroundig the rect in color
#define HOOKDLLS			0x00000004 // Hook all DLLs referenced in PE
#define SUPPRESSD3DEXT		0x00000008 // Disables extended d3d APIs for Vista/Win7/Win8 platforms
#define HOOKENABLED			0x00000010 // Enables task hooking
#define FIXD3DFRAME			0x00000020 // Preserve windows frame in D3D9 programs
#define FORCE16BPP			0x00000040 // Forces 16BPP desktop color depth
#define BLACKWHITE			0x00000080 // Simulate a B&W screen monitor mapping colors to grayscales
#define MARKLOCK			0x00000100 // higlights the locked/unlocked surfaces by surroundig the rect in color
#define SINGLEPROCAFFINITY	0x00000200 // Set Process Affinity to a single core
#define EMULATEREGISTRY		0x00000400 // Emulate registry api to read extra keys
#define CDROMDRIVETYPE		0x00000800 // Pretends that GetDriveType() always returns DRIVE_CDROM
#define NOWINDOWMOVE		0x00001000 // Do not try to update window position & size on D3D rendering
#define FORCECLIPPER		0x00002000 // Set ddraw clipping for real primary surface and primary window - but FastBlt cannot be used!
#define LOCKSYSCOLORS		0x00004000 // Lock Sys Colors changes by SetSysColors() call
#define GDIEMULATEDC		0x00008000 // Map GDI/user32 calls to primary to a memory surface to be stretch-blitted to the primary
#define FULLSCREENONLY		0x00010000 // assume that the program is always in fullscreen mode
#define FONTBYPASS			0x00020000 // bypass font unsupported API
#define MINIMALCAPS			0x00040000 // Simulate minimal HW caps (copied from VMWare virtual screen)
#define DEFAULTMESSAGES		0x00080000 // peocess  offending messages that are typical of a window with default action
#define BUFFEREDIOFIX		0x00100000 // fix buffered IO incompatibilities between pre-Win98 and post-WinNT
#define FILTERMESSAGES		0x00200000 // ignore offending messages that are typical of a window and are hot handled by a fullscreeen app
#define PEEKALLMESSAGES		0x00400000 // force Peek-ing all sort of messages to avoid Win7 message queue saturation that leads to program halt 
#define SURFACEWARN			0x00800000 // warn when a unclassified surface capability is met (debug only!)
#define ANALYTICMODE		0x01000000 // activate analytic mode (debug only!)
#define FORCESHEL			0x02000000 // Forces HEL  (DDCREATE_EMULATIONONLY)
#define CAPMASK				0x04000000 // Enable capability mask (according to ini file)
#define COLORFIX			0x08000000 // Win7 color fix for 8bpp paletized modes
#define NODDRAWBLT			0x10000000 // Suppress ddraw Blt to primary
#define NODDRAWFLIP			0x20000000 // Suppress ddraw Flip to primary
#define NOGDIBLT			0x40000000 // Suppress GDI Blt to video device
#define NOPIXELFORMAT		0x80000000 // Don't fix the pixel format for plain surfaces

// fourth flags DWORD dxw.dwFlags4:
#define NOALPHACHANNEL		0x00000001 // By default, create surfaces with no DDPF_ALPHAPIXELS capability
#define SUPPRESSCHILD		0x00000002 // suppress child process generation (CreateProcessA, ...?)
#define FIXREFCOUNTER		0x00000004 // fixes the reference counter for the main directdraw session (the one connected to primary surfaces)
#define SHOWTIMESTRETCH		0x00000008 // Shows time stretching multiplier on screen overlay
#define ZBUFFERCLEAN		0x00000010 // Clean ZBUFFER upon (fix for "Star Wars  Ep. I - Racer")
#define ZBUFFER0CLEAN		0x00000020 // Clean ZBUFFER upon (fix for "Star Wars  Ep. I - Racer")
#define ZBUFFERALWAYS		0x00000040 // Forces ZBUFFER D3DCMP_ALWAYS condition to show all planes....
#define DISABLEFOGGING		0x00000080 // Disable D3D fogging
#define NOPOWER2FIX			0x00000100 // Handle textures whose size is not a power of 2 (32, 64, 128 ...)
#define NOPERFCOUNTER		0x00000200 // Disables the GetPerfCounter performance metrics API,as if it was not supported....
#define BILINEAR2XFILTER	0x00000400 // attempt to smooth low-res graphic by applying simplified bilinear filtering in emulation mode
#define INTERCEPTRDTSC		0x00000800 // Intercapts RDTSC opcodes to hook at assembly level
#define LIMITSCREENRES		0x00001000 // Limit available screen resolution up to defined maximum
#define NOFILLRECT			0x00002000 // Suppress FillRect calls
#define HOOKGLIDE			0x00004000 // Hook glide calls
#define HIDEDESKTOP			0x00008000 // Hide desktop background
#define STRETCHTIMERS		0x00010000 // Enables timers stretching when TIMESTRETCH is on
#define NOFLIPEMULATION		0x00020000 // Compensate the effect of Flip-ping through Blt operations
#define NOTEXTURES			0x00040000 // Disabled (whitened) textures
#define RETURNNULLREF		0x00080000 // IDirectDraw::Release and IDDrawSurface::Release will always return 0 ref. counter
#define FINETIMING			0x00100000 // Fine timeslider coarse, by 10% steps ranging from about :2 to x2 
#define NATIVERES			0x00200000 // Supports hardware native resolutions (depending on your PC)
#define SUPPORTSVGA			0x00400000 // Supports SVGA standard resolutions
#define SUPPORTHDTV			0x00800000 // Supports HDTV standard resolutions
#define RELEASEMOUSE		0x01000000 // GetCursorPos returns centered coordinates when the mouse cursor is moved outside the window
#define ENABLETIMEFREEZE	0x02000000 // Enable Time Freeze functionality and toggle key
#define HOTPATCH			0x04000000 // Use hot patching technique to handle obfuscated IAT
#define ENABLEHOTKEYS		0x08000000 // Enables hot keys
#define HOTPATCHALWAYS		0x10000000 // Force hot patching to every call
#define NOD3DRESET			0x20000000 // Disables D3D8/9 D3DDevice::Reset method
#define OVERRIDEREGISTRY	0x40000000 // same as EMULATEREGISTRY, but fake keys takeprecedence
#define HIDECDROMEMPTY		0x80000000 // hide empty CDROM drivers

// fifth flags DWORD dxw.dwFlags5:
#define DIABLOTWEAK			0x00000001 // ... ??? ...
#define CLEARTARGET			0x00000002 // forces a D3D clean on target surface upon each BeginScene invocation
#define NOWINPOSCHANGES		0x00000004 // suppress WM_WINDOWPOSCHANGING/CHANGED messages (RollerCoaster Tycoon...)
//#define NOSYSTEMMEMORY		0x00000008 // forces suppression of DDSCAPS_SYSTEMMEMORY capability on surfaces
#define NOBLT				0x00000010 // suppress blit to primary surface
//#define NOSYSTEMEMULATED	0x00000020 // forces suppression of DDSCAPS_SYSTEMMEMORY capability on emulated front & backbuffer surfaces
#define DOFASTBLT			0x00000040 // use FastBlt to primary surface
#define AEROBOOST			0x00000080 // Optimize for AERO environment: does all stretching in sysmemory surfaces
#define QUARTERBLT			0x00000100 // Consider a screen update (to count or limit FPS) only  blt operations bigger than a quarter of the whole primary surface
#define NOIMAGEHLP			0x00000200 // Interceptd Imagehlp.dll unsupported calls (used by "the 5th element")
#define BILINEARFILTER		0x00000400 // experimental bilinear filtering
#define REPLACEPRIVOPS		0x00000800 // replace privileged opcodes, such as IN (Ubik)
#define REMAPMCI			0x00001000 // remap MCI calls coordinates in mciSendString
#define TEXTUREHIGHLIGHT	0x00002000 // highlight textures with random color & grid
#define TEXTUREDUMP			0x00004000 // dump textures to file system as bmp files
#define TEXTUREHACK			0x00008000 // load (replace) hacked textures from file system (bmp files)
#define TEXTURETRANSP		0x00010000 // transparent textures (unimplemented)
#define NORMALIZEPERFCOUNT	0x00020000 // Normalize Performance Counter to a Performance Frequency of 1MHz
#define HYBRIDMODE			0x00040000 // ?????
#define GDICOLORCONV		0x00080000 // do color conversion using GDI
#define INJECTSON			0x00100000 // when starting a son process, inject dxwnd.dll at the beginning of execution
#define ENABLESONHOOK		0x00200000 // forward hooking capability to son processes
#define FREEZEINJECTEDSON	0x00400000 // BEWARE!!!! you must know what you're doing. Leaves the injected son in an endless loop 
#define GDIMODE				0x00800000 // Use GDI to both stretch and color conversion. It seems faster....
#define CENTERTOWIN			0x01000000 // do NOT stretch the image to the whole window client area, but center to it.
#define STRESSRESOURCES		0x02000000 // simulates a lack of resources condition, for testing (debug opt.)
#define MESSAGEPUMP			0x04000000 // inserts a "message pump" loop between repeated operation that may stop the task on Win7 and greater
#define TEXTUREFORMAT		0x08000000 // Apply virtual pixel format to texture surfaces without DDSD_PIXELFORMAT attribute
#define DEINTERLACE			0x10000000 // Deinterlace movies / screens that have black odd lines by interpolating even lines
#define LOCKRESERVEDPALETTE 0x20000000 // lock the reserved palette entries (usually 20: 0-9 and 246-255)
#define UNLOCKZORDER		0x40000000 // Inhibit attempts to keep the main win on top of ZORDER by stripping BringWindowToTop and SetForegroundWindow calls
#define EASPORTSHACK		0X80000000 // Hack to intercept and neutralize some of the hooks set internally by EA Sports games

// sixth flags DWORD dxw.dwFlags6:
#define FORCESWAPEFFECT		0x00000001 // in D3D8/9, forces the SwapEffect value in CreateDevice/Reset operations
#define LEGACYALLOC			0x00000002 // reserve legacy memory segments
#define NODESTROYWINDOW		0x00000004 // suppress the main window destruction
#define NOMOVIES			0x00000008 // suppress MCI Movies
#define SUPPRESSRELEASE		0x00000010 // Avoid releasing critical emulated surfaces - the backbuffer
#define FIXMOVIESCOLOR		0x00000020 // Fixes vfw movies color depth forcing the virtual value 
#define WOW64REGISTRY		0x00000040 // Sets KEY_WOW64_64KEY registry flag
#define DISABLEMAXWINMODE	0x00000080 // invoke ddraw SetAppCompatData(12,0) to set DisableMaxWindowedMode
#define FIXPITCH			0x00000100 // Fixes surface filling made with no concern to actual pitch value
#define POWER2WIDTH			0x00000200 // Fixes .....
#define HIDETASKBAR			0x00000400 // Hides taskbar & start menu during game execution
#define ACTIVATEAPP			0x00000800 // sends WM_ACTIVATEAPP message after every WM_WINPOSCHANGED message
#define NOSYSMEMPRIMARY		0x00001000 // forces suppression of DDSCAPS_SYSTEMMEMORY capability on emulated primary surface
#define NOSYSMEMBACKBUF		0x00002000 // forces suppression of DDSCAPS_SYSTEMMEMORY capability on emulated backbuffer surface
#define CONFIRMONCLOSE		0x00004000 // on close button, ask the user before closing the window / terminating the program
#define TERMINATEONCLOSE	0x00008000 // on WM_CLOSE message, also terminates the program
#define FLIPEMULATION		0x00010000 // create virtual primary and backbuffer as un-attached surfaces and replace Flip with Blt
#define SETZBUFFERBITDEPTHS	0x00020000 // when not set, set the legacy dwZBufferBitDepths field in the directdraw capability from GetCaps()
#define SHAREDDC			0x00040000 // enable sharing window DC and primary surface DC
#define WOW32REGISTRY		0x00080000 // Sets KEY_WOW32_64KEY registry flag
#define STRETCHMOVIES		0x00100000 // Stretches ActiveMovie Windows to the whole window size
#define BYPASSMCI			0x00200000 // bypass all MCI calls returning OK.
#define FIXPIXELZOOM		0x00400000
#define REUSEEMULATEDDC		0x00800000
#define CREATEDESKTOP		0x01000000 // creates a virtual desktop window to replace the real one
#define NOWINDOWHOOKS		0x02000000 // do not intercept window callback routines
#define SYNCPALETTE			0x04000000 // Syncronize GDI to DDRAW palette
#define VIRTUALJOYSTICK		0x08000000 // Enables the DxWnd virtual joystick
#define UNACQUIRE			0x10000000 // Unacquire DirectInput devices when losing focus and acquire back when gaining focus
#define HOOKGOGLIBS			0x20000000 // Hook additional libraries provided by GOG with different names but same functionalities as system ones
#define BYPASSGOGLIBS		0x40000000 // Bypass GOG proxy system libraries loading directly the system libraries from the system folder
#define EMULATERELMOUSE		0x80000000 // Emulates the dinput detection of relative mouse position by keeping the mouse at the center of window and looking for movements

// seventh flags DWORD dxw.dwFlags7:
#define LIMITDDRAW			0x00000001 // Limit the maximum available ddraw object version
#define DISABLEDISABLEALTTAB 0x00000002 // Disables the compatibility patch that disables the Alt-Tab key and other special combinations
#define FIXCLIPPERAREA		0x00000004 // fix clipper area in ddraw & GDI shared DC mode
#define HOOKDIRECTSOUND		0x00000008
#define HOOKSMACKW32			0x00000010

// eighth flags DWORD dxw.dwFlags8:

// logging Tflags DWORD:
#define OUTTRACE			0x00000001 // enables tracing to dxwnd.log in general
#define OUTDDRAWTRACE		0x00000002 // traces DxWnd directdraw screen handling
#define OUTWINMESSAGES		0x00000004 // traces windows messages
#define OUTCURSORTRACE		0x00000008 // traces cursor positions & operations
//#define OUTPROXYTRACE		0x00000010 // enables all operations through proxy functions
//#define DXPROXED			0x00000020 // hook DX proxy methods to log each call in original behaviour
#define ASSERTDIALOG		0x00000040 // show assert messages in Dialog Box
#define OUTIMPORTTABLE		0x00000080 // dump import table contents
#define OUTDEBUG			0x00000100 // detailed debugging indormation
#define OUTREGISTRY			0x00000200 // log registry operations
#define TRACEHOOKS			0x00000400 // log hook operations
#define OUTD3DTRACE			0x00000800 // traces DxWnd direct3d screen handling
#define OUTDXWINTRACE		0x00001000 // traces DxWnd internal operations
#define ADDRELATIVETIME		0x08000000 // log timestamp is relative to previous line
#define NOLOGCLOSE			0x10000000 // avoid closing the log file handle ("Riven, during CD changes ...)
#define ADDTIMESTAMP		0x20000000 // add timestamp (GetTickCount) to log file
#define OUTDEBUGSTRING		0x40000000 // duplicate logs through OutputDebugString()
#define ERASELOGFILE		0x80000000 // clears old trace file before writing new logs

#define EMULATEFLAGS		(EMULATEBUFFER | EMULATESURFACE | LOCKEDSURFACE)
#define HANDLEFPS			(SHOWFPS | SHOWFPSOVERLAY | LIMITFPS | SKIPFPS)
#define TEXTUREMASK			(TEXTUREHIGHLIGHT|TEXTUREDUMP|TEXTUREHACK|TEXTURETRANSP)

// DxWnd host app data to be passed to the hook callback
typedef struct TARGETMAP
{
	char path[MAX_PATH+1];
	char module[60+1];
	char OpenGLLib[20+1];
	int dxversion;
	int coordinates;
	int flags;
	int flags2;
	int flags3;
	int flags4;
	int flags5;
	int flags6;
	int flags7;
	int flags8;
	int tflags;
	short posx;
	short posy;
	short sizx;
	short sizy;
	short MaxFPS;
	short InitTS;
	short FakeVersionId;
	short MaxScreenRes;
	short SwapEffect;
	short MaxDdrawInterface;
}TARGETMAP;

typedef struct
{
	short Status;
	short TaskIdx;
	short IsFullScreen;
	short Width, Height;
	short ColorDepth;
	DDPIXELFORMAT pfd;
	short DXVersion;
	HWND hWnd;
	DWORD dwPid;
	BOOL isLogging;
	DWORD FPSCount;
	int TimeShift;
	short CursorX, CursorY;
	PALETTEENTRY Palette[256];
	BOOL AllowMultiTask;
} DXWNDSTATUS;

extern DXWNDSTATUS DxWndStatus;

int SetTarget(TARGETMAP *);
int StartHook(void);
int EndHook(void);
void GetDllVersion(char *);
int GetHookStatus(DXWNDSTATUS *);
DXWNDSTATUS *GetHookInfo();
void HookInit(TARGETMAP *, HWND);

char *GetDxWndPath();
void *SetHook(void *, void *);
void SetHook(void *, void *, void **, char *);
void OutTrace(const char *, ...);
void OutTraceHex(BYTE *, int);
void *HookAPI(HMODULE, char *, void *, const char *, void *);
void AdjustWindowFrame(HWND, DWORD, DWORD);
char *hexdump(unsigned char *, int);

LRESULT CALLBACK extWindowProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK extChildWindowProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK extDialogWindowProc(HWND, UINT, WPARAM, LPARAM);

// defines below to condition debug message handling

#define OutTraceW if(dxw.dwTFlags & OUTWINMESSAGES) OutTrace
//#define OutTraceX if(dxw.dwTFlags & OUTPROXYTRACE) OutTrace
#define OutTraceDW if(dxw.dwTFlags & OUTDXWINTRACE) OutTrace
#define OutTraceDDRAW if(dxw.dwTFlags & OUTDDRAWTRACE) OutTrace
#define OutTraceD3D if(dxw.dwTFlags & OUTD3DTRACE) OutTrace
#define OutTraceC if(dxw.dwTFlags & OUTCURSORTRACE) OutTrace
#define OutTraceB if(dxw.dwTFlags & OUTDEBUG) OutTrace
#define OutTraceR if(dxw.dwTFlags & OUTREGISTRY) OutTrace
#define OutTraceH if(dxw.dwTFlags & TRACEHOOKS) OutTrace
#define OutTraceP OutTrace
#define OutTraceE OutTrace

#define IsTraceW (dxw.dwTFlags & OUTWINMESSAGES)
//#define IsTraceX (dxw.dwTFlags & OUTPROXYTRACE)
#define IsTraceDW (dxw.dwTFlags & OUTDXWINTRACE)
#define IsTraceDDRAW (dxw.dwTFlags & OUTDDRAWTRACE)
#define IsTraceD3D (dxw.dwTFlags & OUTD3DTRACE)
#define IsTraceC (dxw.dwTFlags & OUTCURSORTRACE)
#define IsTraceR (dxw.dwTFlags & OUTREGISTRY)
#define IsTraceH (dxw.dwTFlags & TRACEHOOKS)
#define IsTraceP (TRUE)
#define IsTraceE (TRUE)
#define IsDebug  (dxw.dwTFlags & OUTDEBUG)
#define IsAssertEnabled (dxw.dwTFlags & ASSERTDIALOG)
#define STEP OutTrace("STEP at %s:%d\n", __FILE__, __LINE__)

typedef enum {
	DXW_SET_COORDINATES = 0,
	DXW_DESKTOP_CENTER,
	DXW_DESKTOP_WORKAREA,
	DXW_DESKTOP_FULL
} Coordinates_Types;

typedef enum {
	DXW_NO_LIMIT = 0,
	DXW_LIMIT_320x200,
	DXW_LIMIT_400x300,
	DXW_LIMIT_640x480,
	DXW_LIMIT_800x600,
	DXW_LIMIT_1024x768,
	DXW_LIMIT_1280x960
} ResolutionLimits_Types;

typedef enum {
	TIMER_TYPE_NONE = 0,
	TIMER_TYPE_USER32,
	TIMER_TYPE_WINMM
} Timer_Types;

typedef struct {
	int w; 
	int h;
} SupportedRes_Type;

extern SupportedRes_Type SupportedSVGARes[];
extern SupportedRes_Type SupportedHDTVRes[];
extern int SupportedDepths[];

#define SUPPORTED_DEPTHS_NUMBER 4
