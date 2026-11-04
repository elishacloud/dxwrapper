#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// Window attributes
enum DWMWINDOWATTRIBUTE
{
	DWMWA_NCRENDERING_ENABLED = 1,      // [get] Is non-client rendering enabled/disabled
	DWMWA_NCRENDERING_POLICY,           // [set] Non-client rendering policy
	DWMWA_TRANSITIONS_FORCEDISABLED,    // [set] Potentially enable/forcibly disable transitions
	DWMWA_ALLOW_NCPAINT,                // [set] Allow contents rendered in the non-client area to be visible on the DWM-drawn frame.
	DWMWA_CAPTION_BUTTON_BOUNDS,        // [get] Bounds of the caption button area in window-relative space.
	DWMWA_NONCLIENT_RTL_LAYOUT,         // [set] Is non-client content RTL mirrored
	DWMWA_FORCE_ICONIC_REPRESENTATION,  // [set] Force this window to display iconic thumbnails.
	DWMWA_FLIP3D_POLICY,                // [set] Designates how Flip3D will treat the window.
	DWMWA_EXTENDED_FRAME_BOUNDS,        // [get] Gets the extended frame bounds rectangle in screen space
	DWMWA_HAS_ICONIC_BITMAP,            // [set] Indicates an available bitmap when there is no better thumbnail representation.
	DWMWA_DISALLOW_PEEK,                // [set] Don't invoke Peek on the window.
	DWMWA_EXCLUDED_FROM_PEEK,           // [set] LivePreview exclusion information
	DWMWA_CLOAK,                        // [set] Cloak or uncloak the window
	DWMWA_CLOAKED,                      // [get] Gets the cloaked state of the window
	DWMWA_FREEZE_REPRESENTATION,        // [set] Force this window to freeze the thumbnail without live update
	DWMWA_LAST
};

// Non-client rendering policy attribute values
enum DWMNCRENDERINGPOLICY
{
	DWMNCRP_USEWINDOWSTYLE, // Enable/disable non-client rendering based on window style
	DWMNCRP_DISABLED,       // Disabled non-client rendering; window style is ignored
	DWMNCRP_ENABLED,        // Enabled non-client rendering; window style is ignored
	DWMNCRP_LAST
};

typedef ULONGLONG DWM_FRAME_COUNT;
typedef ULONGLONG QPC_TIME;

typedef struct _UNSIGNED_RATIO {
	UINT32 uiNumerator;
	UINT32 uiDenominator;
} UNSIGNED_RATIO;

typedef struct _DWM_TIMING_INFO {
	UINT32          cbSize;
	UNSIGNED_RATIO  rateRefresh;
	QPC_TIME        qpcRefreshPeriod;
	UNSIGNED_RATIO  rateCompose;
	QPC_TIME        qpcVBlank;
	DWM_FRAME_COUNT cRefresh;
	UINT            cDXRefresh;
	QPC_TIME        qpcCompose;
	DWM_FRAME_COUNT cFrame;
	UINT            cDXPresent;
	DWM_FRAME_COUNT cRefreshFrame;
	DWM_FRAME_COUNT cFrameSubmitted;
	UINT            cDXPresentSubmitted;
	DWM_FRAME_COUNT cFrameConfirmed;
	UINT            cDXPresentConfirmed;
	DWM_FRAME_COUNT cRefreshConfirmed;
	UINT            cDXRefreshConfirmed;
	DWM_FRAME_COUNT cFramesLate;
	UINT            cFramesOutstanding;
	DWM_FRAME_COUNT cFrameDisplayed;
	QPC_TIME        qpcFrameDisplayed;
	DWM_FRAME_COUNT cRefreshFrameDisplayed;
	DWM_FRAME_COUNT cFrameComplete;
	QPC_TIME        qpcFrameComplete;
	DWM_FRAME_COUNT cFramePending;
	QPC_TIME        qpcFramePending;
	DWM_FRAME_COUNT cFramesDisplayed;
	DWM_FRAME_COUNT cFramesComplete;
	DWM_FRAME_COUNT cFramesPending;
	DWM_FRAME_COUNT cFramesAvailable;
	DWM_FRAME_COUNT cFramesDropped;
	DWM_FRAME_COUNT cFramesMissed;
	DWM_FRAME_COUNT cRefreshNextDisplayed;
	DWM_FRAME_COUNT cRefreshNextPresented;
	DWM_FRAME_COUNT cRefreshesDisplayed;
	DWM_FRAME_COUNT cRefreshesPresented;
	DWM_FRAME_COUNT cRefreshStarted;
	ULONGLONG       cPixelsReceived;
	ULONGLONG       cPixelsDrawn;
	DWM_FRAME_COUNT cBuffersEmpty;
} DWM_TIMING_INFO;

HRESULT DwmFlush();
HRESULT DwmSetWindowAttribute(HWND, DWORD, LPCVOID, DWORD);
HRESULT DwmGetCompositionTimingInfo(HWND hwnd, DWM_TIMING_INFO* pTimingInfo);
