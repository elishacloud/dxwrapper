/**
* Copyright (C) 2019 Elisha Riedlinger
*
* This software is  provided 'as-is', without any express  or implied  warranty. In no event will the
* authors be held liable for any damages arising from the use of this software.
* Permission  is granted  to anyone  to use  this software  for  any  purpose,  including  commercial
* applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*   1. The origin of this software must not be misrepresented; you must not claim that you  wrote the
*      original  software. If you use this  software  in a product, an  acknowledgment in the product
*      documentation would be appreciated but is not required.
*   2. Altered source versions must  be plainly  marked as such, and  must not be  misrepresented  as
*      being the original software.
*   3. This notice may not be removed or altered from any source distribution.
*/

#include "Settings\Settings.h"
#include "Dllmain\Dllmain.h"
#include "Utils.h"
#include "Logging\Logging.h"

namespace Utils
{
	namespace Fullscreen
	{
		// Declare constants
		static constexpr LONG MinWindowWidth = 320;			// Minimum window width for valid window check
		static constexpr LONG MinWindowHeight = 240;		// Minimum window height for valid window check
		static constexpr LONG WindowDelta = 40;				// Delta between window size and screensize for fullscreen check
		static constexpr DWORD TerminationCount = 10;		// Minimum number of loops to check for termination
		static constexpr DWORD TerminationWaitTime = 2000;	// Minimum time to wait for termination (LoopSleepTime * NumberOfLoops)

		// Overload functions
		bool operator==(const RECT& a, const RECT& b)
		{
			return (a.bottom == b.bottom && a.left == b.left && a.right == b.right && a.top == b.top);
		}

		bool operator!=(const RECT& a, const RECT& b)
		{
			return (a.bottom != b.bottom || a.left != b.left || a.right != b.right || a.top != b.top);
		}

		// Declare structures
		struct screen_res
		{
			LONG Width = 0;
			LONG Height = 0;

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

		struct window_update
		{
			HWND hwnd = nullptr;
			HWND ChildHwnd = nullptr;
			RECT rect = { sizeof(rect) };
			screen_res ScreenSize;

			window_update& operator=(const window_update& a)
			{
				hwnd = a.hwnd;
				ChildHwnd = a.ChildHwnd;
				rect.bottom = a.rect.bottom;
				rect.left = a.rect.left;
				rect.right = a.rect.right;
				rect.top = a.rect.top;
				ScreenSize = a.ScreenSize;
				return *this;
			}

			bool operator==(const window_update& a) const
			{
				return (hwnd == a.hwnd && ChildHwnd == a.ChildHwnd && rect == a.rect && ScreenSize == a.ScreenSize);
			}

			bool operator!=(const window_update& a) const
			{
				return (hwnd != a.hwnd || ChildHwnd != a.ChildHwnd || rect != a.rect || ScreenSize != a.ScreenSize);
			}
		};

		struct window_layer
		{
			HWND hwnd = nullptr;
			bool IsMain = false;
			bool IsFullScreen = false;
		};

		struct handle_data
		{
			DWORD process_id = 0;
			HWND best_handle = nullptr;
			DWORD LayerNumber = 0;
			window_layer Windows[256];
			bool AutoDetect = true;
			bool Debug = false;
		};

		struct menu_data
		{
			DWORD process_id = 0;
			bool Menu = false;
		};

		// Declare variables
		bool m_StopThreadFlag = false;
		bool m_ThreadRunningFlag = false;
		HANDLE m_hThread = nullptr;
		DWORD m_dwThreadID = 0;

