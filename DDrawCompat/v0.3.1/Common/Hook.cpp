#define WIN32_LEAN_AND_MEAN

#undef CINTERFACE

#include <list>
#include <sstream>
#include <string>

#include <Windows.h>
#include <Shlwapi.h>
#include <initguid.h>
#include <DbgEng.h>

#include <DDrawCompat/v0.3.1/Common/Hook.h>
#include <DDrawCompat/DDrawLog.h>
#include <DDrawCompat/v0.3.1/Common/Path.h>
#include <DDrawCompat/v0.3.1/Dll/Dll.h>

namespace
{
	IDebugClient4* g_debugClient = nullptr;
	IDebugControl* g_debugControl = nullptr;
	IDebugSymbols* g_debugSymbols = nullptr;
	IDebugDataSpaces4* g_debugDataSpaces = nullptr;
	ULONG64 g_debugBase = 0;
	bool g_isDbgEngInitialized = false;

	LONG WINAPI dbgEngWinVerifyTrust(HWND hwnd, GUID* pgActionID, LPVOID pWVTData);
	PIMAGE_NT_HEADERS getImageNtHeaders(HMODULE module);
	bool initDbgEng();

	FARPROC WINAPI dbgEngGetProcAddress(HMODULE hModule, LPCSTR lpProcName)
	{
		LOG_FUNC("dbgEngGetProcAddress", hModule, lpProcName);
		if (0 == strcmp(lpProcName, "WinVerifyTrust"))
		{
			return LOG_RESULT(reinterpret_cast<FARPROC>(&dbgEngWinVerifyTrust));
		}
		return LOG_RESULT(GetProcAddress(hModule, lpProcName));
	}

	LONG WINAPI dbgEngWinVerifyTrust(
		[[maybe_unused]] HWND hwnd,
		[[maybe_unused]] GUID* pgActionID,
		[[maybe_unused]] LPVOID pWVTData)
	{
		LOG_FUNC("dbgEngWinVerifyTrust", hwnd, pgActionID, pWVTData);
		return LOG_RESULT(0);
	}

