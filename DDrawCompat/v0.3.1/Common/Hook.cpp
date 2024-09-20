#undef CINTERFACE

#include <list>
#include <sstream>
#include <string>

#include <Windows.h>

#include <External/Hooking/Disasm.h>
#include <DDrawCompat/v0.3.1/Common/Hook.h>
#include <DDrawCompat/DDrawLog.h>
#include <DDrawCompat/v0.3.1/Common/Path.h>
#include <DDrawCompat/v0.3.1/Dll/Dll.h>

namespace
{
	PIMAGE_NT_HEADERS getImageNtHeaders(HMODULE module);

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
				if (!(origThunk->u1.Ordinal & IMAGE_ORDINAL_FLAG))
				{
					auto origImport = reinterpret_cast<PIMAGE_IMPORT_BY_NAME>(moduleBase + origThunk->u1.AddressOfData);
					if (0 == strcmp(origImport->Name, procName))
					{
						return reinterpret_cast<FARPROC*>(&thunk->u1.Function);
					}
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

	void hookFunction(void*& origFuncPtr, void* newFuncPtr, const char* funcName)
	{
		BYTE* targetFunc = static_cast<BYTE*>(origFuncPtr);

		std::ostringstream oss;
		oss << Compat32::funcPtrToStr(targetFunc) << ' ';

		char origFuncPtrStr[20] = {};
		if (!funcName)
		{
			sprintf_s(origFuncPtrStr, "%p", origFuncPtr);
			funcName = origFuncPtrStr;
		}

		auto prevTargetFunc = targetFunc;
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
				targetFunc = **reinterpret_cast<BYTE***>(targetFunc + 2);
				if (Compat32::getModuleHandleFromAddress(targetFunc) == Compat32::getModuleHandleFromAddress(prevTargetFunc))
				{
					targetFunc = prevTargetFunc;
					break;
				}
			}
			else
			{
				break;
			}

			Compat32::LogDebug() << Compat32::hexDump(prevTargetFunc, instructionSize) << " -> "
				<< Compat32::funcPtrToStr(targetFunc) << ' ';
			prevTargetFunc = targetFunc;
		}

		if (Compat32::getModuleHandleFromAddress(targetFunc) == Dll::g_currentModule)
		{
			Compat32::Log() << "ERROR: Target function is already hooked: " << funcName;
			return;
		}

		const DWORD trampolineSize = 32;
		BYTE* trampoline = static_cast<BYTE*>(
			VirtualAlloc(nullptr, trampolineSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));
		BYTE* src = targetFunc;
		BYTE* dst = trampoline;
		while (src - targetFunc < 5)
		{
			unsigned instructionSize = Disasm::getInstructionLength(src);
			if (0 == instructionSize)
			{
				return;
			}

			memcpy(dst, src, instructionSize);
			if (0xE8 == *src && 5 == instructionSize)
			{
				*reinterpret_cast<int*>(dst + 1) += src - dst;
			}

			src += instructionSize;
			dst += instructionSize;
		}

		LOG_DEBUG << "Hooking function: " << funcName
			<< " (" << oss.str() << Compat32::hexDump(targetFunc, src - targetFunc) << ')';

		*dst = 0xE9;
		*reinterpret_cast<int*>(dst + 1) = src - (dst + 5);
		DWORD oldProtect = 0;
		VirtualProtect(trampoline, trampolineSize, PAGE_EXECUTE_READ, &oldProtect);

		VirtualProtect(targetFunc, src - targetFunc, PAGE_EXECUTE_READWRITE, &oldProtect);
		targetFunc[0] = 0xE9;
		*reinterpret_cast<int*>(targetFunc + 1) = static_cast<BYTE*>(newFuncPtr) - (targetFunc + 5);
		memset(targetFunc + 5, 0xCC, src - targetFunc - 5);
		VirtualProtect(targetFunc, src - targetFunc, PAGE_EXECUTE_READ, &oldProtect);

		origFuncPtr = trampoline;
		CALL_ORIG_FUNC(FlushInstructionCache)(GetCurrentProcess(), nullptr, 0);

		HMODULE module = nullptr;
		GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_PIN,
			reinterpret_cast<char*>(targetFunc), &module);
	}
}

namespace Compat32
{
	std::string funcPtrToStr(const void* funcPtr)
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

	DWORD getModuleFileOffset(const void* address)
	{
		LOG_FUNC("getModuleFileOffset", address);
		HMODULE mod = getModuleHandleFromAddress(address);
		if (!mod)
		{
			return LOG_RESULT(0);
		}

		PIMAGE_NT_HEADERS ntHeaders = getImageNtHeaders(mod);
		if (!ntHeaders)
		{
			return LOG_RESULT(0);
		}

		DWORD offset = static_cast<const BYTE*>(address) - reinterpret_cast<const BYTE*>(mod);
		auto sectionHeader =  reinterpret_cast<IMAGE_SECTION_HEADER*>(
			&ntHeaders->OptionalHeader.DataDirectory[ntHeaders->OptionalHeader.NumberOfRvaAndSizes]);
		for (unsigned i = 0; i < ntHeaders->FileHeader.NumberOfSections; ++i)
		{
			if (offset >= sectionHeader->VirtualAddress &&
				offset < sectionHeader->VirtualAddress + sectionHeader->SizeOfRawData)
			{
				return LOG_RESULT(sectionHeader->PointerToRawData + offset - sectionHeader->VirtualAddress);
			}
			sectionHeader++;
		}
		return LOG_RESULT(0);
	}

	HMODULE getModuleHandleFromAddress(const void* address)
	{
		HMODULE module = nullptr;
		GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
			static_cast<const char*>(address), &module);
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