		// Function declarations
		void GetScreenSize(HWND&, screen_res&, MONITORINFO&);
		LONG GetBestResolution(screen_res&, LONG, LONG);
		void SetScreenResolution(LONG, LONG);
		void SetScreen(screen_res);
		bool IsMainWindow(HWND);
		bool IsWindowTooSmall(screen_res);
		bool IsWindowFullScreen(screen_res, screen_res);
		bool IsWindowNotFullScreen(screen_res, screen_res);
		void GetWindowSize(HWND&, screen_res&, RECT&);
		BOOL CALLBACK EnumWindowsCallback(HWND, LPARAM);
		HWND FindMainWindow(DWORD, bool, bool = false);
		BOOL CALLBACK EnumMenuWindowsCallback(HWND, LPARAM);
		bool CheckForMenu(DWORD);
		BOOL CALLBACK EnumChildWindowsProc(HWND, LPARAM);
		void SendAltEnter(HWND&);
		void SetFullScreen(HWND&, const MONITORINFO&);
		void CheckForTermination(DWORD);
		DWORD WINAPI StartThreadFunc(LPVOID);
		void MainFunc();
	}
}

using namespace Utils;

//*********************************************************************************
// Screen/monitor functions below
//*********************************************************************************

// Gets the screen size from a wnd handle
void Fullscreen::GetScreenSize(HWND& hwnd, screen_res& Res, MONITORINFO& mi)
{
	GetMonitorInfo(MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY), &mi);
	Res.Width = mi.rcMonitor.right - mi.rcMonitor.left;
	Res.Height = mi.rcMonitor.bottom - mi.rcMonitor.top;
}

// Check with resolution is best
LONG Fullscreen::GetBestResolution(screen_res& ScreenRes, LONG xWidth, LONG xHeight)
{
	//Set vars
	DEVMODE dm = { 0 };
	dm.dmSize = sizeof(dm);
	LONG diff = 40000;
	LONG NewDiff = 0;
	ScreenRes.Width = 0;
	ScreenRes.Height = 0;

	// Get closest resolution
	for (DWORD iModeNum = 0; EnumDisplaySettings(nullptr, iModeNum, &dm) != 0; iModeNum++)
	{
		NewDiff = abs((LONG)dm.dmPelsWidth - xWidth) + abs((LONG)dm.dmPelsHeight - xHeight);
		if (NewDiff < diff)
		{
			diff = NewDiff;
			ScreenRes.Width = (LONG)dm.dmPelsWidth;
			ScreenRes.Height = (LONG)dm.dmPelsHeight;
		}
	}
	return diff;
}

// Sets the resolution of the screen
void Fullscreen::SetScreenResolution(LONG xWidth, LONG xHeight)
{
	DEVMODE newSettings;
	ZeroMemory(&newSettings, sizeof(newSettings));
	if (EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &newSettings) != 0)
	{
		newSettings.dmPelsWidth = xWidth;
		newSettings.dmPelsHeight = xHeight;
		newSettings.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;
		ChangeDisplaySettings(&newSettings, CDS_FULLSCREEN);
	}
}

// Verifies input and sets screen res to the values sent
void Fullscreen::SetScreen(screen_res ScreenRes)
{
	// Verify stored values are large enough
	if (!IsWindowTooSmall(ScreenRes))
	{
		// Get the best screen resolution
		GetBestResolution(ScreenRes, ScreenRes.Width, ScreenRes.Height);

		// Set screen to new resolution
		SetScreenResolution(ScreenRes.Width, ScreenRes.Height);
	}
}

// Resets the screen to the registry-stored values
void Fullscreen::ResetScreen()
{
	// Reset screen settings
	Logging::Log() << "Reseting screen resolution...";
	std::string lpRamp((3 * 256 * 2), '\0');
	HDC hDC = GetDC(nullptr);
	GetDeviceGammaRamp(hDC, &lpRamp[0]);
	Sleep(0);
	SetDeviceGammaRamp(hDC, &lpRamp[0]);
	ReleaseDC(nullptr, hDC);
	Sleep(0);
	ChangeDisplaySettings(nullptr, 0);
}


//*********************************************************************************
// Window functions below
//*********************************************************************************

bool Fullscreen::IsMainWindow(HWND hwnd)
{
	return GetWindow(hwnd, GW_OWNER) == (HWND)0 && IsWindowVisible(hwnd);
}

bool Fullscreen::IsWindowTooSmall(screen_res WindowSize)
{
	return WindowSize.Width < MinWindowWidth || WindowSize.Height < MinWindowHeight;
}

bool Fullscreen::IsWindowFullScreen(screen_res WindowSize, screen_res ScreenSize)
{
	return abs(ScreenSize.Width - WindowSize.Width) <= WindowDelta ||		// Window width matches screen width
		abs(ScreenSize.Height - WindowSize.Height) <= WindowDelta;			// Window height matches screen height
}