	FARPROC* findProcAddressInIat(HMODULE module, const char* procName)
	{
		if (!module || !procName)
		{
			return nullptr;
		}

		PIMAGE_NT_HEADERS ntHeaders = getImageNtHeaders(module);
		if (!ntHeaders)
		{
			return nullptr;
		}

		char* moduleBase = reinterpret_cast<char*>(module);
		PIMAGE_IMPORT_DESCRIPTOR importDesc = reinterpret_cast<PIMAGE_IMPORT_DESCRIPTOR>(moduleBase +
			ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

		for (PIMAGE_IMPORT_DESCRIPTOR desc = importDesc;
			0 != desc->Characteristics && 0xFFFF != desc->Name;
			++desc)
		{
			auto thunk = reinterpret_cast<PIMAGE_THUNK_DATA>(moduleBase + desc->FirstThunk);
			auto origThunk = reinterpret_cast<PIMAGE_THUNK_DATA>(moduleBase + desc->OriginalFirstThunk);
			while (0 != thunk->u1.AddressOfData && 0 != origThunk->u1.AddressOfData)
			{
				auto origImport = reinterpret_cast<PIMAGE_IMPORT_BY_NAME>(
					moduleBase + origThunk->u1.AddressOfData);

				if (0 == strcmp(origImport->Name, procName))
				{
					return reinterpret_cast<FARPROC*>(&thunk->u1.Function);
				}

				++thunk;
				++origThunk;
			}
		}

		return nullptr;
	}

	PIMAGE_NT_HEADERS getImageNtHeaders(HMODULE module)
	{
		PIMAGE_DOS_HEADER dosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(module);
		if (IMAGE_DOS_SIGNATURE != dosHeader->e_magic)
		{
			return nullptr;
		}

		PIMAGE_NT_HEADERS ntHeaders = reinterpret_cast<PIMAGE_NT_HEADERS>(
			reinterpret_cast<char*>(dosHeader) + dosHeader->e_lfanew);
		if (IMAGE_NT_SIGNATURE != ntHeaders->Signature)
		{
			return nullptr;
		}

		return ntHeaders;
	}

	unsigned getInstructionSize(void* instruction)
	{
		const unsigned MAX_INSTRUCTION_SIZE = 15;
		HRESULT result = g_debugDataSpaces->WriteVirtual(g_debugBase, instruction, MAX_INSTRUCTION_SIZE, nullptr);
		if (FAILED(result))
		{
			LOG_ONCE("ERROR: DbgEng: WriteVirtual failed: " << Compat32::hex(result));
			return 0;
		}

		ULONG64 endOffset = 0;
		result = g_debugControl->Disassemble(g_debugBase, 0, nullptr, 0, nullptr, &endOffset);
		if (FAILED(result))
		{
			LOG_ONCE("ERROR: DbgEng: Disassemble failed: " << Compat32::hex(result) << " "
				<< Compat32::hexDump(instruction, MAX_INSTRUCTION_SIZE));
			return 0;
		}

		return static_cast<unsigned>(endOffset - g_debugBase);
	}

	void hookFunction(void*& origFuncPtr, void* newFuncPtr, const char* funcName)
	{
		BYTE* targetFunc = reinterpret_cast<BYTE*>(origFuncPtr);

		std::ostringstream oss;
#ifdef DEBUGLOGS
		oss << Compat32::funcPtrToStr(targetFunc) << ' ';

		char origFuncPtrStr[20] = {};
		if (!funcName)
		{
			sprintf_s(origFuncPtrStr, "%p", origFuncPtr);
			funcName = origFuncPtrStr;
		}

		auto prevTargetFunc = targetFunc;
#endif

		while (true)
		{
			unsigned instructionSize = 0;
			if (0xE9 == targetFunc[0])
			{
				instructionSize = 5;
				targetFunc += instructionSize + *reinterpret_cast<int*>(targetFunc + 1);
			}
			else if (0xEB == targetFunc[0])
			{
				instructionSize = 2;
				targetFunc += instructionSize + *reinterpret_cast<signed char*>(targetFunc + 1);
			}
			else if (0xFF == targetFunc[0] && 0x25 == targetFunc[1])
			{
				instructionSize = 6;
				auto candidateTargetFunc = **reinterpret_cast<BYTE***>(targetFunc + 2);
				if (Compat32::getModuleHandleFromAddress(candidateTargetFunc) == Compat32::getModuleHandleFromAddress(targetFunc))
				{
					break;
				}
				targetFunc = candidateTargetFunc;
			}
			else
			{
				break;
			}
#ifdef DEBUGLOGS
			oss << Compat32::hexDump(prevTargetFunc, instructionSize) << " -> " << Compat32::funcPtrToStr(targetFunc) << ' ';
			prevTargetFunc = targetFunc;
#endif
		}

		if (Compat32::getModuleHandleFromAddress(targetFunc) == Dll::g_currentModule)
		{
			Compat32::Log() << "ERROR: Target function is already hooked: " << funcName;
			return;
		}

		if (!initDbgEng())
		{
			return;
		}

		unsigned totalInstructionSize = 0;
		while (totalInstructionSize < 5)
		{
			unsigned instructionSize = getInstructionSize(targetFunc + totalInstructionSize);
			if (0 == instructionSize)
			{
				return;
			}
			totalInstructionSize += instructionSize;
		}

		LOG_DEBUG << "Hooking function: " << funcName
			<< " (" << oss.str() << Compat32::hexDump(targetFunc, totalInstructionSize) << ')';

		BYTE* trampoline = static_cast<BYTE*>(
			VirtualAlloc(nullptr, totalInstructionSize + 5, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));
		memcpy(trampoline, targetFunc, totalInstructionSize);
		trampoline[totalInstructionSize] = 0xE9;
		reinterpret_cast<int&>(trampoline[totalInstructionSize + 1]) = targetFunc - (trampoline + 5);
		DWORD oldProtect = 0;
		VirtualProtect(trampoline, totalInstructionSize + 5, PAGE_EXECUTE_READ, &oldProtect);

		VirtualProtect(targetFunc, totalInstructionSize, PAGE_EXECUTE_READWRITE, &oldProtect);
		targetFunc[0] = 0xE9;
		reinterpret_cast<int&>(targetFunc[1]) = static_cast<BYTE*>(newFuncPtr) - (targetFunc + 5);
		memset(targetFunc + 5, 0xCC, totalInstructionSize - 5);
		VirtualProtect(targetFunc, totalInstructionSize, PAGE_EXECUTE_READ, &oldProtect);

		FlushInstructionCache(GetCurrentProcess(), nullptr, 0);

		HMODULE module = nullptr;
		GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_PIN,
			reinterpret_cast<char*>(targetFunc), &module);

		origFuncPtr = trampoline;
	}

