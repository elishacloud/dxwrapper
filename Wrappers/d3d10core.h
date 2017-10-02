#pragma once

#define VISIT_PROCS(visit) \
	visit(D3D10CoreCreateDevice) \
	visit(D3D10CoreGetSupportedVersions) \
	visit(D3D10CoreGetVersion) \
	visit(D3D10CoreRegisterLayers)

PROC_CLASS(d3d10core, dll)

#undef VISIT_PROCS
