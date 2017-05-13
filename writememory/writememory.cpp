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

// Declare varables
bool p_StopThreadFlag = false;
bool p_ThreadRunningFlag = false;
HANDLE p_hThread = NULL;
DWORD p_dwThreadID = 0;

// Write specified bytes to memory
void WriteMemory(DWORD AddressPointer, byte BytesToWrite[], size_t SizeOfBytes)
{
	// Make meory writeable
	unsigned long oldProtect;
	VirtualProtect((LPVOID)AddressPointer, SizeOfBytes, PAGE_EXECUTE_READWRITE, &oldProtect);

	// Write to memory
	memcpy((void*)AddressPointer, BytesToWrite, SizeOfBytes);
}

// Writes all bytes in Config to memory
void UpdateAllByteMemory()
{
	HANDLE hProcess = GetCurrentProcess();

	for (UINT x = 1; x <= Config.AddressPointerCount && x <= Config.BytesToWriteCount; x++)
	{
		if (Config.MemoryInfo[x].AddressPointer > 0 && Config.MemoryInfo[x].SizeOfBytes > 0)
		{
			// Get current memory
			byte* lpBuffer = new byte[Config.MemoryInfo[x].SizeOfBytes];
			ReadProcessMemory(hProcess, (LPVOID)Config.MemoryInfo[x].AddressPointer, lpBuffer, Config.MemoryInfo[x].SizeOfBytes, NULL);

			// Set new memory
			WriteMemory(Config.MemoryInfo[x].AddressPointer, Config.MemoryInfo[x].Bytes, Config.MemoryInfo[x].SizeOfBytes);

			// Delete varable
			delete Config.MemoryInfo[x].Bytes;

			// Store new variable
			Config.MemoryInfo[x].Bytes = lpBuffer;
		}
	}
}

// Verify process bytes before hot patching
bool CheckVerificationMemory()
{
	HANDLE hProcess = GetCurrentProcess();

	// Check Verification details
	if (Config.VerifyMemoryInfo.SizeOfBytes > 0 && Config.VerifyMemoryInfo.AddressPointer > 0)
	{
		// Get current memory
		byte* lpBuffer = new byte[Config.VerifyMemoryInfo.SizeOfBytes];
		if (ReadProcessMemory(hProcess, (LPVOID)Config.VerifyMemoryInfo.AddressPointer, lpBuffer, Config.VerifyMemoryInfo.SizeOfBytes, NULL))
		{
			for (UINT x = 0; x < Config.VerifyMemoryInfo.SizeOfBytes; x++)
			{
				if (lpBuffer[x] != Config.VerifyMemoryInfo.Bytes[x])
				{
					// Check failed
					return false;
				}
			}

			// All checks pass return true
			return true;
		}
	}

	// Default to fail
	return false;
}

// Thread to undo hotpatch after ResetMemoryAfter time
static DWORD WINAPI HotPatchThreadFunc(LPVOID pvParam)
{
	UNREFERENCED_PARAMETER(pvParam);

	// Get thread handle
	p_hThread = GetCurrentThread();

	// Set thread flag to running
	p_ThreadRunningFlag = true;

	// Sleep for a while
	DWORD timer = 0;
	while (!p_StopThreadFlag && timer < Config.ResetMemoryAfter)
	{
		Sleep(120);
		timer += 120;
	};

	// Logging
	Compat::Log() << "Undoing hot patched memory...";

	// Undo the hot patch
	UpdateAllByteMemory();

	// Return value
	return 0;
}

// Main sub for hot patching the application
void HotPatchMemory()
{
	if (CheckVerificationMemory())
	{
		// Logging
		Compat::Log() << "Hot patching memory...";

		// Hot patching memory
		UpdateAllByteMemory();

		// Starting thread to undo hotpatch after ResetMemoryAfter time
		if (Config.ResetMemoryAfter > 0) CreateThread(NULL, 0, HotPatchThreadFunc, NULL, 0, &p_dwThreadID);
	}
	else
	{
		if (Config.VerifyMemoryInfo.AddressPointer != 0)
		{
			Compat::Log() << "Verification for hot patch failed";
		}
	}
}

// Is thread running
bool IsHotpatchThreadRunning()
{
	return p_ThreadRunningFlag && GetThreadId(p_hThread) == p_dwThreadID && p_dwThreadID != 0;
}

// Stop hotpatch thread
void StopHotpatchThread()
{
		// Set flag to stop thread
		p_StopThreadFlag = true;

		// Wait for thread to exit
		if (IsHotpatchThreadRunning()) WaitForSingleObject(p_hThread, INFINITE);

		// Close handle
		CloseHandle(p_hThread);
}