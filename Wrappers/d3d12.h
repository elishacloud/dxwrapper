#pragma once

#define VISIT_PROCS_D3D12(visit) \
	visit(GetBehaviorValue, jmpaddr) \
	visit(D3D12CreateDevice, jmpaddr) \
	visit(D3D12GetDebugInterface, jmpaddr) \
	visit(D3D12CoreCreateLayeredDevice, jmpaddr) \
	visit(D3D12CoreGetLayeredDeviceSize, jmpaddr) \
	visit(D3D12CoreRegisterLayers, jmpaddr) \
	visit(D3D12CreateRootSignatureDeserializer, jmpaddr) \
	visit(D3D12CreateVersionedRootSignatureDeserializer, jmpaddr) \
	visit(D3D12EnableExperimentalFeatures, jmpaddr) \
	visit(D3D12SerializeRootSignature, jmpaddr) \
	visit(D3D12SerializeVersionedRootSignature, jmpaddr)

#ifdef PROC_CLASS
PROC_CLASS(d3d12, dll, VISIT_PROCS_D3D12, VISIT_PROCS_BLANK)
#endif
