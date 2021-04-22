#pragma once

#define VISIT_PROCS_SHAREDPROCS(visit) \
	visit(DllCanUnloadNow, jmpaddr) \
	visit(DllGetClassObject, jmpaddr) \
	visit(DllRegisterServer, jmpaddr) \
	visit(DllUnregisterServer, jmpaddr)

#ifdef PROC_CLASS
namespace ShardProcs
{
	using namespace Wrapper;
	VISIT_PROCS_SHAREDPROCS(CREATE_PROC_STUB);
	void Load(HMODULE dll)
	{
		if (dll)
		{
			VISIT_PROCS_SHAREDPROCS(LOAD_ORIGINAL_PROC);
		}
	}
	void AddToArray()
	{
		wrapper_map tmpMap;
		VISIT_PROCS_SHAREDPROCS(STORE_ORIGINAL_PROC);
	}
}
#endif
