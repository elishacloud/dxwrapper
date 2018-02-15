#pragma once

#define VISIT_PROCS(visit) \
	visit(D3D10CoreCreateDevice, jmpaddr) \
	visit(D3D10CoreGetSupportedVersions, jmpaddr) \
	visit(D3D10CoreGetVersion, jmpaddr) \
	visit(D3D10CoreRegisterLayers, jmpaddr)

PROC_CLASS(d3d10core, dll)

#undef VISIT_PROCS