bool Fullscreen::IsWindowNotFullScreen(screen_res WindowSize, screen_res ScreenSize)
{
	return (ScreenSize.Width - WindowSize.Width) > WindowDelta ||			// Window width does not match screen width
		(ScreenSize.Height - WindowSize.Height) > WindowDelta;				// Window height does not match screen height
}

// Gets the window size from a handle
void Fullscreen::GetWindowSize(HWND& hwnd, screen_res& Res, RECT& rect)
{
	GetWindowRect(hwnd, &rect);
	Res.Width = abs(rect.right - rect.left);
	Res.Height = abs(rect.bottom - rect.top);
}

// Enums all windows and returns the handle to the active window
BOOL CALLBACK Fullscreen::EnumWindowsCallback(HWND hwnd, LPARAM lParam)
{
	// Get variables from call back
	handle_data& data = *(handle_data*)lParam;

	// Skip windows that are from a different process ID
	DWORD process_id;
	GetWindowThreadProcessId(hwnd, &process_id);
	if (data.process_id != process_id)
	{
		return true;
	}

	// Skip compatibility class windows
	char class_name[80] = { 0 };
	GetClassName(hwnd, class_name, sizeof(class_name));
	if (strcmp(class_name, "CompatWindowDesktopReplacement") == 0)			// Compatibility class windows
	{
		return true;
	}

	// Skip windows of zero size
	RECT rect = { sizeof(rect) };
	screen_res WindowSize;
	GetWindowSize(hwnd, WindowSize, rect);
	if (WindowSize.Height == 0 && WindowSize.Width == 0)
	{
		return true;
	}

#ifdef _DEBUG
	//Debugging window layers
	if (data.Debug)
	{
		++data.LayerNumber;
		char buffer[7] = { 0 };
		_itoa_s(data.LayerNumber, buffer, 10);
		char* isMain = "";
		if (IsMainWindow(hwnd))
		{
			isMain = "*";
		}
		if (WindowSize.Height != 0 && WindowSize.Width != 0)		// Filter out screens that are zero size
		{
			char buffer1[7] = { 0 }, buffer2[7] = { 0 }, buffer3[7] = { 0 }, buffer4[7] = { 0 };
			_itoa_s(rect.left, buffer1, 10);
			_itoa_s(rect.top, buffer2, 10);
			_itoa_s(rect.right, buffer3, 10);
			_itoa_s(rect.bottom, buffer4, 10);
			Logging::Log() << "Layer " << buffer << " found window class " << isMain << class_name << " | Left: " << buffer1 << " Top: " << buffer2 << " Right: " << buffer3 << " Bottom: " << buffer4;
		}
		return true;
	}
#endif

	// AutoDetect to search for main and fullscreen windows
	if (data.AutoDetect || (Config.SetFullScreenLayer == 0 && Config.SetNamedLayer.size() == 0))
	{
		// Declare vars
		MONITORINFO mi = { sizeof(mi) };
		screen_res ScreenSize;

		// Get window and monitor information
		GetScreenSize(hwnd, ScreenSize, mi);

		// Store window layer information
		++data.LayerNumber;
		data.Windows[data.LayerNumber].hwnd = hwnd;
		data.Windows[data.LayerNumber].IsFullScreen = IsWindowFullScreen(WindowSize, ScreenSize);
		data.Windows[data.LayerNumber].IsMain = IsMainWindow(hwnd);

		// Check if the window is the best window
		if (data.Windows[data.LayerNumber].IsFullScreen && data.Windows[data.LayerNumber].IsMain)
		{
			// Match found returning value
			data.best_handle = hwnd;
			return false;
		}
	}
	// Manually search windows for a specific window
	else
	{
		// Check other windows for a match
		if ((Config.SetNamedLayer.size() == 0 && ++data.LayerNumber == Config.SetFullScreenLayer) ||		// Check for specific window layer
			Settings::IfStringExistsInList(class_name, Config.SetNamedLayer))								// Check for specific window class name
		{
			// Match found returning value
			data.best_handle = hwnd;
			return false;
		}
	}

	// Return to loop again
	return true;
}

