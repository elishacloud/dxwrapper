#pragma once

#define VISIT_PROCS(visit) \
	visit(ApplyCompatResolutionQuirking, jmpaddr) \
	visit(CompatString, jmpaddr) \
	visit(CompatValue, jmpaddr) \
	visit(DXGIDumpJournal, jmpaddr) \
	visit(DXGIRevertToSxS, jmpaddr) \
	visit(PIXBeginCapture, jmpaddr) \
	visit(PIXEndCapture, jmpaddr) \
	visit(PIXGetCaptureState, jmpaddr) \
	visit(CreateDXGIFactory1, jmpaddr) \
	visit(CreateDXGIFactory2, jmpaddr) \
	visit(CreateDXGIFactory, jmpaddr) \
	visit(DXGID3D10CreateDevice, jmpaddr) \
	visit(DXGID3D10CreateLayeredDevice, jmpaddr) \
	visit(DXGID3D10ETWRundown, jmpaddr) \
	visit(DXGID3D10GetLayeredDeviceSize, jmpaddr) \
	visit(DXGID3D10RegisterLayers, jmpaddr) \
	visit(DXGIGetDebugInterface1, jmpaddr) \
	visit(DXGIReportAdapterConfiguration, jmpaddr)

PROC_CLASS(dxgi, dll)

#undef VISIT_PROCS