	bool initDbgEng()
	{
		if (g_isDbgEngInitialized)
		{
			return 0 != g_debugBase;
		}
		g_isDbgEngInitialized = true;

		//********** Begin Edit *************
		typedef HRESULT(STDAPICALLTYPE* PFN_DebugCreate)(_In_ REFIID InterfaceId, _Out_ PVOID* Interface);
		static PFN_DebugCreate pDebugCreate = nullptr;
		static bool RunOnce = true;
		if (RunOnce)
		{
			// Get System32 path
			char syspath[MAX_PATH];
			GetSystemDirectory(syspath, MAX_PATH);

			// Load dbghelp.dll from System32
			char path[MAX_PATH];
			strcpy_s(path, MAX_PATH, syspath);
			PathAppend(path, "dbghelp.dll");
			HMODULE dll = LoadLibrary(path);

			// Try loading dbgeng.dll from System32
			strcpy_s(path, MAX_PATH, syspath);
			PathAppend(path, "dbgeng.dll");
			dll = LoadLibrary(path);

			// Try loading dbgeng.dll from local path
			if (!dll || !GetProcAddress(dll, "DebugCreate"))
			{
				dll = LoadLibrary("dbgeng.dll");
			}

			// Hook function and get process address
			if (dll && GetProcAddress(dll, "DebugCreate"))
			{
				Compat32::hookIatFunction(dll, "GetProcAddress", dbgEngGetProcAddress);

				pDebugCreate = (PFN_DebugCreate)GetProcAddress(dll, "DebugCreate");
			}
			RunOnce = false;
		}
		if (!pDebugCreate)
		{
			Compat32::Log() << "ERROR: DbgEng: failed to get proc address!";
			return false;
		}
		//********** End Edit ***************

		HRESULT result = S_OK;
		if (FAILED(result = pDebugCreate(IID_IDebugClient4, reinterpret_cast<void**>(&g_debugClient))) ||
			FAILED(result = g_debugClient->QueryInterface(IID_IDebugControl, reinterpret_cast<void**>(&g_debugControl))) ||
			FAILED(result = g_debugClient->QueryInterface(IID_IDebugSymbols, reinterpret_cast<void**>(&g_debugSymbols))) ||
			FAILED(result = g_debugClient->QueryInterface(IID_IDebugDataSpaces4, reinterpret_cast<void**>(&g_debugDataSpaces))))
		{
			Compat32::Log() << "ERROR: DbgEng: object creation failed: " << Compat32::hex(result);
			return false;
		}

		result = g_debugClient->OpenDumpFileWide(Compat32::getModulePath(Dll::g_currentModule).c_str(), 0);
		if (FAILED(result))
		{
			Compat32::Log() << "ERROR: DbgEng: OpenDumpFile failed: " << Compat32::hex(result);
			return false;
		}

		g_debugControl->SetEngineOptions(DEBUG_ENGOPT_DISABLE_MODULE_SYMBOL_LOAD);
		result = g_debugControl->WaitForEvent(0, INFINITE);
		if (FAILED(result))
		{
			Compat32::Log() << "ERROR: DbgEng: WaitForEvent failed: " << Compat32::hex(result);
			return false;
		}

		DEBUG_MODULE_PARAMETERS dmp = {};
		result = g_debugSymbols->GetModuleParameters(1, 0, 0, &dmp);
		if (FAILED(result))
		{
			Compat32::Log() << "ERROR: DbgEng: GetModuleParameters failed: " << Compat32::hex(result);
			return false;
		}

		ULONG size = 0;
		result = g_debugDataSpaces->GetValidRegionVirtual(dmp.Base, dmp.Size, &g_debugBase, &size);
		if (FAILED(result) || 0 == g_debugBase)
		{
			Compat32::Log() << "ERROR: DbgEng: GetValidRegionVirtual failed: " << Compat32::hex(result);
			return false;
		}

		return true;
	}
}

namespace Compat32
{
	void closeDbgEng()
	{
		if (g_debugClient)
		{
			g_debugClient->EndSession(DEBUG_END_PASSIVE);
		}
		if (g_debugDataSpaces)
		{
			g_debugDataSpaces->Release();
			g_debugDataSpaces = nullptr;
		}
		if (g_debugSymbols)
		{
			g_debugSymbols->Release();
			g_debugSymbols = nullptr;
		}
		if (g_debugControl)
		{
			g_debugControl->Release();
			g_debugControl = nullptr;
		}
		if (g_debugClient)
		{
			g_debugClient->Release();
			g_debugClient = nullptr;
		}

		g_debugBase = 0;
		g_isDbgEngInitialized = false;
	}

	std::string funcPtrToStr(void* funcPtr)
	{
		std::ostringstream oss;
		HMODULE module = Compat32::getModuleHandleFromAddress(funcPtr);
		if (module)
		{
			oss << Compat32::getModulePath(module).u8string() << "+0x" << std::hex <<
				reinterpret_cast<DWORD>(funcPtr) - reinterpret_cast<DWORD>(module);
		}
		else
		{
			oss << funcPtr;
		}
		return oss.str();
	}

