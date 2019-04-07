#pragma once

#define VISIT_PROCS_D2D1(visit) \
	visit(D2D1CreateFactory, jmpaddr) \
	visit(D2D1MakeRotateMatrix, jmpaddr) \
	visit(D2D1MakeSkewMatrix, jmpaddr) \
	visit(D2D1IsMatrixInvertible, jmpaddr) \
	visit(D2D1InvertMatrix, jmpaddr) \
	visit(D2D1ConvertColorSpace, jmpaddr) \
	visit(D2D1CreateDevice, jmpaddr) \
	visit(D2D1CreateDeviceContext, jmpaddr) \
	visit(D2D1SinCos, jmpaddr) \
	visit(D2D1Tan, jmpaddr) \
	visit(D2D1Vec3Length, jmpaddr) \
	visit(D2D1ComputeMaximumScaleFactor, jmpaddr) \
	visit(D2D1GetGradientMeshInteriorPointsFromCoonsPatch, jmpaddr) \
	visit(D2DTkCreateOn12Device, jmpaddr) \
	visit(D2DTkCreateAlgorithmFactory, jmpaddr)

#ifdef PROC_CLASS
PROC_CLASS(d2d1, dll, VISIT_PROCS_D2D1, VISIT_PROCS_BLANK)
#endif
