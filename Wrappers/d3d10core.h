#pragma once

#define VISIT_PROCS_D3D10CORE(visit) \
	visit(D3D10CoreCreateDevice, jmpaddr) \
	visit(D3D10CoreGetSupportedVersions, jmpaddr) \
	visit(D3D10CoreGetVersion, jmpaddr) \
	visit(D3D10CoreRegisterLayers, jmpaddr)

#ifdef PROC_CLASS
PROC_CLASS(d3d10core, dll, VISIT_PROCS_D3D10CORE)
#endif
