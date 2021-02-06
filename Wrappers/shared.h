#pragma once

#define VISIT_SHAREDPROCS(visit) \
	visit(DllCanUnloadNow, jmpaddr) \
	visit(DllGetClassObject, jmpaddr) \
	visit(DllRegisterServer, jmpaddr) \
	visit(DllUnregisterServer, jmpaddr) \
	visit(SetAppCompatStringPointer, jmpaddr) \
	visit(D3DFree, jmpaddr) \
	visit(D3DMalloc, jmpaddr) \
	visit(D3DRealloc, jmpaddr) \
	visit(Direct3DCreateDevice, jmpaddr) \
	visit(Direct3DCreate, jmpaddr) \
	visit(Direct3D_HALCleanUp, jmpaddr) \
	visit(FlushD3DDevices, jmpaddr) \
	visit(PaletteAssociateNotify, jmpaddr) \
	visit(PaletteUpdateNotify, jmpaddr) \
	visit(SurfaceFlipNotify, jmpaddr) \
	visit(D3DKMTCloseAdapter, jmpaddr) \
	visit(D3DKMTDestroyAllocation, jmpaddr) \
	visit(D3DKMTDestroySynchronizationObject, jmpaddr) \
	visit(D3DKMTSetDisplayPrivateDriverFormat, jmpaddr) \
	visit(D3DKMTSignalSynchronizationObject, jmpaddr) \
	visit(D3DKMTUnlock, jmpaddr) \
	visit(D3DKMTWaitForSynchronizationObject, jmpaddr) \
	visit(D3DKMTCreateAllocation, jmpaddr) \
	visit(D3DKMTCreateSynchronizationObject, jmpaddr) \
	visit(D3DKMTEscape, jmpaddr) \
	visit(D3DKMTGetContextSchedulingPriority, jmpaddr) \
	visit(D3DKMTGetDisplayModeList, jmpaddr) \
	visit(D3DKMTGetMultisampleMethodList, jmpaddr) \
	visit(D3DKMTGetRuntimeData, jmpaddr) \
	visit(D3DKMTGetSharedPrimaryHandle, jmpaddr) \
	visit(D3DKMTLock, jmpaddr) \
	visit(D3DKMTOpenAdapterFromHdc, jmpaddr) \
	visit(D3DKMTOpenResource, jmpaddr) \
	visit(D3DKMTQueryAllocationResidency, jmpaddr) \
	visit(D3DKMTQueryResourceInfo, jmpaddr) \
	visit(D3DKMTRender, jmpaddr) \
	visit(D3DKMTSetAllocationPriority, jmpaddr) \
	visit(D3DKMTSetContextSchedulingPriority, jmpaddr) \
	visit(D3DKMTSetDisplayMode, jmpaddr) \
	visit(D3DKMTSetGammaRamp, jmpaddr) \
	visit(OpenAdapter10, jmpaddr) \
	visit(OpenAdapter10_2, jmpaddr)

#define VISIT_SHAREDPROCS_CONFLICT(visit) \
	visit(D3DKMTDestroyContext, jmpaddr) \
	visit(D3DKMTDestroyDevice, jmpaddr) \
	visit(D3DKMTPresent, jmpaddr) \
	visit(D3DKMTQueryAdapterInfo, jmpaddr) \
	visit(D3DKMTCreateContext, jmpaddr) \
	visit(D3DKMTCreateDevice, jmpaddr) \
	visit(D3DKMTGetDeviceState, jmpaddr) \
	visit(D3DKMTSetVidPnSourceOwner, jmpaddr) \
	visit(D3DKMTWaitForVerticalBlankEvent, jmpaddr)

#define VISIT_PROCS_SHAREDPROCS(visit) \
	VISIT_SHAREDPROCS(visit) \
	VISIT_SHAREDPROCS_CONFLICT(visit)

#ifdef PROC_CLASS
namespace ShardProcs
{
#define CREATE_PROC_STUB_CONFLICT(procName, prodAddr) \
	FARPROC procName ## _var = prodAddr; \
	extern "C" __declspec(naked) void __stdcall CF_ ## procName() \
	{ \
		__asm mov edi, edi \
		__asm jmp procName ## _var \
	} \
	FARPROC procName ## _funct = (FARPROC)*CF_ ## procName;

#define	STORE_ORIGINAL_PROC_CONFLICT(procName, prodAddr) \
	tmpMap.Proc = (FARPROC)*(CF_ ## procName); \
	tmpMap.val = &(procName ## _var); \
	jmpArray.push_back(tmpMap);

	using namespace Wrapper;
	VISIT_SHAREDPROCS(CREATE_PROC_STUB);
	VISIT_SHAREDPROCS_CONFLICT(CREATE_PROC_STUB_CONFLICT);
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
		VISIT_SHAREDPROCS(STORE_ORIGINAL_PROC);
		VISIT_SHAREDPROCS_CONFLICT(STORE_ORIGINAL_PROC_CONFLICT);
	}
#undef CREATE_PROC_STUB_CONFLICT
#undef STORE_ORIGINAL_PROC_CONFLICT
}
#endif
