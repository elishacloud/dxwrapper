#pragma once

#include <string>

#include <Windows.h>

#define CALL_ORIG_FUNC(func) Compat32::g_origFuncPtr<&func>

#define HOOK_FUNCTION(module, func, newFunc) \
	Compat32::hookFunction<&func>(#module, #func, &newFunc)
#define HOOK_SHIM_FUNCTION(func, newFunc) \
	Compat32::hookFunction(reinterpret_cast<void*&>(Compat32::g_origFuncPtr<&func>), newFunc, #func)

namespace Compat32
{
	std::string funcPtrToStr(const void* funcPtr);
	DWORD getModuleFileOffset(const void* address);
	HMODULE getModuleHandleFromAddress(const void* address);

	template <auto origFunc>
	decltype(origFunc) g_origFuncPtr = origFunc;

	template <auto origFunc>
	std::string g_origFuncName;

	FARPROC getProcAddress(HMODULE module, const char* procName);
	void hookFunction(void*& origFuncPtr, void* newFuncPtr, const char* funcName);
	void hookFunction(HMODULE module, const char* funcName, void*& origFuncPtr, void* newFuncPtr);
	void hookFunction(const char* moduleName, const char* funcName, void*& origFuncPtr, void* newFuncPtr);
	void hookIatFunction(HMODULE module, const char* funcName, void* newFuncPtr);

	template <auto origFunc>
	void hookFunction(const char* moduleName, const char* funcName, decltype(origFunc) newFuncPtr)
	{
		g_origFuncName<origFunc> = funcName;
		hookFunction(moduleName, funcName, reinterpret_cast<void*&>(g_origFuncPtr<origFunc>), newFuncPtr);
	}

	void removeShim(HMODULE module, const char* funcName);
}
