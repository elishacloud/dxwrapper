#pragma once

#define CALL_ORIG_FUNC(func) Compat::getOrigFuncPtr<decltype(&func), &func>()

#define HOOK_FUNCTION(module, func, newFunc) \
	Compat::hookFunction<decltype(&func), &func>(#module, #func, &newFunc)

namespace Compat
{
	template <typename OrigFuncPtr, OrigFuncPtr origFunc>
	OrigFuncPtr& getOrigFuncPtr()
	{
		static OrigFuncPtr origFuncPtr = origFunc;
		return origFuncPtr;
	}

	void hookFunction(void*& origFuncPtr, void* newFuncPtr);
	void hookFunction(const char* moduleName, const char* funcName, void*& origFuncPtr, void* newFuncPtr);

	template <typename OrigFuncPtr, OrigFuncPtr origFunc>
	void hookFunction(const char* moduleName, const char* funcName, OrigFuncPtr newFuncPtr)
	{
		hookFunction(moduleName, funcName,
			reinterpret_cast<void*&>(getOrigFuncPtr<OrigFuncPtr, origFunc>()), newFuncPtr);
	}

	void unhookAllFunctions();
}
