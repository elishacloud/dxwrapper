#pragma once

#define VISIT_PROCS(visit) \
	visit(RevertToOldImplementation) \
	visit(D3D10CompileEffectFromMemory) \
	visit(D3D10CompileShader) \
	visit(D3D10CreateBlob) \
	visit(D3D10CreateDevice) \
	visit(D3D10CreateDeviceAndSwapChain) \
	visit(D3D10CreateEffectFromMemory) \
	visit(D3D10CreateEffectPoolFromMemory) \
	visit(D3D10CreateStateBlock) \
	visit(D3D10DisassembleEffect) \
	visit(D3D10DisassembleShader) \
	visit(D3D10GetGeometryShaderProfile) \
	visit(D3D10GetInputAndOutputSignatureBlob) \
	visit(D3D10GetInputSignatureBlob) \
	visit(D3D10GetOutputSignatureBlob) \
	visit(D3D10GetPixelShaderProfile) \
	visit(D3D10GetShaderDebugInfo) \
	visit(D3D10GetVersion) \
	visit(D3D10GetVertexShaderProfile) \
	visit(D3D10PreprocessShader) \
	visit(D3D10ReflectShader) \
	visit(D3D10RegisterLayers) \
	visit(D3D10StateBlockMaskDifference) \
	visit(D3D10StateBlockMaskDisableAll) \
	visit(D3D10StateBlockMaskDisableCapture) \
	visit(D3D10StateBlockMaskEnableAll) \
	visit(D3D10StateBlockMaskEnableCapture) \
	visit(D3D10StateBlockMaskGetSetting) \
	visit(D3D10StateBlockMaskIntersect) \
	visit(D3D10StateBlockMaskUnion)

PROC_CLASS(d3d10, dll)

#undef VISIT_PROCS
