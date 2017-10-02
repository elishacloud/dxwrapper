#pragma once

#define VISIT_PROCS(visit) \
	visit(DllCanUnloadNow) \
	visit(DllGetClassObject) \
	visit(DllRegisterServer) \
	visit(DllUnregisterServer) \
	visit(DebugSetMute) \
	visit(SetAppCompatStringPointer) \
	visit(D3DFree) \
	visit(D3DMalloc) \
	visit(D3DRealloc) \
	visit(Direct3DCreateDevice) \
	visit(Direct3DCreate) \
	visit(Direct3D_HALCleanUp) \
	visit(FlushD3DDevices) \
	visit(PaletteAssociateNotify) \
	visit(PaletteUpdateNotify) \
	visit(SurfaceFlipNotify) \
	visit(D3DKMTCloseAdapter) \
	visit(D3DKMTDestroyAllocation) \
	visit(D3DKMTDestroyContext) \
	visit(D3DKMTDestroyDevice) \
	visit(D3DKMTDestroySynchronizationObject) \
	visit(D3DKMTPresent) \
	visit(D3DKMTQueryAdapterInfo) \
	visit(D3DKMTSetDisplayPrivateDriverFormat) \
	visit(D3DKMTSignalSynchronizationObject) \
	visit(D3DKMTUnlock) \
	visit(D3DKMTWaitForSynchronizationObject) \
	visit(D3DKMTCreateAllocation) \
	visit(D3DKMTCreateContext) \
	visit(D3DKMTCreateDevice) \
	visit(D3DKMTCreateSynchronizationObject) \
	visit(D3DKMTEscape) \
	visit(D3DKMTGetContextSchedulingPriority) \
	visit(D3DKMTGetDeviceState) \
	visit(D3DKMTGetDisplayModeList) \
	visit(D3DKMTGetMultisampleMethodList) \
	visit(D3DKMTGetRuntimeData) \
	visit(D3DKMTGetSharedPrimaryHandle) \
	visit(D3DKMTLock) \
	visit(D3DKMTOpenAdapterFromHdc) \
	visit(D3DKMTOpenResource) \
	visit(D3DKMTQueryAllocationResidency) \
	visit(D3DKMTQueryResourceInfo) \
	visit(D3DKMTRender) \
	visit(D3DKMTSetAllocationPriority) \
	visit(D3DKMTSetContextSchedulingPriority) \
	visit(D3DKMTSetDisplayMode) \
	visit(D3DKMTSetGammaRamp) \
	visit(D3DKMTSetVidPnSourceOwner) \
	visit(D3DKMTWaitForVerticalBlankEvent) \
	visit(OpenAdapter10) \
	visit(OpenAdapter10_2)

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
