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
	namespace WriteMemory
	{
		// Declare varables
		bool m_StopThreadFlag = false;
		bool m_ThreadRunningFlag = false;
		HANDLE m_hThread = nullptr;
		DWORD m_dwThreadID = 0;

		// Function declarations
		void WriteAllByteMemory();
		bool CheckVerificationMemory();
		DWORD WINAPI StartThreadFunc(LPVOID);
		bool IsThreadRunning();
	}
}

using namespace Utils;

// Writes all bytes in Config to memory
void WriteMemory::WriteAllByteMemory()
{
	HANDLE hProcess = GetCurrentProcess();

	for (UINT x = 0; x < Config.MemoryInfo.size(); x++)
	{
		if (Config.MemoryInfo[x].AddressPointer != nullptr && Config.MemoryInfo[x].Bytes.size() != 0)
		{
			// Get current memory
			std::string lpBuffer(Config.MemoryInfo[x].Bytes.size(), '\0');
			void* AddressPointer = Config.MemoryInfo[x].AddressPointer;
			if (ReadProcessMemory(hProcess, AddressPointer, (byte*)&lpBuffer[0], Config.MemoryInfo[x].Bytes.size(), nullptr))
			{
				// Make memory writeable
				DWORD oldProtect;
				if (VirtualProtect(AddressPointer, Config.MemoryInfo[x].Bytes.size(), PAGE_EXECUTE_READWRITE, &oldProtect))
				{
					// Write to memory
					memcpy(AddressPointer, (byte*)&Config.MemoryInfo[x].Bytes[0], Config.MemoryInfo[x].Bytes.size());

					// Restore protection
					VirtualProtect(AddressPointer, Config.MemoryInfo[x].Bytes.size(), oldProtect, &oldProtect);

					// Store new variable
					memcpy((byte*)&Config.MemoryInfo[x].Bytes[0], (byte*)&lpBuffer[0], Config.MemoryInfo[x].Bytes.size());
				}
				else
				{
					Logging::Log() << "Access Denied at memory address: 0x" << Config.MemoryInfo[x].AddressPointer;
				}
			}
			else
			{
				Logging::Log() << "Failed to read memory at address: 0x" << Config.MemoryInfo[x].AddressPointer;
			}
		}
	}
}

// Verify process bytes before writing memory
bool WriteMemory::CheckVerificationMemory()
{
	// Check Verification details
	if (Config.VerifyMemoryInfo.Bytes.size() == 0 || Config.VerifyMemoryInfo.AddressPointer == nullptr)
	{
		return false;
	}

	// Check current memory
	for (UINT x = 0; x < Config.VerifyMemoryInfo.Bytes.size(); x++)
	{
		if (*((byte*)((DWORD)Config.VerifyMemoryInfo.AddressPointer + x)) != (byte)Config.VerifyMemoryInfo.Bytes[x])
		{
			// Check failed
			return false;
		}
	}

	// All checks pass return true
	return true;
}

// Thread to undo memory write after ResetMemoryAfter time
DWORD WINAPI WriteMemory::StartThreadFunc(LPVOID pvParam)
{
	UNREFERENCED_PARAMETER(pvParam);

	// Get thread handle
	InterlockedExchangePointer(&m_hThread, GetCurrentThread());

	// Set thread flag to running
	m_ThreadRunningFlag = true;

	// Sleep for a while
	DWORD timer = 0;
	while (!m_StopThreadFlag && timer < Config.ResetMemoryAfter)
	{
		Sleep(120);
		timer += 120;
	};

	// Logging
	Logging::Log() << "Undoing memory write...";

	// Undo the memory write
	WriteAllByteMemory();

	// Reset thread flag before exiting
	m_ThreadRunningFlag = false;

	// Close handle
	CloseHandle(m_hThread);
	InterlockedExchangePointer(&m_hThread, nullptr);

	// Set thread ID back to 0
	InterlockedExchange(&m_dwThreadID, 0);

	// Return value
	return 0;
}

// Main sub for writing to the memory of the application
void WriteMemory::WriteMemory()
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
			CreateThread(nullptr, 0, StartThreadFunc, nullptr, 0, &m_dwThreadID);
		}
	}
	else
	{
		if (Config.VerifyMemoryInfo.AddressPointer != nullptr)
		{
			Logging::Log() << "Failed verification for memory write!";
		}
	}
}

// Is thread running
bool WriteMemory::IsThreadRunning()
{
	return m_ThreadRunningFlag && m_dwThreadID && GetThreadId(m_hThread) == m_dwThreadID;
}

// Stop thread
void WriteMemory::StopThread()
{
	// Set flag to stop thread
	m_StopThreadFlag = true;

	// Wait for thread to exit
	if (IsThreadRunning())
	{
		Logging::Log() << "Stopping WriteMemory thread...";

		// Wait for thread to exit
		WaitForSingleObject(m_hThread, INFINITE);

		// Thread stopped
		Logging::Log() << "WriteMemory thread stopped";
	}
}
