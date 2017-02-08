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
*
* Some functions taken from source code found in Aqrit's ddwrapper
* http://bitpatch.com/ddwrapper.html
*
* Some functions taken from source code found in DxWnd v 2.03.60
* https://sourceforge.net/projects/dxwnd/
*/

#include "dgame.h"

// Execute a specified string
void Shell(char* fileName)
{
	Compat::Log() << "Running process: " << fileName;

	// Get StartupInfo and ProcessInfo memory size and set process window to hidden
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	si.wShowWindow = SW_HIDE;
	ZeroMemory(&pi, sizeof(pi));

	// Start the child process
	if (!CreateProcess(NULL, fileName, NULL, NULL, true, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
	{
		// Failed to launch process!
		Compat::Log() << "Failed to launch process!";
	}
	else
	{
		// Wait for process to exit
		if (Config.WaitForProcess) WaitForSingleObjectEx(pi.hThread, INFINITE, true);

		// Close thread handle
		CloseHandle(pi.hThread);

		// Close process handle
		CloseHandle(pi.hProcess);
	}
	// Quit function
	return;
}

// DPI virtualization causes:
// Text Clipping, Blurring, or Inconsistent font sizes.
// "Rendering of full-screen DX applications partially off screen" - Mircosoft
// ...drawing(writting) to someplace that doesn't exist may cause crashes... 
// if your going to Disable DPI Scaling then do it as soon as possible
void DisableHighDPIScaling()
{
	Compat::Log() << "Disabling High DPI Scaling...";
	// use GetProcAddress because SetProcessDPIAware exists only on win6+
	// and "High" dpi scaling only exits on win6+?
	HMODULE hUser32 = GetModuleHandle("user32.dll");
	typedef bool(__stdcall* SetProcessDPIAwareFunc)();
	if (hUser32)
	{
		SetProcessDPIAwareFunc setDPIAware = (SetProcessDPIAwareFunc)GetProcAddress(hUser32, "SetProcessDPIAware");
		if (setDPIAware) setDPIAware();
	}
}

// Sets the thread to use only one CPU
void SetSingleCoreAffinity()
{
	HANDLE hProcess = GetCurrentProcess();
	DWORD ProcessAffinityMask;
	DWORD SystemAffinityMask;
	typedef bool(__stdcall* SetProcessAffinityMask_t)(HANDLE, DWORD);
	if (GetProcessAffinityMask(hProcess, &ProcessAffinityMask, &SystemAffinityMask))
	{
		if (ProcessAffinityMask & 1)
		{
			SetProcessAffinityMask_t spam = (SetProcessAffinityMask_t)GetProcAddress(GetModuleHandle("kernel32.dll"), "SetProcessAffinityMask");
			if (spam) spam(hProcess, 1);
			else SetThreadAffinityMask(GetCurrentThread(), 1);
		}
	}
}

// Sets the process to use only one CPU
void SetSingleProcessAffinity(bool first)
{
	int i;
	DWORD ProcessAffinityMask, SystemAffinityMask;
	if (!GetProcessAffinityMask(GetCurrentProcess(), &ProcessAffinityMask, &SystemAffinityMask))
	{
		//OutTraceE("GetProcessAffinityMask: ERROR err=%d\n", GetLastError());
	}
	//OutTraceDW("Process affinity=%x\n", ProcessAffinityMask);
	if (first) {
		for (i = 0; i<(8 * sizeof(DWORD)); i++) {
			if (ProcessAffinityMask & 0x1) break;
			ProcessAffinityMask >>= 1;
		}
		//OutTraceDW("First process affinity bit=%d\n", i);
		ProcessAffinityMask = 0x1;
		for (; i; i--) ProcessAffinityMask <<= 1;
		//OutTraceDW("Process affinity=%x\n", ProcessAffinityMask);
	}
	else {
		for (i = 0; i<(8 * sizeof(DWORD)); i++) {
			if (ProcessAffinityMask & 0x80000000) break;
			ProcessAffinityMask <<= 1;
		}
		i = 31 - i;
		//OutTraceDW("Last process affinity bit=%d\n", i);
		ProcessAffinityMask = 0x1;
		for (; i; i--) ProcessAffinityMask <<= 1;
		//OutTraceDW("Process affinity=%x\n", ProcessAffinityMask);
	}
	if (!SetProcessAffinityMask(GetCurrentProcess(), ProcessAffinityMask))
	{
		//OutTraceE("SetProcessAffinityMask: ERROR err=%d\n", GetLastError());
	}
}
