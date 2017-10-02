#pragma once

#define VISIT_PROCS(visit) \
	visit(GetBehaviorValue) \
	visit(D3D12CreateDevice) \
	visit(D3D12GetDebugInterface) \
	visit(D3D12CoreCreateLayeredDevice) \
	visit(D3D12CoreGetLayeredDeviceSize) \
	visit(D3D12CoreRegisterLayers) \
	visit(D3D12CreateRootSignatureDeserializer) \
	visit(D3D12CreateVersionedRootSignatureDeserializer) \
	visit(D3D12EnableExperimentalFeatures) \
	visit(D3D12SerializeRootSignature) \
	visit(D3D12SerializeVersionedRootSignature)

PROC_CLASS(d3d12, dll)

#undef VISIT_PROCS