// Finds the active window
HWND Fullscreen::FindMainWindow(DWORD process_id, bool AutoDetect, bool Debug)
{
	// Set variables
	HWND WindowsHandle = nullptr;
	handle_data data;
	data.best_handle = nullptr;
	data.process_id = process_id;
	data.AutoDetect = AutoDetect;
	data.LayerNumber = 0;
	data.Debug = Debug;

	// Gets all window layers and looks for a main window that is fullscreen
	EnumWindows(EnumWindowsCallback, (LPARAM)&data);
	WindowsHandle = data.best_handle;

	// If no main fullscreen window found then check for other windows
	if (!WindowsHandle)
	{
		for (DWORD x = 1; x <= data.LayerNumber; x++)
		{
			// Return the first fullscreen window layer
			if (data.Windows[x].IsFullScreen)
			{
				return data.Windows[x].hwnd;
			}
			// If no fullscreen layer then return the first 'main' window
			if (!WindowsHandle && data.Windows[x].IsMain)
			{
				WindowsHandle = data.Windows[x].hwnd;
			}
		}
	}

	// Get first window handle if no handle has been found yet
	if (!WindowsHandle && data.LayerNumber > 0)
	{
		WindowsHandle = data.Windows[1].hwnd;
	}

	// Return the best handle
	return WindowsHandle;
}

// Enums all windows and returns is there is a menu on the window
BOOL CALLBACK Fullscreen::EnumMenuWindowsCallback(HWND hwnd, LPARAM lParam)
{
	// Get  variables from callback
	menu_data& data = *(menu_data*)lParam;

	// Skip windows that are from a different process ID
	DWORD process_id;
	GetWindowThreadProcessId(hwnd, &process_id);
	if (data.process_id != process_id)
	{
		return true;
	}

	// Check if there is menu handle
	if (GetMenu(hwnd))
	{
		data.Menu = true;
		return false;
	}
	return true;
}

// Checks if there is a menu on this window
bool Fullscreen::CheckForMenu(DWORD process_id)
{
	menu_data data;
	data.process_id = process_id;
	EnumWindows(EnumMenuWindowsCallback, (LPARAM)&data);
	return data.Menu;
}

// Get child window handle
BOOL CALLBACK Fullscreen::EnumChildWindowsProc(HWND hwnd, LPARAM lParam)
{
	HWND &data = *(HWND*)lParam;
	data = hwnd;

	return true;
}

// Sends Alt+Enter to window handle
void Fullscreen::SendAltEnter(HWND& hwnd)
{
	SendMessage(hwnd, WM_SYSKEYDOWN, VK_RETURN, 0x20000000);
	SendMessage(hwnd, WM_SYSKEYUP, VK_RETURN, 0x20000000);
}

// Sets the window to fullscreen
void Fullscreen::SetFullScreen(HWND& hwnd, const MONITORINFO& mi)
{
	// Attach to window thread
	DWORD h_ThreadID = GetWindowThreadProcessId(hwnd, nullptr);
	AttachThreadInput(InterlockedCompareExchange(&m_dwThreadID, 0, 0), h_ThreadID, true);

	// Try restoring the window to normal
	PostMessage(hwnd, WM_SYSCOMMAND, SW_SHOWNORMAL, 0);

	// Window placement helps ensure the window can be seen (sometimes windows appear as minimized)
	WINDOWPLACEMENT wp;
	wp.length = sizeof(wp);
	GetWindowPlacement(hwnd, &wp);
	wp.showCmd = SW_MAXIMIZE | SW_RESTORE | SW_SHOW | SW_SHOWMAXIMIZED | SW_SHOWNORMAL;
	wp.flags = WPF_RESTORETOMAXIMIZED;
	SetWindowPlacement(hwnd, &wp);

	// Set window style
	DWORD dwStyle = GetWindowLong(hwnd, GWL_STYLE);
	SetWindowLong(hwnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);

	// Set window to fullscreen
	SetWindowPos(hwnd, HWND_TOP,
		mi.rcMonitor.left, mi.rcMonitor.top,
		mi.rcMonitor.right - mi.rcMonitor.left,
		mi.rcMonitor.bottom - mi.rcMonitor.top,
		SWP_NOOWNERZORDER | SWP_FRAMECHANGED);

	// Set window to forground
	SetForegroundWindow(hwnd);

	// Dettach from window thread
	AttachThreadInput(InterlockedCompareExchange(&m_dwThreadID, 0, 0), h_ThreadID, false);

	// Set focus and activate
	SetFocus(hwnd);
	SetActiveWindow(hwnd);
}


