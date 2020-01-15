/**
* Copyright (C) 2020 Elisha Riedlinger
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
		// Declare variables
		bool m_StopThreadFlag = false;
		bool m_ThreadRunningFlag = false;
		HANDLE m_hThread = nullptr;
		DWORD m_dwThreadID = 0;

		// Function declarations
		bool WriteAllByteMemory();
		DWORD WINAPI StartThreadFunc(LPVOID);
		bool IsThreadRunning();
	}
}

using namespace Utils;

// Checks the value of two data segments
bool WriteMemory::CheckMemoryAddress(void *dataAddr, void *dataBytes, size_t dataSize)
{
	if (!dataAddr || !dataBytes || !dataSize)
	{
		Logging::LogDebug() << __FUNCTION__ << " Error: invalid memory data";
		return false;
	}

	// VirtualProtect first to make sure patch_address is readable
	DWORD dwPrevProtect;
	if (!VirtualProtect(dataAddr, dataSize, PAGE_READONLY, &dwPrevProtect))
	{
		Logging::LogDebug() << __FUNCTION__ << " Error: could not read memory address";
		return false;
	}

	bool flag = (memcmp(dataAddr, dataBytes, dataSize) == 0);

	// Restore protection
	VirtualProtect(dataAddr, dataSize, dwPrevProtect, &dwPrevProtect);

	// Return results
	return flag;
}

// Update memory
bool WriteMemory::UpdateMemoryAddress(void *dataAddr, void *dataBytes, size_t dataSize)
{
	if (!dataAddr || !dataBytes || !dataSize)
	{
		Logging::Log() << __FUNCTION__ << " Error: invalid memory data";
		return false;
	}

	// VirtualProtect first to make sure patch_address is readable
	DWORD dwPrevProtect;
	if (!VirtualProtect(dataAddr, dataSize, PAGE_READWRITE, &dwPrevProtect))
	{
		Logging::Log() << __FUNCTION__ << " Error: could not write to memory address";
		return false;
	}

	// Update memory
	memcpy(dataAddr, dataBytes, dataSize);

	// Restore protection
	VirtualProtect(dataAddr, dataSize, dwPrevProtect, &dwPrevProtect);

	// Flush cache
	FlushInstructionCache(GetCurrentProcess(), dataAddr, dataSize);

	// Return
	return true;
}

// Writes all bytes in Config to memory
bool WriteMemory::WriteAllByteMemory()
{
	for (UINT x = 0; x < Config.MemoryInfo.size(); x++)
	{
		std::vector<byte> tmpArray;
		if (Config.MemoryInfo[x].AddressPointer && Config.MemoryInfo[x].Bytes.size())
		{
			// Backup memory
			if (Config.ResetMemoryAfter > 0)
			{
				tmpArray.resize(Config.MemoryInfo[x].Bytes.size(), '\0');
				if (!UpdateMemoryAddress(&tmpArray[0], Config.MemoryInfo[x].AddressPointer, Config.MemoryInfo[x].Bytes.size()))
				{
					Logging::Log() << __FUNCTION__ << " Failed to backup memory...";
					return false;
				}
			}

			// Write bytes to memory
			if (!UpdateMemoryAddress(Config.MemoryInfo[x].AddressPointer, &Config.MemoryInfo[x].Bytes[0], Config.MemoryInfo[x].Bytes.size()))
			{
				Logging::Log() << __FUNCTION__ << " Failed to write bytes to memory...";
				return false;
			}

			// Prepare new memory to be written
			if (Config.ResetMemoryAfter > 0)
			{
				if (!UpdateMemoryAddress(&Config.MemoryInfo[x].Bytes[0], &tmpArray[0], Config.MemoryInfo[x].Bytes.size()))
				{
					Logging::Log() << __FUNCTION__ << " Failed to prepare new memory bytes...";
					return false;
				}
			}
		}
	}
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
	Logging::Log() << __FUNCTION__ << " Undoing memory write...";

	// Undo the memory write
	if (!WriteAllByteMemory())
	{
		// Logging
		Logging::Log() << __FUNCTION__ << " Failed to undo memory write!";
	}

	// Reset thread flag before exiting
	m_ThreadRunningFlag = false;

	// Close handle
	CloseHandle(InterlockedExchangePointer(&m_hThread, nullptr));

	// Set thread ID back to 0
	InterlockedExchange(&m_dwThreadID, 0);

	// Return value
	return 0;
}

// Main sub for writing to the memory of the application
void WriteMemory::WriteMemory()
{
	if (CheckMemoryAddress(Config.VerifyMemoryInfo.AddressPointer, &Config.VerifyMemoryInfo.Bytes[0], Config.VerifyMemoryInfo.Bytes.size()))
	{
		// Logging
		Logging::Log() << __FUNCTION__ << " Writing bytes to memory...";

		// Write bytes to memory
		if (!WriteAllByteMemory())
		{
			Logging::Log() << __FUNCTION__ << " Failed to write bytes to memory...";
			return;
		}

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
			Logging::Log() << __FUNCTION__ << " Failed verification for memory write!";
		}
	}
}

// Is thread running
bool WriteMemory::IsThreadRunning()
{
	return m_ThreadRunningFlag && InterlockedCompareExchange(&m_dwThreadID, 0, 0) && GetThreadId(InterlockedCompareExchangePointer(&m_hThread, nullptr, nullptr)) == InterlockedCompareExchange(&m_dwThreadID, 0, 0);
}

// Stop thread
void WriteMemory::StopThread()
{
	// Set flag to stop thread
	m_StopThreadFlag = true;

	// Wait for thread to exit
	if (IsThreadRunning())
	{
		Logging::Log() << __FUNCTION__ << " Stopping thread...";

		// Wait for thread to exit
		WaitForSingleObject(InterlockedCompareExchangePointer(&m_hThread, nullptr, nullptr), INFINITE);

		// Thread stopped
		Logging::Log() << __FUNCTION__ << " thread stopped";
	}
}
