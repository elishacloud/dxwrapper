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

#include "Settings\Settings.h"
#include "Utils.h"
#include "Logging\Logging.h"

namespace Utils
{
	// Declare varables
	bool p_StopThreadFlag = false;
	bool p_ThreadRunningFlag = false;
	HANDLE p_hThread = nullptr;
	DWORD p_dwThreadID = 0;

	// Function declarations
	void WriteAllByteMemory();
	bool CheckVerificationMemory();
	DWORD WINAPI WriteMemoryThreadFunc(LPVOID);
	bool IsWriteMemoryThreadRunning();
}

// Writes all bytes in Config to memory
void Utils::WriteAllByteMemory()
{
	HANDLE hProcess = GetCurrentProcess();

	for (UINT x = 0; x < Config.AddressPointerCount && x < Config.BytesToWriteCount; x++)
	{
		if (Config.MemoryInfo[x].AddressPointer != 0 && Config.MemoryInfo[x].SizeOfBytes != 0)
		{
			// Get current memory
			byte* lpBuffer = new byte[Config.MemoryInfo[x].SizeOfBytes];
			void* AddressPointer = (void*)Config.MemoryInfo[x].AddressPointer;
			if (ReadProcessMemory(hProcess, AddressPointer, lpBuffer, Config.MemoryInfo[x].SizeOfBytes, nullptr))
			{
				// Make memory writeable
				DWORD oldProtect;
				if (VirtualProtect(AddressPointer, Config.MemoryInfo[x].SizeOfBytes, PAGE_EXECUTE_READWRITE, &oldProtect))
				{
					// Write to memory
					memcpy(AddressPointer, Config.MemoryInfo[x].Bytes, Config.MemoryInfo[x].SizeOfBytes);

					// Restore protection
					VirtualProtect(AddressPointer, Config.MemoryInfo[x].SizeOfBytes, oldProtect, &oldProtect);

					// Delete varable
					delete Config.MemoryInfo[x].Bytes;

					// Store new variable
					Config.MemoryInfo[x].Bytes = lpBuffer;
				}
				else
				{
					Logging::Log() << "Access Denied at memory address: 0x" << std::showbase << std::hex << Config.MemoryInfo[x].AddressPointer << std::dec << std::noshowbase;
				}
			}
			else
			{
				Logging::Log() << "Failed to read memory at address: 0x" << std::showbase << std::hex << Config.MemoryInfo[x].AddressPointer << std::dec << std::noshowbase;
			}
		}
	}
}

// Verify process bytes before writing memory
bool Utils::CheckVerificationMemory()
{
	HANDLE hProcess = GetCurrentProcess();

	// Check Verification details
	if (Config.VerifyMemoryInfo.SizeOfBytes > 0 && Config.VerifyMemoryInfo.AddressPointer > 0)
	{
		// Get current memory
		byte* lpBuffer = new byte[Config.VerifyMemoryInfo.SizeOfBytes];
		if (ReadProcessMemory(hProcess, (LPVOID)Config.VerifyMemoryInfo.AddressPointer, lpBuffer, Config.VerifyMemoryInfo.SizeOfBytes, nullptr))
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

// Thread to undo memory write after ResetMemoryAfter time
DWORD WINAPI Utils::WriteMemoryThreadFunc(LPVOID pvParam)
{
	UNREFERENCED_PARAMETER(pvParam);

	// Get thread handle
	InterlockedExchangePointer(&p_hThread, GetCurrentThread());

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
	Logging::Log() << "Undoing memory write...";

	// Undo the memory write
	WriteAllByteMemory();

	// Reset thread flag before exiting
	p_ThreadRunningFlag = false;

	// Set thread ID back to 0
	InterlockedExchange(&p_dwThreadID, 0);

	// Return value
	return 0;
}

// Main sub for writing to the memory of the application
void Utils::WriteMemory()
{
	if (CheckVerificationMemory())
	{
		// Logging
		Logging::Log() << "Writing bytes to memory...";

		// Writing bytes to memory
		WriteAllByteMemory();

		// Starting thread to undo memory write after ResetMemoryAfter time
		if (Config.ResetMemoryAfter > 0)
		{
			CreateThread(nullptr, 0, WriteMemoryThreadFunc, nullptr, 0, &p_dwThreadID);
		}
	}
	else
	{
		if (Config.VerifyMemoryInfo.AddressPointer != 0)
		{
			Logging::Log() << "Verification for memory write failed";
		}
	}
}

// Is thread running
bool Utils::IsWriteMemoryThreadRunning()
{
	return p_ThreadRunningFlag && p_dwThreadID != 0 && GetThreadId(p_hThread) == p_dwThreadID;
}

// Stop WriteMemory thread
void Utils::StopWriteMemoryThread()
{
	// Set flag to stop thread
	p_StopThreadFlag = true;

	// Wait for thread to exit
	if (IsWriteMemoryThreadRunning())
	{
		WaitForSingleObject(p_hThread, INFINITE);
	}

	// Close handle
	CloseHandle(p_hThread);
}