//*********************************************************************************
// Process Termination check function below
//*********************************************************************************

// Check if process should be termianted
void Fullscreen::CheckForTermination(DWORD m_ProcessId)
{
	static DWORD countAttempts = 0;
	static bool FoundWindow = false;
	screen_res WindowSize;
	RECT rect = { sizeof(rect) };
	bool WindowTooSmall = false;

	// Get main window hwnd
	HWND hwnd = FindMainWindow(m_ProcessId, true);

	// Get window information
	if (IsWindow(hwnd))
	{
		GetWindowSize(hwnd, WindowSize, rect);
	}

	// Check if there is a valid hwnd
	if (IsWindow(hwnd))
	{
		// Check if window is too small
		if (IsWindowTooSmall(WindowSize))
		{
			WindowTooSmall = true;
		}
		// Good window found
		else
		{
			FoundWindow = true;
		}
	}

	// Check if process is hung and needs to be terminated
	if (FoundWindow &&			// Found at least one good window in the past
		(WindowTooSmall ||		// Current window is too small
			!IsWindow(hwnd)))	// No window found
	{
		// Check for number of loops and wait time before terminating application
		if ((++countAttempts > TerminationCount) &&							// Minimum number of loops
			(countAttempts * Config.LoopSleepTime > TerminationWaitTime))	// Minimum time to wait
		{
			Logging::Log() << "Process not exiting, attempting to terminate process...";

			// Reset screen back to original Windows settings to fix some display errors on exit
			if (Config.ResetScreenRes)
			{
				Fullscreen::ResetScreen();
			}

			// Terminate the current process
			Logging::Log() << "Terminating process!";
			TerminateProcess(OpenProcess(PROCESS_ALL_ACCESS, false, GetCurrentProcessId()), 0);
		}
	}
	// Reset counter
	else
	{
		countAttempts = 0;
	}
}


//*********************************************************************************
// Thread functions below
//*********************************************************************************

// Start main thread function
DWORD WINAPI Fullscreen::StartThreadFunc(LPVOID pvParam)
{
	UNREFERENCED_PARAMETER(pvParam);

	// Starting thread
	Logging::Log() << "Starting fullscreen thread...";

	// Get thread handle
	InterlockedExchangePointer(&m_hThread, GetCurrentThread());
	if (!InterlockedCompareExchangePointer(&m_hThread, nullptr, nullptr)) {
		Logging::Log() << "Failed to get thread handle exiting thread!";
		return 0;
	}

	// Set thread flag to running
	m_ThreadRunningFlag = true;

	// Set threat priority high, trick to reduce concurrency problems
	SetThreadPriority(InterlockedCompareExchangePointer(&m_hThread, nullptr, nullptr), THREAD_PRIORITY_HIGHEST);

	// Start main fullscreen function
	MainFunc();

	// Reset thread flag before exiting
	m_ThreadRunningFlag = false;

	// Close handle
	CloseHandle(InterlockedExchangePointer(&m_hThread, nullptr));

	// Set thread ID back to 0
	InterlockedExchange(&m_dwThreadID, 0);

	// Return value
	return 0;
}

// Create fullscreen thread
void Fullscreen::StartThread()
{
	// Load dxwrapper to prevent program from unloading dll while thread is running
	if (Config.RealWrapperMode != dtype.dxwrapper)
	{
		char buffer[MAX_PATH] = { 0 };
		GetModuleFileName(hModule_dll, buffer, MAX_PATH);
		HMODULE dxwrapperhandle;
		dxwrapperhandle = LoadLibrary(buffer);
	}

	// Start thread
	CreateThread(nullptr, 0, StartThreadFunc, nullptr, 0, &m_dwThreadID);
}

