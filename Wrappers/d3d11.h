#pragma once

#define VISIT_PROCS(visit) \
	visit(D3D11CreateDeviceForD3D12) \
	visit(EnableFeatureLevelUpgrade) \
	visit(CreateDirect3D11DeviceFromDXGIDevice) \
	visit(CreateDirect3D11SurfaceFromDXGISurface) \
	visit(D3D11CoreCreateDevice) \
	visit(D3D11CoreCreateLayeredDevice) \
	visit(D3D11CoreGetLayeredDeviceSize) \
	visit(D3D11CoreRegisterLayers) \
	visit(D3D11CreateDevice) \
	visit(D3D11CreateDeviceAndSwapChain) \
	visit(D3D11On12CreateDevice) \
	visit(D3DPerformance_BeginEvent) \
	visit(D3DPerformance_EndEvent) \
	visit(D3DPerformance_GetStatus) \
	visit(D3DPerformance_SetMarker)

PROC_CLASS(d3d11, dll)

#undef VISIT_PROCS
