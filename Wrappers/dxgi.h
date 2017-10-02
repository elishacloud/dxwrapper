#pragma once

#define VISIT_PROCS(visit) \
	visit(ApplyCompatResolutionQuirking) \
	visit(CompatString) \
	visit(CompatValue) \
	visit(DXGIDumpJournal) \
	visit(DXGIRevertToSxS) \
	visit(PIXBeginCapture) \
	visit(PIXEndCapture) \
	visit(PIXGetCaptureState) \
	visit(CreateDXGIFactory1) \
	visit(CreateDXGIFactory2) \
	visit(CreateDXGIFactory) \
	visit(DXGID3D10CreateDevice) \
	visit(DXGID3D10CreateLayeredDevice) \
	visit(DXGID3D10ETWRundown) \
	visit(DXGID3D10GetLayeredDeviceSize) \
	visit(DXGID3D10RegisterLayers) \
	visit(DXGIGetDebugInterface1) \
	visit(DXGIReportAdapterConfiguration)

PROC_CLASS(dxgi, dll)

#undef VISIT_PROCS