// Is thread running
bool Fullscreen::IsThreadRunning()
{
	return m_ThreadRunningFlag && InterlockedCompareExchange(&m_dwThreadID, 0, 0) && GetThreadId(InterlockedCompareExchangePointer(&m_hThread, nullptr, nullptr)) == InterlockedCompareExchange(&m_dwThreadID, 0, 0);
}

// Stop thread
void Fullscreen::StopThread()
{
	// Set flag to stop thread
	m_StopThreadFlag = true;

	// Wait for thread to exit
	if (IsThreadRunning())
	{
		Logging::Log() << "Stopping Fullscreen thread...";

		// Wait for thread to exit
		WaitForSingleObject(InterlockedCompareExchangePointer(&m_hThread, nullptr, nullptr), INFINITE);

		// Thread stopped
		Logging::Log() << "Fullscreen thread stopped";
	}
}


//*********************************************************************************
// Main fullscreen function below
//*********************************************************************************

void Fullscreen::MainFunc()
{
	// Declare vars
	window_update CurrentLoop;
	window_update PreviousLoop;
	window_update LastFullscreenLoop;
	screen_res WindowSize;
	MONITORINFO mi = { sizeof(mi) };
	char class_name[80] = { 0 };
	bool ChangeDetectedFlag = false;
	bool NoChangeFromLastRunFlag = false;
	bool HasNoMenu = false;
	bool IsNotFullScreenFlag = false;

	// Get process ID
	DWORD m_ProcessId = GetCurrentProcessId();

	// Short sleep to allow other items to load
	Sleep(100);

	// Start main fullscreen loop
	while (!m_StopThreadFlag)
	{
		// Starting loop
#ifdef _DEBUG
		Logging::Log() << "Starting Main Fullscreen loop...";
#endif

		// Get window hwnd for specific layer
		CurrentLoop.hwnd = FindMainWindow(m_ProcessId, false);

		// Start Fullscreen method
		if (Config.FullScreen && IsWindow(CurrentLoop.hwnd))
		{
			// Check if window has a menu
			HasNoMenu = !CheckForMenu(m_ProcessId);

			// Get window child hwnd
			EnumChildWindows(CurrentLoop.hwnd, EnumChildWindowsProc, (LPARAM)&CurrentLoop.ChildHwnd);

			// Get window and monitor information
			GetClassName(CurrentLoop.hwnd, class_name, sizeof(class_name));
			GetWindowSize(CurrentLoop.hwnd, WindowSize, CurrentLoop.rect);
			GetScreenSize(CurrentLoop.hwnd, CurrentLoop.ScreenSize, mi);

			// Check if window is not fullscreen
			IsNotFullScreenFlag = IsWindowNotFullScreen(WindowSize, CurrentLoop.ScreenSize);

			// Check if change is detected
			ChangeDetectedFlag = (IsNotFullScreenFlag ||		// Check if it is not fullscreen
				CurrentLoop != LastFullscreenLoop);				// Check if the window or screen details have changed

			// Check if there is no change from the last run
			NoChangeFromLastRunFlag = (CurrentLoop == PreviousLoop);

#ifdef _DEBUG
			// Debug window changes
			if (ChangeDetectedFlag)
			{
				// ChangeDetected
				if (IsNotFullScreenFlag) Logging::Log() << "Found window not fullscreen";
				if (CurrentLoop.hwnd != LastFullscreenLoop.hwnd) Logging::Log() << "Found new window handle";
				if (CurrentLoop.rect != LastFullscreenLoop.rect) Logging::Log() << "Found rect does not match";
				if (CurrentLoop.ScreenSize != LastFullscreenLoop.ScreenSize) Logging::Log() << "Found screen size does not match";
				if (!HasNoMenu) Logging::Log() << "Found window has a menu";

				// LastRunChangeNotDetectedFlag
				if (CurrentLoop.hwnd != PreviousLoop.hwnd) Logging::Log() << "Found last-window handle changed";
				if (CurrentLoop.rect != PreviousLoop.rect) Logging::Log() << "Found last-window rect does not match";
				if (CurrentLoop.ScreenSize != PreviousLoop.ScreenSize) Logging::Log() << "Found last-screen size does not match";

				// Log windows coordinates
				char buffer1[7] = { 0 }, buffer2[7] = { 0 }, buffer3[7] = { 0 }, buffer4[7] = { 0 };
				_itoa_s(CurrentLoop.rect.left, buffer1, 10);
				_itoa_s(CurrentLoop.rect.top, buffer2, 10);
				_itoa_s(CurrentLoop.rect.right, buffer3, 10);
				_itoa_s(CurrentLoop.rect.bottom, buffer4, 10);
				Logging::Log() << "Window coordinates for selected layer '" << class_name << "' | Left: " << buffer1 << " Top: " << buffer2 << " Right: " << buffer3 << " Bottom: " << buffer4;

				// Log window placement flag
				WINDOWPLACEMENT wp;
				wp.length = sizeof(wp);
				GetWindowPlacement(CurrentLoop.hwnd, &wp);
				if (wp.showCmd)
				{
					char buffer[MAX_PATH] = { 0 };
					strcat_s(buffer, MAX_PATH, "Window has following flags: ");
					if (wp.showCmd & SW_HIDE) strcat_s(buffer, MAX_PATH, " SW_HIDE");
					if (wp.showCmd & SW_MAXIMIZE) strcat_s(buffer, MAX_PATH, " SW_MAXIMIZE");
					if (wp.showCmd & SW_MINIMIZE) strcat_s(buffer, MAX_PATH, " SW_MINIMIZE");
					if (wp.showCmd & SW_RESTORE) strcat_s(buffer, MAX_PATH, " SW_RESTORE");
					if (wp.showCmd & SW_SHOW) strcat_s(buffer, MAX_PATH, " SW_SHOW");
					if (wp.showCmd & SW_SHOWMAXIMIZED) strcat_s(buffer, MAX_PATH, " SW_SHOWMAXIMIZED");
					if (wp.showCmd & SW_SHOWMINIMIZED) strcat_s(buffer, MAX_PATH, " SW_SHOWMINIMIZED");
					if (wp.showCmd & SW_SHOWMINNOACTIVE) strcat_s(buffer, MAX_PATH, " SW_SHOWMINNOACTIVE");
					if (wp.showCmd & SW_SHOWNA) strcat_s(buffer, MAX_PATH, " SW_SHOWNA");
					if (wp.showCmd & SW_SHOWNOACTIVATE) strcat_s(buffer, MAX_PATH, " SW_SHOWNOACTIVATE");
					if (wp.showCmd & SW_SHOWNORMAL) strcat_s(buffer, MAX_PATH, " SW_SHOWNORMAL");
					Logging::Log() << buffer;
				}
			}
#endif

			// Change detected in screen resolution or window
			if (ChangeDetectedFlag && NoChangeFromLastRunFlag && HasNoMenu && IsWindow(CurrentLoop.hwnd))
			{
#ifdef _DEBUG
				// Debug the window
				Logging::Log() << "Entering change detected loop";
				char buffer1[7] = { 0 }, buffer2[7] = { 0 }, buffer3[7] = { 0 }, buffer4[7] = { 0 };
				_itoa_s(CurrentLoop.rect.left, buffer1, 10);
				_itoa_s(CurrentLoop.rect.top, buffer2, 10);
				_itoa_s(CurrentLoop.rect.right, buffer3, 10);
				_itoa_s(CurrentLoop.rect.bottom, buffer4, 10);
				Logging::Log() << "Window coordinates for selected layer '" << class_name << "' | Left: " << buffer1 << " Top: " << buffer2 << " Right: " << buffer3 << " Bottom: " << buffer4;
				FindMainWindow(m_ProcessId, true, true);
#endif

				// Check if window is not too small
				if (Config.ForceWindowResize || !IsWindowTooSmall(WindowSize))
				{
					// Update screen when change detected
					SetWindowPos(CurrentLoop.hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOOWNERZORDER | SWP_FRAMECHANGED | SWP_NOSIZE);

					// Change resolution if not fullscreen and ignore certian windows
					if (IsNotFullScreenFlag &&																									// Check if it is already fullscreen
						!(Config.IgnoreWindowName.size() != 0 && Settings::IfStringExistsInList(class_name, Config.IgnoreWindowName)) &&	// Ignore certian windows
						IsWindow(CurrentLoop.hwnd))																								// Check window handle
					{
						// Get the best screen resolution
						screen_res SizeTemp;
						LONG Delta = GetBestResolution(SizeTemp, WindowSize.Width, WindowSize.Height);

						// Check if the window is same size as a supported screen resolution
						if (Delta <= WindowDelta)
						{
#ifdef _DEBUG
							// Debug the screen resolution
							char buffer5[7] = { 0 }, buffer6[7] = { 0 };
							_itoa_s(WindowSize.Width, buffer5, 10);
							_itoa_s(WindowSize.Height, buffer6, 10);
							Logging::Log() << "Changing resolution on window size " << buffer5 << "x" << buffer6 << " layer: " << class_name;
							_itoa_s(CurrentLoop.ScreenSize.Width, buffer5, 10);
							_itoa_s(CurrentLoop.ScreenSize.Height, buffer6, 10);
							Logging::Log() << "Current screen size is: " << buffer5 << "x" << buffer6;
							_itoa_s(SizeTemp.Width, buffer5, 10);
							_itoa_s(SizeTemp.Height, buffer6, 10);
							Logging::Log() << "Setting resolution to: " << buffer5 << "x" << buffer6;
#endif

							// Set screen to new resolution
							SetScreenResolution(SizeTemp.Width, SizeTemp.Height);

							// Update window and monitor information
							GetWindowSize(CurrentLoop.hwnd, WindowSize, CurrentLoop.rect);
							GetScreenSize(CurrentLoop.hwnd, CurrentLoop.ScreenSize, mi);
						}
						// If non-supported resolution than add window to excluded list
						else
						{
#ifdef _DEBUG
							// Debug log
							Logging::Log() << "Excluding window layer: " << class_name;
#endif

							// Add window to excluded list
							Settings::SetValue("IgnoreWindowName", class_name, &Config.IgnoreWindowName);
						}
					} // Change resolution

					// Set fullscreen
					if (Config.ForceWindowResize ||															// Force Window Resize
						((IsWindowFullScreen(WindowSize, CurrentLoop.ScreenSize) ||							// Check if window size is the same as screen size
						abs(CurrentLoop.rect.bottom) > 30000 || abs(CurrentLoop.rect.right) > 30000) &&		// Check if window is outside coordinate range
						IsWindow(CurrentLoop.hwnd)))														// Check for valide window handle
					{
#ifdef _DEBUG
						// Debug log
						Logging::Log() << "Setting fullscreen on window layer: " << class_name;
#endif

						// Set window to fullscreen
						SetFullScreen(CurrentLoop.hwnd, mi);
					}

					// Send alt+enter
					if (Config.SendAltEnter && IsWindow(CurrentLoop.hwnd))
					{
#ifdef _DEBUG
						// Debug log
						Logging::Log() << "Pressing alt+enter on window layer: " << class_name;
#endif

						// Send Alt+Enter to window
						SendAltEnter(CurrentLoop.hwnd);
					}

#ifdef _DEBUG
					// Debug log
					FindMainWindow(m_ProcessId, true, true);
#endif

					// Update window and monitor information
					GetWindowSize(CurrentLoop.hwnd, WindowSize, CurrentLoop.rect);
					GetScreenSize(CurrentLoop.hwnd, CurrentLoop.ScreenSize, mi);

					// Save last loop information
					LastFullscreenLoop = CurrentLoop;

				} // Window is too small

			} // Change detected in screen resolution or window

		} // Start Fullscreen method

		// Store last loop information
		PreviousLoop = CurrentLoop;

		// Check if appliction needs to be terminated
		if (Config.ForceTermination)
		{
			CheckForTermination(m_ProcessId);
		}

#ifdef _DEBUG
		// Debug logs
		Logging::Log() << "Finish Main Fullscreen loop!";
#endif

		// Wait for a while
		Sleep(Config.LoopSleepTime + (ChangeDetectedFlag * Config.WaitForWindowChanges * Config.WindowSleepTime));

	} // Main while loop
}
