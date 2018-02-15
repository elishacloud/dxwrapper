#pragma once

#define VISIT_PROCS(visit) \
	visit(DllCanUnloadNow, jmpaddr) \
	visit(DllGetClassObject, jmpaddr) \
	visit(DllRegisterServer, jmpaddr) \
	visit(DllUnregisterServer, jmpaddr) \
	visit(DebugSetMute, jmpaddrvoid) \
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
	visit(D3DKMTDestroyContext, jmpaddr) \
	visit(D3DKMTDestroyDevice, jmpaddr) \
	visit(D3DKMTDestroySynchronizationObject, jmpaddr) \
	visit(D3DKMTPresent, jmpaddr) \
	visit(D3DKMTQueryAdapterInfo, jmpaddr) \
	visit(D3DKMTSetDisplayPrivateDriverFormat, jmpaddr) \
	visit(D3DKMTSignalSynchronizationObject, jmpaddr) \
	visit(D3DKMTUnlock, jmpaddr) \
	visit(D3DKMTWaitForSynchronizationObject, jmpaddr) \
	visit(D3DKMTCreateAllocation, jmpaddr) \
	visit(D3DKMTCreateContext, jmpaddr) \
	visit(D3DKMTCreateDevice, jmpaddr) \
	visit(D3DKMTCreateSynchronizationObject, jmpaddr) \
	visit(D3DKMTEscape, jmpaddr) \
	visit(D3DKMTGetContextSchedulingPriority, jmpaddr) \
	visit(D3DKMTGetDeviceState, jmpaddr) \
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
	visit(D3DKMTSetVidPnSourceOwner, jmpaddr) \
	visit(D3DKMTWaitForVerticalBlankEvent, jmpaddr) \
	visit(OpenAdapter10, jmpaddr) \
	visit(OpenAdapter10_2, jmpaddr)

namespace ShardProcs
{
	using namespace Wrapper;
	VISIT_PROCS(ADD_FARPROC_MEMBER);
	void Load(HMODULE dll)
	{
		if (dll)
		{
			VISIT_PROCS(LOAD_ORIGINAL_PROC);
		}
	}
	VISIT_PROCS(CREATE_PROC_STUB)
}

#undef VISIT_PROCS
