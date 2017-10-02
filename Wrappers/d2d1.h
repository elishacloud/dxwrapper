#pragma once

#define VISIT_PROCS(visit) \
	visit(D2D1CreateFactory) \
	visit(D2D1MakeRotateMatrix) \
	visit(D2D1MakeSkewMatrix) \
	visit(D2D1IsMatrixInvertible) \
	visit(D2D1InvertMatrix) \
	visit(D2D1ConvertColorSpace) \
	visit(D2D1CreateDevice) \
	visit(D2D1CreateDeviceContext) \
	visit(D2D1SinCos) \
	visit(D2D1Tan) \
	visit(D2D1Vec3Length) \
	visit(D2D1ComputeMaximumScaleFactor) \
	visit(D2D1GetGradientMeshInteriorPointsFromCoonsPatch) \
	visit(D2DTkCreateOn12Device) \
	visit(D2DTkCreateAlgorithmFactory)

PROC_CLASS(d2d1, dll)

#undef VISIT_PROCS
