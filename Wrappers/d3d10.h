#pragma once

#define VISIT_PROCS_D3D10(visit) \
	visit(RevertToOldImplementation, jmpaddr) \
	visit(D3D10CompileEffectFromMemory, jmpaddr) \
	visit(D3D10CompileShader, jmpaddr) \
	visit(D3D10CreateBlob, jmpaddr) \
	visit(D3D10CreateDevice, jmpaddr) \
	visit(D3D10CreateDeviceAndSwapChain, jmpaddr) \
	visit(D3D10CreateEffectFromMemory, jmpaddr) \
	visit(D3D10CreateEffectPoolFromMemory, jmpaddr) \
	visit(D3D10CreateStateBlock, jmpaddr) \
	visit(D3D10DisassembleEffect, jmpaddr) \
	visit(D3D10DisassembleShader, jmpaddr) \
	visit(D3D10GetGeometryShaderProfile, jmpaddr) \
	visit(D3D10GetInputAndOutputSignatureBlob, jmpaddr) \
	visit(D3D10GetInputSignatureBlob, jmpaddr) \
	visit(D3D10GetOutputSignatureBlob, jmpaddr) \
	visit(D3D10GetPixelShaderProfile, jmpaddr) \
	visit(D3D10GetShaderDebugInfo, jmpaddr) \
	visit(D3D10GetVersion, jmpaddr) \
	visit(D3D10GetVertexShaderProfile, jmpaddr) \
	visit(D3D10PreprocessShader, jmpaddr) \
	visit(D3D10ReflectShader, jmpaddr) \
	visit(D3D10RegisterLayers, jmpaddr) \
	visit(D3D10StateBlockMaskDifference, jmpaddr) \
	visit(D3D10StateBlockMaskDisableAll, jmpaddr) \
	visit(D3D10StateBlockMaskDisableCapture, jmpaddr) \
	visit(D3D10StateBlockMaskEnableAll, jmpaddr) \
	visit(D3D10StateBlockMaskEnableCapture, jmpaddr) \
	visit(D3D10StateBlockMaskGetSetting, jmpaddr) \
	visit(D3D10StateBlockMaskIntersect, jmpaddr) \
	visit(D3D10StateBlockMaskUnion, jmpaddr)

#ifdef PROC_CLASS
PROC_CLASS(d3d10, dll, VISIT_PROCS_D3D10, VISIT_PROCS_BLANK)
#endif
