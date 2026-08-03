/**
* Copyright (C) 2026 Elisha Riedlinger
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

#if (_WIN32_WINNT >= 0x0502)
#include <dbghelp.h>
#else
#pragma warning(push)
#pragma warning(disable : 4091)
#include <dbghelp.h>
#pragma warning(pop)
#endif

#include "Utils.h"
#include "Dllmain\Dllmain.h"
#include "External\Hooking\Disasm.h"
#include "External\Hooking\Hook.h"
#include "Settings\Settings.h"
#include "Logging\Logging.h"

#ifndef CREATE_MINIDUMP
//#define CREATE_MINIDUMP
#endif

namespace Utils
{
	static LPTOP_LEVEL_EXCEPTION_FILTER g_previousFilter = nullptr;

	typedef BOOL(WINAPI* MiniDumpWriteDump_t)(
		HANDLE,
		DWORD,
		HANDLE,
		MINIDUMP_TYPE,
		const PMINIDUMP_EXCEPTION_INFORMATION,
		const PMINIDUMP_USER_STREAM_INFORMATION,
		const PMINIDUMP_CALLBACK_INFORMATION
		);
}

static void WriteMiniDump(EXCEPTION_POINTERS* ExceptionInfo)
{
	using namespace Utils;

	// Recursion guard, call once per-process
	static LONG inDump = 0;
	if (InterlockedExchange(&inDump, 1))
	{
		return;
	}

	// Get dump path
	char dumpPath[MAX_PATH] = {};
	{
		char modulePath[MAX_PATH] = {};

		// Get the path of dxwrapper.dll
		DWORD len = GetModuleFileNameA(hModule_dll, modulePath, MAX_PATH);
		if (len == 0 || len == MAX_PATH)
		{
			modulePath[0] = '\0';
		}
		// Strip filename to get directory
		else
		{
			char* lastSlash = strrchr(modulePath, '\\');
			if (lastSlash)
			{
				*(lastSlash + 1) = '\0'; // keep trailing slash
			}
		}

		// Build full dump path
		strcpy_s(dumpPath, modulePath);

		SYSTEMTIME st;
		GetLocalTime(&st);

		char filename[64];
		sprintf_s(filename, "dxwrapper-crash-%04d%02d%02d-%02d%02d%02d-%03d.dmp",
			st.wYear, st.wMonth, st.wDay,
			st.wHour, st.wMinute, st.wSecond,
			st.wMilliseconds);

		strcat_s(dumpPath, filename);
	}

	// Log event
	Logging::LogFormat("Attempting to create dump file: %s", dumpPath);

	HANDLE hFile = CreateFileA(dumpPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		return;
	}

	// Dynamically load dbghelp.dll
	HMODULE hDbgHelp = nullptr;
	{
		char DlgHelpPath[MAX_PATH] = {};

		UINT len = GetSystemDirectoryA(DlgHelpPath, MAX_PATH);
		if (len > 0 && len < MAX_PATH - 20)
		{
			strcat_s(DlgHelpPath, "\\dbghelp.dll");
		}
		else
		{
			strcpy_s(DlgHelpPath, "dbghelp.dll");
		}

		hDbgHelp = LoadLibraryA(DlgHelpPath);
	}
	if (!hDbgHelp)
	{
		CloseHandle(hFile);
		return;
	}

	MiniDumpWriteDump_t pMiniDumpWriteDump = (MiniDumpWriteDump_t)GetProcAddress(hDbgHelp, "MiniDumpWriteDump");

	if (pMiniDumpWriteDump)
	{
		MINIDUMP_EXCEPTION_INFORMATION mei = {};
		mei.ThreadId = GetCurrentThreadId();
		mei.ExceptionPointers = ExceptionInfo;
		mei.ClientPointers = FALSE;

		pMiniDumpWriteDump(
			GetCurrentProcess(),
			GetCurrentProcessId(),
			hFile,
			MiniDumpWithFullMemory,
			&mei,
			NULL,
			NULL
		);
	}

	FreeLibrary(hDbgHelp);
	CloseHandle(hFile);
}

// Your existing exception handler function
LONG WINAPI Utils::VectoredExceptionHandler(EXCEPTION_POINTERS* ExceptionInfo)
{
	__try
	{
		if (!ExceptionInfo || !ExceptionInfo->ExceptionRecord)
		{
			return EXCEPTION_CONTINUE_SEARCH;
		}

		const DWORD code = ExceptionInfo->ExceptionRecord->ExceptionCode;
		void* faultAddr = ExceptionInfo->ExceptionRecord->ExceptionAddress;

		// Simulate instruction patching (NOP fill) for specific exceptions
		if (Config.HandleExceptions)
		{
			switch (code)
			{
			case STATUS_INTEGER_DIVIDE_BY_ZERO:
			case STATUS_INTEGER_OVERFLOW:
			case STATUS_PRIVILEGED_INSTRUCTION:
			case STATUS_ILLEGAL_INSTRUCTION:
			case STATUS_ACCESS_VIOLATION:
				__try
				{
					char moduleName[MAX_PATH] = {};
					__try
					{
						GetModuleFromAddress(faultAddr, moduleName, MAX_PATH);
					}
					__except (EXCEPTION_EXECUTE_HANDLER)
					{
						moduleName[0] = '\0';
					}

					if (!moduleName[0])
					{
						strcpy_s(moduleName, "unknown");
					}

					Logging::LogFormat(
						__FUNCTION__ " "
						"Warning: Exception caught at address: %p "
						"code: 0x%08X"
						"module: %s",
						faultAddr,
						code,
						moduleName
					);
				}
				__except (EXCEPTION_EXECUTE_HANDLER) {}

				// Use custom disassembler to get the instruction length
				__try
				{
					unsigned instrLen = Disasm::getInstructionLength(faultAddr);
					if (instrLen == 0 || instrLen > 15) // Sanity check
					{
						Logging::LogFormat(__FUNCTION__ " Error: Invalid instruction length, skipping patch.");
						break;
					}

					DWORD oldProtect = 0;
					if (Config.HandleExceptions == 2)
					{
						ExceptionInfo->ContextRecord->Eip += instrLen;
						Logging::LogFormat(__FUNCTION__ " Skipping instruction and continuing execution.");
						return EXCEPTION_CONTINUE_EXECUTION;
					}
					else if (VirtualProtect(faultAddr, instrLen, PAGE_EXECUTE_READWRITE, &oldProtect))
					{
						memset(faultAddr, 0x90, instrLen); // Patch with NOPs
						VirtualProtect(faultAddr, instrLen, oldProtect, &oldProtect);
						FlushInstructionCache(GetCurrentProcess(), faultAddr, instrLen);

						ExceptionInfo->ContextRecord->Eip += instrLen;
						Logging::LogFormat(__FUNCTION__ " Patched instruction with NOPs, continuing execution.");
						return EXCEPTION_CONTINUE_EXECUTION;
					}
					else
					{
						Logging::LogFormat(__FUNCTION__ " Error: VirtualProtect failed, error=%d", GetLastError());
					}
				}
				__except (EXCEPTION_EXECUTE_HANDLER) {}
				break;
			}
		}

		switch (code)
		{
		case STATUS_PRIVILEGED_INSTRUCTION:
		{
			size_t size = Disasm::getInstructionLength(faultAddr);

			if (!size || size > 15)
			{
				return EXCEPTION_CONTINUE_SEARCH;
			}

			if (ExceptionInfo->ContextRecord)
			{
				static DWORD count = 0;
				if (count++ < 10)
				{
					__try
					{
						auto rec = ExceptionInfo->ExceptionRecord;
						auto ctx = ExceptionInfo->ContextRecord;

						char moduleName[MAX_PATH] = {};
						__try
						{
							GetModuleFromAddress(rec->ExceptionAddress, moduleName, MAX_PATH);
						}
						__except (EXCEPTION_EXECUTE_HANDLER)
						{
							moduleName[0] = '\0';
						}

						if (!moduleName[0])
						{
							strcpy_s(moduleName, "unknown");
						}

						Logging::LogFormat(
							"Skipping PRIVILEGED INSTRUCTION: addr=%p module=%s "
							"Registers: EIP=0x%08X ECX=0x%08X EAX=0x%08X",
							rec->ExceptionAddress,
							moduleName,
							ctx->Eip,
							ctx->Ecx,
							ctx->Eax
						);
					}
					__except (EXCEPTION_EXECUTE_HANDLER) {}
				}

				ExceptionInfo->ContextRecord->Eip += size;
				return EXCEPTION_CONTINUE_EXECUTION;
			}
		}
		break;

		case EXCEPTION_ACCESS_VIOLATION:
		case EXCEPTION_ILLEGAL_INSTRUCTION:
		case EXCEPTION_IN_PAGE_ERROR:
		case EXCEPTION_STACK_OVERFLOW:
		{
			// Log error
			__try
			{
				auto rec = ExceptionInfo->ExceptionRecord;
				auto ctx = ExceptionInfo->ContextRecord;

				DWORD rw = 0;
				void* badAddr = nullptr;

				if (rec->ExceptionCode == EXCEPTION_ACCESS_VIOLATION &&
					rec->NumberParameters >= 2)
				{
					rw = rec->ExceptionInformation[0];
					badAddr = (void*)rec->ExceptionInformation[1];
				}

				char moduleName[MAX_PATH] = {};
				__try
				{
					GetModuleFromAddress(rec->ExceptionAddress, moduleName, MAX_PATH);
				}
				__except (EXCEPTION_EXECUTE_HANDLER)
				{
					moduleName[0] = '\0';
				}

				if (!moduleName[0])
				{
					strcpy_s(moduleName, "unknown");
				}

				if (ctx)
				{
					Logging::LogFormat(
						"Error occurred in the application: "
						"code=0x%08X flags=0x%08X addr=%p rw=%d bad=%p module=%s "
						"Registers: EIP=0x%08X ECX=0x%08X EAX=0x%08X",
						rec->ExceptionCode,
						rec->ExceptionFlags,
						rec->ExceptionAddress,
						rw,
						badAddr,
						moduleName,
						ctx->Eip,
						ctx->Ecx,
						ctx->Eax
					);
				}
				else
				{
					Logging::LogFormat(
						"Error occurred in the application: "
						"code=0x%08X flags=0x%08X addr=%p rw=%d bad=%p module=%s",
						rec->ExceptionCode,
						rec->ExceptionFlags,
						rec->ExceptionAddress,
						rw,
						badAddr,
						moduleName
					);
				}
			}
			__except (EXCEPTION_EXECUTE_HANDLER) {}

#ifndef CREATE_MINIDUMP
			if (Config.EnableDumpFileCreation)
#endif // CREATE_MINIDUMP
			{
				static LONG runonce = 1;
				if (InterlockedExchange(&runonce, 0))
				{
					WriteMiniDump(ExceptionInfo);
				}
			}
		}
		break;

		case 0x40010006: // debug/runtime
		case 0x406D1388: // thread naming
		case 0xE06D7363: // C++ exception
			return EXCEPTION_CONTINUE_SEARCH;

		default:
		{
			char moduleName[MAX_PATH] = {};
			__try
			{
				GetModuleFromAddress(faultAddr, moduleName, MAX_PATH);
			}
			__except (EXCEPTION_EXECUTE_HANDLER)
			{
				moduleName[0] = '\0';
			}

			if (!moduleName[0])
			{
				strcpy_s(moduleName, "unknown");
			}

			Logging::LogFormat("Exception: code=0x%08X at %p in %s", code, faultAddr, moduleName);
		}
		break;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		return EXCEPTION_CONTINUE_SEARCH;
	}

	return EXCEPTION_CONTINUE_SEARCH;
}
