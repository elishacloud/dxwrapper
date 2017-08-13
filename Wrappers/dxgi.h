#pragma once

#define VISIT_DXGI_PROCS(visit) \
	visit(ApplyCompatResolutionQuirking) \
	visit(CompatString) \
	visit(CompatValue) \
	visit(DXGIDumpJournal) \
	visit(DXGIRevertToSxS) \
	visit(PIXBeginCapture) \
	visit(PIXEndCapture) \
	visit(PIXGetCaptureState) \
	visit(SetAppCompatStringPointer) \
	visit(CreateDXGIFactory1) \
	visit(CreateDXGIFactory2) \
	visit(CreateDXGIFactory) \
	visit(DXGID3D10CreateDevice) \
	visit(DXGID3D10CreateLayeredDevice) \
	visit(DXGID3D10ETWRundown) \
	visit(DXGID3D10GetLayeredDeviceSize) \
	visit(DXGID3D10RegisterLayers) \
	visit(DXGIGetDebugInterface1) \
	visit(DXGIReportAdapterConfiguration) \
	visit(D3DKMTCloseAdapter) \
	visit(D3DKMTDestroyAllocation) \
	visit(D3DKMTDestroyContext) \
	visit(D3DKMTDestroyDevice) \
	visit(D3DKMTDestroySynchronizationObject) \
	visit(D3DKMTQueryAdapterInfo) \
	visit(D3DKMTSetDisplayPrivateDriverFormat) \
	visit(D3DKMTSignalSynchronizationObject) \
	visit(D3DKMTUnlock) \
	visit(OpenAdapter10) \
	visit(OpenAdapter10_2) \
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
	visit(D3DKMTPresent) \
	visit(D3DKMTQueryAllocationResidency) \
	visit(D3DKMTQueryResourceInfo) \
	visit(D3DKMTRender) \
	visit(D3DKMTSetAllocationPriority) \
	visit(D3DKMTSetContextSchedulingPriority) \
	visit(D3DKMTSetDisplayMode) \
	visit(D3DKMTSetGammaRamp) \
	visit(D3DKMTSetVidPnSourceOwner) \
	visit(D3DKMTWaitForSynchronizationObject) \
	visit(D3DKMTWaitForVerticalBlankEvent)

namespace dxgi
{
	class dxgi_dll
	{
	public:
		HMODULE dll = nullptr;
		VISIT_DXGI_PROCS(ADD_FARPROC_MEMBER);

		void Load()
		{
			if (Config.WrapperMode != dtype.dxgi && Config.WrapperMode != dtype.Auto)
			{
				return;
			}

			// Load real dll
			dll = Wrapper::LoadDll(dtype.dxgi);

			// Load dll functions
			if (dll)
			{
				VISIT_DXGI_PROCS(LOAD_ORIGINAL_PROC);
			}
		}

		void Unhook() {}
	};

	extern dxgi_dll module;
}
