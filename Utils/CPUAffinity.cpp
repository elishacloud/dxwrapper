/**
* Copyright (C) 2025 Elisha Riedlinger
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

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <winternl.h>
#include <tlhelp32.h>
#include <map>
#include "Utils.h"
#include "Settings\Settings.h"
#include "Logging\Logging.h"

namespace Utils
{
	constexpr THREADINFOCLASS ThreadBasicInformation = (THREADINFOCLASS)0;
	constexpr THREADINFOCLASS ThreadPriority = (THREADINFOCLASS)2;

	using NtQueryInformationThreadFunc = NTSTATUS(WINAPI*)(HANDLE, int, PVOID, ULONG, PULONG);
	using NtSetInformationThreadFunc = NTSTATUS(WINAPI*)(HANDLE, int, PVOID, ULONG);

	NtQueryInformationThreadFunc g_ntQueryInformationThread = nullptr;
	NtSetInformationThreadFunc g_ntSetInformationThread = nullptr;

	struct THREAD_BASIC_INFORMATION {
		NTSTATUS ExitStatus;
		PVOID TebBaseAddress;
		CLIENT_ID ClientId;
		KAFFINITY AffinityMask;
		KPRIORITY Priority;
		KPRIORITY BasePriority;
	};

	struct SuspiciousThread {
		HANDLE thread;
		DWORD threadId;
	};

	HANDLE g_hPriorityFixThread = nullptr;
	HANDLE g_hStopEvent = nullptr;

	// Function declarations
	DWORD GetCoresUsedByProcess();
	DWORD_PTR GetCPUMask();
	DWORD WINAPI PriorityFixThreadProc(LPVOID);
}

// Check the number CPU cores is being used by the process
DWORD Utils::GetCoresUsedByProcess()
{
	int numCores = 0;
	DWORD_PTR ProcessAffinityMask, SystemAffinityMask;
	if (GetProcessAffinityMask(GetCurrentProcess(), &ProcessAffinityMask, &SystemAffinityMask))
	{
		while (ProcessAffinityMask)
		{
			if (ProcessAffinityMask & 1)
			{
				++numCores;
			}
			ProcessAffinityMask >>= 1;
		}
	}
	return numCores;
}

// Get processor mask
DWORD_PTR Utils::GetCPUMask()
{
	static DWORD_PTR nMask = 0;
	if (nMask)
	{
		return nMask;
	}

	DWORD_PTR ProcessAffinityMask, SystemAffinityMask;
	if (GetProcessAffinityMask(GetCurrentProcess(), &ProcessAffinityMask, &SystemAffinityMask))
	{
		DWORD_PTR AffinityLow = 1;
		while (AffinityLow && (AffinityLow & SystemAffinityMask) == 0)
		{
			AffinityLow <<= 1;
		}
		if (AffinityLow)
		{
			nMask = ((AffinityLow << (Config.SingleProcAffinity - 1)) & SystemAffinityMask) ? (AffinityLow << (Config.SingleProcAffinity - 1)) : AffinityLow;
		}
	}

	Logging::Log() << __FUNCTION__ << " Setting CPU mask: " << Logging::hex(nMask);
	return nMask;
}

// Set Single Core Affinity
void Utils::SetProcessAffinity()
{
	Logging::Log() << "Setting SingleCoreAffinity...";
	SetProcessAffinityMask(GetCurrentProcess(), GetCPUMask());
}

void Utils::SetThreadAffinity(DWORD threadId)
{
	static DWORD_PTR cpuAffinityMask = GetCPUMask();

	HANDLE hThread = OpenThread(THREAD_QUERY_INFORMATION | THREAD_SET_INFORMATION, FALSE, threadId);
	if (hThread)
	{
		SetThreadAffinityMask(hThread, cpuAffinityMask);
		CloseHandle(hThread);
	}
}

void Utils::ApplyThreadAffinity()
{
	const DWORD pid = GetCurrentProcessId();
	const HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if (INVALID_HANDLE_VALUE == snapshot)
	{
		Logging::Log() << "ERROR: CreateToolhelp32Snapshot failed: " << Logging::hex(GetLastError());
		return;
	}

	THREADENTRY32 te = {};
	te.dwSize = sizeof(te);
	if (!Thread32First(snapshot, &te))
	{
		CloseHandle(snapshot);
		Logging::Log() << "ERROR: Thread32First failed: " << Logging::hex(GetLastError());
		return;
	}

	do {
		if (pid == te.th32OwnerProcessID)
		{
			SetThreadAffinity(te.th32ThreadID);
		}
	} while (Thread32Next(snapshot, &te));

	CloseHandle(snapshot);
}

DWORD WINAPI Utils::PriorityFixThreadProc(LPVOID)
{
	DWORD currentPid = GetCurrentProcessId();

	while (WaitForSingleObject(g_hStopEvent, 10) == WAIT_TIMEOUT)
	{
		HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
		if (snapshot == INVALID_HANDLE_VALUE)
		{
			continue;
		}

		THREADENTRY32 te = { sizeof(te) };
		if (Thread32First(snapshot, &te))
		{
			do {
				if (te.th32OwnerProcessID == currentPid)
				{
					HANDLE hThread = OpenThread(THREAD_QUERY_INFORMATION | THREAD_SET_INFORMATION, FALSE, te.th32ThreadID);
					if (hThread)
					{
						THREAD_BASIC_INFORMATION tbi = {};
						NTSTATUS status = g_ntQueryInformationThread(hThread, ThreadBasicInformation, &tbi, sizeof(tbi), nullptr);
						if (NT_SUCCESS(status))
						{
							if (tbi.Priority >= 16 && tbi.BasePriority < 16)
							{
								ULONG prio = THREAD_PRIORITY_HIGHEST;
								status = g_ntSetInformationThread(hThread, ThreadPriority, &prio, sizeof(prio));
								if (!NT_SUCCESS(status))
								{
									//Log(L"NtSetInformationThread failed for thread %u\n", it->first, 0, 0);
								}
							}
						}
						CloseHandle(hThread);
					}
				}
			} while (Thread32Next(snapshot, &te));
		}
		CloseHandle(snapshot);
	}

	return 0;
}

void Utils::StartPriorityMonitor()
{
	if (!g_ntQueryInformationThread || !g_ntSetInformationThread)
	{
		HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
		if (ntdll)
		{
			g_ntQueryInformationThread = (NtQueryInformationThreadFunc)GetProcAddress(ntdll, "NtQueryInformationThread");
			g_ntSetInformationThread = (NtSetInformationThreadFunc)GetProcAddress(ntdll, "NtSetInformationThread");
		}
	}

	if (!g_ntQueryInformationThread || !g_ntSetInformationThread)
	{
		return;
	}

	if (!g_hStopEvent)
	{
		g_hStopEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr); // Manual-reset, initially non-signaled
	}

	if (!g_hPriorityFixThread)
	{
		g_hPriorityFixThread = CreateThread(nullptr, 0, PriorityFixThreadProc, nullptr, 0, nullptr);
	}
}

void Utils::StopPriorityMonitor()
{
	if (g_hStopEvent)
	{
		SetEvent(g_hStopEvent);
		if (g_hPriorityFixThread)
		{
			WaitForSingleObject(g_hPriorityFixThread, INFINITE);
			CloseHandle(g_hPriorityFixThread);
			g_hPriorityFixThread = nullptr;
		}
		CloseHandle(g_hStopEvent);
		g_hStopEvent = nullptr;
	}
}
