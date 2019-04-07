#pragma once

#define VISIT_PROCS_D3D11(visit) \
	visit(D3D11CreateDeviceForD3D12, jmpaddr) \
	visit(EnableFeatureLevelUpgrade, jmpaddr) \
	visit(CreateDirect3D11DeviceFromDXGIDevice, jmpaddr) \
	visit(CreateDirect3D11SurfaceFromDXGISurface, jmpaddr) \
	visit(D3D11CoreCreateDevice, jmpaddr) \
	visit(D3D11CoreCreateLayeredDevice, jmpaddr) \
	visit(D3D11CoreGetLayeredDeviceSize, jmpaddr) \
	visit(D3D11CoreRegisterLayers, jmpaddr) \
	visit(D3D11CreateDevice, jmpaddr) \
	visit(D3D11CreateDeviceAndSwapChain, jmpaddr) \
	visit(D3D11On12CreateDevice, jmpaddr) \
	visit(D3DPerformance_BeginEvent, jmpaddr) \
	visit(D3DPerformance_EndEvent, jmpaddr) \
	visit(D3DPerformance_GetStatus, jmpaddr) \
	visit(D3DPerformance_SetMarker, jmpaddr)

#ifdef PROC_CLASS
PROC_CLASS(d3d11, dll, VISIT_PROCS_D3D11, VISIT_PROCS_BLANK)
#endif
