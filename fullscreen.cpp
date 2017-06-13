/**
* Copyright (C) 2017 Elisha Riedlinger
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

#include "cfg.h"
#include "dllmain.h"
#include "fullscreen.h"

// Declare constants
static constexpr LONG MinWindowWidth = 320;			// Minimum window width for valid window check
static constexpr LONG MinWindowHeight = 240;		// Minimum window height for valid window check
static constexpr LONG WindowDelta = 40;				// Delta between window size and screensize for fullscreen check
static constexpr LONG TerminationCount = 10;		// Minimum number of loops to check for termination
static constexpr LONG TerminationWaitTime = 2000;	// Minimum time to wait for termination (LoopSleepTime * NumberOfLoops)

// Declare varables
bool StopThreadFlag = false;
bool ThreadRunningFlag = false;
HANDLE m_hThread = nullptr;
DWORD m_dwThreadID = 0;

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
struct window_update
{
	HWND hwnd = nullptr;
	RECT rect = { sizeof(rect) };
	screen_res ScreenSize;

	window_update& operator=(const window_update& a)
	{
		hwnd = a.hwnd;
		rect.bottom = a.rect.bottom;
		rect.left = a.rect.left;
		rect.right = a.rect.right;
		rect.top = a.rect.top;
		ScreenSize = a.ScreenSize;
		return *this;
	}

	bool operator==(const window_update& a) const
	{
		return (hwnd == a.hwnd && rect == a.rect && ScreenSize == a.ScreenSize);
	}

	bool operator!=(const window_update& a) const
	{
		return (hwnd != a.hwnd || rect != a.rect || ScreenSize != a.ScreenSize);
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
	uint8_t LayerNumber = 0;
	window_layer Windows[256];
	bool AutoDetect = true;
	bool Debug = false;
};

struct menu_data
{
	DWORD process_id = 0;
	bool Menu = false;
};

// Function declarations
bool IsWindowTooSmall(screen_res);
HWND FindMainWindow(DWORD, bool, bool = false);
void MainFullScreenFunc();

// Exception handler
int filterException(int code, PEXCEPTION_POINTERS ex)
{
	Compat::Log() << "Exception caught code:" << code << " details:" << ex->ExceptionRecord->ExceptionInformation;
	return EXCEPTION_EXECUTE_HANDLER;
}

//*********************************************************************************
// Screen/monitor functions below
//*********************************************************************************

// Gets the screen size from a wnd handle
void GetScreenSize(HWND& hwnd, screen_res& Res, MONITORINFO& mi)
{
	__try
	{
		GetMonitorInfo(MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY), &mi);
		Res.Width = mi.rcMonitor.right - mi.rcMonitor.left;
		Res.Height = mi.rcMonitor.bottom - mi.rcMonitor.top;
	}
	__except (filterException(GetExceptionCode(), GetExceptionInformation()))
	{
		// Set hwnd back to NULL
		hwnd = nullptr;
	}
}

// Check with resolution is best
LONG GetBestResolution(screen_res& ScreenRes, LONG xWidth, LONG xHeight)
{
	//Set vars
	DEVMODE dm = { 0 };
	dm.dmSize = sizeof(dm);
	LONG diff = 40000;
	LONG NewDiff = 0;
	ScreenRes.Width = 0;
	ScreenRes.Height = 0;

	// Get closest resolution
	for (int iModeNum = 0; EnumDisplaySettings(nullptr, iModeNum, &dm) != 0; iModeNum++)
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
void SetScreenResolution(LONG xWidth, LONG xHeight)
{
	DEVMODE newSettings;
	if (EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &newSettings) != 0)
	{
		newSettings.dmPelsWidth = xWidth;
		newSettings.dmPelsHeight = xHeight;
		newSettings.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;
		ChangeDisplaySettings(&newSettings, CDS_FULLSCREEN);
	}
}

// Verifies input and sets screen res to the values sent
void SetScreen(screen_res ScreenRes)
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

// Checks is the current screen res is smaller and returns the smaller screen res
void CheckCurrentScreenRes(screen_res& m_ScreenRes)
{
	// Declare vars
	HWND hwnd;
	MONITORINFO mi = { sizeof(mi) };
	screen_res ScreenSize;

	// Get window handle
	hwnd = FindMainWindow(GetCurrentProcessId(), true);

	// Get monitor size
	GetScreenSize(hwnd, ScreenSize, mi);

	// Check if screen resolution is too small
	if (!IsWindowTooSmall(ScreenSize))
	{
		// Save screen resolution
		m_ScreenRes.Width = ScreenSize.Width;
		m_ScreenRes.Height = ScreenSize.Height;
	}
}

// Resets the screen to the registry-stored values
void ResetScreen()
{
	// Reset resolution
	ChangeDisplaySettings(nullptr, 0);
}

//*********************************************************************************
// Window functions below
//*********************************************************************************

bool IsWindowTooSmall(screen_res WindowSize)
{
	return WindowSize.Width < MinWindowWidth || WindowSize.Height < MinWindowHeight;
}

bool IsWindowFullScreen(screen_res WindowSize, screen_res ScreenSize)
{
	return abs(ScreenSize.Width - WindowSize.Width) <= WindowDelta ||		// Window width matches screen width
		abs(ScreenSize.Height - WindowSize.Height) <= WindowDelta;			// Window height matches screen height
}

bool IsWindowNotFullScreen(screen_res WindowSize, screen_res ScreenSize)
{
	return (ScreenSize.Width - WindowSize.Width) > WindowDelta ||			// Window width does not match screen width
		(ScreenSize.Height - WindowSize.Height) > WindowDelta;				// Window height does not match screen height
}

// Checks if the handle is the main window handle
bool IsMainWindow(HWND hwnd)
{
	bool flag = false;
	__try
	{
		flag = GetWindow(hwnd, GW_OWNER) == (HWND)0 && IsWindowVisible(hwnd);
	}
	__except (filterException(GetExceptionCode(), GetExceptionInformation()))
	{
		// Do nothing
	}
	return flag;
}

// Checks if the window has any menu items
bool IsWindowMenu(HWND hwnd)
{
	bool flag = false;
	__try
	{
		if (GetMenu(hwnd)) flag = true;
	}
	__except (filterException(GetExceptionCode(), GetExceptionInformation()))
	{
		// Do nothing
	}
	return flag;
}

// Gets the class name to the window from a handle
void GetMyClassName(HWND hwnd, char* class_name, int size)
{
	__try
	{
		GetClassName(hwnd, class_name, size);
	}
	__except (filterException(GetExceptionCode(), GetExceptionInformation()))
	{
		// Set hwnd back to NULL
		hwnd = nullptr;
	}
}

// Gets the window size from a handle
void GetWindowSize(HWND& hwnd, screen_res& Res, RECT& rect)
{
	__try
	{
		GetWindowRect(hwnd, &rect);
		Res.Width = abs(rect.right - rect.left);
		Res.Height = abs(rect.bottom - rect.top);
	}
	__except (filterException(GetExceptionCode(), GetExceptionInformation()))
	{
		// Handle error
		hwnd = nullptr;
	}
}

// Gets the process ID of a window from a window handle
DWORD GetMyWindowThreadProcessId(HWND hwnd)
{
	DWORD process_id = 0;
	__try
	{
		GetWindowThreadProcessId(hwnd, &process_id);
	}
	__except (filterException(GetExceptionCode(), GetExceptionInformation()))
	{
		// Do nothing
	}
	return process_id;
}

// Enums all windows and returns the handle to the active window
BOOL CALLBACK EnumWindowsCallback(HWND hwnd, LPARAM lParam)
{
	// Get varables from call back
	handle_data& data = *(handle_data*)lParam;

	// Skip windows that are from a different process ID
	DWORD process_id = GetMyWindowThreadProcessId(hwnd);
	if (data.process_id != process_id)
	{
		return true;
	}

	// Skip compatibility class windows
	char class_name[80] = "";
	GetMyClassName(hwnd, class_name, sizeof(class_name));
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
		char buffer[7];
		_itoa_s(data.LayerNumber, buffer, 10);
		char* isMain = "";
		if (IsMainWindow(hwnd)) isMain = "*";
		if (WindowSize.Height != 0 && WindowSize.Width != 0)		// Filter out screens that are zero size
		{
			char buffer1[7], buffer2[7], buffer3[7], buffer4[7];
			_itoa_s(rect.left, buffer1, 10);
			_itoa_s(rect.top, buffer2, 10);
			_itoa_s(rect.right, buffer3, 10);
			_itoa_s(rect.bottom, buffer4, 10);
			Compat::Log() << "Layer " << buffer << " found window class " << isMain << class_name << " | Left: " << buffer1 << " Top: " << buffer2 << " Right: " << buffer3 << " Bottom: " << buffer4;
		}
		return true;
	}
#endif

	// AutoDetect to search for main and fullscreen windows
	if (data.AutoDetect || (Config.SetFullScreenLayer == 0 && Config.NamedLayerCount == 0))
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
		if ((Config.NamedLayerCount == 0 && ++data.LayerNumber == Config.SetFullScreenLayer) ||			// Check for specific window layer
			IfStringExistsInList(class_name, Config.szSetNamedLayer, Config.NamedLayerCount))			// Check for specific window class name
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
HWND FindMainWindow(DWORD process_id, bool AutoDetect, bool Debug)
{
	// Set varables
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
		for (UINT x = 1; x <= data.LayerNumber; x++)
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
BOOL CALLBACK EnumMenuWindowsCallback(HWND hwnd, LPARAM lParam)
{
	// Get  varables from callback
	menu_data& data = *(menu_data*)lParam;

	// Skip windows that are from a different process ID
	DWORD process_id = GetMyWindowThreadProcessId(hwnd);
	if (data.process_id != process_id)
	{
		return true;
	}

	// Check if there is menu handle
	if (IsWindowMenu(hwnd))
	{
		data.Menu = true;
		return false;
	}
	return true;
}

// Checks if there is a menu on this window
bool CheckForMenu(DWORD process_id)
{
	menu_data data;
	data.process_id = process_id;
	EnumWindows(EnumMenuWindowsCallback, (LPARAM)&data);
	return data.Menu;
}

// Sends Alt+Enter to window handle
void SendAltEnter(HWND& hwnd)
{
	__try
	{
		SendMessage(hwnd, WM_SYSKEYDOWN, VK_RETURN, 0x20000000);
		SendMessage(hwnd, WM_SYSKEYUP, VK_RETURN, 0x20000000);
	}
	__except (filterException(GetExceptionCode(), GetExceptionInformation()))
	{
		// Set hwnd back to NULL
		hwnd = nullptr;
	}
}

// Sets the window to fullscreen
void SetFullScreen(HWND& hwnd, const MONITORINFO& mi)
{
	__try
	{
		// Attach to window thread
		DWORD h_ThreadID = GetWindowThreadProcessId(hwnd, nullptr);
		AttachThreadInput(m_dwThreadID, h_ThreadID, true);

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
		AttachThreadInput(m_dwThreadID, h_ThreadID, false);

		// Set focus and activate
		SetFocus(hwnd);
		SetActiveWindow(hwnd);
	}
	__except (filterException(GetExceptionCode(), GetExceptionInformation()))
	{
		// Set hwnd back to NULL
		hwnd = nullptr;
	}
}

//*********************************************************************************
// Process Termination check function below
//*********************************************************************************

// Check if process should be termianted
void CheckForTermination(DWORD m_ProcessId)
{
	static int countAttempts = 0;
	static bool FoundWindow = false;
	screen_res WindowSize;
	RECT rect = { sizeof(rect) };
	bool WindowTooSmall = false;

	// Get main window hwnd
	HWND hwnd = FindMainWindow(m_ProcessId, true);

	// Get window information
	if (IsWindow(hwnd)) GetWindowSize(hwnd, WindowSize, rect);

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
		if ((++countAttempts > TerminationCount) &&						// Minimum number of loops
			(countAttempts * Config.LoopSleepTime > TerminationWaitTime))		// Minimum time to wait
		{
			RunExitFunctions(true);
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
DWORD WINAPI StartThreadFunc(LPVOID pvParam)
{
	UNREFERENCED_PARAMETER(pvParam);

	// Starting thread
	Compat::Log() << "Starting fullscreen thread...";

	// Get thread handle
	m_hThread = GetCurrentThread();
	if (!m_hThread) {
		Compat::Log() << "Failed to get thread handle exiting thread!";
		return 0;
	}

	// Set thread flag to running
	ThreadRunningFlag = true;

	// Set threat priority high, trick to reduce concurrency problems
	SetThreadPriority(m_hThread, THREAD_PRIORITY_HIGHEST);

	// Start main fullscreen function
	MainFullScreenFunc();

	// Set thread ID back to 0
	m_dwThreadID = 0;

	// Return value
	return 0;
}

// Create fullscreen thread
void StartFullscreenThread()
{
	// Get dxwrapper path
	char buffer[MAX_PATH];
	GetModuleFileName(hModule_dll, buffer, MAX_PATH);

	// Load dxwrapper to prevent program from unloading dll while thread is running
	HMODULE dxwrapperhandle;
	dxwrapperhandle = LoadLibrary(buffer);

	// Start thread
	CreateThread(nullptr, 0, StartThreadFunc, nullptr, 0, &m_dwThreadID);
}

// Is thread running
bool IsFullscreenThreadRunning()
{
	return ThreadRunningFlag && GetThreadId(m_hThread) == m_dwThreadID && m_dwThreadID != 0;
}

// Stop thread
void StopFullscreenThread()
{
	if (Config.FullScreen || Config.ForceTermination)
	{
		if (IsFullscreenThreadRunning())
		{
			Compat::Log() << "Stopping thread...";

			// Set flag to stop thread
			StopThreadFlag = true;

			// Wait for thread to exit
			WaitForSingleObject(m_hThread, INFINITE);
		}

		// Close handle
		CloseHandle(m_hThread);

		// Thread stopped
		Compat::Log() << "Thread stopped";
	}
}

//*********************************************************************************
// Main fullscreen function below
//*********************************************************************************

void MainFullScreenFunc()
{
	// Declare vars
	window_update CurrentLoop;
	window_update PreviousLoop;
	window_update LastFullscreenLoop;
	screen_res WindowSize;
	MONITORINFO mi = { sizeof(mi) };
	char class_name[80] = "";
	bool ChangeDetectedFlag = false;
	bool NoChangeFromLastRunFlag = false;
	bool HasNoMenu = false;
	bool IsNotFullScreenFlag = false;

	// Get process ID
	DWORD m_ProcessId = GetCurrentProcessId();

	// Short sleep to allow other items to load
	Sleep(100);

	// Start main fullscreen loop
	while (!StopThreadFlag)
	{
		// Starting loop
#ifdef _DEBUG
		Compat::Log() << "Starting Main Fullscreen loop...";
#endif

		// Get window hwnd for specific layer
		CurrentLoop.hwnd = FindMainWindow(m_ProcessId, false);

		// Start Fullscreen method
		if (Config.FullScreen && IsWindow(CurrentLoop.hwnd))
		{
			// Check if window has a menu
			HasNoMenu = !CheckForMenu(m_ProcessId);

			// Get window and monitor information
			GetMyClassName(CurrentLoop.hwnd, class_name, sizeof(class_name));
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
				if (IsNotFullScreenFlag) Compat::Log() << "Found window not fullscreen";
				if (CurrentLoop.hwnd != LastFullscreenLoop.hwnd) Compat::Log() << "Found new window handle";
				if (CurrentLoop.rect != LastFullscreenLoop.rect) Compat::Log() << "Found rect does not match";
				if (CurrentLoop.ScreenSize != LastFullscreenLoop.ScreenSize) Compat::Log() << "Found screen size does not match";
				if (!HasNoMenu) Compat::Log() << "Found window has a menu";

				// LastRunChangeNotDetectedFlag
				if (CurrentLoop.hwnd != PreviousLoop.hwnd) Compat::Log() << "Found last-window handle changed";
				if (CurrentLoop.rect != PreviousLoop.rect) Compat::Log() << "Found last-window rect does not match";
				if (CurrentLoop.ScreenSize != PreviousLoop.ScreenSize) Compat::Log() << "Found last-screen size does not match";

				// Log windows coordinates
				char buffer1[7], buffer2[7], buffer3[7], buffer4[7];
				_itoa_s(CurrentLoop.rect.left, buffer1, 10);
				_itoa_s(CurrentLoop.rect.top, buffer2, 10);
				_itoa_s(CurrentLoop.rect.right, buffer3, 10);
				_itoa_s(CurrentLoop.rect.bottom, buffer4, 10);
				Compat::Log() << "Window coordinates for selected layer '" << class_name << "' | Left: " << buffer1 << " Top: " << buffer2 << " Right: " << buffer3 << " Bottom: " << buffer4;

				// Log window placement flag
				WINDOWPLACEMENT wp;
				wp.length = sizeof(wp);
				GetWindowPlacement(CurrentLoop.hwnd, &wp);
				if (wp.showCmd)
				{
					char buffer[MAX_PATH] = "";
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
					Compat::Log() << buffer;
				}
			}
#endif

			// Change detected in screen resolution or window
			if (ChangeDetectedFlag && NoChangeFromLastRunFlag && HasNoMenu && IsWindow(CurrentLoop.hwnd))
			{
#ifdef _DEBUG
				// Debug the window
				Compat::Log() << "Entering change detected loop";
				char buffer1[7], buffer2[7], buffer3[7], buffer4[7];
				_itoa_s(CurrentLoop.rect.left, buffer1, 10);
				_itoa_s(CurrentLoop.rect.top, buffer2, 10);
				_itoa_s(CurrentLoop.rect.right, buffer3, 10);
				_itoa_s(CurrentLoop.rect.bottom, buffer4, 10);
				Compat::Log() << "Window coordinates for selected layer '" << class_name << "' | Left: " << buffer1 << " Top: " << buffer2 << " Right: " << buffer3 << " Bottom: " << buffer4;
				FindMainWindow(m_ProcessId, true, true);
#endif

				// Check if window is not too small
				if (Config.ForceWindowResize || !IsWindowTooSmall(WindowSize))
				{
					// Change resolution if not fullscreen and ignore certian windows
					if (IsNotFullScreenFlag &&																										// Check if it is already fullscreen
						!(Config.IgnoreWindowCount > 0 && IfStringExistsInList(class_name, Config.szIgnoreWindowName, Config.IgnoreWindowCount)) && // Ignore certian windows
						IsWindow(CurrentLoop.hwnd))																									// Check window handle
					{
						// Get the best screen resolution
						screen_res SizeTemp;
						LONG Delta = GetBestResolution(SizeTemp, WindowSize.Width, WindowSize.Height);

						// Check if the window is same size as a supported screen resolution
						if (Delta <= WindowDelta)
						{
#ifdef _DEBUG
							// Debug the screen resolution
							char buffer5[7], buffer6[7];
							_itoa_s(WindowSize.Width, buffer5, 10);
							_itoa_s(WindowSize.Height, buffer6, 10);
							Compat::Log() << "Changing resolution on window size " << buffer5 << "x" << buffer6 << " layer: " << class_name;
							_itoa_s(CurrentLoop.ScreenSize.Width, buffer5, 10);
							_itoa_s(CurrentLoop.ScreenSize.Height, buffer6, 10);
							Compat::Log() << "Current screen size is: " << buffer5 << "x" << buffer6;
							_itoa_s(SizeTemp.Width, buffer5, 10);
							_itoa_s(SizeTemp.Height, buffer6, 10);
							Compat::Log() << "Setting resolution to: " << buffer5 << "x" << buffer6;
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
							Compat::Log() << "Excluding window layer: " << class_name;
#endif

							// Add window to excluded list
							SetConfigList(Config.szIgnoreWindowName, Config.IgnoreWindowCount, class_name);
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
						Compat::Log() << "Setting fullscreen on window layer: " << class_name;
#endif

						// Set window to fullscreen
						SetFullScreen(CurrentLoop.hwnd, mi);
					}

					// Send alt+enter
					if (Config.SendAltEnter && IsWindow(CurrentLoop.hwnd))
					{
#ifdef _DEBUG
						// Debug log
						Compat::Log() << "Pressing alt+enter on window layer: " << class_name;
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

					// Update and store CurrentScreenRes
					if (Config.ResetScreenRes) CallCheckCurrentScreenRes();

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
		Compat::Log() << "Finish Main Fullscreen loop!";
#endif

		// Wait for a while
		Sleep(Config.LoopSleepTime + (ChangeDetectedFlag * Config.WaitForWindowChanges * Config.WindowSleepTime));

	} // Main while loop
}