	HMODULE getModuleHandleFromAddress(void* address)
	{
		HMODULE module = nullptr;
		GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
			static_cast<char*>(address), &module);
		return module;
	}

	FARPROC getProcAddress(HMODULE module, const char* procName)
	{
		if (!module || !procName)
		{
			return nullptr;
		}

		PIMAGE_NT_HEADERS ntHeaders = getImageNtHeaders(module);
		if (!ntHeaders)
		{
			return nullptr;
		}

		char* moduleBase = reinterpret_cast<char*>(module);
		PIMAGE_EXPORT_DIRECTORY exportDir = reinterpret_cast<PIMAGE_EXPORT_DIRECTORY>(
			moduleBase + ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
		auto exportDirSize = ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;

		DWORD* rvaOfNames = reinterpret_cast<DWORD*>(moduleBase + exportDir->AddressOfNames);
		WORD* nameOrds = reinterpret_cast<WORD*>(moduleBase + exportDir->AddressOfNameOrdinals);
		DWORD* rvaOfFunctions = reinterpret_cast<DWORD*>(moduleBase + exportDir->AddressOfFunctions);

		char* func = nullptr;
		if (0 == HIWORD(procName))
		{
			WORD ord = LOWORD(procName);
			if (ord < exportDir->Base || ord >= exportDir->Base + exportDir->NumberOfFunctions)
			{
				return nullptr;
			}
			func = moduleBase + rvaOfFunctions[ord - exportDir->Base];
		}
		else
		{
			for (DWORD i = 0; i < exportDir->NumberOfNames; ++i)
			{
				if (0 == strcmp(procName, moduleBase + rvaOfNames[i]))
				{
					func = moduleBase + rvaOfFunctions[nameOrds[i]];
				}
			}
		}

		if (func &&
			func >= reinterpret_cast<char*>(exportDir) &&
			func < reinterpret_cast<char*>(exportDir) + exportDirSize)
		{
			std::string forw(func);
			auto separatorPos = forw.find_first_of('.');
			if (std::string::npos == separatorPos)
			{
				return nullptr;
			}
			HMODULE forwModule = GetModuleHandle(forw.substr(0, separatorPos).c_str());
			std::string forwFuncName = forw.substr(separatorPos + 1);
			if ('#' == forwFuncName[0])
			{
				int32_t ord = std::atoi(forwFuncName.substr(1).c_str());
				if (ord < 0 || ord > 0xFFFF)
				{
					return nullptr;
				}
				return getProcAddress(forwModule, reinterpret_cast<const char*>(ord));
			}
			else
			{
				return getProcAddress(forwModule, forwFuncName.c_str());
			}
		}

		return reinterpret_cast<FARPROC>(func);
	}

	void hookFunction(void*& origFuncPtr, void* newFuncPtr, const char* funcName)
	{
		::hookFunction(origFuncPtr, newFuncPtr, funcName);
	}

	void hookFunction(HMODULE module, const char* funcName, void*& origFuncPtr, void* newFuncPtr)
	{
		FARPROC procAddr = getProcAddress(module, funcName);
		if (!procAddr)
		{
			LOG_DEBUG << "ERROR: Failed to load the address of a function: " << funcName;
			return;
		}

		origFuncPtr = procAddr;
		::hookFunction(origFuncPtr, newFuncPtr, funcName);
	}

	void hookFunction(const char* moduleName, const char* funcName, void*& origFuncPtr, void* newFuncPtr)
	{
		HMODULE module = LoadLibrary(moduleName);
		if (!module)
		{
			return;
		}
		hookFunction(module, funcName, origFuncPtr, newFuncPtr);
		FreeLibrary(module);
	}

	void hookIatFunction(HMODULE module, const char* funcName, void* newFuncPtr)
	{
		FARPROC* func = findProcAddressInIat(module, funcName);
		if (func)
		{
			LOG_DEBUG << "Hooking function via IAT: " << funcName << " (" << funcPtrToStr(*func) << ')';
			DWORD oldProtect = 0;
			VirtualProtect(func, sizeof(func), PAGE_READWRITE, &oldProtect);
			*func = static_cast<FARPROC>(newFuncPtr);
			DWORD dummy = 0;
			VirtualProtect(func, sizeof(func), oldProtect, &dummy);
		}
	}

	void removeShim(HMODULE module, const char* funcName)
	{
		void* shimFunc = GetProcAddress(module, funcName);
		if (shimFunc)
		{
			void* realFunc = getProcAddress(module, funcName);
			if (realFunc && shimFunc != realFunc)
			{
				static std::list<void*> shimFuncs;
				shimFuncs.push_back(shimFunc);
				std::string shimFuncName("[shim]");
				shimFuncName += funcName;
				hookFunction(shimFuncs.back(), realFunc, shimFuncName.c_str());
			}
		}
	}
}
