#define WIN32_LEAN_AND_MEAN

#include <map>
#include <utility>

#include <Windows.h>
#include <detours.h>

#include "DDrawLog.h"
#include "Hook.h"

namespace Compat21
{
	namespace
	{
		struct HookedFunctionInfo
		{
			void* trampoline;
			void* newFunction;
		};

		std::map<void*, HookedFunctionInfo> g_hookedFunctions;

		FARPROC getProcAddress(HMODULE module, const char* procName)
		{
			if (!module || !procName)
			{
				return nullptr;
			}

			PIMAGE_DOS_HEADER dosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(module);
			if (IMAGE_DOS_SIGNATURE != dosHeader->e_magic) {
				return nullptr;
			}
			char* moduleBase = reinterpret_cast<char*>(module);

			PIMAGE_NT_HEADERS ntHeader = reinterpret_cast<PIMAGE_NT_HEADERS>(
				reinterpret_cast<char*>(dosHeader) + dosHeader->e_lfanew);
			if (IMAGE_NT_SIGNATURE != ntHeader->Signature)
			{
				return nullptr;
			}

			PIMAGE_EXPORT_DIRECTORY exportDir = reinterpret_cast<PIMAGE_EXPORT_DIRECTORY>(
				moduleBase + ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);

			DWORD* rvaOfNames = reinterpret_cast<DWORD*>(moduleBase + exportDir->AddressOfNames);

			for (DWORD i = 0; i < exportDir->NumberOfNames; ++i)
			{
				if (0 == strcmp(procName, moduleBase + rvaOfNames[i]))
				{
					WORD* nameOrds = reinterpret_cast<WORD*>(moduleBase + exportDir->AddressOfNameOrdinals);
					DWORD* rvaOfFunctions = reinterpret_cast<DWORD*>(moduleBase + exportDir->AddressOfFunctions);
					return reinterpret_cast<FARPROC>(moduleBase + rvaOfFunctions[nameOrds[i]]);
				}
			}

			return nullptr;
		}

		void hookFunction(const char* funcName, void*& origFuncPtr, void* newFuncPtr)
		{
			const auto it = g_hookedFunctions.find(origFuncPtr);
			if (it != g_hookedFunctions.end())
			{
				origFuncPtr = it->second.trampoline;
				return;
			}

			void* const hookedFuncPtr = origFuncPtr;

			DetourTransactionBegin();
			const bool attachSuccessful = NO_ERROR == DetourAttach(&origFuncPtr, newFuncPtr);
			const bool commitSuccessful = NO_ERROR == DetourTransactionCommit();
			if (!attachSuccessful || !commitSuccessful)
			{
				if (funcName)
				{
					Logging::Log() << "Failed to hook a function: " << funcName;
				}
				else
				{
					Logging::Log() << "Failed to hook a function: " << origFuncPtr;
				}
				return;
			}

			g_hookedFunctions[hookedFuncPtr] = { origFuncPtr, newFuncPtr };
		}
	}

	namespace Compat
	{
		void hookFunction(void*& origFuncPtr, void* newFuncPtr)
		{
			Compat21::hookFunction(nullptr, origFuncPtr, newFuncPtr);
		}

		void hookFunction(const char* moduleName, const char* funcName, void*& origFuncPtr, void* newFuncPtr)
		{
			FARPROC procAddr = getProcAddress(GetModuleHandle(moduleName), funcName);
			if (!procAddr)
			{
				Logging::LogDebug() << "Failed to load the address of a function: " << funcName;
				return;
			}

			origFuncPtr = procAddr;
			Compat21::hookFunction(funcName, origFuncPtr, newFuncPtr);
		}

		void unhookAllFunctions()
		{
			for (auto& hookedFunc : g_hookedFunctions)
			{
				DetourTransactionBegin();
				DetourDetach(&hookedFunc.second.trampoline, hookedFunc.second.newFunction);
				DetourTransactionCommit();
			}
		}
	}
}
